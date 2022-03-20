#!/usr/bin/perl -w

#
# This is a cut down version of my mklogpages.pl script.
#
# Now that we are no longer using the old static HTMLpages,
# there's no real value in continuing to create them, so I
# ripped out everything but the parts that dump JSON data
# for the newer web pages to use.
#

use utf8;
use strict;
#use Encode qw(encode_utf8);
no warnings 'utf8';
use English qw( −no_match_vars );
use Data::Dumper;
use DBI;
#use HTML::Entities;
#use Getopt::Long;
use JSON qw(encode_json);
#use File::Random qw(random_file);
#use Cwd qw(getcwd);
#use Parallel::ForkManager 0.7.6;
use Time::HiRes qw(time);

my $LOG_DIR         = '/var/www/html/log/data';
my $PG_DB           = 'i3log';

my $JS_DATE_CACHE       = "$LOG_DIR/log_navigation_dates.js";
my $NEW_DATE_CACHE      = "$LOG_DIR/date_counts.json";
my $NEW_PINKFISH_CACHE  = "$LOG_DIR/pinkfish.json";
my $NEW_PIE_CACHE       = "$LOG_DIR/pie.json";
my $MUD_SPEAKER_FILE    = "$LOG_DIR/i3.speakers";

sub open_postgres_db {
    my $DB_NAME = shift;

    my $db = DBI->connect("dbi:Pg:dbname=$DB_NAME", 'wiley', 'tardis69',
        { AutoCommit => 1, RaiseError => 1, PrintError => 0, });
    return $db;
}

sub save_json_cache {
    my $data = shift;
    my $filename = shift;

    my $json_data = encode_json($data);
    die "Invalid JSON data for $filename!" if !defined $json_data;
    open FP, ">$filename" or die "Cannot open output page $filename: $!";
    print FP $json_data;
    close FP;
}

#CREATE TABLE pinkfish_map (
#            pinkfish TEXT PRIMARY KEY NOT NULL,
#            html TEXT
#        );
sub fetch_pinkfish_map {
    my $db = shift;

    my $rv = $db->selectall_hashref(qq!
        SELECT pinkfish, html
          FROM pinkfish_map
        ;!, 'pinkfish');
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv;
}

#CREATE TABLE hours (
#            hour INTEGER PRIMARY KEY NOT NULL,
#            pinkfish TEXT
#        );
sub fetch_hours {
    my $db = shift;

    my $rv = $db->selectall_hashref(qq!
        SELECT hour, hours.pinkfish, html
          FROM hours
     LEFT JOIN pinkfish_map
            ON (hours.pinkfish = pinkfish_map.pinkfish)
        ;!, 'hour');
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv;
}

#CREATE TABLE channels (
#            channel TEXT PRIMARY KEY NOT NULL,
#            pinkfish TEXT
#        );
sub fetch_channels {
    my $db = shift;

    my $rv = $db->selectall_hashref(qq!
        SELECT channel, channels.pinkfish, html
          FROM channels
     LEFT JOIN pinkfish_map
            ON (channels.pinkfish = pinkfish_map.pinkfish)
        ;!, 'channel');
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv;
}

#CREATE TABLE speakers (
#            speaker TEXT PRIMARY KEY NOT NULL,
#            pinkfish TEXT
#        );
sub fetch_speakers {
    my $db = shift;

    my $rv = $db->selectall_hashref(qq!
        SELECT speaker, speakers.pinkfish, html
          FROM speakers
     LEFT JOIN pinkfish_map
            ON (speakers.pinkfish = pinkfish_map.pinkfish)
        ;!, 'speaker');
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv;
}

sub fetch_row_count {
    my $db = shift;

    my $rv = $db->selectrow_arrayref(qq!
        SELECT COUNT(*) FROM i3log;
        !);
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv ? $rv->[0] : 0;
}

