#!/usr/bin/perl -w

use utf8;
use strict;
use Encode qw(encode_utf8);
no warnings 'utf8';
use English;
use DBI;
use Data::Dumper;
use JSON qw(encode_json decode_json);
use Time::HiRes qw(time sleep alarm);
use Try::Tiny;
no warnings 'once';

my $DB_FILE         = '/home/wiley/lib/i3/wiley.bkp-20190223.db';
my $JSON_DIR        = '/home/wiley/public_html/logdata';
my $PG_DB           = 'i3log';
my $JSON_PINKFISH   = '/home/wiley/public_html/logpages/pinkfish.json';
my $JSON_HOURS      = '/home/wiley/public_html/logpages/hours.json';
my $JSON_CHANNELS   = '/home/wiley/public_html/logpages/channels.json';
my $JSON_SPEAKERS   = '/home/wiley/public_html/logpages/speakers.json';
my $JSON_DATES      = '/home/wiley/public_html/logpages/date_counts.json';

# /usr/bin/bzip2 -9cq wiley.bkp-20190220.db
my $KOMPRESSOR      = '/usr/bin/bzip2';
my $KOMPRESSOR_ARGS = '-9cq';
my $KOMPRESSOR_EXT  = 'bz2';
my $UNKOMPRESSOR    = '/usr/bin/bzcat';

sub open_sqlite_db {
    my $DATABASE = shift;

    my $db = DBI->connect("DBI:SQLite:dbname=$DATABASE", '', '',
        { AutoCommit => 1, PrintError => 1, });
    return $db;
}

sub open_postgres_db {
    my $DATABASE = shift;

    my $db = DBI->connect("dbi:Pg:dbname=$DATABASE", 'wiley', 'tardis69',
        { AutoCommit => 0, RaiseError => 1, PrintError => 0, });
    return $db;
}

