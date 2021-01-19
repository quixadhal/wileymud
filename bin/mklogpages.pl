#!/usr/bin/perl -w

use utf8;
use strict;
use Encode qw(encode_utf8);
no warnings 'utf8';
use English qw( −no_match_vars );
use Data::Dumper;
use DBI;
use HTML::Entities;
use Getopt::Long;
use JSON qw(encode_json);
use File::Random qw(random_file);
use Cwd qw(getcwd);
use Parallel::ForkManager 0.7.6;
use Time::HiRes qw(time);

my $URL_HOME        = "http://wileymud.themud.org/~wiley";
my $LOG_HOME        = "$URL_HOME/logpages";
my $LIVE_PAGE       = "$LOG_HOME/";

#my $LIVE_DB_FILE    = '/home/wiley/lib/i3/wiley.db';
#my $DB_FILE         = '/home/wiley/lib/i3/wiley.bkp-20190223.db';
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
my $QR_ICON         = "$URL_HOME/stuff/qrcode.png";
my $QR_BIG_ICON     = "$URL_HOME/stuff/qrcode_big.png";
my $PIE_ICON        = "$URL_HOME/gfx/pie_chart.png";
my $GITHUB_ICON     = "$URL_HOME/gfx/github1600.png";
my $ICON_WIDTH      = 48;

my $MUDLIST_IMG     = "<img class=\"glowing\" src=\"$MUDLIST_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $LOG_IMG         = "<img class=\"glowing\" src=\"$LOG_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $DISCORD_IMG     = "<img class=\"glowing\" src=\"$DISCORD_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $SERVER_IMG      = "<img class=\"glowing\" src=\"$SERVER_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $QR_IMG          = "<img src=\"$QR_ICON\" width=\"53\" height=\"53\" border=\"0\" />";
my $PIE_IMG         = "<img class=\"glowing\" src=\"$PIE_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
my $GITHUB_IMG      = "<img class=\"glowing\" src=\"$GITHUB_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";

my $MUDLIST_LINK    = "<a href=\"$URL_HOME/mudlist.php\" alt=\"Mudlist\" title=\"Mudlist\">$MUDLIST_IMG</a>";
my $LOG_LINK        = "<a href=\"$LOG_HOME\" alt=\"Logs\" title=\"Logs\">$LOG_IMG</a>";
my $DISCORD_LINK    = "<a href=\"https://discord.gg/kUduSsJ\" alt=\"Discord\" title=\"Discord\">$DISCORD_IMG</a>";
my $SERVER_LINK     = "<a href=\"$URL_HOME/server.php\" alt=\"Server\" title=\"Server\">$SERVER_IMG</a>";
my $QR_LINK         = "<a href=\"$QR_BIG_ICON\" alt=\"リック転がし\" title=\"リック転がし\">$QR_IMG</a>";
my $PIE_LINK        = "<a href=\"$URL_HOME/pie.php\" alt=\"PIE!\" title=\"PIE!\">$PIE_IMG</a>";
my $GITHUB_LINK     = "<a href=\"https://github.com/quixadhal/wileymud\" alt=\"Source\" title=\"Source\">$GITHUB_IMG</a>";

my $OVERLAY_ICON    = "$URL_HOME/gfx/archive_stamp.png";
my $OVERLAY_IMG     = "<img class=\"overlay-fixed\" src=\"$OVERLAY_ICON\" />";

my $JQUI_CSS        = "$LOG_HOME/jquery/jquery-ui.css";
my $JQUI_THEME      = "$LOG_HOME/jquery/jquery-ui.theme.css";
my $JQ              = "$LOG_HOME/jquery.js";
my $JQUI            = "$LOG_HOME/jquery/jquery-ui.js";
my $MOMENT          = "$LOG_HOME/moment.js";
my $MOMENT_TZ       = "$LOG_HOME/moment-timezone.js";
my $NAVBAR          = "$LOG_HOME/navbar.js";
my $LOGPAGE_FUNC    = "$LOG_HOME/logpage_func.js";
my $LOGPAGE_STYLE   = "$LOG_HOME/logpage.css";

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
my $do_dates        = 1;
my $do_cache        = 1;
my $pause           = 1;
my $use_live        = 0;
my $debug_page      = 0;
my $do_json         = 1;
my $do_censor       = 0;
my $thread_count    = 0;
my $partition_size  = 200;
my $dark_mode       = 1;

