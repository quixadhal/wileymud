#!/usr/bin/perl -w
use strict;
use English qw( −no_match_vars );
use Data::Dumper;
use POSIX qw(strftime);
use Date::Calc qw(Add_Delta_YM);
use File::Path qw(make_path);

my $log_dir = "/home/wiley/lib/log";

my $now_date = strftime "%Y-%m-%d", localtime;
my ($now_year, $now_month, $now_day) = split /\-/, $now_date;
my ($last_year, $last_month, $last_day) = Add_Delta_YM( $now_year, $now_month, $now_day, 0, -1);
$last_year = sprintf "%04d", $last_year;
$last_month = sprintf "%02d", $last_month;
$last_day = sprintf "%02d", $last_day;

my $symlink_target = readlink("$log_dir/runlog") if -l "$log_dir/runlog";

opendir DP, $log_dir or die "Can't opendir $log_dir $!";
my @need_to_compress = grep { /^runlog\.\d{8}\-\d{6}$/ && -f "$log_dir/$_" } readdir DP;
closedir DP;

foreach my $filename (@need_to_compress) {
    print "skip $filename (symlink)\n"  if defined $symlink_target and $symlink_target eq "$log_dir/$filename";
    next if defined $symlink_target and $symlink_target eq "$log_dir/$filename";

    print "comnpressing $filename\n";
    system "/usr/bin/bzip2", "-9v", "$log_dir/$filename";
}

opendir DP, $log_dir or die "Can't opendir $log_dir $!";
my @need_to_move = grep { /^runlog\.\d{8}\-\d{6}\.bz2$/ && -f "$log_dir/$_" } readdir DP;
closedir DP;

foreach my $filename (@need_to_move) {
    print "skip $filename (symlink)\n" if defined $symlink_target and $symlink_target eq "$log_dir/$filename";
    next if defined $symlink_target and $symlink_target eq "$log_dir/$filename";

    $filename =~ /\.(\d{4})(\d\d)(\d\d)\-/;
    my ($file_year, $file_month, $file_day) = ($1, $2, $3);
    my $file_ym = 0 + "$file_year$file_month";
    my $last_ym = 0 + "$last_year$last_month";

    print "skip $filename (date)\n" if $file_ym > $last_ym;
    next if $file_ym > $last_ym;

    my $target_dir =  "$log_dir/$file_year/$file_month";
    make_path $target_dir, {verbose => 1} if ! -d $target_dir;

    print "moving $filename to $target_dir\n";
    rename "$log_dir/$filename", "$target_dir/$filename";
}

