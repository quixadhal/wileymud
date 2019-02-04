#!/usr/bin/perl -w

use utf8;
use strict;
use Encode qw(encode_utf8);
no warnings 'utf8';
use English;
use Data::Dumper;
use DBI;
use Date::Manip::Date;
use HTML::Entities;

my $DB_FILE = '/home/wiley/lib/i3/wiley.db';
my $PAGE_DIR = "/home/wiley/public_html/logpages";
my $page_size = 100;
my $db = undef;

sub add_speaker {
    my $db = shift;
    my $speakers = shift;
    my $speaker = shift;
    return $speakers if !defined $speaker or length "$speaker" < 1;
    $speaker = lc $speaker;

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
        '%^B_RED%^%^WHITE%^',
        '%^B_GREEN%^%^WHITE%^',
        '%^B_BLUE%^%^WHITE%^',
        '%^B_MAGENTA%^%^WHITE%^',
        '%^B_RED%^%^BLACK%^',
        '%^B_GREEN%^%^BLACK%^',
        '%^B_MAGENTA%^%^BLACK%^',
        '%^B_CYAN%^%^BLACK%^',
        '%^B_YELLOW%^%^BLACK%^',
        '%^B_WHITE%^%^BLACK%^',
    );
    my $color_count = scalar @color_map;
    my $speaker_count = scalar keys %$speakers;

    if( ! exists $speakers->{$speaker} ) {
        # New speaker, insert to the database and reload it.
        my $new_color = @color_map[$speaker_count % $color_count];
        #$db->begin_work;
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
        $speakers = load_speakers($db);
    }

    return $speakers;
}

sub add_channel {
    my $db = shift;
    my $channels = shift;
    my $channel = shift;
    return $channels if !defined $channel or length "$channel" < 1;
    $channel = lc $channel;

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
    my $channel_count = scalar keys %$channels;

    if( ! exists $channels->{$channel} ) {
        # New speaker, insert to the database and reload it.
        my $new_color = @color_map[$channel_count % $color_count];
        #$db->begin_work;
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
        $channels = load_channels($db);
    }

    return $channels;
}

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
        '%^B_RED%^%^WHITE%^',
        '%^B_GREEN%^%^WHITE%^',
        '%^B_BLUE%^%^WHITE%^',
        '%^B_MAGENTA%^%^WHITE%^',
        '%^B_RED%^%^BLACK%^',
        '%^B_GREEN%^%^BLACK%^',
        '%^B_MAGENTA%^%^BLACK%^',
        '%^B_CYAN%^%^BLACK%^',
        '%^B_YELLOW%^%^BLACK%^',
        '%^B_WHITE%^%^BLACK%^',
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


$db = DBI->connect("DBI:SQLite:dbname=$DB_FILE", '', '', { AutoCommit => 1, PrintError => 1, });
my $row_count = fetch_row_count($db);
my $pinkfish_map = load_pinkfish_map($db);
my $hours = load_hours($db);

update_all_channels($db);
update_all_speakers($db);

my $channels = load_channels($db);
my $speakers = load_speakers($db);

#print STDERR "Channels: ".Dumper($channels)."\n";
#print STDERR "Speakers: ".Dumper($speakers)."\n";

printf "Found %d rows\n", $row_count;

my $pages_done = 0;
for( my $i = 0; $i < $row_count; $i += $page_size ) {
    my $filename = sprintf "%s/%09d.html", $PAGE_DIR, $i;
    my $next_page = sprintf "%09d.html", $i + $page_size;
    my $prev_page = sprintf "%09d.html", $i - $page_size;

    my $page_number = $i / $page_size;
    my $next_page_number = $i + 1;
    my $prev_page_number = $i - 1;

    # Skip pages already generated unless it's the last page
    next if ($i < $row_count - $page_size) and -f $filename;
    printf "    Generating %s\n", $filename;
    my $page = fetch_page($db, $i, $page_size);
    die "Invalid data!" if !defined $page;


    # Make the page
    open FP, ">$filename" or die "Cannot open output page $filename: $!";

    print FP <<EOM
<html>
    <head>
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
                <td align="left" width="34%">
EOM
;
    if( $i - $page_size >= 0 ) {
        printf FP "<a href=\"%s\">Previous Page (%d)</a>", $prev_page, ($i-$page_size) / $page_size;
    } else {
        printf FP "No Previous Page";
    }

    if( $i < $row_count + $page_size ) {
        printf FP "<a href=\"%s\">Next Page (%d)</a>", $next_page, ($i+$page_size) / $page_size;
    } else {
        printf FP "No Next Page";
    }

    print FP <<EOM
                </td>
                <td align="center" width="8%">
                    &nbsp;
                </td>
                <td align="center" width="8%">
                    <a href="mudlist.html">Mudlist</a>
                </td>
                <td align="center" width="8%">
                    <a href="https://themud.org/chanhist.php#Channel=all">Other Logs</a>
                </td>
                <td align="center" width="8%">
                    <a href="server.php">Server</a>
                </td>
                <td align="right" width="34%" onmouseover="pagegen.style.color='#00FF00'; timespent.style.color='#00FF00';" onmouseout="pagegen.style.color='#1F1F1F'; timespent.style.color='#1F1F1F';">
                    &nbsp;
                    <a href="javascript:;" onmousedown="toggleDiv('source');">
                        <span id="pagegen" style="color: #1F1F1F">
                            Page &nbsp; Source
                        </span>
                    </a>
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
    # Emit the HTML garbage, and the table headers
    my $counter = 0;
    foreach my $row (@$page) {
        # YYYY-MM-DD date
        # HH:MM:SS time -- colored by time of day
        # Channel -- colored by channel name
        # Speaker@Mud -- colored by speaker name
        # Message -- fixed font

        # Emit each data row as a table row
        my $bg_color = ($counter % 2) ? "#000000" : "#1F1F1F";

        #$channels = add_channel($db, $channels, $row->{channel}) if defined $row->{channel};
        #$speakers = add_speaker($db, $speakers, $row->{speaker}) if defined $row->{speaker};

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
        $message = encode_entities($message);

        # Filter known pinkfish codes and make them HTML
        foreach my $pf_key (keys %$pinkfish_map) {
            my $pf_replacement = $pinkfish_map->{$pf_key}{html};

            $message =~ s/$pf_key/$pf_replacement/gmx;
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

    $pages_done++;
    last if $pages_done > 9;
}

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