my %COLORS          = ();

sub do_help {
    local $| = 1;

    print STDERR <<EOM;
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
    --dates             - Save SQL date information to cache file.  Default is yes.
    --cache             - Save cached data to JSON files.  Default is yes.
    --pause             - Pause for 5 seconds before starting.  Default is yes.
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
    --threads N         - To attempt to optimize performance, we support threading
                          now, and you can specify how many threads to use.  If you
                          give a value less and 2, thrading is not used.  The default
                          is 0.
    --partition N       - If using threads, we allow you to set the parition size
                          used when doing a query and sorting the results, since
                          pulling back all 5000 entries seems to be a bit slower
                          than not using threads at all.
    --dark              - Dark mode is the normal color scheme.  If you wanted
                          blinding white backgrounds, you can specify !dark.
EOM
#    --live              - Use the live database for the most current data.  Default
#                          is no.
#                          Because SQLite doesn't handle locks well, if this is true,
#                          the run will be limited to 10 pages at a time.
#    --pagesize N        - Page size, defaults to $page_size.
    exit(1);
}

sub display_options {
    my $row_count = shift || 0;
    my $page_count = shift || 0;

    local $| = 1;
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
        printf "We will generate %s pages.\n", $dark_mode ? "dark" : "light ";
        printf "We will %s threads.\n", $thread_count > 1 ? (sprintf "use %d", $thread_count) : "NOT use";
        if( $thread_count > 1 ) {
            printf "We will partition our results into %d day chunks.\n", $partition_size;
        } else {
            printf "We will NOT partition our results.\n";
        }
    } else {
        printf "We will NOT process pages.\n";
    }
    printf "We will %sgenerate navigation data.\n", $do_navbar ? "" : "NOT ";
    printf "We will %sgenerate I3 speaker data.\n", $do_speakers ? "" : "NOT ";
    printf "We will %supdate speaker and channel color data.\n", $do_update ? "" : "NOT ";
    printf "We will %ssave SQL cached date information.\n", $do_dates ? "" : "NOT ";
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
    'dates!'            => \$do_dates,
    'cache!'            => \$do_cache,
    'json!'             => \$do_json,
    'censor!'           => \$do_censor,
    'threads=i'         => \$thread_count,
    'partition=i'       => \$partition_size,
    'dark!'             => \$dark_mode,
);

if ($dark_mode) {
    %COLORS = (
        page_background             => "black",
        page_text                   => "#D0D0D0",
        page_link                   => "#FFFFBF",
        page_vlink                  => "#FFA040",
        odd_row                     => "black",
        even_row                    => "#1F1F1F",
        faint_text                  => "#1F1F1F",
        header_text                 => "#DDDDDD",
        input                       => "#d0d0d0",
        input_border                => "#101010",
        input_background            => "#101010",
        selected_input              => "#f0f0f0",
        selected_input_border       => "#101010",
        selected_input_background   => "#303030",
    );
} else {
    %COLORS = (
        page_background             => "white",
        page_text                   => "#303030",
        page_link                   => "#000040",
        page_vlink                  => "#0050C0",
        odd_row                     => "white",
        even_row                    => "#E0E0E0",
        header_text                 => "#222222",
        input                       => "#d0d0d0",
        input_border                => "#101010",
        input_background            => "#101010",
        selected_input              => "#f0f0f0",
        selected_input_border       => "#101010",
        selected_input_background   => "#303030",
    );
}

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

sub update_date_cache {
    my $date_counts = shift or die "No date_count data provided.";

    local $| = 1;
    print "Saving cached versions of SQL date information...";
    save_date_cache($date_counts, $DATE_CACHE);
    print "done.\n";
}