sub open_postgres_tcp_db {
    my $DATABASE = shift;

    my $db = DBI->connect("dbi:Pg:dbname=$DATABASE;host=localhost;port=5432", 'wiley', 'tardis69',
        { AutoCommit => 0, RaiseError => 1, PrintError => 0, });
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

sub import_compressed_json {
    my $filename = shift;

    local $/ = undef;
    open FP, "$UNKOMPRESSOR $filename|" or die "Cannot open data file $filename: $!";
    my $data = <FP>;
    close FP;
    my $object = decode_json($data);
    return $object;
}

sub create_tables {
    my $DATABASE = shift;

    my $db = open_postgres_tcp_db($DATABASE);
    my $rc = undef;
    try {
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS pinkfish_map (
                    pinkfish TEXT PRIMARY KEY NOT NULL,
                    html TEXT NOT NULL
                );
            !);
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS hours (
                    hour INTEGER PRIMARY KEY NOT NULL,
                    pinkfish TEXT NOT NULL REFERENCES pinkfish_map (pinkfish)
                );
            !);
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS channels (
                    channel TEXT PRIMARY KEY NOT NULL,
                    pinkfish TEXT NOT NULL REFERENCES pinkfish_map (pinkfish)
                );
            !);
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS speakers (
                    speaker TEXT PRIMARY KEY NOT NULL,
                    pinkfish TEXT NOT NULL REFERENCES pinkfish_map (pinkfish)
                );
            !);
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS i3log (
                    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'),
                    local TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(),
                    is_emote BOOLEAN,
                    is_url BOOLEAN,
                    is_bot BOOLEAN,
                    channel TEXT NOT NULL,
                    speaker TEXT NOT NULL,
                    mud TEXT NOT NULL,
                    message TEXT
                );
            !);
        $rc = $db->do(qq!
                CREATE INDEX IF NOT EXISTS ix_i3log_local ON i3log (local);
            !);
        $rc = $db->do(qq!
                ALTER TABLE i3log DROP CONSTRAINT IF EXISTS ix_i3log_row;
            !);
        $rc = $db->do(qq!
                ALTER TABLE i3log ADD CONSTRAINT ix_i3log_row UNIQUE 
                    (created, local, is_emote, is_url, is_bot,
                     channel, speaker, mud, message);
            !);
        $rc = $db->do(qq!
                DROP VIEW IF EXISTS page_view;
            !);
        $rc = $db->do(qq!
                CREATE VIEW page_view AS
                 SELECT i3log.local,
                        i3log.is_emote,
                        i3log.is_url,
                        i3log.is_bot,
                        i3log.channel,
                        i3log.speaker,
                        i3log.mud,
                        i3log.message,
                        to_char(i3log.local, 'YYYY-MM-DD')  the_date,
                        to_char(i3log.local, 'HH24:MI:SS')  the_time,
                        to_char(i3log.local, 'YYYY')        the_year,
                        to_char(i3log.local, 'MM')          the_month,
                        to_char(i3log.local, 'DD')          the_day,
                        to_char(i3log.local, 'HH24')        the_hour,
                        to_char(i3log.local, 'MI')          the_minute,
                        to_char(i3log.local, 'SS')          the_second,
                        date_part('hour', i3log.local)      int_hour,
                        hours.pinkfish                      hour_color,
                        channels.pinkfish                   channel_color,
                        speakers.pinkfish                   speaker_color,
                        pinkfish_map_hour.html              hour_html,
                        pinkfish_map_channel.html           channel_html,
                        pinkfish_map_speaker.html           speaker_html
                   FROM i3log
              LEFT JOIN hours
                     ON (date_part('hour', i3log.local) = hours.hour)
              LEFT JOIN channels
                     ON (lower(i3log.channel) = channels.channel)
              LEFT JOIN speakers
                     ON (lower(i3log.speaker) = speakers.speaker)
              LEFT JOIN pinkfish_map pinkfish_map_hour
                     ON (hours.pinkfish = pinkfish_map_hour.pinkfish)
              LEFT JOIN pinkfish_map pinkfish_map_channel
                     ON (channels.pinkfish = pinkfish_map_channel.pinkfish)
              LEFT JOIN pinkfish_map pinkfish_map_speaker
                     ON (speakers.pinkfish = pinkfish_map_speaker.pinkfish);
            !);
        # Note that the version in mklogpages.pl (for SQLite) uses
        #   WHERE date(i3log.local) = ?
        #   ORDER BY i3log.local ASC;
        #
        # When using this view, you would make that
        #   WHERE date(local) = ?
        #   ORDER BY local ASC;
        #   
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS urls (
                    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'),
                    processed BOOLEAN,
                    channel TEXT NOT NULL,
                    speaker TEXT NOT NULL,
                    mud TEXT NOT NULL,
                    url TEXT,
                    message TEXT,
                    checksum TEXT
                );
            !);
        $rc = $db->do(qq!
                CREATE TABLE IF NOT EXISTS log (
                    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'),
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
    } catch {
        warn "Transaction aborted because $_ ($rc)"; # Try::Tiny copies $@ into $_
        eval { $db->rollback(); };
    };
    $db->commit();
    $db->disconnect();
}

sub fill_pinkfish_table {
    my $DATABASE = shift;
    my $data = shift;

    my $db = open_postgres_tcp_db($DATABASE);
    try {
        my $sth = $db->prepare(qq!
                INSERT INTO pinkfish_map (pinkfish, html) VALUES (?,?)
                ON CONFLICT (pinkfish)
                DO NOTHING;
            !);
            foreach my $row (map { $data->{$_} }
                (sort { length $b <=> length $a } keys %$data)) {
                $sth->execute( $row->{pinkfish}, $row->{html} );
            }
    } catch {
        warn "Transaction aborted because $_"; # Try::Tiny copies $@ into $_
        eval { $db->rollback(); };
    };
    $db->commit();
    $db->disconnect();
}

