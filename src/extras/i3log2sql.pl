#!/usr/bin/perl -w

use strict;
use English;
use Data::Dumper;

use Time::HiRes qw(sleep time alarm);
use Date::Parse;
use HTML::Entities;
use DBI;
use Net::Twitter;
use Digest::SHA qw(sha256_base64);

my $TEXT_FILE = '/home/bloodlines/lib/secure/log/allchan.log';
my $ARCHIVE = '/home/bloodlines/lib/secure/log/archive/allchan.log-*';
my $CHATTER = '/home/bloodlines/lib/secure/save/chat.o';

my $LOGDIR = '/home/bloodlines/lib/log/chan';
my $LOCAL_MUD = 'Bloodlines';
my $network = 'i3';
my $dbc = DBI->connect('DBI:Pg:dbname=i3logs;host=localhost;port=5432;sslmode=prefer', 'bloodlines', 'tardis69', { AutoCommit => 0, PrintError => 0, });
my $BE_A_TWIT = 0;

=head1 SQL

CREATE TABLE bots (
    channel     TEXT,
    speaker     TEXT,
    mud         TEXT NOT NULL
);

CREATE UNIQUE INDEX ix_bottable ON bots(channel, speaker, mud);

CREATE TABLE chanlogs (
    msg_date    TIMESTAMP WITHOUT TIME ZONE DEFAULT now() NOT NULL,
    network     TEXT NOT NULL,
    channel     TEXT NOT NULL,
    speaker     TEXT NOT NULL,
    mud         TEXT NOT NULL,
    is_emote    BOOLEAN DEFAULT false,
    message     TEXT,
    is_url      BOOLEAN DEFAULT false,
    twat        BOOLEAN DEFAULT false,
    is_bot      BOOLEAN DEFAULT false,
    id          SERIAL NOT NULL,
    checksum    TEXT
);

CREATE INDEX ix_msg_date ON chanlogs (msg_date);
CREATE INDEX ix_channel ON chanlogs (channel);
CREATE INDEX ix_speaker ON chanlogs (speaker);
CREATE INDEX ix_mud ON chanlogs (mud);
CREATE UNIQUE INDEX ix_chanlogs ON chanlogs (msg_date, network, channel, speaker, mud, is_emote, message); 
CREATE INDEX ix_twat ON chanlogs (twat);
CREATE INDEX ix_bot ON chanlogs (is_bot);
CREATE INDEX ix_checksum ON chanlogs (checksum);

CREATE VIEW today AS
    SELECT to_char(chanlogs.msg_date, 'MM/DD HH24:MI'::text) AS "time", chanlogs.channel, (chanlogs.speaker || '@'::text) || chanlogs.mud AS speaker, chanlogs.message
    FROM chanlogs
    WHERE chanlogs.msg_date >= (now() - '1 day'::interval)
    ORDER BY chanlogs.msg_date;

CREATE FUNCTION fn_wordcount(text) RETURNS integer AS
'   my $text = $_[0];
    return undef if !defined $text;
    my @words = split /\\s+/, $text;
    return undef if !defined @words;
    return scalar(@words);'
LANGUAGE plperlu;

CREATE FUNCTION fn_properwordcount(text) RETURNS integer AS
'   my $text = $_[0];
    return undef if !defined $text;
    my @words = split /[^a-zA-Z0-9_-]+/, $text;
    my $count = 0;
    foreach (@words) {
      $count++ if /[a-zA-Z0-9_-]{5,}/;
    }
    return undef if !defined @words;
    return $count;'
LANGUAGE plperlu;

CREATE VIEW words AS
    SELECT speaker, sum(wordcount) AS words
    FROM (  SELECT speaker, length(message) AS wordcount
            FROM chanlogs
            WHERE NOT is_bot AND msg_date >= now() - INTERVAL '1 weeks'
            GROUP BY speaker, message )
    AS foo
    GROUP BY speaker
    ORDER BY words DESC;

-- insert into bots (channel, speaker, mud) select distinct channel, speaker, mud from chanlogs where speaker ilike 'gribbles'; 
-- begin; update chanlogs set is_bot = true where NOT is_bot and channel IN (select distinct channel from bots) and speaker IN (select distinct speaker from bots) and mud IN (select distinct mud from bots);

CREATE FUNCTION fn_sha256(text) RETURNS text
    AS '
    use Digest::SHA qw(sha256_base64);
    my $data = $_[0];
    my $b64 = sha256_base64($data);
    my $padlen = length($b64) % 4;
    my $result = $b64 . ("=" x $padlen);

    return $result;
