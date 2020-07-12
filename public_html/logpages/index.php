<?php
global $time_start;
$time_start = microtime(true);

global $LOG_HOME;
global $PAGE_DIR;
global $CHAT_TEXT_FILE;
global $DB_FILE;
global $PG_DB;
global $PG_USERNAME;
global $PG_PASSWORD;
global $URL_ONLY;
global $CENSOR;

$URL_ONLY       = (isset($_GET["url"]) || isset($_POST["url"])) ? true : false;
//$CENSOR         = (isset($_GET["censor"]) || isset($_POST["censor"])) ? false : true;
$CENSOR         = true;
$URL_HOME       = "http://wileymud.themud.org/~wiley";
$LOG_HOME       = "$URL_HOME/logpages";
$LIVE_PAGE      = "$LOG_HOME/";

$DB_FILE        = '/home/wiley/lib/i3/wiley.db';
//$DB_FILE        = '/home/wiley/lib/i3/wiley.bkp-20190211.db';
$PAGE_DIR       = '/home/wiley/public_html/logpages';
$CHAT_TEXT_FILE = "/home/wiley/lib/i3/i3.allchan.log";
$BACKGROUND_DIR = "/home/wiley/public_html/gfx/wallpaper/";

$PG_DB          = "i3log";
$PG_USERNAME    = "wiley";
$PG_PASSWORD    = "tardis69";

$BEGIN_ICON     = "$URL_HOME/gfx/nav/begin.png";
$PREV_ICON      = "$URL_HOME/gfx/nav/previous.png";
$NEXT_ICON      = "$URL_HOME/gfx/nav/next.png";
$END_ICON       = "$URL_HOME/gfx/nav/end.png";
$UP_ICON        = "$URL_HOME/gfx/nav/green/up.png";
$DOWN_ICON      = "$URL_HOME/gfx/nav/green/down.png";
$TOP_ICON       = "$URL_HOME/gfx/nav/green/top.png";
$BOTTOM_ICON    = "$URL_HOME/gfx/nav/green/bottom.png";

$MUDLIST_ICON   = "$URL_HOME/gfx/mud.png";
$LOG_ICON       = "$URL_HOME/gfx/other_logs.png";
$DISCORD_ICON   = "$URL_HOME/gfx/discord.png";
$SERVER_ICON    = "$URL_HOME/gfx/server_icon.png";
$QR_ICON        = "$URL_HOME/stuff/qrcode.png";
$QR_BIG_ICON    = "$URL_HOME/stuff/qrcode_big.png";
$PIE_ICON       = "$URL_HOME/gfx/pie_chart.png";
$GITHUB_ICON    = "$URL_HOME/gfx/github1600.png";
$ICON_WIDTH     = 48;

$MUDLIST_IMG    = "<img src=\"$MUDLIST_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
$LOG_IMG        = "<img src=\"$LOG_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
$DISCORD_IMG    = "<img src=\"$DISCORD_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
$SERVER_IMG     = "<img src=\"$SERVER_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
$QR_IMG         = "<img src=\"$QR_ICON\" width=\"53\" height=\"53\" border=\"0\" />";
$PIE_IMG        = "<img src=\"$PIE_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";
$GITHUB_IMG     = "<img src=\"$GITHUB_ICON\" width=\"$ICON_WIDTH\" height=\"$ICON_WIDTH\" border=\"0\" />";

$MUDLIST_LINK   = "<a href=\"$URL_HOME/mudlist.php\" alt=\"Mudlist\" title=\"Mudlist\">$MUDLIST_IMG</a>";
$LOG_LINK       = "<a href=\"https://themud.org/chanhist.php#Channel=all\" alt=\"Other Logs\" title=\"Other Logs\">$LOG_IMG</a>";
$DISCORD_LINK   = "<a href=\"https://discord.gg/kUduSsJ\" alt=\"Discord\" title=\"Discord\">$DISCORD_IMG</a>";
$SERVER_LINK    = "<a href=\"$URL_HOME/server.php\" alt=\"Server\" title=\"Server\">$SERVER_IMG</a>";
$QR_LINK        = "<a href=\"$QR_BIG_ICON\" alt=\"リック転がし\" title=\"リック転がし\">$QR_IMG</a>";
$PIE_LINK       = "<a href=\"$URL_HOME/pie.php\" alt=\"PIE!\" title=\"PIE!\">$PIE_IMG</a>";
$GITHUB_LINK    = "<a href=\"https://github.com/quixadhal/wileymud\" alt=\"Source\" title=\"Source\">$GITHUB_IMG</a>";

