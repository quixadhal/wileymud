#!/usr/bin/perl -w

use utf8;
use strict;

my $YOUTUBE_DL = "/home/wiley/bin/youtube-dl";

no warnings 'utf8';

use English qw( −no_match_vars );
use JSON;
use Data::Dumper;

for my $url (@ARGV) {
    my $json = undef;
    #open(FP, "-|", $YOUTUBE_DL, "--flat-playlist", "-J", $url) or die "Can't open $YOUTUBE_DL $!";
    open(FP, "-|", $YOUTUBE_DL, "-J", $url) or die "Can't open $YOUTUBE_DL $!";
    {
        local $/ = undef;
        $json = <FP>;
    }
    close FP;
    #print "JSON: $json\n";
    my $data = decode_json($json);
    #print Dumper($data);
    #printf "Title: %s\n", $data->{title};
    foreach my $row (@{ $data->{entries} }) {
        printf STDERR "%s %s\n", $row->{id}, $row->{title};
        printf "https://www.youtube.com/watch?v=%s\n", $row->{id};
    }
}