'
    LANGUAGE plperlu;

ALTER FUNCTION fn_sha256(text) OWNER TO bloodlines;

CREATE OR REPLACE FUNCTION fn_update_checksum() RETURNS trigger AS $fn_update_checksum$
BEGIN
  new.checksum := fn_sha256(to_char(new.msg_date, 'YYYY-MM-DD HH:MI:SS')||new.channel||new.speaker||new.mud||new.message);
  RETURN new;
END;
$fn_update_checksum$ LANGUAGE plpgsql;

CREATE TRIGGER trg_update_checksum
    BEFORE INSERT OR UPDATE ON chanlogs
    FOR EACH ROW
    EXECUTE PROCEDURE fn_update_checksum();

=cut

my $twitter;

if( $BE_A_TWIT ) {
    $twitter = Net::Twitter->new(
        traits              => [qw/API::REST OAuth/],
        consumer_key        => 'dszLvOopdZUclvqUxy8A',
        consumer_secret     => '5iiKA6XQjzV0b21QjFN0ueY9hHSVLKAwg4J7z9KWIHg',
        access_token        => '385780619-iVtS1JeQv0l2bGMfKlhkHzMGcrPqvAjINTQ1ld3f',
        access_token_secret => '5rUcxpkJhw08UDGVFtfvHx1dMaAWY81VrCTQSTG834',
    );
}

my $add_entry_sql = $dbc->prepare( qq!
    INSERT INTO chanlogs (msg_date, network, channel, speaker, mud, message, is_url, is_bot)
    VALUES (?,trim(?),trim(?),trim(?),trim(?),trim(?),?,?)
    !);

my $botlist = bot_list();

sub sha256 {
    my $data = shift;
    my $b64 = sha256_base64($data);
    my $padlen = length($b64) % 4;
    my $result = $b64 . ("=" x $padlen);

    return $result;
}

sub most_recent_sql {
    my $res = $dbc->selectrow_hashref(qq!

        SELECT *
          FROM chanlogs
      ORDER BY msg_date DESC
         LIMIT 1

    !, undef);
    print STDERR $DBI::errstr."\n" if !defined $res;
    return $res;
}

sub is_already_there {
    my $checksum = shift;
    my $res = $dbc->selectrow_hashref(qq!

        SELECT id, checksum
          FROM chanlogs
         WHERE checksum = '$checksum'
         LIMIT 1

    !, undef);
    print STDERR $DBI::errstr."\n" if defined $DBI::errstr and !defined $res;
    return 1 if $res && $res->{'checksum'} && $res->{'checksum'} eq $checksum;
    return undef;
}

sub bot_list {
    my $res = $dbc->selectall_arrayref(qq!

        SELECT *
          FROM bots
      ORDER BY channel, speaker, mud DESC

    !, { Slice => {} } );
    print STDERR $DBI::errstr."\n" if !defined $res;
    return $res;
}

