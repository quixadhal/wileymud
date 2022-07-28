#!/usr/bin/perl -w
use utf8;
use strict;
use English qw( −no_match_vars );
use Sort::Key::Natural qw(natsort);
use POSIX qw(strftime);
use Time::HiRes qw(time);
use Data::Dumper;
no warnings 'utf8';

$Data::Dumper::Sortkeys = \&dump_filter;

my $start_time = time();

my %game_start_name = (
    x4ep1_gamestart_terran2     => 'Terran 2',
    x4ep1_gamestart_discover    => 'Discover',
    x4ep1_gamestart_tutorial    => 'Tutorial',
);

sub dump_filter {
    my ($hash) = @_;
    return [ natsort keys %$hash ];
}

sub xml_key_value_pairs {
    my $str = shift;
    my $data = {};

    return $data if !defined $str;

    my $prev_key = "";
    while(1) {
        $str =~ /\G\s*([^=]+)=\"([^\"]*)\"/cgsmix;
        my ($k,$v) = ($1,$2);
        last if !defined $k or $k eq $prev_key;
        $prev_key = $k;
        $data->{$k} = $v;
        #print "$k => $v\n";
    };

    return $data;
}

my $spin_pos = 0;
sub spin {
    my @spin_chars = (qw(- \\ | /));
    #my @spin_chars = (qw(. o O * O o));
    print STDERR $spin_chars[$spin_pos % (scalar @spin_chars)] . "\b";
    $spin_pos++;
}

opendir DP, '.' or die "Can't opendir '.' $!";
my @saves = grep { /\.xml.gz$/i && -f "./$_" } readdir DP;
closedir DP;

my $data = {};

print STDERR "Loading ";
foreach my $filename (natsort @saves) {
    spin();
    #print "Working on $filename...";
    my $raw_xml = "";
    open FP, "/usr/bin/zcat $filename |" or die "Cannot open $filename: $!";
    {
        local $/ = undef;
        $raw_xml = <FP>;
    }
    close FP;
    #print "Data read in... " . (length $raw_xml) . " bytes\n";

    $raw_xml =~ /^<savegame>\s*<info>\s*<save\s+(.*?)(\/>|\s*<\/save>)\s*/cgsmix;
    my ($save_info) = ($1);
    $raw_xml =~ /\G<game\s+(.*?)(\/>|\s*<\/game>)\s*/cgsmix;
    my ($game_info) = ($1);
    $raw_xml =~ /\G<player\s+(.*?)(\/>|\s*<\/player>)\s*/cgsmix;
    my ($player_info) = ($1);

    my $filedata = {
        filename    => $filename,
        save        => xml_key_value_pairs($save_info),
        game        => xml_key_value_pairs($game_info),
        player      => xml_key_value_pairs($player_info),
    };

    $data->{$filename} = $filedata;
    #print "done.\n";
}
my $end_time = time();
printf STDERR "completed in %.3f seconds!\n", ($end_time - $start_time);

#print Dumper($data);

my $header_str = "%-14.14s %-16.16s %-10.10s %-16.16s %26.26s\n";
my $format_str = "%-14.14s %-16.16s %-10.10s %-16.16s %19.19s PLAYER\n"
               . "%79.79s MONEY\n"
               . "%79.79s LOCAT\n"
               . "%79.79s START\n"
               ;
printf $header_str, 'FILENAME', 'SAVE', 'VERSION', 'DATE', 'INFO';
printf $header_str, '-'x14, '-'x16, '-'x10, '-'x16, '-'x26;
foreach my $filename (natsort keys %$data) {
    my $file_name           = $filename;
    $file_name =~ s/\.gz$//;
    my $save_name           = $data->{$filename}{save}{name};
    my $version             = $data->{$filename}{game}{version} . '.' . 
                              $data->{$filename}{game}{build};
    my $save_date           = strftime "%Y-%m-%d %H:%M",
                              localtime $data->{$filename}{save}{date};
    my $player_name         = $data->{$filename}{player}{name};
    my $player_cash         = $data->{$filename}{player}{money};
    my $player_location     = $data->{$filename}{player}{location};
    my $game_start          = $game_start_name{$data->{$filename}{game}{start}};

    printf $format_str, $file_name, $save_name, $version, $save_date, $player_name,
           $player_cash,
           $player_location,
           $game_start
           ;
}
