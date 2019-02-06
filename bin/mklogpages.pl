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

#my $DB_FILE = '/home/wiley/lib/i3/wiley.db';
my $DB_FILE = '/home/wiley/lib/i3/wiley.db.bkp-20190205';
my $PAGE_DIR = '/home/wiley/public_html/logpages';

my $db = undef;

my $page_start = 0;
my $page_size = 100;
my $page_limit = 10;

sub do_help {
    print STDERR <<EOM
usage:  $PROGRAM_NAME [-h] [-s start page] [-l page limit] [-p page size]
long options:
    --help              - This helpful help!
    --start N           - Page to start from, 0 is the first page.
    --limit N           - Number of pages to do, defaults to 10.
    --pagesize N        - Page size, defaults to 100.
EOM
    ;
    exit(1);
}

Getopt::Long::Configure("gnu_getopt");
Getopt::Long::Configure("auto_version");
GetOptions(
    'help|h'            => sub { do_help() },
    'start|s:i'         => \$page_start,
    'limit|l:10'        => \$page_limit,
    'pagesize|p:100'    => \$page_size,
);

sub fetch_row_count {
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
    my $speakers = load_speakers($db);
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

            my $insert_sql = $db->prepare( qq!
                INSERT INTO speakers ( speaker, pinkfish )
                VALUES (?,?)
                !);
            my $rv = $insert_sql->execute( $speaker, $new_color );
            if($rv) {
                #$db->commit if !($counter % 100);
                #$db->commit;
                print "Added $speaker as $new_color\n";
            } else {
                print STDERR $DBI::errstr."\n";
                #$db->rollback;
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
    my $channels = load_channels($db);
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

            my $insert_sql = $db->prepare( qq!
                INSERT INTO channels ( channel, pinkfish )
                VALUES (?,?)
                !);
            my $rv = $insert_sql->execute( $channel, $new_color );
            if($rv) {
                #$db->commit if !($counter % 100);
                #$db->commit;
                print "Added $channel as $new_color\n";
            } else {
                print STDERR $DBI::errstr."\n";
                #$db->rollback;
            }
        }
    } else {
        print STDERR $DBI::errstr."\n";
    }
}