sub update_cache {
    my $pinkfish_map = shift or die "No pinkfish_map data provided.";
    my $hours = shift or die "No hours data provided.";
    my $channels = shift or die "No channels data provided.";
    my $speakers = shift or die "No speakers data provided.";

    local $| = 1;
    print "Saving cached versions of SQL data...";
    save_json_cache($pinkfish_map, $PINKFISH_CACHE);
    save_json_cache($hours, $HOUR_CACHE);
    save_json_cache($channels, $CHANNEL_CACHE);
    save_json_cache($speakers, $SPEAKER_CACHE);
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

sub fetch_current_date {
    my $db = shift;

    my $date = undef;
    my $rv = $db->selectrow_arrayref("SELECT date('now') the_date;");
    print STDERR $DBI::errstr."\n" if ! $rv;
    die "Cannot obtain a valid date???: $!" if !defined $rv;
    return $rv ? $rv->[0] : undef;
}

sub fetch_yester_date {
    my $db = shift;

    my $date = undef;
    my $rv = $db->selectrow_arrayref("SELECT date('yesterday') the_date;");
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

# $date_range is expected to be a string that is a set of dates
# joined together by commas, which is fed to SQL as the target of
# an IN clause.
#
sub fetch_pages_by_date_range {
    my $db = shift;
    my $chosen_dates = shift;
    local $| = 1;

    $chosen_dates = [ { date => fetch_current_date($db), i => 0 } ] if !defined $chosen_dates;

    #my $date_range = "'" . join("', '", map { $_->{date} } (@$chosen_dates)) . "'";
    #print "Date Range: $date_range\n";
    
    my $start_date = $chosen_dates->[0]->{date};
    my $end_date = $chosen_dates->[-1]->{date};


    printf "Fetching data for %d dates from database...", (scalar @$chosen_dates);
    my $st = time();
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
              WHERE date(i3log.local) BETWEEN ? AND ?
           ORDER BY i3log.local ASC;
        ;!, { Slice => {} }, ($start_date, $end_date));

    print STDERR $DBI::errstr."\n" if ! $rv;
    my $et = time() - $st;
    printf "done in %9.6f seconds.\n", $et;

    #my @dates = split /\',\s\'/, substr($date_range, 1, -1);

    printf "Processing data from result set...";
    $st = time();
    my $result = {};
    foreach my $row (@$chosen_dates) {
        $result->{$row->{date}} = { page => [], i => $row->{i} };
    }
    foreach my $row (@$rv) {
        my $date = $row->{the_date};
        push @{ $result->{$date}{page} }, ($row);
    }

=pod
    foreach my $row (@$chosen_dates) {
        my $date = $row->{date};
        my $i = $row->{i};
        #print "Date $date, i $i\n";
        $result->{$date} = {
            page    => [grep { $_->{the_date} eq $date } @$rv],
            i       => $i,
        };
    }
=cut

    $et = time() - $st;
    printf "done in %9.6f seconds.\n", $et;

    return $result;
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

    local $| = 1;
    print "Generating navigation data...";

    my $filename = "$PAGE_DIR/navbar.js";

    my @date_list = ();
    foreach my $row (@$date_counts) {
        push @date_list, ($row->{the_date}) if -f page_path($row);
    }
    my $big_list = join(",\n", map { sprintf "\"%s\"", $_; } (@date_list));
    my $last_date = (scalar @date_list < 1) ? fetch_current_date($db) : $date_list[-1];

    open FP, ">$filename" or die "Cannot open navbar script $filename: $!";
    print FP <<EOM;
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
                element.className = "not-glowing";
            }
        }

        function brighten(element) {
            if(element !== null) {
                element.style.opacity = "1.0";
                element.className = "glowing";
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
    close FP;
    print "done.\n";
}

sub dump_speakers {
    my $speakers = shift or die "No speakers data provided.";

    local $| = 1;
    print "Dumping I3 speaker file for WileyMUD...";

    my $filename = sprintf "%s/i3.speakers.new", $PAGE_DIR;
    open FP, ">$filename" or die "Cannot open speaker file $filename: $!";

    printf FP "#COUNT\nCount   %d\nEnd\n\n", scalar keys %$speakers;
    foreach my $row (map { $speakers->{$_} } (sort keys %$speakers)) {
        printf FP "#SPEAKER\nName    %s\nColor   %s\nEnd\n\n", $row->{speaker}, $row->{pinkfish};
    }
    printf FP "#END\n";

    close FP;

    print "done.\n";
}

sub do_pause {
    my $pause = shift;

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
}

sub get_random_background {
    my $BACKGROUND_DIR = shift;
    my $URL_HOME = shift;

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

    return ($BACKGROUND_IMG, $background_image);
}

sub generate_html_page {
    my %args = @_;

    # Globals
    # $JQUI_CSS
    # $JQUI_THEME
    # $JQ
    # $JQUI
    # $MOMENT
    # $MOMENT_TZ
    # $NAVBAR
    # $MUDLIST_LINK
    # $LOG_LINK
    # $DISCORD_LINK
    # $ICON_WIDTH, $ICON_WIDTH
    # $UP_ICON
    # $BEGIN_ICON
    # $PREV_ICON
    # $NEXT_ICON
    # $END_ICON
    # $DOWN_ICON
    # $SERVER_LINK

    my $filename            = $args{filename};
    my $page_background     = $args{page_background};
    my $background_image    = $args{background_image};
    my $overlay_image       = $args{overlay_image};
    my $today               = $args{today};
    my $page                = $args{page};
    my $channels            = $args{channels};
    my $speakers            = $args{speakers};
    my $pinkfish_map        = $args{pinkfish_map};
    my $do_censor           = $args{do_censor};
    my $first_page          = $args{first_page};
    my $first_date          = $args{first_date};
    my $prev_page           = $args{prev_page};
    my $prev_date           = $args{prev_date};
    my $next_page           = $args{next_page};
    my $next_date           = $args{next_date};
    my $last_page           = $args{last_page};
    my $last_date           = $args{last_date};

    open FP, ">$filename" or die "Cannot open output page $filename: $!";
    print FP <<EOM;
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
        <script src="$LOGPAGE_FUNC"></script>

        <script type="text/javascript">
            function setup() {
                setup_rows();
                localize_rows();
                // This if for the live page ONLY, pointless for static pages.
                if (!window.location.hash) {
                    //console.log("bottom");
                    //scroll_bottom();
                } else {
                    //console.log("top");
                }
                //setTimeout(function () { location.reload(true); }, 30 * 60 * 1000);
            }
        </script>
        <link rel="stylesheet" href="$LOGPAGE_STYLE">
        <style>
            html, body {
                background-color: $COLORS{"page_background"} !important;
                color: $COLORS{"page_text"} !important;
            }
            a:link {
                color: $COLORS{"page_link"} !important;
            }
            a:visited {
                color: $COLORS{"page_vlink"} !important;
            }
            input, select, textarea {
                border-color: $COLORS{"input_border"} !important;
                background-color: $COLORS{"input_background"} !important;
                color: $COLORS{"input"} !important;
            }
            input:focus, textarea:focus {
                border-color: $COLORS{"selected_input_border"} !important;
                background-color: $COLORS{"selected_input_background"} !important;
                color: $COLORS{"selected_input"} !important;
            }
            #navbar {
                background-color: $COLORS{'page_background'} !important;
            }
            #content-header {
                background-color: $COLORS{'page_background'} !important;
            }
        </style>
    </head>
    <body onload="setup();">
        $background_image
        $overlay_image
        <table id="navbar" width="99%" align="center">
            <tr>
                <td align="left" width="20%">
                    $MUDLIST_LINK
                    $LOG_LINK
                    $PIE_LINK
                </td>
                <td align="right" width="20%">