$OVERLAY_ICON   = "$URL_HOME/gfx/archive_stamp.png";
$OVERLAY_IMG    = "<img class=\"overlay-fixed\" src=\"$OVERLAY_ICON\" />";

$JQUI_CSS       = "$LOG_HOME/jquery/jquery-ui.css";
$JQUI_THEME     = "$LOG_HOME/jquery/jquery-ui.theme.css";
$JQ             = "$LOG_HOME/jquery.js";
$JQUI           = "$LOG_HOME/jquery/jquery-ui.js";
$MOMENT         = "$LOG_HOME/moment.js";
$MOMENT_TZ      = "$LOG_HOME/moment-timezone.js";
$NAVBAR         = "$LOG_HOME/navbar.js";

$PINKFISH_CACHE = "$PAGE_DIR/pinkfish.json";
$CHANNEL_CACHE  = "$PAGE_DIR/channels.json";
$SPEAKER_CACHE  = "$PAGE_DIR/speakers.json";
$DATE_CACHE     = "$PAGE_DIR/date_counts.json";
$HOUR_CACHE     = "$PAGE_DIR/hours.json";

function random_image($dir) {
    $old_dir = getcwd();
    chdir($dir);

    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $file_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($file_list);

    chdir($old_dir);
    return $file_list[$pick];
}

function db_connect() {
    global $DB_FILE;
    global $PG_DB;
    global $PG_USERNAME;
    global $PG_PASSWORD;

    $db = null;
    try {
        //$db = new PDO( "pgsql:host=localhost;port=5432;dbname=$PG_DB;user=$PG_USERNAME;password=$PG_PASSWORD", null, null, array(
        $db = new PDO( "pgsql:dbname=$PG_DB;user=$PG_USERNAME;password=$PG_PASSWORD", null, null, array(
            //PDO::ATTR_PERSISTENT        => true, 
            PDO::ATTR_ERRMODE           => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_EMULATE_PREPARES  => false,
        ));
    } catch(PDOException $e) {
        echo $e->getMessage();
    }
    return $db;
}

function now_date($db) {
    $sql = "SELECT date('now') the_date;";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $row = $sth->fetch();
        $query_date = $row['the_date'];
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $query_date;
}

function yesterday_date($db) {
    $sql = "SELECT date('yesterday') the_date;";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $row = $sth->fetch();
        $query_date = $row['the_date'];
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $query_date;
}

function fetch_pinkfish_map($db) {
    $sql = "
        SELECT pinkfish, html
          FROM pinkfish_map
      ORDER BY LENGTH(pinkfish) DESC;
    ";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[$row['pinkfish']] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    //$json_data = json_encode($result);
    //file_put_contents($PINKFISH_CACHE, $json_data);
    return $result;
}

function reverseSortByKeyLength($a, $b) {
    return strlen($b) - strlen($a);
}