sub simple_fetch_page {
    my $db = shift;
    my $offset = shift || 0;
    my $page_size = shift || 100;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectall_arrayref(qq!
             SELECT i3log.created,
                    i3log.is_emote,
                    i3log.is_url,
                    i3log.is_bot,
                    i3log.channel,
                    i3log.speaker,
                    i3log.mud,
                    i3log.message,
                    substr(created, 1, 10)     AS the_date,
                    substr(created, 12, 8)     AS the_time,
                    substr(created, 12, 2)     AS the_hour
               FROM i3log
           ORDER BY created ASC
             LIMIT  $page_size
             OFFSET $offset
        ;!, { Slice => {} } );
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

sub fetch_date_counts {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    # 2006-09-25 11:57:45.000
    # YYYY-MM-DD HH:MM:SS.UUU
    # 1    6  9  12 15 18 21
    my $rv = $db->selectall_arrayref(qq!
        SELECT      SUBSTR(created, 1, 10) AS the_date,
                    SUBSTR(created, 1, 4) AS the_year,
                    SUBSTR(created, 6, 2) AS the_month,
                    SUBSTR(created, 9, 2) AS the_day,
                    COUNT(*) AS count
        FROM        i3log
        GROUP BY    the_date
        ORDER BY    the_date ASC;
        ;!, { Slice => {} } );
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

sub fetch_page_by_date {
    my $db = shift;
    my $date = shift;
    die "Invalid database handle!" if !defined $db;

    my $rv = undef;

    if( !defined $date ) {
        $rv = $db->selectrow_arrayref("select date('now');");
        if($rv) {
            $date = $rv->[0];
        } else {
            print STDERR $DBI::errstr."\n";
        }
    }
    die "Cannot obtain a valid date???: $!" if !defined $date;

    $rv = $db->selectall_arrayref(qq!
             SELECT i3log.created,
                    i3log.is_emote,
                    i3log.is_url,
                    i3log.is_bot,
                    i3log.channel,
                    i3log.speaker,
                    i3log.mud,
                    i3log.message,
                    SUBSTR(created, 1, 10)     AS the_date,
                    SUBSTR(created, 12, 8)     AS the_time,
                    SUBSTR(created, 1, 4)      AS the_year,
                    SUBSTR(created, 6, 2)      AS the_month,
                    SUBSTR(created, 9, 2)      AS the_day,
                    SUBSTR(created, 12, 2)     AS the_hour,
                    SUBSTR(created, 15, 2)     AS the_minute,
                    SUBSTR(created, 18, 2)     AS the_second,
                    CAST(SUBSTR(created, 12, 2) AS INTEGER) AS the_hour,
                    hours.pinkfish             AS hour_color,
                    channels.pinkfish          AS channel_color,
                    speakers.pinkfish          AS speaker_color,
                    pinkfish_map_hour.html     AS hour_html,
                    pinkfish_map_channel.html  AS channel_html,
                    pinkfish_map_speaker.html  AS speaker_html
               FROM i3log
          LEFT JOIN hours
                 ON (the_hour = hours.hour)
          LEFT JOIN channels
                 ON (lower(i3log.channel) = channels.channel)
          LEFT JOIN speakers
                 ON (lower(i3log.speaker) = speakers.speaker)
          LEFT JOIN pinkfish_map pinkfish_map_hour
                 ON (hour_color = pinkfish_map_hour.pinkfish)
          LEFT JOIN pinkfish_map pinkfish_map_channel
                 ON (channel_color = pinkfish_map_channel.pinkfish)
          LEFT JOIN pinkfish_map pinkfish_map_speaker
                 ON (speaker_color = pinkfish_map_speaker.pinkfish)
              WHERE date(i3log.created) = ?
           ORDER BY created ASC;
        ;!, { Slice => {} }, ($date) );
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;

}

sub fetch_page {
    my $db = shift;
    my $offset = shift || 0;
    my $page_size = shift || 100;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectall_arrayref(qq!
             SELECT i3log.created,
                    i3log.is_emote,
                    i3log.is_url,
                    i3log.is_bot,
                    i3log.channel,
                    i3log.speaker,
                    i3log.mud,
                    i3log.message,
                    SUBSTR(created, 1, 10)     AS the_date,
                    SUBSTR(created, 12, 8)     AS the_time,
                    CAST(SUBSTR(created, 12, 2) AS INTEGER) AS the_hour,
                    hours.pinkfish             AS hour_color,
                    channels.pinkfish          AS channel_color,
                    speakers.pinkfish          AS speaker_color,
                    pinkfish_map_hour.html     AS hour_html,
                    pinkfish_map_channel.html  AS channel_html,
                    pinkfish_map_speaker.html  AS speaker_html
               FROM i3log
          LEFT JOIN hours
                 ON (the_hour = hours.hour)
          LEFT JOIN channels
                 ON (lower(i3log.channel) = channels.channel)
          LEFT JOIN speakers
                 ON (lower(i3log.speaker) = speakers.speaker)
          LEFT JOIN pinkfish_map pinkfish_map_hour
                 ON (hour_color = pinkfish_map_hour.pinkfish)
          LEFT JOIN pinkfish_map pinkfish_map_channel
                 ON (channel_color = pinkfish_map_channel.pinkfish)
          LEFT JOIN pinkfish_map pinkfish_map_speaker
                 ON (speaker_color = pinkfish_map_speaker.pinkfish)
           ORDER BY created ASC
             LIMIT  $page_size
             OFFSET $offset
        ;!, { Slice => {} } );
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

#CREATE TABLE pinkfish_map (
#            pinkfish TEXT PRIMARY KEY NOT NULL,
#            html TEXT
#        );
sub load_pinkfish_map {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectall_hashref(qq!
        SELECT pinkfish, html
          FROM pinkfish_map
        ;!, 'pinkfish');
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

#CREATE TABLE hours (
#            hour INTEGER PRIMARY KEY NOT NULL,
#            pinkfish TEXT
#        );
sub load_hours {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectall_hashref(qq!
        SELECT hour, hours.pinkfish, html
          FROM hours
     LEFT JOIN pinkfish_map
            ON (hours.pinkfish = pinkfish_map.pinkfish)
        ;!, 'hour');
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

#CREATE TABLE channels (
#            channel TEXT PRIMARY KEY NOT NULL,
#            pinkfish TEXT
#        );
sub load_channels {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectall_hashref(qq!
        SELECT channel, channels.pinkfish, html
          FROM channels
     LEFT JOIN pinkfish_map
            ON (channels.pinkfish = pinkfish_map.pinkfish)
        ;!, 'channel');
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

#CREATE TABLE speakers (
#            speaker TEXT PRIMARY KEY NOT NULL,
#            pinkfish TEXT
#        );
sub load_speakers {
    my $db = shift;
    die "Invalid database handle!" if !defined $db;

    my $rv = $db->selectall_hashref(qq!
        SELECT speaker, speakers.pinkfish, html
          FROM speakers
     LEFT JOIN pinkfish_map
            ON (speakers.pinkfish = pinkfish_map.pinkfish)
        ;!, 'speaker');
    if($rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

sub next_page_date {
    my $date_counts = shift;
    my $page_year = shift;
    my $page_month = shift;
    my $page_day = shift;


}

sub generate_navbar_script {
    my $date_counts = shift;
    die "Invalid date information!" if !defined $date_counts;

    my $filename = "$PAGE_DIR/navbar.js";

    my @date_list = ();
    foreach my $row (@$date_counts) {
        push @date_list, ($row->{the_date});
    }
    my $big_list = join(",\n", map { sprintf "\"%s\"", $_; } (@date_list));
    my $last_date = $date_list[-1];

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
            var url_bits = window.location.href.toString().split('/');
            url_bits[url_bits.length - 1] = '../../'
                + dateString.substr(0,4) + '/'
                + dateString.substr(5,2) + '/'
                + dateString + '.html';
            window.location.href = url_bits.join('/');
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

$db = DBI->connect("DBI:SQLite:dbname=$DB_FILE", '', '', { AutoCommit => 1, PrintError => 1, });
my $row_count = fetch_row_count($db);
#my $page_count = $row_count / $page_size;
my $date_counts = fetch_date_counts($db);

#printf "Found %d rows, which is %d pages of %d rows each.\n", $row_count, $page_count, $page_size;
printf "Found %d rows over %d pages.\n", $row_count, scalar @$date_counts;
printf "Starting from page %d, as requested!\n", $page_start;
printf "Stopping after %d pages\n", $page_limit;
printf "Pausing 5 seconds before charging ahead!\n";

sleep 5;

my $pinkfish_map = load_pinkfish_map($db);
my $hours = load_hours($db);

print "Updating speaker and channel info...\n";
update_all_channels($db);
update_all_speakers($db);

my $channels = load_channels($db);
my $speakers = load_speakers($db);

#print STDERR "Channels: ".Dumper($channels)."\n";
#print STDERR "Speakers: ".Dumper($speakers)."\n";

print "Generating navigation data...\n";
generate_navbar_script($date_counts);

my $pages_done = 0;
#foreach my $day_row (@$date_counts) {
for( my $i = $page_start; $i < scalar @$date_counts; $i++ ) {
    my $day_row = $date_counts->[$i];
    my $today = $day_row->{the_date};

    last if $pages_done >= $page_limit or $pages_done >= scalar @$date_counts;
    $pages_done++;

    my $dir_path = sprintf "%s/%s", $PAGE_DIR, $day_row->{the_year};
    mkdir $dir_path if ! -d $dir_path;
    $dir_path = sprintf "%s/%s/%s", $PAGE_DIR, $day_row->{the_year}, $day_row->{the_month};
    mkdir $dir_path if ! -d $dir_path;

    my $filename = sprintf "%s/%s/%s/%s.html", $PAGE_DIR,
        $day_row->{the_year}, $day_row->{the_month},
        $day_row->{the_date};

    if( $i < ((scalar @$date_counts) - 2) and -f $filename ) {
        # Always generate the very last, and next to last page again.
        printf "    Skipping %s!\n", $filename;
        next;
    }

    my $prev_row = undef;
    my $prev_page = undef;
    my $next_row = undef;
    my $next_page = undef;
    if( $i > 0 ) {
        # We have a previous page.
        $prev_row = $date_counts->[$i - 1];
        $prev_page = sprintf "../../%s/%s/%s.html",
            $prev_row->{the_year}, $prev_row->{the_month},
            $prev_row->{the_date};
    }
    if( $i < (scalar @$date_counts) - 1 ) {
        # We have a next page.
        $next_row = $date_counts->[$i + 1];
        $next_page = sprintf "../../%s/%s/%s.html",
            $next_row->{the_year}, $next_row->{the_month},
            $next_row->{the_date};
    }

    my $page = fetch_page_by_date($db, $today);
    die "No data for $today? $!" if !defined $page;
    printf "    Generating %s... %d messages.\n", $today, scalar @$page;

    open FP, ">$filename" or die "Cannot open output page $filename: $!";

    print FP <<EOM
<html>
    <head>
        <meta charset="utf-8">
        <link rel="stylesheet" href="../../jquery/jquery-ui.css">
        <link rel="stylesheet" href="../../jquery/jquery-ui.theme.css">
        <script src="../../jquery.js"></script>
        <script src="../../jquery/jquery-ui.js"></script>
        <script src="../../navbar.js"></script>

        <script language="javascript">
            function toggleDiv(divID) {
                if(document.getElementById(divID).style.display == 'none') {
                    document.getElementById(divID).style.display = 'block';
                } else {
                    document.getElementById(divID).style.display = 'none';
                }
            }
        </script>
        <script type="text/javascript">
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
            function setup_work() {
                count = document.getElementById("content").getElementsByTagName("tr").length;
                for(var i = 0; i < count; i++) {
                    row_on(i);
                }
            }
            function setup() {
                setup_work();
            }
        </script>
        <style>
            html, body { table-layout: fixed; max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            table { table-layout: fixed; max-width: 99%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
        </style>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="setup();">
        <table id="navbar" width="99%" align="center">
            <tr>
                <td align="left" width="10%">
                    <a href="mudlist.html">Mudlist</a>
                </td>
                <td align="right" width="20%">
EOM
    ;

    if(defined $prev_page) {
        printf FP "<a href=\"%s\">Previous Page (%s)</a>", $prev_page, $prev_row->{the_date};
    } else {
        printf FP "No Previous Page";
    }

    print FP <<EOM
                </td>
                <td align="center" width="10%">
                    <input type="text" id="datepicker" size="10">
                </td>
                <td align="left" width="20%">
EOM
    ;

    if(defined $next_page) {
        printf FP "<a href=\"%s\">Next Page (%s)</a>", $next_page, $next_row->{the_date};
    } else {
        printf FP "No Next Page";
    }

    print FP <<EOM
                </td>
                <td align="right">
                    <a href="https://themud.org/chanhist.php#Channel=all">Other Logs</a>
                </td>
                <td align="right" width="10%">
                    <a href="server.php">Server</a>
                </td>
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

        printf FP "<tr id=\"row_%d\" style=\"display:none\">\n", $counter;
        printf FP "<td bgcolor=\"%s\">%s</td>\n", $bg_color, $row->{the_date};
        printf FP "<td bgcolor=\"%s\">%s%s%s</td>\n", $bg_color, $hour_html, $row->{the_time}, "</span>";
        printf FP "<td bgcolor=\"%s\">%s%s%s</td>\n", $bg_color, $channel_html, $row->{channel}, "</span>";
        printf FP "<td bgcolor=\"%s\">%s%s@%s%s</td>\n", $bg_color, $speaker_html, $row->{speaker}, $row->{mud}, "</span>";

        my $message = $row->{message};
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

        printf FP "<td bgcolor=\"%s\"><span style=\"font-family: monospace; white-space: pre-wrap;\">%s</span></td>\n",  $bg_color, $message;
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

print "Dumping I3 speaker file for WileyMUD...\n";
#$channels = load_channels($db);
$speakers = load_speakers($db);

$db->disconnect();

my $filename = sprintf "%s/i3.speakers.new", $PAGE_DIR;
open FP, ">$filename" or die "Cannot open speaker file $filename: $!";

printf FP "#COUNT\nCount   %d\nEnd\n\n", scalar keys %$speakers;
foreach my $row (map { $speakers->{$_} } (sort keys %$speakers)) {
    printf FP "#SPEAKER\nName    %s\nColor   %s\nEnd\n\n", $row->{speaker}, $row->{pinkfish};
}
printf FP "#END\n";

close FP;

print "Done!\n";
exit 0;
