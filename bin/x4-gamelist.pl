#!/usr/bin/perl -w
use utf8;
use strict;
use English qw( −no_match_vars );
use Sort::Key::Natural qw(natsort);
use POSIX qw(strftime);
use Data::Dumper;
no warnings 'utf8';

$Data::Dumper::Sortkeys = \&dump_filter;

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

opendir DP, '.' or die "Can't opendir '.' $!";
my @saves = grep { /\.xml.gz$/i && -f "./$_" } readdir DP;
closedir DP;

my $data = {};

foreach my $filename (natsort @saves) {
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

#print Dumper($data);

my $format_str = "%-17.17s %-16.16s %-10.10s %-16.16s %s\n";
printf $format_str, qw(FILENAME SAVE VERSION DATE PLAYER);
printf $format_str, '-'x17, '-'x16, '-'x10, '-'x16, '-'x20;
foreach my $filename (natsort keys %$data) {
    my $save_name   = $data->{$filename}{save}{name};
    my $version     = $data->{$filename}{game}{version} . '.' . 
                      $data->{$filename}{game}{build};
    my $save_date   = strftime "%Y-%m-%d %H:%M",
                      localtime $data->{$filename}{save}{date};
    my $player_name = $data->{$filename}{player}{name};

    printf $format_str, $filename, $save_name, $version, $save_date, $player_name;
}