sub fill_hours_table {
    my $DATABASE = shift;
    my $data = shift;

    my $db = open_postgres_tcp_db($DATABASE);
    try {
        my $sth = $db->prepare(qq!
                INSERT INTO hours (hour, pinkfish) VALUES (?,?)
                ON CONFLICT (hour)
                DO NOTHING;
            !);
            foreach my $row (map { $data->{$_} }
                (sort keys %$data)) {
                $sth->execute( $row->{hour}, $row->{pinkfish} );
            }
    } catch {
        warn "Transaction aborted because $_"; # Try::Tiny copies $@ into $_
        eval { $db->rollback(); };
    };
    $db->commit();
    $db->disconnect();
}

sub fill_channels_table {
    my $DATABASE = shift;
    my $data = shift;

    my $db = open_postgres_tcp_db($DATABASE);
    try {
        my $sth = $db->prepare(qq!
                INSERT INTO channels (channel, pinkfish) VALUES (?,?)
                ON CONFLICT (channel)
                DO NOTHING;
            !);
            foreach my $row (map { $data->{$_} }
                (sort keys %$data)) {
                $sth->execute( $row->{channel}, $row->{pinkfish} );
            }
    } catch {
        warn "Transaction aborted because $_"; # Try::Tiny copies $@ into $_
        eval { $db->rollback(); };
    };
    $db->commit();
    $db->disconnect();
}

sub fill_speakers_table {
    my $DATABASE = shift;
    my $data = shift;

    my $db = open_postgres_tcp_db($DATABASE);
    try {
        my $sth = $db->prepare(qq!
                INSERT INTO speakers (speaker, pinkfish) VALUES (?,?)
                ON CONFLICT (speaker)
                DO NOTHING;
            !);
            foreach my $row (map { $data->{$_} }
                (sort keys %$data)) {
                $sth->execute( $row->{speaker}, $row->{pinkfish} );
            }
    } catch {
        warn "Transaction aborted because $_"; # Try::Tiny copies $@ into $_
        eval { $db->rollback(); };
    };
    $db->commit();
    $db->disconnect();
}

sub fill_i3log_table {
    my $DATABASE = shift;
    my $data = shift;

    my $db = open_postgres_tcp_db($DATABASE);
    try {
        # select (to_timestamp('2006-09-24 23:55:22.000', 'YYYY-MM-DD HH24:MI:SS.MS')
        #         at time zone 'US/Pacific')::TIMESTAMP WITH TIME ZONE;
        # ----------------------
        # 2006-09-24 23:55:22-07
        #
        # select (to_timestamp('2006-09-24 23:55:22.000', 'YYYY-MM-DD HH24:MI:SS.MS')
        #         at time zone 'UTC')::TIMESTAMP WITHOUT TIME ZONE;
        # ------------------------
        # 2006-09-25 06:55:22
        #
        # So, assuming our created timestamps are indeed LOCAL time, we can use these
        # to provide postgres with our timezone friendly data

        my $sth = $db->prepare(qq!
                INSERT INTO i3log ( created, local, is_emote, is_url, is_bot,
                                    channel, speaker, mud, message)
                VALUES (
                    (to_timestamp(?,'YYYY-MM-DD HH24:MI:SS.MS')
                    AT TIME ZONE 'UTC')::TIMESTAMP WITHOUT TIME ZONE,
                    (to_timestamp(?,'YYYY-MM-DD HH24:MI:SS.MS')
                    AT TIME ZONE 'US/Pacific')::TIMESTAMP WITH TIME ZONE,
                    ?,?,?,?,?,?,?)
                ON CONFLICT ON CONSTRAINT ix_i3log_row
                DO NOTHING;
            !);
            foreach my $row (@$data) {
                $sth->execute(
                    $row->{created}, $row->{local}, $row->{is_emote},
                    $row->{is_url}, $row->{is_bot}, $row->{channel},
                    $row->{speaker}, $row->{mud}, $row->{message}
                );
            }
    } catch {
        warn "Transaction aborted because $_"; # Try::Tiny copies $@ into $_
        eval { $db->rollback(); };
    };
    $db->commit();
    $db->disconnect();
}