EOM

                #printf FP "<img id=\"scroll_top_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" onclick=\"scroll_top()\"/>\n", $TOP_ICON, $ICON_WIDTH, $ICON_WIDTH;
                printf FP "<img id=\"scroll_up_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" onclick=\"scroll_up()\"/>\n", $UP_ICON, $ICON_WIDTH, $ICON_WIDTH;

                if(defined $first_page) {
                    printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"glowing\" style=\"opacity: 1.0;\" /></a>\n", $first_page, $first_date, $first_date, $BEGIN_ICON, $ICON_WIDTH, $ICON_WIDTH;
                } else {
                    printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"not-glowing\" style=\"opacity: 0.5;\" />\n", $BEGIN_ICON, $ICON_WIDTH, $ICON_WIDTH;
                }

                if(defined $prev_page) {
                    printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"glowing\" style=\"opacity: 1.0;\" /></a>\n", $prev_page, $prev_date, $prev_date, $PREV_ICON, $ICON_WIDTH, $ICON_WIDTH;
                } else {
                    printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"not-glowing\" style=\"opacity: 0.5;\" />\n", $PREV_ICON, $ICON_WIDTH, $ICON_WIDTH;
                }

                print FP <<EOM;
                </td>
                <td align="center" width="10%">
                    <input type="text" id="datepicker" size="10" value="$today" style="font-size: 16px; text-align: center;" />
                </td>
                <td align="left" width="20%">