function load_pinkfish_map($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        uksort($data, "reverseSortByKeyLength");
        //print "\nPINKFISH_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

function fetch_channels($db) {
    $sql = "
        SELECT channel, channels.pinkfish, html
          FROM channels
     LEFT JOIN pinkfish_map
            ON (channels.pinkfish = pinkfish_map.pinkfish);
    ";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[$row['channel']] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function load_channels($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        //uksort($data, "reverseSortByKeyLength");
        //print "\nPINKFISH_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

function fetch_speakers($db) {
    $sql = "
        SELECT speaker, speakers.pinkfish, html
          FROM speakers
     LEFT JOIN pinkfish_map
            ON (speakers.pinkfish = pinkfish_map.pinkfish);
    ";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[$row['speaker']] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function load_speakers($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        //uksort($data, "reverseSortByKeyLength");
        //print "\nPINKFISH_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

function fetch_date_counts($db) {
    $sql = "
        SELECT      to_char(local, 'YYYY-MM-DD')  the_date,
                    to_char(local, 'YYYY')        the_year,
                    to_char(local, 'MM')          the_month,
                    to_char(local, 'DD')          the_day,
                    COUNT(*) count
        FROM        i3log
        GROUP BY    the_date, the_year, the_month, the_day
        ORDER BY    the_date ASC;
    ";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[$row['the_date']] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function load_date_counts($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        ksort($data, SORT_NATURAL);
        //print "\nDATE_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

function load_hours($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        //uksort($data, "reverseSortByKeyLength");
        //print "\nPINKFISH_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

function fetch_page_by_date($db, $query_date = NULL) {
    if( $query_date === NULL ) {
        $query_date = now_date($db);
    }
    $sql = "
             SELECT *
               FROM page_view
              WHERE date(local) = :query_date
           ORDER BY local ASC;
    ";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->bindParam(':query_date', $query_date);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function fetch_urls_by_date($db, $query_date = NULL) {
    if( $query_date === NULL ) {
        $query_date = now_date($db);
    }
    $sql = "
             SELECT *
               FROM page_view
              WHERE date(local) = :query_date
                AND (is_url OR speaker = 'URLbot')
           ORDER BY local ASC;
    ";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        $sth->bindParam(':query_date', $query_date);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function file_tail($filepath, $lines = 1, $adaptive = true) {
    $f = @fopen($filepath, "rb");
    if ($f === false) return false;

    // Sets buffer size
    if (!$adaptive) $buffer = 4096;
    else $buffer = ($lines < 2 ? 64 : ($lines < 10 ? 512 : 4096));

    fseek($f, -1, SEEK_END); // Jump to last character

    // Read it and adjust line number if necessary
    // (Otherwise the result would be wrong if file doesn't end with a blank line)
    if (fread($f, 1) != "\n") $lines -= 1;
    
    // Start reading
    $output = '';
    $chunk = '';

    // While we would like more
    while (ftell($f) > 0 && $lines >= 0) {
        $seek = min(ftell($f), $buffer); // Figure out how far back we should jump
        fseek($f, -$seek, SEEK_CUR); // Do the jump (backwards, relative to where we are)
        $output = ($chunk = fread($f, $seek)) . $output; // Read a chunk and prepend it to our output
        fseek($f, -mb_strlen($chunk, '8bit'), SEEK_CUR); // Jump back to where we started reading
        $lines -= substr_count($chunk, "\n"); // Decrease our line counter
    }

    // While we have too many lines
    // (Because of buffer size we might have read too many)
    while ($lines++ < 0) {
        $output = substr($output, strpos($output, "\n") + 1); // Find first newline and remove all text before that
    }

    fclose($f);
    return $output;
}

function load_page_by_date($db, $query_date = NULL, $pinkfish_map, $hours, $channels, $speakers) {
    global $CHAT_TEXT_FILE;

    $result = array();

    if( $query_date === NULL ) {
        $query_date = now_date($db);
    }
    //$file_lines = array_filter(explode( "\n", file_tail( $CHAT_TEXT_FILE, $LINES_TO_READ ) ), "is_bot_line");
    $raw_lines = explode("\n", file_tail($CHAT_TEXT_FILE, 5000));
    $file_lines = array_filter(
        $raw_lines,
        function ($line) use ($query_date) {
            # log line date format: 2006.09.25-06.57,45000
            if(strlen($line) < 10) {
                return false;
            }
            $line_date = substr($line, 0, 10);
            $line_date[4] = '-';
            $line_date[7] = '-';
            return( $line_date == $query_date );
        });
    foreach ($file_lines as $line) {
        $row = array();

        $line_data = explode( "\t", $line );
        if ( sizeof( $line_data ) < 5 ) {
            continue;
        }
        # target format:        2006-09-25 11:57:45.000
        # log line date format: 2006.09.25-06.57,45000
        #                       0          1
        #                       0          1
        $line_data[0][4]  = '-';
        $line_data[0][7]  = '-';
        $line_data[0][10] = ' ';
        $line_data[0][13] = ':';
        $line_data[0][16] = ':';
        $microsecond    = substr($line_data[0], 19, 3);
        $line_data[0]   = substr_replace($line_data[0], "." . $microsecond, 19);
        $the_hour       = substr($line_data[0], 11, 2);
        $int_hour       = (int)$the_hour;

        $who            = explode( "@", $line_data[2] );
        $speaker        = $who[0];
        $mud            = $who[1];

        $hour_color     = $hours[$int_hour]['pinkfish'];
        $channel_color  = $channels[strtolower($line_data[1])]['pinkfish'];
        $speaker_color  = $speakers[strtolower($who[0])]['pinkfish'];

        $row = [
            'local'         => $line_data[0],
            'is_emote'      => ($line_data[3] == "t") ? 1 : 0,
            'is_url'        => null,
            'is_bot'        => null,
            'channel'       => $line_data[1],
            'speaker'       => $who[0],
            'mud'           => $who[1],
            'message'       => implode("\t", array_slice($line_data, 4)),
            'the_date'      => substr($line_data[0], 0, 10),
            'the_time'      => substr($line_data[0], 11, 8),
            'the_year'      => substr($line_data[0], 0, 4),
            'the_month'     => substr($line_data[0], 5, 2),
            'the_day'       => substr($line_data[0], 8, 2),
            'the_hour'      => $the_hour,
            'the_minute'    => substr($line_data[0], 14, 2),
            'the_second'    => substr($line_data[0], 17, 2),
            'int_hour'      => (int)$the_hour,
            'hour_color'    => $hour_color,
            'channel_color' => $channel_color,
            'speaker_color' => $speaker_color,
            'hour_html'     => $pinkfish_map[$hour_color]['html'],
            'channel_html'  => $pinkfish_map[$channel_color]['html'],
            'speaker_html'  => $pinkfish_map[$speaker_color]['html']
        ];
        $result[] = $row;
    }
    return $result;
}

function page_url($page_counts, $day) {
    global $LOG_HOME;

    $pathname = sprintf("%s/%s/%s/%s.html", $LOG_HOME,
        $page_counts[$day]['the_year'],
        $page_counts[$day]['the_month'],
        $page_counts[$day]['the_date']);
    return $pathname;
}

function page_path($page_counts, $day) {
    global $PAGE_DIR;

    $pathname = sprintf("%s/%s/%s/%s.html", $PAGE_DIR,
        $page_counts[$day]['the_year'],
        $page_counts[$day]['the_month'],
        $page_counts[$day]['the_date']);
    return $pathname;
}

function handle_colors( $pinkfish_map, $message ) {
    foreach ($pinkfish_map as $pf) {
        $pattern = '/'.preg_quote($pf['pinkfish']).'/';
        $repl = $pf['html'];
        $message = preg_replace($pattern, $repl, $message);
    }
    return $message;
}

$pinkfish_map = load_pinkfish_map($PINKFISH_CACHE);
$channels = load_channels($CHANNEL_CACHE);
$speakers = load_speakers($SPEAKER_CACHE);
$hours = load_hours($HOUR_CACHE);

//$pinkfish_map = fetch_pinkfish_map($db);
//$channels = fetch_channels($db);
//$speakers = fetch_speakers($db);

$db = db_connect();

$this_day = now_date($db);
$the_other_day = yesterday_date($db);
$date_counts = load_date_counts($DATE_CACHE);
// Normally, this is all good, however if there have been no messages
// yet today, it might be that there really were no messages OR that
// the cache is out of date, so let's grab it from the database to
// be sure...
$today      = $date_counts[array_key_last($date_counts)]['the_date'];
if( $this_day !== $today  && $the_other_day !== $today ) {
    // If we have yesterday, just use that, otherwise assume
    // the cache isn't valid.
    $date_counts = fetch_date_counts($db);
    $json_data = json_encode($date_counts);
    file_put_contents($DATE_CACHE, $json_data);
}

$date_keys = array_keys($date_counts);
$first_date = $date_counts[array_key_first($date_counts)]['the_date'];
$today      = $date_counts[array_key_last($date_counts)]['the_date'];
$yesterday  = $date_counts[$date_keys[array_search($today, $date_keys)-1]]['the_date'];

// A link to the very top
$top_link = sprintf("<img id=\"scroll_top_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" onclick=\"scroll_top()\" />\n",
    $TOP_ICON, $ICON_WIDTH, $ICON_WIDTH);
// A shiny new scroll up button
$up_link = sprintf("<img id=\"scroll_up_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" onclick=\"scroll_up()\" />\n",
    $UP_ICON, $ICON_WIDTH, $ICON_WIDTH);
// Navigation for going back
$first_link = sprintf("<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" /></a>\n",
    page_url($date_counts, $first_date), $first_date, $first_date, $BEGIN_ICON, $ICON_WIDTH, $ICON_WIDTH);
$prev_link = sprintf("<a href=\"%s\" alt=\"%s\" title=\"%s\"><img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" /></a>\n",
    page_url($date_counts, $yesterday), $yesterday, $yesterday, $PREV_ICON, $ICON_WIDTH, $ICON_WIDTH);
// but there is no going forward
$next_link = sprintf("<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" />\n",
    $NEXT_ICON, $ICON_WIDTH, $ICON_WIDTH);
$last_link = sprintf("<img src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 0.5;\" />\n",
    $END_ICON, $ICON_WIDTH, $ICON_WIDTH);
// and finally, our new button to scroll down to a row
$down_link = sprintf("<img id=\"scroll_down_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 1.0;\" onclick=\"scroll_down()\" />\n",
    $DOWN_ICON, $ICON_WIDTH, $ICON_WIDTH);
// and for real, finally, a jump to bottom button
$bottom_link = sprintf("<img id=\"scroll_bottom_button\" src=\"%s\" width=\"%d\" height=\"%d\" border=\"0\" style=\"opacity: 1.0;\" onclick=\"scroll_bottom()\" />\n",
    $BOTTOM_ICON, $ICON_WIDTH, $ICON_WIDTH);

$page = $URL_ONLY ? fetch_urls_by_date($db, $today) : fetch_page_by_date($db, $today);
//$page = load_page_by_date($db, $today, $pinkfish_map, $hours, $channels, $speakers);

$local_refresh = strftime('%H:%M %Z');
$local_hour = (int)substr($local_refresh, 0, 2);
$local_time = $pinkfish_map[ $hours[$local_hour]['pinkfish'] ]['html'] . $local_refresh . "</span>";

$db = null;

$BACKGROUND     = random_image($BACKGROUND_DIR);
$BACKGROUND_IMG = "<img class=\"overlay-bg\" src=\"$URL_HOME/gfx/wallpaper/$BACKGROUND\" />";

header("Cache-Control: no-cache");
header("Pragma: no-cache");

?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <meta http-equiv="refresh" content="1800" />
        <!-- Global site tag (gtag.js) - Google Analytics -->
        <script async src="https://www.googletagmanager.com/gtag/js?id=UA-163395867-1"></script>
        <script>
          window.dataLayer = window.dataLayer || [];
          function gtag(){dataLayer.push(arguments);}
          gtag('js', new Date());

          gtag('config', 'UA-163395867-1');
        </script>
        <link rel="stylesheet" href="<?php echo $JQUI_CSS;?>">
        <link rel="stylesheet" href="<?php echo $JQUI_THEME;?>">
        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JQUI;?>""></script>
        <script src="<?php echo $MOMENT;?>""></script>
        <script src="<?php echo $MOMENT_TZ;?>""></script>
        <script src="<?php echo $NAVBAR;?>"></script>

        <script language="javascript">
            function debug_local_time() {
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

                var momentObj = moment().tz(yourTimeZone);
                var momentStr = momentObj.format('YYYY-MM-DD HH:mm:ss z');
                var momentHour = momentObj.hour();

                var yt = document.getElementById("yourTime");
                yt.innerHTML = "[" + yourLocale + " " + yourTimeZone + "] " + momentStr;
                yt.style.color = hour_map[momentHour];
            }

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

                var rows = $(`[id^=row_]`);
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
                debug_local_time();
                localize_rows();
                // This if for the live page ONLY, pointless for static pages.
                scroll_bottom();
                setTimeout(function () { location.reload(true); }, 30 * 60 * 1000);
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
            #navbar { position: fixed; top: 0; z-index: 2; height: 58px; background-color: black; }
            #content-header { position: fixed; top: 58px; z-index: 1; width: 100%; background-color: black; }
            #content { padding-top: 48px; }
            .overlay-fixed { position: fixed; top: 48px; left: 0px; width: 100%; height: 100%; z-index: 999; opacity: 0.3; pointer-events: none; }
            .overlay-bg { position: fixed; top: 81px; z-index: 998; opacity: 0.15; pointer-events: none; object-fit: cover; width: 100%; height: 100%; left: 50%; transform: translateX(-50%); }
            .unblurred { font-family: monospace; white-space: pre-wrap; }
            .blurry:not(:hover) { filter: blur(3px); font-family: monospace; white-space: pre-wrap; }
            .blurry:hover { font-family: monospace; white-space: pre-wrap; }
        </style>
        <style>
            @keyframes blinking {
                0% {
                    opacity: 0;
                }
                49% {
                    opacity: 0;
                }
                50% {
                    opacity: 1;
                }
                100% {
                    opacity: 1;
                }
            }
            .flash_tag {
                animation: blinking 1.5s infinite;
            }
        </style>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="setup();">
        <?php echo $BACKGROUND_IMG; ?>
        <!-- <?php echo $OVERLAY_IMG; ?> -->
        <table id="navbar" width="99%" align="center">
            <tr>
                <td align="left" width="25%">
                    <?php echo $MUDLIST_LINK;?>
                    <?php echo $LOG_LINK;?>
                    <?php echo $PIE_LINK;?>
                </td>
                <td align="right" width="20%">
                    <?php echo $up_link; ?>
                    <?php echo $first_link; ?>
                    <?php echo $prev_link; ?>
                </td>
                <td align="center" width="10%">
                    <input type="text" id="datepicker" size="10" value="<?php echo $today; ?>" style="font-size: 16px; text-align: center;" />
                </td>
                <td align="left" width="20%">
                    <?php echo $next_link; ?>
                    <?php echo $last_link; ?>
                    <?php echo $down_link; ?>
                </td>
                <td align="right" width="25%">
                    <span style="vertical-align: top; color: #ccaa00"> <?php echo $local_time; ?> </span>
                    <?php echo $SERVER_LINK;?>
                    <?php echo $GITHUB_LINK;?>
                    <?php echo $DISCORD_LINK;?>
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
<?php
    $counter = 0;
    foreach ($page as $row) {
        $bg_color     = ($counter % 2) ? "#000000" : "#1F1F1F";

        $hour_html    = $row['hour_html'];
        $channel_html = $row['channel_html'];
        $channel      = $row['channel']; // text only channel name
        if( empty($channel_html) || is_null($channel_html) ) {
            $channel_html = $channels['default']['html'];
        }
        $speaker_html = $row['speaker_html'];
        if( empty($speaker_html) || is_null($speaker_html) ) {
            $speaker_html = $speakers['default']['html'];
        }

        $date_col     = $row['the_date'];
        $time_col     = sprintf("%s%s", $hour_html, $row['the_time']);
        $channel_col  = sprintf("%s%s", $channel_html, $row['channel']);
        $speaker_col  = sprintf("%s%s@%s", $speaker_html, $row['speaker'], $row['mud']);

        $message = $row['message'];
        $message = preg_replace('/^\%\^RESET\%\^/', '', $message);
        $message = htmlentities($message, ENT_QUOTES|ENT_SUBSTITUTE, 'UTF-8');
        $message = handle_colors($pinkfish_map, $message);

        $message = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/i', '<a href="$1" target="I3-link">$1</a>', $message);
        $message = preg_replace('/YouTube\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'YouTube $1 <a href="https://youtu.be/$2" target="I3-link">[$2]</a>', $message);
        $message = preg_replace('/IMDB\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'IMDB $1 <a href="https://www.imdb.com/title/$2/" target="I3-link">[$2]</a>', $message);
        $message = preg_replace('/Steam\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'Steam $1 <a href="http://store.steampowered.com/app/$2/" target="I3-link">[$2]</a>', $message);
        $message = preg_replace('/Dailymotion\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'Dailymotion $1 <a href="https://www.dailymotion.com/video/$2" target="I3-link">[$2]</a>', $message);

        $span_class = "unblurred";
        if($CENSOR) {
            if($channel == "free_speech") {
                $span_class = "blurry";
            } else if(preg_match('/on\sfree_speech\&gt\;/', $message) > 0) {
                $span_class = "blurry";
            }
        }
        $message = "<span class=\"$span_class\">$message</span>";
?>
        <tr id="row_<?php echo $counter;?>" style="display:none">
            <td bgcolor="<?php echo $bg_color;?>"><?php echo $date_col;?></td>
            <td bgcolor="<?php echo $bg_color;?>"><?php echo $time_col;?></span></td>
            <td bgcolor="<?php echo $bg_color;?>"><?php echo $channel_col;?></span></td>
            <td bgcolor="<?php echo $bg_color;?>"><?php echo $speaker_col;?></span></td>
            <td bgcolor="<?php echo $bg_color;?>"><?php echo $message;?></span></td>
        </tr>
<?php
        $counter++;
    }
    $time_end = microtime(true); $time_spent = $time_end - $time_start;
    $bg_color = ($counter % 2) ? "#000000" : "#1F1F1F";
?>
        <tr id="row_<?php echo $counter;?>" style="display:none">
            <td colspan="4" align="right" bgcolor="<?php echo $bg_color;?>"><span id="yourTime" style="color: #1F1F1F; display:none">&nbsp;</span></td>
            <td align="right" bgcolor="<?php echo $bg_color;?>"><?php printf("%7.3f seconds",$time_spent);?></td>
        </tr>
            </tbody>
        </table>
    </body>
</html>