my $pinkfish = import_json( $JSON_PINKFISH );
my $hours = import_json( $JSON_HOURS );
my $channels = import_json( $JSON_CHANNELS );
my $speakers = import_json( $JSON_SPEAKERS );
my $date_counts = import_json( $JSON_DATES );

#print Dumper($pinkfish) . "\n";
#print Dumper($hours) . "\n";
#print Dumper($channels) . "\n";
#print Dumper($speakers) . "\n";
#print Dumper($date_counts) . "\n";

my $start_time = time();
create_tables( $PG_DB );
fill_pinkfish_table( $PG_DB, $pinkfish );
fill_hours_table( $PG_DB, $hours );
fill_channels_table( $PG_DB, $channels );
fill_speakers_table( $PG_DB, $speakers );

foreach my $date (sort keys %$date_counts) {
    # fetch JSON data for each archived day
    # munge results into a format postgresql can handle
    # push into fill_i3log_table
    #print Dumper($date_counts->{$date}) . "\n";
    my $json_year = $date_counts->{$date}{the_year};
    my $json_month = $date_counts->{$date}{the_month};
    my $json_day = $date_counts->{$date}{the_day};
    my $json_data = import_compressed_json( "$JSON_DIR/$json_year/$json_month/$date.json.$KOMPRESSOR_EXT" );
    print "Importing $json_year-$json_month-$json_day\n";
    my @data = ();
    foreach my $row (@$json_data) {
        my $output = {};
        # row->{created} looks like => '2006-09-24 23:55:22.000'
        #
        $output->{created} = $row->{created}, # We let the query do timezone conversion
        $output->{local} = $row->{created}, # We let the query do timezone conversion
        $output->{is_emote} = $row->{is_emote};
        $output->{is_url} = $row->{is_url};
        $output->{is_bot} = $row->{is_bot};
        $output->{channel} = $row->{channel};
        $output->{speaker} = $row->{speaker};
        $output->{mud} = $row->{mud};
        $output->{message} = $row->{message};
        push @data, $output;
    }
    #print Dumper(\@data) . "\n";
    fill_i3log_table( $PG_DB, \@data );
}

my $time_spent = time() - $start_time;
print "ALL operations took $time_spent seconds.\n";

exit 1;

# INSERT INTO pinkfish (pinkfish, html) VALUES (?,?)
# ON CONFLICT (pinkfish)
# DO UPDATE SET html = EXCLUDED.html;
#
# INSERT INTO pinkfish (pinkfish, html) VALUES (?,?)
# ON CONFLICT (pinkfish)
# DO NOTHING;
#

#my $sqlite_start = time();
#my $sqlite = open_sqlite_db($DB_FILE);
#my $sqlite_version = $DBD::SQLite::sqlite_version;
#$sqlite->disconnect();
#my $sqlite_length = time() - $sqlite_start;

#my $postgres_start = time();
#my $postgres = open_postgres_tcp_db($PG_DB);
#my $postgres_version = $postgres->{pg_server_version};
#$postgres->disconnect();
#my $postgres_length = time() - $postgres_start;

# x0y0z
#$postgres_version =~ /(\d{1,3})0(\d{1,3})0(\d{1,3})/;
#my ($postgres_major, $postgres_minor, $postgres_revision) = ($1, $2, $3);
#$postgres_version = sprintf "%d.%d.%d", $postgres_major, $postgres_minor, $postgres_revision;

#print "    SQLite version:          $sqlite_version\n";
#print "           connect time:     $sqlite_length\n";
#print "PostgreSQL version:          $postgres_version\n";
#print "           connect time:     $postgres_length\n";