sub parse_log_line {
    my $line = shift;
    return undef if !defined $line;
    my @parts = split /\t/, $line;
    return undef if scalar(@parts) != 4;

    my %log_entry = ();

    my $timestamp = substr($parts[0], 11, 8);
    substr($timestamp, 2, 1) = ':';
    substr($timestamp, 5, 1) = ':';
    my $datestamp = substr($parts[0], 0, 10);
    substr($datestamp, 4, 1) = '-';
    substr($datestamp, 7, 1) = '-';

    $log_entry{'msg_date'} = "$datestamp $timestamp";       # Timestamp YYYY-MM-DD HH:MM:SS
    $log_entry{'network'} = $network;                       # Network is always i3

    my $channel = $parts[1];                                # Channel
    $log_entry{'channel'} = $channel;

    my $speaker = $parts[2];
    my @bits = split /@/, $speaker;
    #my $name = lcfirst $bits[0];
    my $name = $bits[0];
    my $mudname = join('@', @bits[1 .. scalar(@bits)-1]);

    $log_entry{'speaker'} = $name;                          # Character
    $log_entry{'mud'} = $mudname;                           # Mud

    my $message = $parts[3];
    $log_entry{'message'} = $message;                       # Message body

    $log_entry{'is_emote'} = undef;                         # Can't tell from the logs without more parsing...
    $log_entry{'is_url'} = 0;                               # Default false, but may be set if matched below
    $log_entry{'is_url'} = 1 if $message =~ /((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)+/;

    #$message = encode_entities($message);
    #$message = s/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/<a href="$1" target="I3-link">$1<\/a>/;
    #print Dumper(\%log_entry) if $log_entry{'channel'} eq 'free_speech';

    return \%log_entry;
}

sub load_logs {
    my $recent = most_recent_sql();
    my $recent_date = str2time($recent->{'msg_date'});
    my $oldest_date = undef;
    my $is_old = 0;

    my @files = ( $TEXT_FILE, reverse sort glob $ARCHIVE );
    my @lines = ();

    foreach my $file ( @files ) {
        open FH, '<', $file or die "Cannot open log $file: $!";
        while(my $line = <FH>) {
            chomp $line;
            my @parts = split /\t/, $line;
            if( scalar(@parts) == 4) {
                my $oldest = parse_log_line($line);
                $oldest_date = str2time($oldest->{'msg_date'});
                $is_old = 1 if $oldest_date < $recent_date;
                push @lines, $line if $oldest_date >= $recent_date;
            }
            #last if $is_old;
        }
        close FH;
        #@lines = grep /((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)+/, @lines;
        # $lines[0] should be the oldest log entry for the given file, so if it's older than the newest sql entry, we need go no further back.
        #my $oldest = parse_log_line($lines[0]);
        #my $oldest_date = str2time($oldest->{'msg_date'});
        print "$file : $oldest_date - $recent_date\n";
        print "$file is OLDER than SQL\n" if $is_old;
        #print "$file is NEWER than SQL\n" if $oldest_date >= $recent_date;
        last if $is_old;
    }
    @lines = sort @lines;
    my $total = scalar @lines;
    print "Collected $total lines to insert\n";
    my $done = 0;
    my @tweets = ();
    foreach my $line (@lines) {
        my $entry = parse_log_line($line);
        my $entry_date = str2time($entry->{'msg_date'});
        if ((defined $entry) and ($entry_date >= $recent_date)) {
            my $work = add_entry($entry);
            if( $work ) {
                $done++;
                push @tweets, $entry;
            }
        }
    }
    print "Inserted $done lines\n";

    if( $BE_A_TWIT ) {
        my $rate_limit = $twitter->rate_limit_status()->{'remaining_hits'};
        my $ip_limit = $twitter->rate_limit_status({ authenticate => 0 })->{'remaining_hits'};
        print "Rate Limit: $rate_limit\n";
        print "IP Rate Limit: $ip_limit\n";

        eval {
            local $SIG{ALRM} = sub { die "Exceeded Timeout of 50 seconds for twitter loop." };
            alarm 50;
            foreach my $entry (@tweets) {
                my $guy = sprintf("<%s> %s@%s", $entry->{'channel'}, $entry->{'speaker'}, $entry->{'mud'});
                #my $guylen = length($guy);
                my $msg = $entry->{'message'};
                #my $msglen = length($msg);
                my $output = "$guy: $msg";
                $output = (substr($output, 0, 137) . "...") if(length($output) > 140);

                if( $rate_limit > 1 ) {
                    $rate_limit--;

                    print "Sent Twitter $output\n";
                    my $twit = $twitter->update($output);
                    print "Error: $!\n" if $!;
                    print "Twitter said " . Dumper($twit) . "\n";
                } else {
                    print "Skipped Twitter $output\n";
                }
            }
            alarm 0;
        };
        print "Error: $@\n" if $@; # and $EVAL_ERROR =~ /^Exceeded Timeout/;
    }
}

sub add_entry {
    my $data = shift;
    my $is_bot = 0;

    $is_bot = 1 if grep {   ( !defined $_->{'channel'} || $data->{'channel'} eq $_->{'channel'} )
                        &&  ( !defined $_->{'speaker'} || $data->{'speaker'} eq $_->{'speaker'} )
                        &&  $data->{'mud'} eq $_->{'mud'} 
                        } @$botlist;

    my $string = $data->{'msg_date'} . $data->{'channel'} . $data->{'speaker'} . $data->{'mud'} . $data->{'message'};
    my $checksum = sha256($string);
    return 0 if is_already_there($checksum);
    
    my $rv = $add_entry_sql->execute($data->{'msg_date'}, $data->{'network'}, $data->{'channel'}, $data->{'speaker'}, $data->{'mud'}, $data->{'message'}, $data->{'is_url'}, $is_bot);
    if($rv) {
        $dbc->commit;
        return 1;
    } else {
        print STDERR $DBI::errstr."\n";
        $dbc->rollback;
        return 0;
    }
}

#system ("chmod" "644" "$CHATTER");
#system ("chmod" "644" "$TEXT_FILE");
#system ("chmod" "644" "$ARCHIVE");

load_logs();

$dbc->disconnect();
exit 1;
