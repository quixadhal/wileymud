#!/usr/bin/perl -w

use utf8;
use strict;
use Encode qw(encode_utf8);
no warnings 'utf8';
use English;
use Data::Dumper;
use DBI;
use Date::Manip::Date;

my $DB_FILE = '/home/wiley/lib/i3/wiley_new.db';
my $I3LOG_FILE = '/home/wiley/lib/i3/i3.allchan.log';
my $db = undef;
my $counter = 0;

sub setup_i3log_table {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $table_sql = $db->prepare( qq!
        CREATE TABLE IF NOT EXISTS i3log ( 
            created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            is_emote INTEGER, 
            is_url INTEGER, 
            is_bot INTEGER, 
            channel TEXT, 
            speaker TEXT, 
            mud TEXT, 
            message TEXT 
        );
        !);
    my $rv = $table_sql->execute();
    if($rv) {
        $db->commit;
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
}

sub setup_urls_table {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $table_sql = $db->prepare( qq!
        CREATE TABLE IF NOT EXISTS urls ( 
            created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            processed INTEGER, 
            channel TEXT, 
            speaker TEXT, 
            mud TEXT, 
            url TEXT, 
            message TEXT 
        );
        !);
    my $rv = $table_sql->execute();
    if($rv) {
        $db->commit;
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
}

sub setup_log_table {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $table_sql = $db->prepare( qq!
        CREATE TABLE IF NOT EXISTS log ( 
            created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            logtype TEXT DEFAULT 'INFO', 
            filename TEXT, 
            function TEXT, 
            line INTEGER, 
            area_file TEXT, 
            area_line INTEGER, 
            character TEXT, 
            character_room INTEGER, 
            victim TEXT, 
            victim_room INTEGER, 
            message TEXT 
        );
        !);
    my $rv = $table_sql->execute();
    if($rv) {
        $db->commit;
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
}

sub check_offset {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectrow_arrayref(qq!
        SELECT COUNT(*) FROM i3log;
        !);
    if($rv) {
    #    $db->commit;
        return $rv->[0];
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }

    return 0;
}

sub munge_date {
    my $input = shift;
    my $output = undef;
    # 2017.09.29-18.26,31000  intergossip     Sinistrad@Dead Souls Dev        f       pangean
    # Date needs to be in the format 2017-07-20 02:49:47.250

    #print STDERR "DEBUG: file_date == $input\n";
    $input =~ /^\s*(\d\d\d\d)\.(\d\d)\.(\d\d)\-(\d\d)\.(\d\d)\,(\d\d)(\d\d\d)\s*$/;
    my ($year, $month, $day, $hour, $minute, $second, $ms) = ($1,$2,$3,$4,$5,$6,$7);
    #print STDERR "DEBUG:      year == $year\n";
    #print STDERR "DEBUG:     month == $month\n";
    #print STDERR "DEBUG:       day == $day\n";
    #print STDERR "DEBUG:      hour == $hour\n";
    #print STDERR "DEBUG:       min == $minute\n";
    #print STDERR "DEBUG:       sec == $second\n";
    #print STDERR "DEBUG:        ms == $ms\n";
    #my $date = ParseDateFormat("%Y\\.%m\\.%d\\-%H\\.%M\\,%s", $file_date);
    my $standard = sprintf("%04d-%02d-%02d-%02d:%02d:%02d EST", $year, $month, $day, $hour, $minute, $second);
    #print STDERR "DEBUG:  standard == $standard\n";

    my $date = new Date::Manip::Date;
    $date->parse($standard);
    $output = $date->printf("%g");
    #print STDERR "DEBUG:    output == $output\n";
    $date->convert('UTC');
    $output = $date->printf("%g");
    #print STDERR "DEBUG:    output == $output\n";
    $output = $date->printf("%Y-%m-%d %H:%M:%S");
    $output = sprintf("%s.%03d", $output, $ms);
    #print STDERR "DEBUG:    output == $output\n";
    return $output;
}

my $offset = shift;
$offset = 0 if !defined $offset;

sub ctrl_c {
    my $signame = shift;

    print "Processed offset $offset .. current $counter == " . ($counter - $offset) . ".\n";
    $db->commit();
    $db->disconnect();
    exit 1;
}

$SIG{$_} = \&ctrl_c foreach qw( HUP INT );

#$db = DBI->connect("DBI:SQLite:dbname=$DB_FILE", '', '', { AutoCommit => 1, PrintError => 0, });
$db = DBI->connect("DBI:SQLite:dbname=$DB_FILE", '', '', { AutoCommit => 0, PrintError => 0, });
setup_i3log_table($db);
setup_urls_table($db);
setup_log_table($db);
$offset = check_offset($db) if !$offset;

my $insert_sql = $db->prepare( qq!
    INSERT INTO i3log ( created, is_emote, is_url, is_bot, channel, speaker, mud, message )
    VALUES (?,?,?,?,?,?,?,?)
    !);
open FP, "$I3LOG_FILE" or die "Cannot find I3 LOG file $I3LOG_FILE: $!";
my $line = "";
my $success = 0;
my $fail = 0;
while($line = <FP>) {
    $counter++;
    next if $counter <= $offset;
    chomp $line;
    my @bits = split /\t/, $line;
    my ($speaker, $mud) = split /\@/, $bits[2];
    my $date = munge_date($bits[0]);
    # 2017.09.29-18.26,31000  intergossip     Sinistrad@Dead Souls Dev        f       pangean
    # Date needs to be in the format 2017-07-20 02:49:47.250
    my $rv = $insert_sql->execute(
        $date,
        ($bits[3] and $bits[3] eq 't') ? 1 : 0,
        0,
        0,
        $bits[1],
        $speaker,
        $mud,
        $bits[4]
    );
    if($rv) {
        $db->commit if !($counter % 100);
        $success++;
    } else {
        print STDERR $DBI::errstr."\n";
        $fail++;
        $db->rollback;
        last;
    }
    print "$success succeeded, $fail failed, of $counter processed.\n" if !($counter % 100);
}
close FP;
$db->commit();
$db->disconnect();
print "$success succeeded, $fail failed.\n";