sub update_all_speakers {
    my $db = shift;

    local $| = 1;
    print "Updating speaker info...";

    my @color_map = (
        "%^BLACK%^%^BOLD%^",
        "%^RED%^",
        "%^GREEN%^",
        "%^ORANGE%^",
        "%^BLUE%^",
        "%^MAGENTA%^",
        "%^CYAN%^",
        "%^WHITE%^",
        "%^RED%^%^BOLD%^",
        "%^GREEN%^%^BOLD%^",
        "%^YELLOW%^",
        "%^MAGENTA%^%^BOLD%^",
        "%^BLUE%^%^BOLD%^",
        "%^CYAN%^%^BOLD%^",
        "%^WHITE%^%^BOLD%^",
        '%^WHITE%^%^B_RED%^',
        '%^WHITE%^%^B_GREEN%^',
        '%^WHITE%^%^B_BLUE%^',
        '%^WHITE%^%^B_MAGENTA%^',
        '%^BLACK%^%^B_RED%^',
        '%^BLACK%^%^B_GREEN%^',
        '%^BLACK%^%^B_MAGENTA%^',
        '%^BLACK%^%^B_CYAN%^',
        '%^BLACK%^%^B_YELLOW%^',
        '%^BLACK%^%^B_WHITE%^',
    );
    my $color_count = scalar @color_map;
    my $speakers = fetch_speakers($db);
    my $speaker_count = scalar keys %$speakers;

    my $result = $db->selectall_arrayref(qq!
        SELECT DISTINCT speaker FROM i3log;
        ;!, { Slice => {} } );
    if($result) {
        foreach my $row (@$result) {
            next if !defined $row->{speaker};
            my $speaker = lc $row->{speaker};
            next if exists $speakers->{$speaker};
            my $new_color = @color_map[$speaker_count % $color_count];
            $speakers->{$speaker} = { 'speaker' => $speaker, 'pinkfish' => $new_color };
            $speaker_count = scalar keys %$speakers;

            $db->begin_work();
            my $insert_sql = $db->prepare( qq!
                INSERT INTO speakers ( speaker, pinkfish )
                VALUES (?,?)
                ON CONFLICT (speaker)
                DO NOTHING;
                !);
            my $rv = $insert_sql->execute( $speaker, $new_color );
            if($rv) {
                #$db->commit if !($counter % 100);
                $db->commit;
                print "Added $speaker as $new_color\n";
            } else {
                print STDERR $DBI::errstr."\n";
                $db->rollback;
            }
        }
    } else {
        print STDERR $DBI::errstr."\n";
    }

    print "done.\n";
}

sub update_all_channels {
    my $db = shift;

    local $| = 1;
    print "Updating channel info...";

    my @color_map = (
        "%^RED%^",
        "%^GREEN%^",
        "%^ORANGE%^",
        "%^BLUE%^",
        "%^MAGENTA%^",
        "%^CYAN%^",
        "%^WHITE%^",
        "%^RED%^%^BOLD%^",
        "%^GREEN%^%^BOLD%^",
        "%^YELLOW%^",
        "%^MAGENTA%^%^BOLD%^",
        "%^BLUE%^%^BOLD%^",
        "%^CYAN%^%^BOLD%^",
        "%^WHITE%^%^BOLD%^",
    );
    my $color_count = scalar @color_map;
    my $channels = fetch_channels($db);
    my $channel_count = scalar keys %$channels;

    my $result = $db->selectall_arrayref(qq!
        SELECT DISTINCT channel FROM i3log;
        ;!, { Slice => {} } );
    if($result) {
        foreach my $row (@$result) {
            next if !defined $row->{channel};
            my $channel = lc $row->{channel};
            next if exists $channels->{$channel};
            my $new_color = @color_map[$channel_count % $color_count];
            $channels->{$channel} = { 'channel' => $channel, 'pinkfish' => $new_color };
            $channel_count = scalar keys %$channels;

            $db->begin_work();
            my $insert_sql = $db->prepare( qq!
                INSERT INTO channels ( channel, pinkfish )
                VALUES (?,?)
                ON CONFLICT (channel)
                DO NOTHING;
                !);
            my $rv = $insert_sql->execute( $channel, $new_color );
            if($rv) {
                #$db->commit if !($counter % 100);
                $db->commit;
                print "Added $channel as $new_color\n";
            } else {
                print STDERR $DBI::errstr."\n";
                $db->rollback;
            }
        }
    } else {
        print STDERR $DBI::errstr."\n";
    }

    print "done.\n";
}

