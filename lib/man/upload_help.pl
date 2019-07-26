#!/usr/bin/perl -w

use strict;
use utf8;
use English;
use Data::Dumper;
use Text::ParseWords;
use DBI;

my %keywords = ();
my %entries = ();
my $entry_count = 0;

foreach my $filename (qw(help_table wizhelp_table)) {
    open FH, $filename or die "Cannot open $filename: $!";
    my @these_keywords = ();
    my @message_lines = ();

    while(<FH>) {
        chomp;
        next if /^$/ and scalar @these_keywords < 1;
        if(scalar @these_keywords < 1) {
            #@keywords = (split /\s+/, $_);
            @these_keywords = quotewords('\s+', 0, $_);
        } elsif (/^#~?$/) {
            $entry_count++;
            warn "Duplicate found?" if exists $entries{$entry_count};
            my $main_keyword = $these_keywords[0];
            $entries{$entry_count} = {
                id          => $entry_count,
                immortal    => $filename eq 'new_wizhelp_table' ? 1 : 0,
                message     => "$main_keyword\r\n" .
                               join("\r\n", @message_lines) .
                               "\r\n",
                keywords    => [ @these_keywords ],
            };
            foreach my $keyword (@these_keywords) {
                $keywords{$keyword} = {
                    id          => $entry_count,
                    keyword     => $keyword,
                };
            }
            @these_keywords = ();
            @message_lines = ();
        } else {
            push @message_lines, $_;
        }
    }
    close FH;
}

#print Dumper(\%entries);
#print Dumper(\%keywords);

print "Connecting to SQL...\n";
my $sql = undef;
my $rv = undef;
my $dbc = DBI->connect("dbi:Pg:dbname=wileymud", 'wiley', 'tardis69',
    { AutoCommit => 0, RaiseError => 1, PrintError => 0, });

# First let's make sure the tables are set up...
$sql = $dbc->prepare( qq!
    CREATE TABLE IF NOT EXISTS help_messages (
        updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(),
        immortal BOOLEAN NOT NULL DEFAULT false,
        set_by TEXT NOT NULL DEFAULT 'SYSTEM',
        id INTEGER PRIMARY KEY NOT NULL,
        message TEXT
    );
    !);
$rv = $sql->execute();
if($rv) {
    $dbc->commit;
} else {
    print STDERR $DBI::errstr."\n";
    $dbc->rollback;
}
print "help_messages setup.\n";
$sql = $dbc->prepare( qq!
    CREATE TABLE IF NOT EXISTS help_keywords (
        id INTEGER NOT NULL REFERENCES help_messages(id),
        keyword TEXT PRIMARY KEY NOT NULL
    );
    !);
$rv = $sql->execute();
if($rv) {
    $dbc->commit;
} else {
    print STDERR $DBI::errstr."\n";
    $dbc->rollback;
}
print "help_keywords setup.\n";

# Now truncate both of them in case they already existed and had stuff in them.
$sql = $dbc->prepare( qq!
    TRUNCATE help_keywords, help_messages;
    !);
$rv = $sql->execute();
if($rv) {
    $dbc->commit;
} else {
    print STDERR $DBI::errstr."\n";
    $dbc->rollback;
}
print "tables truncated.\n";

$sql = $dbc->prepare( qq!
    INSERT INTO help_messages (id, message, immortal) VALUES (?,?,?);
    !);
foreach my $k (sort {$a <=> $b} keys %entries) {
    print "inserting help_message $k.\n";
    $rv = $sql->execute($entries{$k}{id}, $entries{$k}{message}, $entries{$k}{immortal});
    if(!$rv) {
        print STDERR $DBI::errstr."\n";
        $dbc->rollback;
        exit 0;
    }
}
$dbc->commit;

$sql = $dbc->prepare( qq!
    INSERT INTO help_keywords (id, keyword) VALUES (?,?);
    !);
foreach my $k (sort {lc $a cmp lc $b} keys %keywords) {
    print "inserting help_keyword $k.\n";
    $rv = $sql->execute($keywords{$k}{id}, $keywords{$k}{keyword});
    if(!$rv) {
        print STDERR $DBI::errstr."\n";
        $dbc->rollback;
        exit 0;
    }
}
$dbc->commit;
$dbc->disconnect();
print "DONE!\n";

