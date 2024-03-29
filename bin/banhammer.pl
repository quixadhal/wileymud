#!/usr/bin/perl -w
use strict;
use English qw( −no_match_vars );
use Data::Dumper;
use Getopt::Long;

# A normal apache installatino will rotate the log files every day,
# and the ones from 2 days ago or older will be gzipped.
#
# This reads the entire log set, however many there are, to build
# the list of naughty people.
#
# You might want to omit the older (gzipped) ones if you want to let
# people make more "mistakes" before being banned.
#
#[Sat Jan 12 01:09:28.335163 2019] [:error] [pid 26727] [client 193.112.73.100:56810] script '/hoem/www/cmdd.php' not found or unable to stat

my $log_path = "/var/log/apache2";
my $blacklist = "/etc/iptables/ipset.blacklist";

my $do_compressed   = 1;
my $ban_strikes     = 10;
my $do_forgive      = 1;
my $do_kid          = 0;

sub do_help {
    print STDERR <<EOM
usage:  $PROGRAM_NAME [-h] [-c] [-b ban-threshold]
long options:
    --help              - This helpful help!
    --compressed        - Scan the older compressed logs too, default is yes.
    --ban               - Number of hits needed for a ban, default is $ban_strikes.
    --forgive           - Wipe the blacklist before running, to forgive old errors.
    --kidding           - Just do a dry run and print what WOULD be done.
NOTE:
    This program must be run as ROOT to be able to read the web
    server's log files AND edit the system's firewall rules.
EOM
    ;
    exit(1);
}

do_help() if $UID != 0;

Getopt::Long::Configure("gnu_getopt");
Getopt::Long::Configure("auto_version");
GetOptions(
    'help|h'            => sub { do_help() },
    'compressed|c!'     => \$do_compressed,
    'ban|b=i'           => \$ban_strikes,
    'forgive|f!'        => \$do_forgive,
    'kidding|kid|k'     => \$do_kid,
);

opendir DP, $log_path or die "Can't opendir $log_path $!";
my @logs = ();
if( $do_compressed ) {
    @logs = grep { /^error\.log(\.\d+)?(\.gz)?$/ && -f "$log_path/$_" } readdir DP;
} else {
    @logs = grep { /^error\.log(\.\d+)?$/ && -f "$log_path/$_" } readdir DP;
}
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
            next if $ip =~ /^73.180.188.164/;       # ktp is ok.
            next if $ip =~ /^104\.156\.100\.167/;   # Oops, new external address
            #next if $ip =~ /^172\.92\.143\.166/;    # This is my external address
            next if $ip =~ /^45\.64\.56\.66/;       # *Kelly I3 router
            next if $ip =~ /^97\.107\.133\.86/;     # *dalet I3 router
            next if $ip =~ /^204\.209\.44\.3/;      # *i4 I3 router
            next if $ip =~ /^136\.144\.155\.250/;   # *wpr I3 router
            next if $ip =~ /^192\.168\.0/;
            next if $ip =~ /^127\.0/;
            $error_count{$ip}++;
        }
        close FP;
    }
}

# select ip from bans where reason = 'HTTP Spam' order by ip;
#
#     const char *sql_ip   = "INSERT INTO bans ( ban_type, expires, ip, set_by, reason )
#                           "VALUES ('IP', to_timestamp($1),$2,$3,$4) "
#                           "ON CONFLICT ON CONSTRAINT bans_type_ip "
#                           "DO UPDATE SET updated = now(), "
#                           "              enabled = true, "
#                           "              ban_type = 'IP', "
#                           "              expires = to_timestamp($1), "
#                           "              name = NULL, ip = $2, "
#                           "              set_by = $3, reason = $4;";

my $prefix = $do_kid ? "KID: " : "";

printf "${prefix}(RE)Creating blacklist...";
system "/usr/sbin/ipset create blacklist hash:ip hashsize 4096 -exist" if !$do_kid;
printf "done.\n";

# If we're feeling forgiving.... do this right before we add new people
if( $do_forgive ) {
    printf "${prefix}Flushing blacklist...";
    system "/usr/sbin/ipset flush blacklist" if !$do_kid;
    printf "done.\n";
}

foreach my $ip (sort keys %error_count) {
    next if $error_count{$ip} < $ban_strikes;
    system "/usr/sbin/ipset add blacklist $ip -exist" if !$do_kid;
    printf "${prefix}Added ${ip} to blacklist.\n";
}

printf "${prefix}Saving new blacklist...";
system "cp -p $blacklist $blacklist.bkp" if !$do_kid;
system "/usr/sbin/ipset save >$blacklist" if !$do_kid;
printf "done.\n";

#ipset create foo hash:ip netmask 30
#ipset add foo 46.229.168.0/24