sub fetch_date_counts {
    my $db = shift;

    my $rv = $db->selectall_arrayref(qq!
        SELECT      to_char(local, 'YYYY-MM-DD')  the_date,
                    to_char(local, 'YYYY')        the_year,
                    to_char(local, 'MM')          the_month,
                    to_char(local, 'DD')          the_day,
                    COUNT(*) count
        FROM        i3log
        GROUP BY    the_date, the_year, the_month, the_day
        ORDER BY    the_date ASC;
        ;!, { Slice => {} } );
    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv;
}

sub save_date_cache {
    my $date_data = shift;
    my $filename = shift;

    my $data = {};
    foreach my $row (@$date_data) {
        $data->{$row->{the_date}} = $row;
    }

    my $json_data = encode_json($data);
    die "Invalid JSON data for $filename!" if !defined $json_data;
    open FP, ">$filename" or die "Cannot open output page $filename: $!";
    print FP $json_data;
    close FP;
}

sub generate_pie_stuff {
    my $db = shift;

    my $result;
    my $data = {};

    my $quote_sql = {};
    $quote_sql->{today} = qq!
        SELECT speaker, message
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now())
        OFFSET floor(random() * (
            SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now())
            )) LIMIT 1
    !;
    $quote_sql->{yesterday} = qq!
        SELECT speaker, message
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now()) - '1 day'::interval
        OFFSET floor(random() * (
            SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now()) - '1 day'::interval
            )) LIMIT 1
    !;
    $quote_sql->{week} = qq!
        SELECT speaker, message
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 week'::interval AND now()
        OFFSET floor(random() * (
            SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 week'::interval AND now()
            )) LIMIT 1
    !;
    $quote_sql->{month} = qq!
        SELECT speaker, message
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 month'::interval AND now()
        OFFSET floor(random() * (
            SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 month'::interval AND now()
            )) LIMIT 1
    !;
    $quote_sql->{year} = qq!
        SELECT speaker, message
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 year'::interval AND now()
        OFFSET floor(random() * (
            SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 year'::interval AND now()
            )) LIMIT 1
    !;
    $quote_sql->{all} = qq!
        SELECT speaker, message
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
        OFFSET floor(random() * (
            SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot)
            ) LIMIT 1
    !;
    my $quotes = {};
    foreach (qw(today yesterday week month year all)) {
        $result = $db->selectall_arrayref( $quote_sql->{$_}, { Slice => {} } );
        $quotes->{$_} = $result->[0];
    }
    $data->{quotes} = $quotes;

    my $channel_sql = {};
    my $speaker_sql = {};
    $channel_sql->{today} = qq!
        SELECT channel, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now())
        GROUP BY channel
        ORDER BY count DESC
        LIMIT 12
    !;
    $speaker_sql->{today} = qq!
        SELECT username AS speaker, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now())
        GROUP BY username
        ORDER BY count DESC
        LIMIT 12
    !;
    $channel_sql->{yesterday} = qq!
        SELECT channel, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now()) - '1 day'::interval
        GROUP BY channel
        ORDER BY count DESC
        LIMIT 12
    !;
    $speaker_sql->{yesterday} = qq!
        SELECT username AS speaker, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND date(local) = date(now()) - '1 day'::interval
        GROUP BY username
        ORDER BY count DESC
        LIMIT 12
    !;
    $channel_sql->{week} = qq!
        SELECT channel, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 week'::interval AND now()
        GROUP BY channel
        ORDER BY count DESC
        LIMIT 12
    !;
    $speaker_sql->{week} = qq!
        SELECT username AS speaker, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 week'::interval AND now()
        GROUP BY username
        ORDER BY count DESC
        LIMIT 12
    !;
    $channel_sql->{month} = qq!
        SELECT channel, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 month'::interval AND now()
        GROUP BY channel
        ORDER BY count DESC
        LIMIT 12
    !;
    $speaker_sql->{month} = qq!
        SELECT username AS speaker, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 month'::interval AND now()
        GROUP BY username
        ORDER BY count DESC
        LIMIT 12
    !;
    $channel_sql->{year} = qq!
        SELECT channel, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 year'::interval AND now()
        GROUP BY channel
        ORDER BY count DESC
        LIMIT 12
    !;
    $speaker_sql->{year} = qq!
        SELECT username AS speaker, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
                AND local BETWEEN date_trunc('day', now()) - '1 year'::interval AND now()
        GROUP BY username
        ORDER BY count DESC
        LIMIT 12
    !;
    $channel_sql->{all} = qq!
        SELECT channel, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
        GROUP BY channel
        ORDER BY count DESC
        LIMIT 12
    !;
    $speaker_sql->{all} = qq!
        SELECT username AS speaker, count(*)
        FROM i3log
        WHERE speaker <> 'URLbot' AND NOT is_bot
        GROUP BY username
        ORDER BY count DESC
        LIMIT 12
    !;
    my $speakers = {};
    my $channels = {};
    foreach (qw(today yesterday week month year all)) {
        $result = $db->selectall_arrayref( $speaker_sql->{$_}, { Slice => {} } );
        $speakers->{$_} = $result;
    }
    $data->{speakers} = $speakers;
    foreach (qw(today yesterday week month year all)) {
        $result = $db->selectall_arrayref( $channel_sql->{$_}, { Slice => {} } );
        $channels->{$_} = $result;
    }
    $data->{channels} = $channels;

    # old dates.json
    my $dates;
    my $dates_sql = qq!
        SELECT date(now()) AS today,
               date(now() - '1 day'::interval) AS yesterday,
               date(now() - '1 week'::interval) AS week,
               date(now() - '1 month'::interval) AS month,
               date(now() - '1 year'::interval) AS year,
               date(min(local)) AS all
          FROM i3log
    !;
    $result = $db->selectall_arrayref( $dates_sql, { Slice => {} } );
    $dates = $result->[0];
    $data->{dates} = $dates;

    # Collect all results into one thing to save
    save_json_cache($data, $NEW_PIE_CACHE);
}

