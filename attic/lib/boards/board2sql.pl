#!/usr/bin/perl -w

use strict;
use English;
use Data::Dumper;

use Time::HiRes qw(sleep time alarm);
use POSIX qw(strftime);
use DBI;

die "You must provide at least one filename!" if scalar @ARGV < 1;
my $dbc = DBI->connect("DBI:Pg:dbname=wiley;host=localhost;port=5432;sslmode=prefer", "quixadhal", "tardis69", { AutoCommit => 0, PrintError => 0, });
my $insert_query = $dbc->prepare( qq!
   INSERT INTO board_messages ( board_id, message_id, message_date, message_sender, message_header, message_text )
               VALUES ( ?, ?, to_timestamp( ?, 'Dy Mon DD HH24:MI:SS YYYY'), trim(?), trim(?), ?);
   !);

foreach my $filename (@ARGV) {
    $filename =~ /^(\d+)\.txt$/;
    my $vnum = $1;

    die "Invalid file: $filename" if !defined $vnum;

    open FP, $filename or die "Cannot open $filename: $!";
    my $id = 0;
    my $rv = $dbc->do( qq!
        DELETE FROM board_messages WHERE board_id = ?
    !, undef, $vnum);
    if($rv) {
        print "CLEARED: $vnum\n";
        $dbc->commit;
    } else {
        $dbc->rollback;
    }
    print "Processing $filename...\n";
    while(my $line = <FP>) {
        chomp $line;
        $line =~ /^([^\(]+)\((\w+)\)\s+(.+)$/;
        my ($subject, $sender, $date) = ($1, $2, $3);
        next if !defined $subject;
        next if !defined $sender;
        next if !defined $date;
        my $body = "";
        while(my $bodyline = <FP>) {
            chomp $bodyline;
            last if $bodyline eq ".";
            $body .= "$bodyline\r\n";
        }
        my $rv = $insert_query->execute($vnum, $id, $date, $sender, $subject, $body);
        if($rv) {
            print "ADDED:   $subject ($sender) $date\n";
            $dbc->commit;
            $id++;
        } else {
            print "FAILED:  $subject ($sender) $date\n";
            $dbc->rollback;
        }
    }
    print "Done!\n";
    close FP;
}

$dbc->disconnect();
1;
