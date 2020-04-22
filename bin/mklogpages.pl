#!/usr/bin/perl -w

use utf8;
use strict;
use Encode qw(encode_utf8);
no warnings 'utf8';
use English;
use Data::Dumper;
use DBI;
use HTML::Entities;
use Getopt::Long;
use JSON qw(encode_json);
use File::Random qw(random_file);
use Cwd qw(getcwd);

my $URL_HOME        = "http://wileymud.themud.org/~wiley";
my $LOG_HOME        = "$URL_HOME/logpages";
my $LIVE_PAGE       = "$LOG_HOME/";

my $LIVE_DB_FILE    = '/home/wiley/lib/i3/wiley.db';
my $DB_FILE         = '/home/wiley/lib/i3/wiley.bkp-20190223.db';
my $BACKGROUND_DIR  = '/home/wiley/public_html/gfx/wallpaper/';
my $PAGE_DIR        = '/home/wiley/public_html/logpages';
my $JSON_DIR        = '/home/wiley/public_html/logdata';
my $PG_DB           = 'i3log';

# /usr/bin/bzip2 -9cq wiley.bkp-20190220.db
my $KOMPRESSOR      = '/usr/bin/bzip2';
my $KOMPRESSOR_ARGS = '-9cq';
my $KOMPRESSOR_EXT  = 'bz2';
my $UNKOMPRESSOR    = '/usr/bin/bzcat';

# /usr/bin/xz -9eq -T0 wiley.bkp-20190220.db
#my $KOMPRESSOR      = '/usr/bin/xz';
#my $KOMPRESSOR_ARGS = '-9ceq -T0';
#my $KOMPRESSOR_EXT  = 'xz';
#my $UNKOMPRESSOR    = '/usr/bin/xzcat';

my $BEGIN_ICON      = "$URL_HOME/gfx/nav/begin.png";
my $PREV_ICON       = "$URL_HOME/gfx/nav/previous.png";
my $NEXT_ICON       = "$URL_HOME/gfx/nav/next.png";
my $END_ICON        = "$URL_HOME/gfx/nav/end.png";
my $UP_ICON         = "$URL_HOME/gfx/nav/green/up.png";
my $DOWN_ICON       = "$URL_HOME/gfx/nav/green/down.png";
my $TOP_ICON        = "$URL_HOME/gfx/nav/green/top.png";
my $BOTTOM_ICON     = "$URL_HOME/gfx/nav/green/bottom.png";

my $MUDLIST_ICON    = "$URL_HOME/gfx/mud.png";
my $LOG_ICON        = "$URL_HOME/gfx/log.png";
my $DISCORD_ICON    = "$URL_HOME/gfx/discord.png";
my $SERVER_ICON     = "$URL_HOME/gfx/server_icon.png";
my $ICON_WIDTH      = 48;

my $MUDLIST_IMG     = "<img src=\"$MUDLIST_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $LOG_IMG         = "<img src=\"$LOG_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $DISCORD_IMG     = "<img src=\"$DISCORD_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $SERVER_IMG      = "<img src=\"$SERVER_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";

my $MUDLIST_LINK    = "<a href=\"$URL_HOME/mudlist.php\" alt=\"Mudlist\" title=\"Mudlist\">$MUDLIST_IMG</a>";
my $LOG_LINK        = "<a href=\"https://themud.org/chanhist.php#Channel=all\" alt=\"Other Logs\" title=\"Other Logs\">$LOG_IMG</a>";
my $DISCORD_LINK    = "<a href=\"https://discord.gg/kUduSsJ\" alt=\"Discord\" title=\"Discord\">$DISCORD_IMG</a>";
my $SERVER_LINK     = "<a href=\"$URL_HOME/server.php\" alt=\"Server\" title=\"Server\">$SERVER_IMG</a>";

my $OVERLAY_ICON    = "$URL_HOME/gfx/archive_stamp.png";
my $OVERLAY_IMG     = "<img class=\"overlay-fixed\" src=\"$OVERLAY_ICON\" />";

my $JQUI_CSS        = "$LOG_HOME/jquery/jquery-ui.css";
my $JQUI_THEME      = "$LOG_HOME/jquery/jquery-ui.theme.css";
my $JQ              = "$LOG_HOME/jquery.js";
my $JQUI            = "$LOG_HOME/jquery/jquery-ui.js";
my $MOMENT          = "$LOG_HOME/moment.js";
my $MOMENT_TZ       = "$LOG_HOME/moment-timezone.js";
my $NAVBAR          = "$LOG_HOME/navbar.js";

my $PINKFISH_CACHE  = "$PAGE_DIR/pinkfish.json";
my $CHANNEL_CACHE   = "$PAGE_DIR/channels.json";
my $SPEAKER_CACHE   = "$PAGE_DIR/speakers.json";
my $DATE_CACHE      = "$PAGE_DIR/date_counts.json";
my $HOUR_CACHE      = "$PAGE_DIR/hours.json";