EOM

                if(defined $next_page) {
                    printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"glowing\" style=\"opacity: 1.0;\" /></a>\n", $next_page, $next_date, $next_date, $NEXT_ICON, $ICON_WIDTH, $ICON_WIDTH;
                } else {
                    printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"not-glowing\" style=\"opacity: 0.5;\" />\n", $NEXT_ICON, $ICON_WIDTH, $ICON_WIDTH;
                }

                if(defined $last_page) {
                    printf FP "<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"glowing\" style=\"opacity: 1.0;\" /></a>\n", $last_page, $last_date, $last_date, $END_ICON, $ICON_WIDTH, $ICON_WIDTH;
                } else {
                    printf FP "<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" class=\"not-glowing\" style=\"opacity: 0.5;\" />\n", $END_ICON, $ICON_WIDTH, $ICON_WIDTH;
                }

                printf FP "<img id=\"scroll_down_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 1.0;\" onclick=\"scroll_down()\"/>\n", $DOWN_ICON, $ICON_WIDTH, $ICON_WIDTH;
                #printf FP "<img id=\"scroll_bottom_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 1.0;\" onclick=\"scroll_bottom()\"/>\n", $BOTTOM_ICON, $ICON_WIDTH, $ICON_WIDTH;

                print FP <<EOM;
                    </td>
                    <td align="right" width="20%">
                        <span style="vertical-align: top; color: #555555"> --:-- PDT </span>
                        $SERVER_LINK
                        $GITHUB_LINK
                        $DISCORD_LINK
                    </td>
                </tr>
            </table>
            <table id="content-header-outside" width="99%" align="center">
                <tr id="content-header">
                    <td id="dateheader" align="left" width="80px" style="color: $COLORS{"header_text"}; min-width: 80px;">Date</td>
                    <td id="timeheader" align="left" width="60px" style="color: $COLORS{"header_text"}; min-width: 40px;">Time</td>
                    <td id="channelheader" align="left" width="80px" style="color: $COLORS{"header_text"}; min-width: 100px;">Channel</td>
                    <td id="speakerheader" align="left" width="200px" style="color: $COLORS{"header_text"}; min-width: 200px;">Speaker</td>
                    <td align="left">&nbsp;</td>
                </tr>
            </table>
            <table id="content" width="99%" align="center">
                <thead>
                <tr>
                    <td id="dateheader" align="left" width="80px" style="color: $COLORS{"header_text"}; min-width: 80px;">Date</td>
                    <td id="timeheader" align="left" width="60px" style="color: $COLORS{"header_text"}; min-width: 40px;">Time</td>
                    <td id="channelheader" align="left" width="80px" style="color: $COLORS{"header_text"}; min-width: 100px;">Channel</td>
                    <td id="speakerheader" align="left" width="200px" style="color: $COLORS{"header_text"}; min-width: 200px;">Speaker</td>
                    <th align="left">&nbsp;</th>
                </tr>
                </thead>
                <tbody>
