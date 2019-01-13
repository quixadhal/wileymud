#!/usr/bin/perl -w
use strict;
use Data::Dumper;

# A normal apache installatino will rotate the log files every day,
# and the ones from 2 days ago or older will be gzipped.
#
# This reads the entire log set, however many there are, to build
# the list of naughty people.
#
# You might want to omit the older (gzipped) ones if you want to let
# people make more "mistakes" before being banned.
#
#[Sat Jan 12 01:09:28.335163 2019] [:error] [pid 26727] [client 193.112.73.100:56810] script '/var/www/html/cmdd.php' not found or unable to stat

my $log_path = "/var/log/apache2";
my $blacklist = "/etc/iptables/ipset.blacklist";
my $ban_strikes = 10;

opendir DP, $log_path or die "Can't opendir $log_path $!";
my @logs = grep { /^error\.log(\.\d+)?(\.gz)?$/ && -f "$log_path/$_" } readdir DP;
closedir DP;

my %error_count = ();

foreach my $filename (@logs) {
    $filename = "$log_path/$filename";
    $filename = "gunzip -c $filename |" if $filename =~ /\.gz$/;

    if ( open FP, "$filename" ) {
        while(<FP>) {
            chomp;
            /\[client\s+(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\:\d+\]/;
            my ($ip) = ($1);
            next if !defined $ip;
            next if $ip =~ /^192\.168\.0/;
            next if $ip =~ /^127\.0/;
            $error_count{$ip}++;
        }
        close FP;
    }
}

foreach my $ip (sort keys %error_count) {
    next if $error_count{$ip} < $ban_strikes;
    system "ipset add blacklist $ip";
}

system "ipset save >$blacklist";