sub generate_js_dates {
    my $db = shift;
    my $date_counts = shift;
    die "Invalid date information!" if !defined $date_counts;

    local $| = 1;
    print "Generating navigation data...";

    my $filename = $JS_DATE_CACHE;

    my @date_list = ();
    foreach my $row (@$date_counts) {
        push @date_list, ($row->{the_date}); # if -f page_path($row);
    }
    my $big_list = join(",\n", map { sprintf "\"%s\"", $_; } (@date_list));

    open FP, ">$filename" or die "Cannot open script $filename: $!";
    print FP "var validDates = [\n$big_list\n];\n";
    close FP;
    print "done.\n";
}

sub dump_speakers {
    my $speakers = shift or die "No speakers data provided.";

    local $| = 1;
    print "Dumping I3 speaker file for WileyMUD...";

    open FP, ">$MUD_SPEAKER_FILE" or die "Cannot open speaker file $MUD_SPEAKER_FILE: $!";
    printf FP "#COUNT\nCount   %d\nEnd\n\n", scalar keys %$speakers;
    foreach my $row (map { $speakers->{$_} } (sort keys %$speakers)) {
        printf FP "#SPEAKER\nName    %s\nColor   %s\nEnd\n\n", $row->{speaker}, $row->{pinkfish};
    }
    printf FP "#END\n";
    close FP;

    print "done.\n";
}

sub main {
    print "Initializing...\n";

    my $DATABASE        = open_postgres_db($PG_DB);

    my $row_count       = fetch_row_count($DATABASE);
    my $date_counts     = fetch_date_counts($DATABASE);
    my $pinkfish_map    = fetch_pinkfish_map($DATABASE);
    my $hours           = fetch_hours($DATABASE);

    update_all_channels($DATABASE);
    update_all_speakers($DATABASE);

    my $channels        = fetch_channels($DATABASE);
    my $speakers        = fetch_speakers($DATABASE);

    save_date_cache($date_counts, $NEW_DATE_CACHE);
    save_json_cache($pinkfish_map, $NEW_PINKFISH_CACHE);

    generate_js_dates($DATABASE, $date_counts);                 # $JS_DATE_CACHE
    generate_pie_stuff($DATABASE);                              # $NEW_PIE_CACHE

    dump_speakers($speakers);                                   # Mud file i3.speakers

    print "Done!\n";
}

main();
exit 0;