EOM

                my $counter = 0;
                foreach my $row (@$page) {
                    # YYYY-MM-DD date
                    # HH:MM:SS time -- colored by time of day
                    # Channel -- colored by channel name
                    # Speaker@Mud -- colored by speaker name
                    # Message -- fixed font

                    # Emit each data row as a table row
                    my $bg_color = ($counter % 2) ? $COLORS{"odd_row"} : $COLORS{"even_row"};

                    my $hour_html = $row->{hour_html} || "--**--NULL--**--";
                    my $channel_html = $row->{channel_html} || $channels->{default}{html} || "--**--NULL--**--";
                    my $speaker_html = $row->{speaker_html} || $speakers->{default}{html} || "--**--NULL--**--";
                    my $channel = $row->{channel}; # text only channel name
                    $channel = "日本語" if $channel eq "japanese";

                    printf FP "<tr id=\"row_%d\" style=\"display:none\">\n", $counter;
                    printf FP "<td onclick=\"col_click(this)\" bgcolor=\"%s\">%s</td>\n", $bg_color, $row->{the_date};
                    printf FP "<td onclick=\"col_click(this)\" bgcolor=\"%s\">%s%s%s</td>\n", $bg_color, $hour_html, $row->{the_time}, "</span>";
                    printf FP "<td onclick=\"col_click(this)\" bgcolor=\"%s\">%s%s%s</td>\n", $bg_color, $channel_html, $channel, "</span>";
                    printf FP "<td onclick=\"col_click(this)\" bgcolor=\"%s\">%s%s@%s%s</td>\n", $bg_color, $speaker_html, $row->{speaker}, $row->{mud}, "</span>";

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
                    $span_class = "blurry" if $do_censor and $channel eq "bsg";
                    $span_class = "blurry" if $do_censor and $message =~ /on\sfree_speech\&gt\;/;
                    $span_class = "blurry" if $do_censor and $message =~ /on\sbsg\&gt\;/;
                    $span_class = "blurry" if $do_censor and $message =~ /^spoiler:/i;
                    $span_class = "blurry" if $do_censor and $message =~ /\[(spoiler|redacted)\]/i;

                    printf FP "<td bgcolor=\"%s\"><span class=\"$span_class\">%s</span></td>\n",  $bg_color, $message;
                    printf FP "</tr>\n";

                    $counter++;
                }
                print FP <<EOM;
            </tbody>
        </table>
    </body>
</html>
EOM
    close FP;
}

sub generate_json_page {
    my %args = @_;

    foreach (qw(overwrite date_counts index json_filename page)) {
        die "$_ not provided!" if !exists $args{$_};
    }

    my $overwrite           = $args{overwrite};
    my $date_counts         = $args{date_counts};
    my $i                   = $args{index};         # the index into the entire data set
    my $filename            = $args{json_filename};
    my $page                = $args{page};

    if( $i < ((scalar @$date_counts) - 2) and -f $filename ) {
        if( !$overwrite ) {
            # Always generate the very last, and next to last page again.
            return;
        }
    }

    my $json_dump = JSON->new->utf8->allow_nonref->canonical->pretty->encode($page);
    open FP, ">$filename" or die "Cannot open dump page $filename: $!";
    print FP "$json_dump\n";
    close FP;
}