my $page_size       = 100;
my $page_start      = 0;
my $page_limit      = 0;
my $overwrite       = 0;
my $do_speakers     = 1;
my $do_update       = 1;
my $do_navbar       = 1;
my $do_pages        = 1;
my $do_cache        = 1;
my $pause           = 1;
my $use_live        = 0;
my $debug_page      = 0;
my $do_json         = 1;
my $do_censor       = 0;

sub do_help {
    print STDERR <<EOM
usage:  $PROGRAM_NAME [-h]
long options:
    --help              - This helpful help!
    --start N           - Page to start from, defaults to $page_start.
                          Pages are numbered from 0 to N, however there is a
                          special case.  Passing in a negative number will
                          generate pages from the most recent minus the
                          number passed.  So, -2 would generate yesterday's
                          page and today's page.
    --limit N           - Number of pages to do, the default is ALL of them.
    --overwrite         - Force overwriting of pages, default is to skip.
    --pages             - Process pages as specified by start and limit.
                          The default is yes, but the option is provided
                          in case you wanted to call it with --no-pages to
                          ONLY update other things.
    --speakers          - Generate i3.speakers MUD file.  Default is yes.
    --navbar            - Generate navigation data.  Default is yes.
    --update            - Update speaker and channel color data.  Default is yes.
    --cache             - Save cached data to JSON files.  Default is yes.
    --pause             - Pause for 5 seconds before starting.  Default is yes.
    --live              - Use the live database for the most current data.  Default
                          is no.
                          Because SQLite doesn't handle locks well, if this is true,
                          the run will be limited to 10 pages at a time.
    --debug             - This changes the next-link button for the next to the last
                          page, so it points to another static page, rather than to
                          the live page.  The static page has a dark red background,
                          so you know it's not live data.  By default, this points
                          to the live page, as the END link always does.
    --json              - Also emit a JSON dump of the data for each page, to allow
                          for simple exporting of the data to others.  You can use
                          --json with --no-pages to only regenerate the JSON data.
    --censor            - Adds a blur effect to any messages on the free_speech
                          channel, in case you wanted to generate censored pages.
                          The default is no censorship.
EOM
    ;
    #--pagesize N        - Page size, defaults to $page_size.
    exit(1);
}

sub display_options {
    my $row_count = shift || 0;
    my $page_count = shift || 0;

    #printf "We are running against %s.\n", ($use_live ? $LIVE_DB_FILE : $DB_FILE);
    printf "We are running against PostgreSQL now!\n";
    printf "Found %d rows over %d pages.\n", $row_count, $page_count;
    if( $do_pages or $do_json ) {
        printf "Starting from page %d, as requested.\n", $page_start;
        if($page_limit) {
            printf "Stopping after %d pages\n", $page_limit;
        } else {
            printf "Doing all %d pages\n", ($page_start >= 0) ? ($page_count - $page_start) : (-$page_start);
        }
        printf "The next-to-the-last page will point to %s.\n", $debug_page ? "a static page" : "the live page";
        printf "We will %soverwrite existing files.\n", $overwrite ? "" : "NOT ";
        printf "We will %soutput HTML pages.\n", $do_pages ? "" : "NOT ";
        printf "We will %sexport JSON data for each page.\n", $do_json ? "" : "NOT ";
        printf "We will %scensor messages on free_speech.\n", $do_censor ? "" : "NOT ";
    } else {
        printf "We will NOT process pages.\n";
    }
    printf "We will %sgenerate navigation data.\n", $do_navbar ? "" : "NOT ";
    printf "We will %sgenerate I3 speaker data.\n", $do_speakers ? "" : "NOT ";
    printf "We will %supdate speaker and channel color data.\n", $do_update ? "" : "NOT ";
    printf "We will %ssave JSON cached data.\n", $do_cache ? "" : "NOT ";
    printf "We will %spause 5 seconds before starting%s\n", ($pause ? "" : "NOT "), ($pause ? "..............." : ".");
}

Getopt::Long::Configure("gnu_getopt");
Getopt::Long::Configure("auto_version");
GetOptions(
    'help|h'            => sub { do_help() },
    'start=i'           => \$page_start,
    'limit=i'           => \$page_limit,
    'overwrite!'        => \$overwrite,
    'speakers!'         => \$do_speakers,
    'navbar!'           => \$do_navbar,
    'update!'           => \$do_update,
    'pause!'            => \$pause,
    'live!'             => \$use_live,
    'debug!'            => \$debug_page,
    'pages!'            => \$do_pages,
    'cache!'            => \$do_cache,
    'json!'             => \$do_json,
    'censor!'           => \$do_censor,
);

sub open_postgres_db {
    my $DB_NAME = shift;

    my $db = DBI->connect("dbi:Pg:dbname=$DB_NAME", 'wiley', 'tardis69',
        { AutoCommit => 1, RaiseError => 1, PrintError => 0, });
    return $db;
}

