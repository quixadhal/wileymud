#!/usr/bin/perl -w

use utf8;
use strict;
no warnings 'utf8';
use English qw( −no_match_vars );
use DBI;
use JSON qw(encode_json decode_json);

sub open_postgres_db {
    my $DB_NAME = shift;

    my $db = DBI->connect("dbi:Pg:dbname=$DB_NAME", 'wiley', 'tardis69',
        { AutoCommit => 1, RaiseError => 1, PrintError => 0, });
    return $db;
}

sub import_json {
    my $filename = shift;

    local $/ = undef;
    open FP, "<$filename" or die "Cannot open data file $filename: $!";
    my $data = <FP>;
    close FP;
    my $object = decode_json($data);
    return $object;
}

sub check_result_exists {
    my $db = shift;
    my $data = shift;

    my $rv = $db->selectrow_arrayref(qq!
        SELECT COUNT(*) FROM speedtest
        WHERE internal_ip = ? AND local >= timezone('US/Pacific', ?);
        !, {}, ($data->{interface}{internalIp}, $data->{timestamp}));
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv ? $rv->[0] : 0;
}

sub add_result {
    my $db = shift;
    my $data = shift;

    my $insert_sql = $db->prepare( qq!
        INSERT INTO speedtest (
            local,
            ping,
            download,
            upload,
            internal_ip,
            external_ip,
            server_id,
            name,
            location,
            country,
            host,
            host_ip,
            result_url,
            wifi
        )
        VALUES (timezone('US/Pacific', ?),?,?,?,?,?,?,?,?,?,?,?,?);
        !);
    my $rv = $insert_sql->execute(
        $data->{timestamp},
        $data->{ping}{latency},
        sprintf("%.2f", ($data->{download}{bandwidth} * 8.0 / 1000000.0)),
        sprintf("%.2f", ($data->{upload}{bandwidth} * 8.0 / 1000000.0)),
        $data->{interface}{internalIp},
        $data->{interface}{externalIp},
        $data->{server}{id},
        $data->{server}{name},
        $data->{server}{location},
        $data->{server}{country},
        $data->{server}{host},
        $data->{server}{ip},
        $data->{result}{url},
        ($data->{interface}{internalIp} eq '192.168.0.11' ? 1 : 0)
    );
    if($rv) {
        printf "Added result from %s at %s\n", $data->{interface}{internalIp}, $data->{timestamp};
    } else {
        print STDERR $DBI::errstr."\n";
    }
}

sub upload_results {
    my $db = shift;
    my $filename = shift;

    if( -r $filename ) {
        my $data = import_json($filename);
        if( !check_result_exists($db, $data) ) {
            add_result($db, $data);
        }
    }
}

sub dump_average {
    my $db = shift;
    my $filename = shift;

    my $result = $db->selectall_arrayref(qq!
        SELECT      internal_ip,
                    AVG(download) AS download,
                    AVG(upload) AS upload,
                    AVG(ping) AS ping
        FROM        speedtest
        WHERE       local BETWEEN now() - '1 week'::interval AND now()
        GROUP BY    internal_ip
        HAVING internal_ip IN ('192.168.0.10', '192.168.0.11')
        ;!, { Slice => {} } );
    if($result) {
        my $data = {};
        foreach my $row (@$result) {
            $row->{wire} = '???';
            $row->{wire} = 'wired'  if $row->{internal_ip} eq '192.168.0.10';
            $row->{wire} = 'wi-fi'  if $row->{internal_ip} eq '192.168.0.11';
            $data->{$row->{internal_ip}} = $row;
        }
        my $json_data = encode_json($data);
        die "Invalid JSON data for $filename!" if !defined $json_data;
        open FP, ">$filename" or die "Cannot open output page $filename: $!";
        print FP $json_data;
        close FP;
    }
}

my $PG_DB           = 'speedtest';
my $LINUX_FILE      = '/share/leninbackup/speedtest_wifi.json';
my $WINDOWS_FILE    = '/share/leninbackup/speedtest.json';
my $AVERAGE_FILE    = '/share/leninbackup/speedtest_avg.json';
my $DATABASE        = open_postgres_db($PG_DB);

upload_results($DATABASE, $LINUX_FILE);
upload_results($DATABASE, $WINDOWS_FILE);
dump_average($DATABASE, $AVERAGE_FILE);

#create table speedtest (
#    local       TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(),
#    ping        FLOAT,
#    download    FLOAT,
#    upload      FLOAT,
#    internal_ip TEXT,
#    external_ip TEXT,
#    server_id   INTEGER,
#    name        TEXT,
#    location    TEXT,
#    country     TEXT,
#    host        TEXT,
#    host_ip     TEXT,
#    result_url  TEXT,
#    wifi        BOOLEAN DEFAULT 'f'
#);
#