sub generate_page {
    my %args = @_;

    foreach (qw(now_date yester_date do_pages do_json do_censor overwrite 
        pinkfish_map channels speakers
        date_counts index page_limit pages_done pages_todo page)) {
        die "$_ not provided!" if !exists $args{$_};
    }

    #my $LOCAL_DB        = open_postgres_db($PG_DB); # When forking, we can't use the original
    my $now_date        = $args{now_date};
    my $yester_date     = $args{yester_date};
    my $do_pages        = $args{do_pages};
    my $do_json         = $args{do_json};
    my $do_censor       = $args{do_censor};
    my $overwrite       = $args{overwrite};
    my $pinkfish_map    = $args{pinkfish_map};
    my $channels        = $args{channels};
    my $speakers        = $args{speakers};

    my $date_counts     = $args{date_counts};
    my $i               = $args{index};         # the index into the entire data set
    my $page_limit      = $args{page_limit};
    my $pages_done      = $args{pages_done};    # the number of pages we've done
    my $pages_todo      = $args{pages_todo};
    my $page            = $args{page};          # This is our collected page data

    my $day_row         = $args{day_row}            = $date_counts->[$i];
    my $today           = $args{today}              = $day_row->{the_date};
    my $filename        = $args{filename}           = page_path($day_row);
    my $json_filename   = $args{json_filename}      = json_path($day_row);
    my $this_is_the_end = $args{this_is_the_end}    = ($i == (scalar @$date_counts) - 1) ? 1 : undef;

    if( $i < ((scalar @$date_counts) - 2) and -f $filename ) {
        if( !$overwrite ) {
            # Always generate the very last, and next to last page again.
            printf "            Skipping %s.\n", $filename;
            return;
        }
    }

    my ($first_row, $first_page, $first_date,
        $prev_row, $prev_page, $prev_date,
        $next_row, $next_page, $next_date,
        $last_row, $last_page, $last_date);

    if( $i > 0 ) {
        # We have a previous page.
        $prev_row   = $args{prev_row}   = $date_counts->[$i - 1];
        $prev_date  = $args{prev_date}  = $prev_row->{the_date};
        $prev_page  = $args{prev_page}  = page_url($prev_row);

        # We also are not ON the first page, so we should have that too.
        $first_row  = $args{first_row}  = $date_counts->[0];
        $first_date = $args{first_date} = $first_row->{the_date};
        $first_page = $args{first_page} = page_url($first_row);
    }

    if( $i < (scalar @$date_counts) - 2 ) {
        # We have a next page.
        $next_row   = $args{next_row}   = $date_counts->[$i + 1];
        $next_date  = $args{next_date}  = $next_row->{the_date};
        $next_page  = $args{next_page}  = page_url($next_row);
    } elsif( $i < (scalar @$date_counts) - 1 ) {
        my $now_date = $args{now_date};
        my $yester_date = $args{yester_date};

        if( $debug_page or ($date_counts->[-1]->{the_date} eq $yester_date) ) {
            # We crossed midnight, but nobody has said anything yet!
            $next_row   = $args{next_row}   = $date_counts->[$i + 1];
            $next_date  = $args{next_date}  = $next_row->{the_date};
            $next_page  = $args{next_page}  = page_url($next_row);
        } elsif( $date_counts->[-1]->{the_date} eq $now_date ) {
            # We are in normal status.
            $next_row   = $args{next_row}   = $date_counts->[$i + 1];
            $next_date  = $args{next_date}  = "LIVE " . $next_row->{the_date};
            $next_page  = $args{next_page}  = $LIVE_PAGE;
        }
    }
        #if( $debug_page ) {
        #    # We want the static page.
        #    $next_row   = $args{next_row}   = $date_counts->[$i + 1];
        #    $next_date  = $args{next_date}  = $next_row->{the_date};
        #    $next_page  = $args{next_page}  = page_url($next_row);
        #} else {
        #    # We want the live page.
        #    $next_row   = $args{next_row}   = $date_counts->[$i + 1];
        #    $next_date  = $args{next_date}  = "LIVE " . $next_row->{the_date};
        #    $next_page  = $args{next_page}  = $LIVE_PAGE;
        #}
    # else {
    #        # We want the live page, and really should be hidden under the live page.
    #    $next_row   = $args{next_row}   = $date_counts->[$i];
    #    $next_date  = $args{next_date}  = "LIVE " . $next_row->{the_date};
    #    if( $debug_page ) {
    #        $next_page  = $args{next_page}  = page_url($next_row);
    #    } else {
    #        $next_page  = $args{next_page}  = $LIVE_PAGE;
    #    }
    #}

    # We ALWAYS have a last page, as that's the current page.  But,
    # this one is special.  Instead of the usual static page, we want
    # to point you to the live page that gets updated on load.
    $last_row   = $args{last_row}   = $date_counts->[-1];
    $last_date  = $args{last_date}  = $this_is_the_end ? "LIVE" : ("LIVE " . $last_row->{the_date});
    $last_page  = $args{last_page}  = $LIVE_PAGE;

    #my $page_background = $this_is_the_end ? "#1F0000" : "black";
    my $page_background     = $args{page_background}    = $this_is_the_end ? "#070707" : "black";
    my $overlay_image       = $args{overlay_image}      = $this_is_the_end ? $OVERLAY_IMG : "";

    my ($BACKGROUND_IMG, $background_image) = get_random_background($BACKGROUND_DIR, $URL_HOME);
    $args{BACKGROUND_IMG}   = $BACKGROUND_IMG;
    $args{background_image} = $background_image;

    #my $page    = $args{page}   = fetch_page_by_date($LOCAL_DB, $today);
    #die "No data for $today? $!" if !defined $page;
    #$LOCAL_DB->disconnect();

    printf "    [%5d] PID $$ Generating %s... %d messages.\n",
        ($pages_todo - $pages_done), $today, scalar @$page;

    generate_html_page(%args) if $do_pages;
    generate_json_page(%args) if $do_json;
}