sub open_postgres_tcp_db {
    my $DB_NAME = shift;

    my $db = DBI->connect("dbi:Pg:dbname=$DB_NAME;host=localhost;port=5432", 'wiley', 'tardis69',
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

sub import_compressed_json {
    my $filename = shift;

    local $/ = undef;
    open FP, "$UNKOMPRESSOR $filename|" or die "Cannot open data file $filename: $!";
    my $data = <FP>;
    close FP;
    my $object = decode_json($data);
    return $object;
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
}

sub update_all_channels {
    my $db = shift;

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

sub fetch_current_date {
    my $db = shift;

    my $date = undef;
    my $rv = $db->selectrow_arrayref("SELECT date('now') the_date;");
    print STDERR $DBI::errstr."\n" if ! $rv;
    die "Cannot obtain a valid date???: $!" if !defined $rv;
    return $rv ? $rv->[0] : undef;
}

# ALTER TABLE i3log RENAME TO i3bkp;
# CREATE TABLE i3log (
#             created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')),
#             local DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','localtime')),
#             is_emote INTEGER,
#             is_url INTEGER,
#             is_bot INTEGER,
#             channel TEXT,
#             speaker TEXT,
#             mud TEXT,
#             message TEXT
#         );
# INSERT INTO i3log SELECT created, STRFTIME('%Y-%m-%d %H:%M:%f', 
#     STRFTIME('%Y-%m-%d %H:%M:%f', created, 'localtime'), 'localtime')
#     AS local, is_emote, is_url, is_bot, channel, speaker, mud, message FROM i3bkp;
# CREATE INDEX ix_local ON i3log(local);
#
# This routine should never have anything to do anymore, but there's no real harmn in
# letting it run... if the work is 0 rows, the time is minimal.
#
# In case you're wondering, this is to work around a bug I didn't realize happened.
# For whatever reason,, the default of UTC fails in SQLite and actually generates
# timestamps that are 16 hours shifted, rather than the 8 they should be.
#
# Unfortunately, some of the rows will have been shifted 10 hours when I was living
# in GMT-5, and 16 hours when I moved to GMT-8.  Oh well...
#
sub fetch_page_by_date {
    my $db = shift;
    my $date = shift;

    $date = fetch_current_date($db) if !defined $date;

    my $rv = $db->selectall_arrayref(qq!
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
                 ON (lower(i3log.username) = speakers.speaker)
          LEFT JOIN pinkfish_map pinkfish_map_hour
                 ON (hours.pinkfish = pinkfish_map_hour.pinkfish)
          LEFT JOIN pinkfish_map pinkfish_map_channel
                 ON (channels.pinkfish = pinkfish_map_channel.pinkfish)
          LEFT JOIN pinkfish_map pinkfish_map_speaker
                 ON (speakers.pinkfish = pinkfish_map_speaker.pinkfish)
              WHERE date(i3log.local) = ?
           ORDER BY i3log.local ASC;
        ;!, { Slice => {} }, ($date) );

    print STDERR $DBI::errstr."\n" if ! $rv;
    return $rv;
}

sub page_url {
    my $row = shift;

    my $pathname = sprintf "%s/%s/%s/%s.html", $LOG_HOME,
        $row->{the_year}, $row->{the_month},
        $row->{the_date};

    return $pathname;
}

sub page_path {
    my $row = shift;

    my $dir_path = sprintf "%s/%s", $PAGE_DIR, $row->{the_year};
    mkdir $dir_path if ! -d $dir_path;
    $dir_path = sprintf "%s/%s/%s", $PAGE_DIR, $row->{the_year}, $row->{the_month};
    mkdir $dir_path if ! -d $dir_path;

    my $pathname = sprintf "%s/%s/%s/%s.html", $PAGE_DIR,
        $row->{the_year}, $row->{the_month},
        $row->{the_date};

    return $pathname;
}

sub json_path {
    my $row = shift;

    my $index_file      = "$JSON_DIR/index.php";
    my $dir_path        = sprintf "%s/%s", $JSON_DIR, $row->{the_year};
    my $symlink_target  = sprintf "%s/%s/index.php", $JSON_DIR, $row->{the_year};
    mkdir $dir_path if ! -d $dir_path;
    symlink $index_file, $symlink_target if ! -e $symlink_target;

    $dir_path           = sprintf "%s/%s/%s", $JSON_DIR, $row->{the_year}, $row->{the_month};
    $symlink_target     = sprintf "%s/%s/%s/index.php", $JSON_DIR, $row->{the_year}, $row->{the_month};
    mkdir $dir_path if ! -d $dir_path;
    symlink $index_file, $symlink_target if ! -e $symlink_target;

    my $pathname = sprintf "%s/%s/%s/%s.json", $JSON_DIR,
        $row->{the_year}, $row->{the_month},
        $row->{the_date};

    return $pathname;
}

sub generate_navbar_script {
    my $db = shift;
    my $date_counts = shift;
    die "Invalid date information!" if !defined $date_counts;

    my $filename = "$PAGE_DIR/navbar.js";

    my @date_list = ();
    foreach my $row (@$date_counts) {
        push @date_list, ($row->{the_date}) if -f page_path($row);
    }
    my $big_list = join(",\n", map { sprintf "\"%s\"", $_; } (@date_list));
    my $last_date = (scalar @date_list < 1) ? fetch_current_date($db) : $date_list[-1];

    open FP, ">$filename" or die "Cannot open navbar script $filename: $!";
    print FP <<EOM
        var valid_dates = [
            $big_list
        ];
        var bits = window.location.href.toString().split('/');
        var my_date = bits[bits.length-1].substr(0,10);

        function checkDate(date) {
            var m = date.getMonth();
            var d = date.getDate();
            var y = date.getFullYear();
             
            // First convert the date in to the yyyy-mm-dd format 
            // Take note that we will increment the month count by 1 
            var candidate_date = ("0000" + y).slice(-4) + '-' 
                + ("00" + (m + 1)).slice(-2) + '-' 
                + ("00" + d).slice(-2);
             
            if (\$.inArray(candidate_date, valid_dates) != -1 ) {
                return [true];
            }
            return [false];
        }

        function gotoNewPage(dateString) {
            if( dateString == "$last_date" ) {
                window.location.href = "$LIVE_PAGE";
            } else {
                var url_bits = window.location.href.toString().split('/');
                var logpages = url_bits.indexOf("logpages");
                var new_bits = [];

                if(logpages >= 0) {
                    var i;
                    for(i = 0; i <= logpages; i++) {
                        new_bits[i] = url_bits[i];
                    }
                    new_bits[new_bits.length] = dateString.substr(0,4);
                    new_bits[new_bits.length] = dateString.substr(5,2);
                    new_bits[new_bits.length] = dateString + '.html';
                    window.location.href = new_bits.join('/');
                } else {
                    // We can't find it, so use the old code and pray
                    url_bits[url_bits.length - 3] = dateString.substr(0,4);
                    url_bits[url_bits.length - 2] = dateString.substr(5,2);
                    url_bits[url_bits.length - 1] = dateString + '.html';
                    window.location.href = url_bits.join('/');
                }
            }
        }

        function toggleDiv(divID) {
            if(document.getElementById(divID).style.display == 'none') {
                document.getElementById(divID).style.display = 'block';
            } else {
                document.getElementById(divID).style.display = 'none';
            }
        }

        function toggle_row( row ) {
            r = document.getElementById("row_" + row);
            if(r !== null) {
                if(r.style.display !== "none") {
                    r.style.display = "none";
                } else {
                    r.style.display = "table-row";
                }
            }
        }

        function row_on( row ) {
            r = document.getElementById("row_" + row);
            if(r !== null) {
                if(r.style.display == "none") {
                    r.style.display = "table-row";
                }
            }
        }

        function row_off( row ) {
            r = document.getElementById("row_" + row);
            if(r !== null) {
                if(r.style.display !== "none") {
                    r.style.display = "none";
                }
            }
        }

        function row_visible( row ) {
            r = document.getElementById("row_" + row);
            if(r !== null) {
                if(r.style.display !== "none") {
                    var bounds = r.getBoundingClientRect();
                    return bounds.top < window.innerHeight && bounds.bottom > 0;
                }
            }
            return false;
        }

        function jump_to_row( row ) {
            r = document.getElementById("row_" + row);
            if(r !== null) {
                if(r.style.display !== "none") {
                    // NOTE: \$("#row_" + row + ":in-viewport")
                    // \$(":below-the-fold") \$(":above-the-top")
                    // \$(":left-of-screen") \$(":right-of-screen")
                    //window.scrollTo(r.offsetLeft, r.offsetTop);
                    r.scrollIntoView();
                }
            }
        }

        function count_rows_in_view() {
            count = document.getElementById("content").getElementsByTagName("tr").length;
            row = 0;
            z = document.getElementById("row_" + row);
            if(z !== null) {
                var i = 1;
                for(i = 1; i < count; i++) {
                    if(row_visible(i) == false) {
                        break;
                    }
                }
                return i;
            }
            return 0;
        }

        function dim(element) {
            if(element !== null) {
                element.style.opacity = "0.5";
            }
        }

        function brighten(element) {
            if(element !== null) {
                element.style.opacity = "1.0";
            }
        }

        function on_scroll() {
            var body = document.body;
            var html = document.documentElement;
            var bd = document.getElementById("scroll_down_button");
            var bb = document.getElementById("scroll_bottom_button");
            var bu = document.getElementById("scroll_up_button");
            var bt = document.getElementById("scroll_top_button");
            var doc_height = Math.max( body.scrollHeight, body.offsetHeight,
                html.clientHeight, html.scrollHeight, html.offsetHeight );

            if( window.innerHeight >= doc_height ) {
                // The page fits entirely on the screen, no scrolling possible.
                dim(bd);
                dim(bb);
                dim(bu);
                dim(bt);
            } else if( (window.innerHeight + window.pageYOffset) >=
                (document.body.offsetHeight) ) {
                // We are at the bottom of the page.
                dim(bd);
                dim(bb);
                brighten(bu);
                brighten(bt);
            } else if( window.pageYOffset <= 1 ) {
                // We are at the top of the page.
                brighten(bd);
                brighten(bb);
                dim(bu);
                dim(bt);
            } else {
                // We're somewhere in the middle.
                brighten(bd);
                brighten(bb);
                brighten(bu);
                brighten(bt);
            }
        }

        function scroll_up() {
            window.scrollBy(0, -Math.floor(window.innerHeight * 0.90));
        }

        function scroll_down() {
            window.scrollBy(0, Math.floor(window.innerHeight * 0.90));
        }

        function scroll_top() {
            window.scrollTo(0,0);
        }

        function scroll_bottom() {
            window.scrollBy(0, document.body.offsetHeight);
        }

        function setup_rows() {
            count = document.getElementById("content").getElementsByTagName("tr").length;
            for(var i = 0; i < count; i++) {
                row_on(i);
            }
            on_scroll(); // Call once, in case the page cannot scroll
            \$(window).on("scroll", function() {
                    on_scroll();
            });
        }

        \$( function() {
            \$( "#datepicker" ).datepicker({
                beforeShowDay   : checkDate,
                onSelect        : gotoNewPage,
                changeYear      : true,
                changeMonth     : true,
                dateFormat      : "yy-mm-dd",
                defaultDate     : my_date,
             });
        });
EOM
    ;
    close FP;
}

print "Initializing...\n";

my $DATABASE = open_postgres_db($PG_DB);

my $row_count = fetch_row_count($DATABASE);
my $date_counts = fetch_date_counts($DATABASE);

if( $page_start < 0 ) {
    $page_start = (scalar @$date_counts) + $page_start;
}

display_options($row_count, scalar @$date_counts);
if( $pause ) {
    sleep 1;
    print "4............\n";
    sleep 1;
    print "3.........\n";
    sleep 1;
    print "2......\n";
    sleep 1;
    print "1...\n";
    sleep 1;
    print "GO!\n\n";
} else {
    print "\n";
}

my $pinkfish_map = fetch_pinkfish_map($DATABASE);
my $hours = fetch_hours($DATABASE);

if( $do_update ) {
    print "Updating speaker and channel info...\n";
    update_all_channels($DATABASE);
    update_all_speakers($DATABASE);
}

my $channels = fetch_channels($DATABASE);
my $speakers = fetch_speakers($DATABASE);

if( $do_cache ) {
    print "Saving cached versions of SQL data...\n";
    save_date_cache($date_counts, $DATE_CACHE);
    save_json_cache($pinkfish_map, $PINKFISH_CACHE);
    save_json_cache($hours, $HOUR_CACHE);
    save_json_cache($channels, $CHANNEL_CACHE);
    save_json_cache($speakers, $SPEAKER_CACHE);
}

if( $do_navbar ) {
    print "Generating navigation data...\n";
    generate_navbar_script($DATABASE, $date_counts);
}

if( $do_speakers ) {
    print "Dumping I3 speaker file for WileyMUD...\n";

    my $filename = sprintf "%s/i3.speakers.new", $PAGE_DIR;
    open FP, ">$filename" or die "Cannot open speaker file $filename: $!";

    printf FP "#COUNT\nCount   %d\nEnd\n\n", scalar keys %$speakers;
    foreach my $row (map { $speakers->{$_} } (sort keys %$speakers)) {
        printf FP "#SPEAKER\nName    %s\nColor   %s\nEnd\n\n", $row->{speaker}, $row->{pinkfish};
    }
    printf FP "#END\n";

    close FP;
}

if( $do_pages or $do_json ) {
    $page_limit = (scalar @$date_counts) if $page_limit < 1;
    #$page_limit = 30 if $use_live and $page_limit > 30;

    my $pages_todo = (scalar @$date_counts) - $page_start;
    $pages_todo = $page_limit if $page_limit < $pages_todo;

    my $pages_done = 0;
    #foreach my $day_row (@$date_counts)
    for( my $i = $page_start; $i < scalar @$date_counts; $i++ ) {
        my $day_row = $date_counts->[$i];
        my $today = $day_row->{the_date};

        last if $pages_done >= $page_limit or $pages_done >= scalar @$date_counts;
        $pages_done++;

        my $filename = page_path($day_row);

        if( $i < ((scalar @$date_counts) - 2) and -f $filename ) {
            if( !$overwrite ) {
                # Always generate the very last, and next to last page again.
                printf "            Skipping %s.\n", $filename;
                next;
            }
        }

        my $first_row   = undef;
        my $first_page  = undef;
        my $first_date  = undef;
        my $prev_row    = undef;
        my $prev_page   = undef;
        my $prev_date   = undef;
        my $next_row    = undef;
        my $next_page   = undef;
        my $next_date   = undef;
        my $last_row    = undef;
        my $last_page   = undef;
        my $last_date   = undef;
        my $this_is_the_end = ($i == (scalar @$date_counts) - 1) ? 1 : undef;

        if( $i > 0 ) {
            # We have a previous page.
            $prev_row  = $date_counts->[$i - 1];
            $prev_date = $prev_row->{the_date};
            $prev_page = page_url($prev_row);

            # We also are not ON the first page, so we should have that too.
            $first_row  = $date_counts->[0];
            $first_date = $first_row->{the_date};
            $first_page = page_url($first_row);
        }
        if( $i < (scalar @$date_counts) - 2 ) {
            # We have a next page.
            $next_row  = $date_counts->[$i + 1];
            $next_date = $next_row->{the_date};
            $next_page = page_url($next_row);
        } elsif( $i < (scalar @$date_counts) - 1 ) {
            if( $debug_page ) {
                # We want the static page.
                $next_row  = $date_counts->[$i + 1];
                $next_date = $next_row->{the_date};
                $next_page = page_url($next_row);
            } else {
                # We want the live page.
                $next_row  = $date_counts->[$i + 1];
                $next_date = "LIVE " . $next_row->{the_date};
                $next_page = $LIVE_PAGE;
            }
        }

        # We ALWAYS have a last page, as that's the current page.  But,
        # this one is special.  Instead of the usual static page, we want
        # to point you to the live page that gets updated on load.
        $last_row  = $date_counts->[-1];
        $last_date = $this_is_the_end ? "LIVE" : ("LIVE " . $last_row->{the_date});
        $last_page = $LIVE_PAGE;

        #my $page_background = $this_is_the_end ? "#1F0000" : "black";
        my $page_background = $this_is_the_end ? "#070707" : "black";
        my $overlay_image = $this_is_the_end ? $OVERLAY_IMG : "";

        my $old_dir = getcwd;
        chdir $BACKGROUND_DIR;
        my $BACKGROUND = random_file(
            -dir    => '.',
            -check  => qr/\.(jpg|png)$/,
        );
        chdir $old_dir;
        #print "RANDOM BACKGROUND: $BACKGROUND\n";
        my $BACKGROUND_IMG = "<img class=\"overlay-bg\" src=\"$URL_HOME/gfx/wallpaper/$BACKGROUND\" />";
        my $background_image = $BACKGROUND_IMG;

        my $page = fetch_page_by_date($DATABASE, $today);
        die "No data for $today? $!" if !defined $page;
        printf "    [%5d] Generating %s... %d messages.\n",
            ($pages_todo - $pages_done), $today, scalar @$page;

        if ($do_pages) {
            open FP, ">$filename" or die "Cannot open output page $filename: $!";

            print FP <<EOM
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <!-- Global site tag (gtag.js) - Google Analytics -->
        <script async src="https://www.googletagmanager.com/gtag/js?id=UA-163395867-1"></script>
        <script>
          window.dataLayer = window.dataLayer || [];
          function gtag(){dataLayer.push(arguments);}
          gtag('js', new Date());

          gtag('config', 'UA-163395867-1');
        </script>
        <link rel="stylesheet" href="$JQUI_CSS">
        <link rel="stylesheet" href="$JQUI_THEME">
        <script src="$JQ"></script>
        <script src="$JQUI"></script>
        <script src="$MOMENT"></script>
        <script src="$MOMENT_TZ"></script>
        <script src="$NAVBAR"></script>

        <script type="text/javascript">
            function localize_rows() {
		// 0 -> 23
                var hour_map = [
		    '#555555',
		    '#555555',
		    '#555555',
		    '#555555',
		    '#bb0000',
		    '#bb0000',
		    '#bbbb00',
		    '#bbbb00',
		    '#ffff55',
		    '#ffff55',
		    '#00bb00',
		    '#00bb00',
		    '#55ff55',
		    '#55ff55',
		    '#bbbbbb',
		    '#bbbbbb',
		    '#55ffff',
		    '#55ffff',
		    '#00bbbb',
		    '#00bbbb',
		    '#5555ff',
		    '#5555ff',
		    '#0000bb',
		    '#0000bb'
                ];

                var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
                var yourLocale = (navigator.languages && navigator.languages.length) ?
                    navigator.languages[0] : navigator.language;

                var rows = \$(`[id^=row_]`);
                for( var i = 0; i < rows.length-1; i++ ) {
                    var tds = rows[i].getElementsByTagName("td");
                    var oldDate = tds[0].innerHTML;
                    var tdspan = tds[1].getElementsByTagName("span");
                    var oldTime = tdspan[0].innerHTML;

                    var oldMoment = moment.tz(oldDate + " " + oldTime, "America/Los_Angeles");
                    var newMoment = oldMoment.clone().tz(yourTimeZone);
                    //var newMoment = oldMoment.clone().tz("Asia/Tokyo");
                    var newDate = newMoment.format('YYYY-MM-DD');
                    var newTime = newMoment.format('HH:mm:ss');
                    var newHour = newMoment.hour();

                    tds[0].innerHTML = newDate;
                    tdspan[0].innerHTML = newTime;
                    tdspan[0].style.color = hour_map[newHour];
                }
            }

            function setup() {
                setup_rows();
                localize_rows();
                // This if for the live page ONLY, pointless for static pages.
                //scroll_bottom();
                //setTimeout(function () { location.reload(true); }, 30 * 60 * 1000);
            }
        </script>
        <style>
            html, body { table-layout: fixed; max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            table { table-layout: fixed; max-width: 99%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
            a:active, a:focus { outline: 0; border: none; -moz-outline-style: none; }
            input, select, textarea { border-color: #101010; background-color: #101010; color: #d0d0d0; }
            input:focus, textarea:focus { border-color: #101010; background-color: #303030; color: #f0f0f0; }
            #navbar { position: fixed; top: 0; height: 58px; background-color: $page_background; }
            #content-header { position: fixed; top: 58px; width: 100%; background-color: $page_background; }
            #content { padding-top: 48px; }
            .overlay-fixed { position: fixed; top: 48px; left: 0px; width: 100%; height: 100%; z-index: 999; opacity: 0.3; pointer-events: none; }
            .overlay-bg { position: fixed; top: 81px; z-index: 998; opacity: 0.15; pointer-events: none; object-fit: cover; width: 100%; height: 100%; left: 50%; transform: translateX(-50%); }
            .unblurred { font-family: monospace; white-space: pre-wrap; }
            .blurry:not(:hover) { filter: blur(3px); font-family: monospace; white-space: pre-wrap; }
            .blurry:hover { font-family: monospace; white-space: pre-wrap; }
        </style>
    </head>
    <body bgcolor="$page_background" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="setup();">
        $background_image
        $overlay_image
        <table id="navbar" width="99%" align="center">
            <tr>
                <td align="left" width="20%">
                    $MUDLIST_LINK
                    $LOG_LINK
                    $DISCORD_LINK
                </td>
                <td align="right" width="20%">
EOM
            ;

            #printf FP "<img id=\"scroll_top_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" onclick=\"scroll_top()\"/>\n", $TOP_ICON, $ICON_WIDTH, $ICON_WIDTH;
            printf FP "<img id=\"scroll_up_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" onclick=\"scroll_up()\"/>\n", $UP_ICON, $ICON_WIDTH, $ICON_WIDTH;

            if(defined $first_page) {
                printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" /></a>\n", $first_page, $first_date, $first_date, $BEGIN_ICON, $ICON_WIDTH, $ICON_WIDTH;
            } else {
                printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" />\n", $BEGIN_ICON, $ICON_WIDTH, $ICON_WIDTH;
            }

            if(defined $prev_page) {
                printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" /></a>\n", $prev_page, $prev_date, $prev_date, $PREV_ICON, $ICON_WIDTH, $ICON_WIDTH;
            } else {
                printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" />\n", $PREV_ICON, $ICON_WIDTH, $ICON_WIDTH;
            }

            print FP <<EOM
                </td>
                <td align="center" width="10%">
                    <input type="text" id="datepicker" size="10" value="$today" style="font-size: 16px; text-align: center;" />
                </td>
                <td align="left" width="20%">
EOM
            ;

            if(defined $next_page) {
                printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" /></a>\n", $next_page, $next_date, $next_date, $NEXT_ICON, $ICON_WIDTH, $ICON_WIDTH;
            } else {
                printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" />\n", $NEXT_ICON, $ICON_WIDTH, $ICON_WIDTH;
            }

            if(defined $last_page) {
                printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" /></a>\n", $last_page, $last_date, $last_date, $END_ICON, $ICON_WIDTH, $ICON_WIDTH;
            } else {
                printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" />\n", $END_ICON, $ICON_WIDTH, $ICON_WIDTH;
            }

            printf FP "<img id=\"scroll_down_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 1.0;\" onclick=\"scroll_down()\"/>\n", $DOWN_ICON, $ICON_WIDTH, $ICON_WIDTH;
            #printf FP "<img id=\"scroll_bottom_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 1.0;\" onclick=\"scroll_bottom()\"/>\n", $BOTTOM_ICON, $ICON_WIDTH, $ICON_WIDTH;

            print FP <<EOM
                </td>
                <td align="right" width="20%">
                    $SERVER_LINK
                </td>
            </tr>
        </table>
        <table id="content-header-outside" width="99%" align="center">
            <tr id="content-header">
                <td id="dateheader" align="left" width="80px" style="color: #DDDDDD; min-width: 80px;">Date</td>
                <td id="timeheader" align="left" width="60px" style="color: #DDDDDD; min-width: 40px;">Time</td>
                <td id="channelheader" align="left" width="80px" style="color: #DDDDDD; min-width: 100px;">Channel</td>
                <td id="speakerheader" align="left" width="200px" style="color: #DDDDDD; min-width: 200px;">Speaker</td>
                <td align="left">&nbsp;</td>
            </tr>
        </table>
        <table id="content" width="99%" align="center">
            <thead>
            <tr>
                <th id="dateheader" align="left" width="80px" style="color: #DDDDDD; min-width: 80px;">Date</th>
                <th id="timeheader" align="left" width="60px" style="color: #DDDDDD; min-width: 40px;">Time</th>
                <th id="channelheader" align="left" width="80px" style="color: #DDDDDD; min-width: 100px;">Channel</th>
                <th id="speakerheader" align="left" width="200px" style="color: #DDDDDD; min-width: 200px;">Speaker</th>
                <th align="left">&nbsp;</th>
            </tr>
            </thead>
            <tbody>
EOM
            ;

            my $counter = 0;
            foreach my $row (@$page) {
                # YYYY-MM-DD date
                # HH:MM:SS time -- colored by time of day
                # Channel -- colored by channel name
                # Speaker@Mud -- colored by speaker name
                # Message -- fixed font

                # Emit each data row as a table row
                my $bg_color = ($counter % 2) ? "#000000" : "#1F1F1F";

                my $hour_html = $row->{hour_html} || "--**--NULL--**--";
                my $channel_html = $row->{channel_html} || $channels->{default}{html} || "--**--NULL--**--";
                my $speaker_html = $row->{speaker_html} || $speakers->{default}{html} || "--**--NULL--**--";
                my $channel = $row->{channel}; # text only channel name

                printf FP "<tr id=\"row_%d\" style=\"display:none\">\n", $counter;
                printf FP "<td bgcolor=\"%s\">%s</td>\n", $bg_color, $row->{the_date};
                printf FP "<td bgcolor=\"%s\">%s%s%s</td>\n", $bg_color, $hour_html, $row->{the_time}, "</span>";
                printf FP "<td bgcolor=\"%s\">%s%s%s</td>\n", $bg_color, $channel_html, $row->{channel}, "</span>";
                printf FP "<td bgcolor=\"%s\">%s%s@%s%s</td>\n", $bg_color, $speaker_html, $row->{speaker}, $row->{mud}, "</span>";

                my $message = $row->{message};
                $message = "" if !defined $message;
                $message =~ s/^\%\^RESET\%\^//i;
                # encode_entities
                $message = encode_entities($message, '<>&"');

                # Filter known pinkfish codes and make them HTML
                if( $message =~ /\%\^/gsmx ) {
                    #printf STDERR "        Message has pinkfish codes: %s\n", $message;

                    foreach my $pf_key (sort { length $b <=> length $a } keys %$pinkfish_map) {
                        my $quoted = quotemeta $pf_key;
                        my $regex = qr/$quoted/;
                        my $pf_rep = $pinkfish_map->{$pf_key}{html};

                        #printf STDERR "Checking %s (%s) ...\n", $pf_key, $quoted;

                        if ( $message =~ /$regex/gsmx ) {
                            #printf STDERR "Found match for %s\n", $quoted;
                            $message =~ s/$regex/$pf_rep/gsmx;
                        }
                    }
                }

                # Try to match links so we can make them clickable
                $message =~ s/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/<a href="$1" target="I3-link">$1<\/a>/gsmix;
                $message =~ s/YouTube\s+(<span.*?>)\s*\[([^\]]*)\]/YouTube $1 <a href="https:\/\/youtu.be\/$2" target="I3-link">[$2]<\/a>/gsmix;
                $message =~ s/IMDB\s+(<span.*?>)\s*\[([^\]]*)\]/IMDB $1 <a href="https:\/\/www.imdb.com\/title\/$2\/" target="I3-link">[$2]<\/a>/gsmix;
                $message =~ s/Steam\s+(<span.*?>)\s*\[([^\]]*)\]/Steam $1 <a href="http:\/\/store.steampowered.com\/app\/$2\/" target="I3-link">[$2]<\/a>/gsmix;
                $message =~ s/Dailymotion\s+(<span.*?>)\s*\[([^\]]*)\]/Dailymotion $1 <a href="https:\/\/www.dailymotion.com\/video\/$2" target="I3-link">[$2]<\/a>/gsmix;

                my $span_style = "font-family: monospace; white-space: pre-wrap;";
                my $span_class = "unblurred";
                $span_class = "blurry" if $do_censor and $channel eq "free_speech";
                $span_class = "blurry" if $do_censor and $message =~ /on\sfree_speech\&gt\;/;

                printf FP "<td bgcolor=\"%s\"><span class=\"$span_class\">%s</span></td>\n",  $bg_color, $message;
                printf FP "</tr>\n";

                $counter++;
            }
            print FP <<EOM
            </tbody>
        </table>
    </body>
</html>
EOM
            ;
            close FP;
        }

        # Do JSON here
        next if ! $do_json;

        $filename = json_path($day_row);
        if( $i < ((scalar @$date_counts) - 2) and -f $filename ) {
            if( !$overwrite ) {
                # Always generate the very last, and next to last page again.
                next;
            }
        }
        #my $json_dump = encode_json($page);
        my $json_dump = JSON->new->utf8->allow_nonref->canonical->pretty->encode($page);
        #open FP, "|$KOMPRESSOR $KOMPRESSOR_ARGS >$filename.$KOMPRESSOR_EXT" or die "Cannot open dump page $filename.$KOMPRESSOR_EXT: $!";
        open FP, ">$filename" or die "Cannot open dump page $filename: $!";
        print FP "$json_dump\n";
        close FP;
    }
}

if( $do_navbar ) {
    print "Regenerating navigation data...\n";
    generate_navbar_script($DATABASE, $date_counts);
}

print "Done!\n";
exit 0;