sub main {
    print "Initializing...\n";

    my $DATABASE = open_postgres_db($PG_DB);

    my $row_count = fetch_row_count($DATABASE);
    my $date_counts = fetch_date_counts($DATABASE);
    my $pinkfish_map = fetch_pinkfish_map($DATABASE);
    my $hours = fetch_hours($DATABASE);

    if( $page_start < 0 ) {
        $page_start = (scalar @$date_counts) + $page_start;
    }

    display_options($row_count, scalar @$date_counts);
    do_pause($pause);
    update_all_channels($DATABASE) if $do_update;
    update_all_speakers($DATABASE) if $do_update;

    my $channels = fetch_channels($DATABASE);
    my $speakers = fetch_speakers($DATABASE);

    update_date_cache($date_counts) if $do_dates;
    update_cache($pinkfish_map, $hours, $channels, $speakers) if $do_cache;
    generate_navbar_script($DATABASE, $date_counts) if $do_navbar;
    dump_speakers($speakers) if $do_speakers;

    my $now_date = fetch_current_date($DATABASE);
    my $yester_date = fetch_yester_date($DATABASE);

    if( $do_pages or $do_json ) {
        $page_limit = (scalar @$date_counts) if $page_limit < 1;
        #$page_limit = 30 if $use_live and $page_limit > 30;

        my $pages_todo = (scalar @$date_counts) - $page_start;
        $pages_todo = $page_limit if $page_limit < $pages_todo;

        $thread_count = 0 if $thread_count < 2;
        #$thread_count = 40 if $thread_count > 40;

        my $pm = Parallel::ForkManager->new($thread_count); # 0 means no forking

        my $pages_done = 0;
        my @chosen_dates = ();
        for( my $i = $page_start; $i < scalar @$date_counts; $i++ ) {
            last if $pages_done >= $page_limit or $pages_done >= scalar @$date_counts;
            $pages_done++;
            push @chosen_dates, { date => $date_counts->[$i]->{the_date}, i => $i };
        }

        # Here, we need to partition the dates into clumps of about 200.
        printf "We have %d total dates to process.\n", (scalar @chosen_dates);
        #my $page_set = {};
        my @page_sets = ();

        if( $thread_count > 1 ) {
            my @partition = ();
            my $p = 0;

            foreach my $date (@chosen_dates) {
                if( $p >= $partition_size) {
                    my $result = fetch_pages_by_date_range($DATABASE, \@partition);
                    #$page_set = { %$page_set, %$result } if defined $result;
                    push @page_sets, $result if defined $result;
                    @partition = ($date);
                    $p = 1;
                } else {
                    push @partition, $date;
                    $p++;
                }
            }
            if( (scalar @partition) > 0) {
                my $result = fetch_pages_by_date_range($DATABASE, \@partition);
                push @page_sets, $result if defined $result;
                #$page_set = { %$page_set, %$result } if defined $result;
                @partition = ();
                $p = 0;
            }
        } else {
            #$page_set = fetch_pages_by_date_range($DATABASE, \@chosen_dates);
            my $result = fetch_pages_by_date_range($DATABASE, \@chosen_dates);
            push @page_sets, $result if defined $result;
        }

        SET_LOOP:
        for(my $j = 0; $j < (scalar @page_sets); $j++) {
            $pm->start($j) and next SET_LOOP;

            srand(time + ($j * $$));
            my $page_set = $page_sets[$j];
            $pages_done = $j * $partition_size;

            foreach my $date (sort keys %$page_set) {
                $pages_done++;
                my $row = $page_set->{$date};
                my $i = $row->{i};
                my $page = $row->{page};

                generate_page( 
                    now_date        => $now_date,
                    yester_date     => $yester_date,
                    do_pages        => $do_pages,
                    do_json         => $do_json,
                    do_censor       => $do_censor,
                    overwrite       => $overwrite,
                    pinkfish_map    => $pinkfish_map,
                    channels        => $channels,
                    speakers        => $speakers,
                    date_counts     => $date_counts,
                    index           => $i,
                    page_limit      => $page_limit,
                    pages_done      => $pages_done,
                    pages_todo      => $pages_todo,
                    page            => $page,
                    );
            }

            $pm->finish();
        }
        $pm->wait_all_children;
    }

    generate_navbar_script($DATABASE, $date_counts) if $do_navbar;
    print "Done!\n";
}

main();
exit 0;


