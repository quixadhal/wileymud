<?php
global $time_start;
$time_start = microtime(true);

global $isLocal;
global $isMobile;
global $pinkfishMap;
global $hourColors;
global $channelColors;
global $colorMap;
global $CHAT_COLOR_FILE;
global $CHAT_TEXT_FILE;
global $CHAT_DATA_FILE;
global $file_lines;
global $bg;
global $dump;
global $CHANNEL_ONLY;

$CHAT_COLOR_FILE    = "/home/bloodlines/lib/secure/save/chat.o";
$CHAT_TEXT_FILE     = "/home/wiley/lib/i3/i3.allchan.log";
$CHAT_DATA_FILE     = "/home/wiley/lib/i3/i3.speakers.json";

$LINES_TO_READ      = 2500;
$PAGE_SIZE          = 25;
$URL_ONLY           = 0;
$CHANNEL_ONLY       = 0;

//header("Location: http://wiley.the-firebird.net/~wiley/i3log.php");
//header("Location: http://wileymud.i3.themud.org/~wiley/i3log.php");
//header("Location: http://wileymud.lpmud.org/~wiley/i3log.php");
$DESKTOP_URL        = "http://wileymud.i3.themud.org/~wiley/i3log.php";
$MOBILE_URL         = "http://wileymud.i3.themud.org/~wiley/i3log_m.php";

function is_mobile() {
    global $_REQUEST;
    global $_SERVER;

    if( preg_match(
        "/(android|avantgo|blackberry|bolt|boost|cricket|docomo|fone|hiptop|mini|mobi|palm|phone|pie|tablet|up\.browser|up\.link|webos|wos)/i",
        $_SERVER["HTTP_USER_AGENT"]) ) {
        //echo "<hr><h1>Found Match</h1><hr><br>";
        return true;
    }
    //echo "<hr><h1>UA: \"".$_SERVER["HTTP_USER_AGENT"]."\"</h1><hr><br>";
    //echo "<hr><h1>No match</h1><hr><br>";
    return false;
}

$isMobile = is_mobile();
if( isset($_REQUEST) && isset($_REQUEST["mobile"]) ) {
    $isMobile = true;
} else if( isset($_REQUEST) && isset($_REQUEST["desktop"]) ) {
    $isMobile = false;
}

//if( $isMobile ) {
//    header("Location: $MOBILE_URL");
//} else {
//    header("Location: $DESKTOP_URL");
//}

if( isset($_REQUEST) && isset($_REQUEST["url"]) ) {
    $URL_ONLY = 1;
} else if( isset($_REQUEST) && isset($_REQUEST["channel"]) ) {
    $CHANNEL_ONLY = $_REQUEST["channel"];
}

function numbered_source($filename)
{
    $lines = implode(range(1, count(file($filename))), '<br />');
    $content = highlight_file($filename, true);
    $style = '
    <style type="text/css"> 
        .num { 
        float: left; 
        color: gray; 
        font-size: 13px;    
        font-family: monospace; 
        text-align: right; 
        margin-right: 6pt; 
        padding-right: 6pt; 
        border-right: 1px solid gray;} 

        body {margin: 0px; margin-left: 5px;} 
        td {vertical-align: top; white-space: normal;} 
        code {white-space: nowrap;} 
    </style>
    '; 
    return "$style\n<table><tr><td class=\"num\">\n$lines\n</td><td>\n$content\n</td></tr></table>"; 
}

function init_pinkfish_map() {
    global $pinkfishMap;

    $pinkfishMap = array(
        '%^RESET%^'                 => '',                              // 00

        '%^BLACK%^'                 => '<SPAN style="color: #000000">',
        '%^RED%^'                   => '<SPAN style="color: #bb0000">',
        '%^GREEN%^'                 => '<SPAN style="color: #00bb00">',
        '%^ORANGE%^'                => '<SPAN style="color: #bbbb00">',
        '%^BLUE%^'                  => '<SPAN style="color: #0000bb">',
        '%^MAGENTA%^'               => '<SPAN style="color: #bb00bb">',
        '%^CYAN%^'                  => '<SPAN style="color: #00bbbb">',
        '%^GREY%^'                  => '<SPAN style="color: #bbbbbb">',
        '%^DARKGREY%^'              => '<SPAN style="color: #555555">',
        '%^LIGHTRED%^'              => '<SPAN style="color: #ff5555">', // 10
        '%^LIGHTGREEN%^'            => '<SPAN style="color: #55ff55">',
        '%^YELLOW%^'                => '<SPAN style="color: #ffff55">',
        '%^LIGHTBLUE%^'             => '<SPAN style="color: #5555ff">',
        '%^PINK%^'                  => '<SPAN style="color: #ff55ff">',
        '%^LIGHTMAGENTA%^'          => '<SPAN style="color: #ff55ff">',
        '%^LIGHTCYAN%^'             => '<SPAN style="color: #55ffff">',
        '%^WHITE%^'                 => '<SPAN style="color: #ffffff">',

        '%^B_RED%^%^WHITE%^'        => '<SPAN style="background-color: #bb0000; color: #ffffff">',
        '%^B_GREEN%^%^WHITE%^'      => '<SPAN style="background-color: #00bb00; color: #ffffff">',
        '%^B_BLUE%^%^WHITE%^'       => '<SPAN style="background-color: #0000bb; color: #ffffff">', // 20
        '%^B_MAGENTA%^%^WHITE%^'    => '<SPAN style="background-color: #bb00bb; color: #ffffff">',

        '%^B_RED%^%^YELLOW%^'       => '<SPAN style="background-color: #bb0000; color: #ffff55">',
        '%^B_GREEN%^%^YELLOW%^'     => '<SPAN style="background-color: #00bb00; color: #ffff55">',
        '%^B_BLUE%^%^YELLOW%^'      => '<SPAN style="background-color: #0000bb; color: #ffff55">',
        '%^B_MAGENTA%^%^YELLOW%^'   => '<SPAN style="background-color: #bb00bb; color: #ffff55">',

        '%^B_RED%^%^BLACK%^'        => '<SPAN style="background-color: #bb0000; color: #000000">',
        '%^B_GREEN%^%^BLACK%^'      => '<SPAN style="background-color: #00bb00; color: #000000">',
        '%^B_MAGENTA%^%^BLACK%^'    => '<SPAN style="background-color: #bb00bb; color: #000000">',
        '%^B_CYAN%^%^BLACK%^'       => '<SPAN style="background-color: #00bbbb; color: #000000">',
        '%^B_YELLOW%^%^BLACK%^'     => '<SPAN style="background-color: #ffff55; color: #000000">', // 30
        '%^B_WHITE%^%^BLACK%^'      => '<SPAN style="background-color: #ffffff; color: #000000">',

        '%^B_CYAN%^%^BLUE%^'        => '<SPAN style="background-color: #00bbbb; color: #0000bb">',
        '%^B_YELLOW%^%^BLUE%^'      => '<SPAN style="background-color: #ffff55; color: #0000bb">',
        '%^B_WHITE%^%^BLUE%^'       => '<SPAN style="background-color: #ffffff; color: #0000bb">',

        '%^B_YELLOW%^%^GREEN%^'     => '<SPAN style="background-color: #ffff55; color: #00bb00">',
        '%^B_WHITE%^%^GREEN%^'      => '<SPAN style="background-color: #ffffff; color: #00bb00">',

        // Cheating...
        '%^FLASH%^%^LIGHTGREEN%^'   => '<SPAN style="color: #55ff55">',

        // In this case, our web page background is black, so pretend there is none.
        // That's because on a terminal, it just resets the character cells and looks fine, but
        // in HTML, it makes the background stick out if the table cell isn't also black.
        //'%^B_BLACK%^%^GREY%^'       => '<SPAN style="background-color: #000000; color: #bbbbbb">',
        //'%^B_BLACK%^%^WHITE%^'      => '<SPAN style="background-color: #000000; color: #ffffff">',
        '%^B_BLACK%^%^GREY%^'       => '<SPAN style="color: #bbbbbb">',
        '%^B_BLACK%^%^WHITE%^'      => '<SPAN style="color: #ffffff">',     // 40

        '%^B_RED%^%^GREEN%^'        => '<SPAN style="background-color: #bb0000; color: #00bb00">',
        '%^B_RED%^%^LIGHTGREEN%^'   => '<SPAN style="background-color: #bb0000; color: #00ff00">'
    );
}

function get_hour_colors() {
    global $pinkfishMap;

    $colors = array(
        "00" => "%^DARKGREY%^",
        "01" => "%^DARKGREY%^",
        "02" => "%^DARKGREY%^",
        "03" => "%^DARKGREY%^",
        "04" => "%^RED%^",
        "05" => "%^RED%^",
        "06" => "%^ORANGE%^",
        "07" => "%^ORANGE%^",
        "08" => "%^YELLOW%^",
        "09" => "%^YELLOW%^",
        "10" => "%^GREEN%^",
        "11" => "%^GREEN%^",
        "12" => "%^LIGHTGREEN%^",
        "13" => "%^LIGHTGREEN%^",
        "14" => "%^WHITE%^",
        "15" => "%^WHITE%^",
        "16" => "%^LIGHTCYAN%^",
        "17" => "%^LIGHTCYAN%^",
        "18" => "%^CYAN%^",
        "19" => "%^CYAN%^",
        "20" => "%^LIGHTBLUE%^",
        "21" => "%^LIGHTBLUE%^",
        "22" => "%^BLUE%^",
        "23" => "%^BLUE%^"
    );

    foreach ($colors as $k => $v) {
        if( array_key_exists( $v, $pinkfishMap )) {
            $colors[$k] = $pinkfishMap[$v];
        }
    }
    return $colors;
}

function get_channel_colors() {
    global $pinkfishMap;

    $colors = array(
        "intermud"    => "%^B_BLACK%^%^GREY%^",
        "muds"        => "%^B_BLACK%^%^GREY%^",
        "connections" => "%^B_BLACK%^%^WHITE%^",
        "death"       => "%^LIGHTRED%^",
        "cre"         => "%^LIGHTGREEN%^",
        "admin"       => "%^LIGHTMAGENTA%^",
        "newbie"      => "%^B_YELLOW%^%^BLACK%^",
        "gossip"      => "%^B_BLUE%^%^YELLOW%^",

        "wiley"       => "%^YELLOW%^",
        "ds"          => "%^YELLOW%^",
        "dchat"	      => "%^CYAN%^",
        "intergossip" => "%^GREEN%^",
        "intercre"    => "%^ORANGE%^",
        "pyom"        => "%^FLASH%^%^LIGHTGREEN%^",
        "free_speech" => "%^PINK%^",
        "url"         => "%^WHITE%^",
        "dwchat"      => "%^LIGHTBLUE%^",
        "dgd"         => "%^MAGENTA%^",
        "wileymud"    => "%^FLASH%^%^CYAN%^",

        "ibuild"      => "%^B_RED%^%^YELLOW%^",
        "ichat"       => "%^B_RED%^%^GREEN%^",
        "mbchat"      => "%^B_RED%^%^GREEN%^",
        "pchat"       => "%^B_RED%^%^LIGHTGREEN%^",
        "i2game"      => "%^B_BLUE%^",
        "i2chat"      => "%^B_GREEN%^",
        "i3chat"      => "%^B_RED%^",
        "i2code"      => "%^B_YELLOW%^%^RED%^",
        "i2news"      => "%^B_YELLOW%^%^BLUE%^",
        "imudnews"    => "%^B_YELLOW%^%^CYAN%^",
        "irc"         => "%^B_BLUE%^%^GREEN%^",
        "ifree"       => "%^B_BLUE%^%^GREEN%^",

        "default"      => "%^LIGHTBLUE%^",
        "default-IMC2" => "%^B_BLUE%^%^WHITE%^"
    );

    foreach ($colors as $k => $v) {
        if( array_key_exists( $v, $pinkfishMap )) {
            $colors[$k] = $pinkfishMap[$v];
        }
    }
    return $colors;
}

function get_lpc_chatter_colors() {
    global $pinkfishMap;
    global $CHAT_COLOR_FILE;
    global $CHAT_DATA_FILE;
    global $dump;

    $colors = array ();

    $text = file_get_contents( $CHAT_COLOR_FILE );
    $lines = explode("\n", $text);
    $line = $lines[1]; // Stores all on one long line...
    $line = substr($line, 11, -3);
    $mapping = explode(",", $line);
    $colors = array();
    $colors['default'] = $pinkfishMap["%^GREY%^"];
    for($i = 0; $i < sizeof($mapping); $i++ ) {
        $map = explode(":", $mapping[$i]);
        if(sizeof($map) < 2) {
            continue;
        }
        $mapname = substr($map[0], 1, -1); // Strip quotes
        $mapcolor = substr($map[1], 1, -1); // Strip quotes
        if(array_key_exists($mapcolor, $pinkfishMap)) {
            $checking = array();
            $checking[$mapname] = $pinkfishMap[$mapcolor];
            $check = json_encode( $checking, JSON_PRETTY_PRINT );
            if( json_last_error() != JSON_ERROR_NONE ) {
                //echo "<hr>".json_last_error_msg()."<br>";
                //echo "Mapname $i is: \"$mapname\"<br>";
                //echo "Mapcolor $i is: \"$mapcolor\"<br><hr>";
            } else {
                $colors[$mapname] = $pinkfishMap[$mapcolor];
            }
        }
        //echo "colors[$mapname] = ".htmlentities($colors[$mapname])."<br>";
    }
    $colors['default'] = $pinkfishMap["%^GREY%^"];

    $dump = json_encode( $colors, JSON_PRETTY_PRINT );
    if( json_last_error() != JSON_ERROR_NONE ) {
        echo "<hr>".json_last_error_msg()."<br><hr>";
    } else {
        file_put_contents($CHAT_DATA_FILE, $dump);
    }
    //echo "<hr>".count($colors)." colors<br><hr>";
    return $colors;
}

function get_json_chatter_colors() {
    global $CHAT_DATA_FILE;
    global $dump;

    if( file_exists( $CHAT_DATA_FILE ) ) {
        $colors = array();
        $dump = file_get_contents( $CHAT_DATA_FILE );
        $colors = json_decode( $dump, true );
        if( json_last_error() != JSON_ERROR_NONE ) {
            echo "<hr>".json_last_error_msg()."<br>";
        }
        //echo "<hr>".count($colors)." colors<br><hr>";
        return $colors;
    } else {
        return get_lpc_chatter_colors();
    }
}

function get_chatter_colors() {
    return get_json_chatter_colors();
    //return get_lpc_chatter_colors();
}

function is_local_ip() {
    $visitor_ip = $_SERVER['REMOTE_ADDR'];
    $varr = explode(".", $visitor_ip);
    if($varr[0] == "192" && $varr[1] == "168")
        return 1;
    return 0;
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

// This needs to become javascript enhanced to move the time to the local browser zone
// var d = new Date();
// var tz_offset = d.getTimezoneOffset();
//
//number of minutes from GMT
//
function get_time_color($time) {
    global $hourColors;
    global $isMobile;

    $hour = substr( $time, 11, 2 );
    $min  = substr( $time, 14, 2 );
    $sec  = substr( $time, 17, 2 );
    if( ! $isMobile ) {
        $timestr = "$hour:$min.$sec";
    } else {
        $timestr = "$hour:$min";
    }
    $fancyLad = $hourColors[$hour] . $timestr . "</SPAN>";
    return $fancyLad;
}

function get_channel_color($channel) {
    global $channelColors;

    $chanLow = strtolower($channel);

    $channelColor = $channelColors["default"];
    if( array_key_exists( $chanLow, $channelColors )) {
        $channelColor = $channelColors[$chanLow];
    }
    $fancyLad = "$channelColor" . $channel . "</SPAN>";
    return $fancyLad;
}

function get_speaker_color($who, $where) {
    global $pinkfishMap;
    global $colorMap;
    global $CHAT_DATA_FILE;
    global $dump;
    global $isMobile;

    $who = strip_colors($who);
    $who = preg_replace("/Kreezxil/", "The Annoying Kreezxil", $who);

    $whoLow = strtolower($who);
    $speakerColor = $colorMap["default"];
    if( array_key_exists( $whoLow, $colorMap )) {
        $speakerColor = $colorMap[$whoLow];
    } else {
        // Add a new entry!
        $chatCounter = count( $colorMap ) - 1;
        $colorIndex = $chatCounter % count( $pinkfishMap );
        $colorKey = array_keys($pinkfishMap)[$colorIndex];
        if( $colorKey == '%^RESET%^' ) {
            $colorIndex++;
            $colorKey = array_keys($pinkfishMap)[$colorIndex];
        }
        if( $colorKey == '%^BLACK%^' ) {
            $colorIndex++;
            $colorKey = array_keys($pinkfishMap)[$colorIndex];
        }
        if( $colorKey == '%^DARKGREY%^' ) {
            $colorIndex++;
            $colorKey = array_keys($pinkfishMap)[$colorIndex];
        }
        $speakerColor = $pinkfishMap[$colorKey];
        $colorMap[$whoLow] = $speakerColor;
        $dump = json_encode( $colorMap, JSON_PRETTY_PRINT );
        if( json_last_error() != JSON_ERROR_NONE ) {
            echo "<hr>".json_last_error_msg()."<br>";
            echo "whoLow $chatCounter is: \"$whoLow\"<br>";
        } else {
            file_put_contents($CHAT_DATA_FILE, $dump);
        }
    }
    if( ! $isMobile ) {
        $fancyLad = "$speakerColor" . $who . "@" . $where . "</SPAN>";
    } else {
        $who_bits = array();
        $who_bits = explode(" ", $who);
        $who = $who_bits[sizeof($who_bits) - 1];
        
        $fancyLad = "$speakerColor" . $who . "</SPAN>";
    }
    return $fancyLad;
}

function get_background_color($channel, $counter) {
    global $isMobile;

    if( $isMobile ) {
        $hi_colors = array(
            "default"       => "#2F2F2F",
            "wiley"         => "#3F2F00",
            "ds"            => "#3F3F00",
            "dchat"         => "#003F3F",
            "intergossip"   => "#003F00",
            "intercre"      => "#3F2F00",
            "free_speech"   => "#3F0000",
            "dwchat"        => "#00003F",
            "dgd"           => "#2F003F",
        );
    } else {
        $hi_colors = array(
            "default"       => "#1F1F1F",
            "wiley"         => "#2F1F00",
            "ds"            => "#2F1F00",
            "dchat"         => "#002F2F",
            "intergossip"   => "#002F00",
            "intercre"      => "#2F1F00",
            "free_speech"   => "#2F0000",
            "dwchat"        => "#00002F",
            "dgd"           => "#1F002F",
        );
    }

    if( $isMobile ) {
        $lo_colors = array(
            "default"       => "#000000",
            "wiley"         => "#1F0F00",
            "ds"            => "#1F1F00",
            "dchat"         => "#001F1F",
            "intergossip"   => "#001F00",
            "intercre"      => "#1F0F00",
            "free_speech"   => "#1F0000",
            "dwchat"        => "#00001F",
            "dgd"           => "#0F001F",
        );
    } else {
        $lo_colors = array(
            "default"       => "#000000",
            "wiley"         => "#0F0700",
            "ds"            => "#0F0F00",
            "dchat"         => "#000F0F",
            "intergossip"   => "#000F00",
            "intercre"      => "#0F0700",
            "free_speech"   => "#0F0000",
            "dwchat"        => "#00000F",
            "dgd"           => "#07000F",
        );
    }

    if( $isMobile ) {
        if( $counter % 2 ) {
            $choice = array_key_exists( $channel, $lo_colors) ? $lo_colors[$channel] : $lo_colors["default"];
        } else {
            $choice = array_key_exists( $channel, $hi_colors) ? $hi_colors[$channel] : $hi_colors["default"];
        }
    } else {
        //echo "<br>Channel: $channel, Counter: $counter, Choice: $choice<br>";
        $choice = ($counter % 2) ? "#000000" : "#1F1F1F";
    }

    return $choice;
}

function is_bot_line($line) {
    $result = false;
    $result = preg_match("/\tCron\@WileyMUD.?\t.*It\'s\s+ALIVE\!/", $line) ? $line : false;
    if( $result ) {
        return $result;
    }
    $result = preg_match("/\tCron\@WileyMUD.?\t.*It\'s\s+going\s+DOWN\!/", $line) ? $line : false;
    if( $result ) {
        return $result;
    }
    $result = preg_match("/\tCron\@WileyMUD.?\t.*Not connected to I3/", $line) ? $line : false;
    if( $result ) {
        return $result;
    }
    $result = preg_match("/\tCron\@WileyMUD.?\t.*Version/", $line) ? $line : false;
    if( $result ) {
        return $result;
    }
    $result = preg_match("/\twiley\tCron\@WileyMUD.?\t/", $line);
    if ($result) {
        return false;
    }
    $result = preg_match("/\twiley\tfortune\@lpcdb\-dev\t/", $line);
    if ($result) {
        return false;
    }
    return $line;
}

function is_urlbot_line($line) {
    $result = false;
    $result = preg_match("/\turl\tURLbot\@WileyMUD.?\t/", $line);
    if ($result) {
        return $line;
    }
    return false;
}

function is_specific_channel_line( $line ) {
    global $CHANNEL_ONLY;

    $result = false;
    $result = preg_match("/\t$CHANNEL_ONLY\t/", $line);
    if ($result) {
        return $line;
    }
    return false;
}

function strip_colors( $message ) {
    $message = preg_replace("/\%\^RED\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^RED\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^RED\%\^/", "", $message);

    $message = preg_replace("/\%\^ORANGE\%\^/", "", $message);
    $message = preg_replace("/\%\^YELLOW\%\^/", "", $message);

    $message = preg_replace("/\%\^GREEN\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^GREEN\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^GREEN\%\^/", "", $message);

    $message = preg_replace("/\%\^WHITE\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^WHITE\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^WHITE\%\^/", "", $message);

    $message = preg_replace("/\%\^CYAN\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^CYAN\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^CYAN\%\^/", "", $message);

    $message = preg_replace("/\%\^BLUE\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^BLUE\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^BLUE\%\^/", "", $message);

    $message = preg_replace("/\%\^BLACK\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^BLACK\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^BLACK\%\^/", "", $message);

    $message = preg_replace("/\%\^MAGENTA\%\^\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^MAGENTA\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^MAGENTA\%\^/", "", $message);


    $message = preg_replace("/\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^BOLD/", "", $message); // Silly Pinkfish uses delimiters, rather than full tokens, sometimes.
    $message = preg_replace("/\%\^FLASH\%\^/", "", $message);
    $message = preg_replace("/\%\^FLASH/", "", $message); // Silly Pinkfish uses delimiters, rather than full tokens, sometimes.
    $message = preg_replace("/\%\^RESET\%\^/", "", $message);
    return $message;
}

function handle_colors( $message ) {
    $message = preg_replace("/\%\^RED\%\^\%\^BOLD\%\^/", "<span style=\"color: #ff5555;\">", $message);
    $message = preg_replace("/\%\^RED\%\^BOLD\%\^/", "<span style=\"color: #ff5555;\">", $message);
    $message = preg_replace("/\%\^RED\%\^/", "<span style=\"color: #bb0000;\">", $message);

    $message = preg_replace("/\%\^ORANGE\%\^/", "<span style=\"color: #bbbb00;\">", $message);
    $message = preg_replace("/\%\^YELLOW\%\^/", "<span style=\"color: #ffff55;\">", $message);

    $message = preg_replace("/\%\^GREEN\%\^\%\^BOLD\%\^/", "<span style=\"color: #55ff55;\">", $message);
    $message = preg_replace("/\%\^GREEN\%\^BOLD\%\^/", "<span style=\"color: #55ff55;\">", $message);
    $message = preg_replace("/\%\^GREEN\%\^/", "<span style=\"color: #00bb00;\">", $message);

    $message = preg_replace("/\%\^WHITE\%\^\%\^BOLD\%\^/", "<span style=\"color: #ffffff;\">", $message);
    $message = preg_replace("/\%\^WHITE\%\^BOLD\%\^/", "<span style=\"color: #ffffff;\">", $message);
    $message = preg_replace("/\%\^WHITE\%\^/", "<span style=\"color: #bbbbbb;\">", $message);

    $message = preg_replace("/\%\^CYAN\%\^\%\^BOLD\%\^/", "<span style=\"color: #55ffff;\">", $message);
    $message = preg_replace("/\%\^CYAN\%\^BOLD\%\^/", "<span style=\"color: #55ffff;\">", $message);
    $message = preg_replace("/\%\^CYAN\%\^/", "<span style=\"color: #00bbbb;\">", $message);

    $message = preg_replace("/\%\^BLUE\%\^\%\^BOLD\%\^/", "<span style=\"color: #5555ff;\">", $message);
    $message = preg_replace("/\%\^BLUE\%\^BOLD\%\^/", "<span style=\"color: #5555ff;\">", $message);
    $message = preg_replace("/\%\^BLUE\%\^/", "<span style=\"color: #0000bb;\">", $message);

    $message = preg_replace("/\%\^BLACK\%\^\%\^BOLD\%\^/", "<span style=\"color: #bbbbbb;\">", $message);
    $message = preg_replace("/\%\^BLACK\%\^BOLD\%\^/", "<span style=\"color: #bbbbbb;\">", $message);
    $message = preg_replace("/\%\^BLACK\%\^/", "<span style=\"color: #555555;\">", $message);

    $message = preg_replace("/\%\^MAGENTA\%\^\%\^BOLD\%\^/", "<span style=\"color: #bb00bb;\">", $message);
    $message = preg_replace("/\%\^MAGENTA\%\^BOLD\%\^/", "<span style=\"color: #ff55ff;\">", $message);
    $message = preg_replace("/\%\^MAGENTA\%\^/", "<span style=\"color: #ff55ff;\">", $message);


    $message = preg_replace("/\%\^BOLD\%\^/", "", $message);
    $message = preg_replace("/\%\^BOLD/", "", $message); // Silly Pinkfish uses delimiters, rather than full tokens, sometimes.
    $message = preg_replace("/\%\^FLASH\%\^/", "", $message);
    $message = preg_replace("/\%\^FLASH/", "", $message); // Silly Pinkfish uses delimiters, rather than full tokens, sometimes.
    $message = preg_replace("/\%\^RESET\%\^/", "</span>", $message);
    return $message;
}

$isLocal = is_local_ip();

init_pinkfish_map();
$hourColors = get_hour_colors();
$channelColors = get_channel_colors();
$colorMap = get_chatter_colors();

//2016.04.13-06.14,33000	intergossip	Cratylus@Dead Souls Dev	the problem with listening to music in the client office

//$file_lines = explode( "\n", file_get_contents( $CHAT_TEXT_FILE ) );
//$file_lines = explode( "\n", file_tail( $CHAT_TEXT_FILE, 2500 ) );
if( $URL_ONLY == 1 ) {
    $file_lines = array_filter(explode( "\n", file_tail( $CHAT_TEXT_FILE, $LINES_TO_READ * 20 ) ), "is_urlbot_line");
    //$file_lines = preg_grep("/URLbot\@WileyMUD/", $file_lines);
} else if( $CHANNEL_ONLY ) {
    $file_lines = array_filter(explode( "\n", file_tail( $CHAT_TEXT_FILE, $LINES_TO_READ * 20 ) ), "is_specific_channel_line");
} else {
    $file_lines = array_filter(explode( "\n", file_tail( $CHAT_TEXT_FILE, $LINES_TO_READ ) ), "is_bot_line");
}

$bg = 0;
?>
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
            var windowlen = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
            var current = 0;
            var pagesize = Math.max(parseInt(windowlen / 36), 10);
            var count = 0;
            var loops = 0;
            var refreshMe;

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
            function page_forward() {
                for(var i = current; i < current + pagesize; i++) {
                    row_off(i);
                }
                if (current + pagesize < count) {
                    current = current + pagesize;
                    document.getElementById("current_button").value=Math.trunc(current/pagesize)+1;
                }
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
            }
            function page_forward_ten() {
                for(var i = current; i < current + pagesize; i++) {
                    row_off(i);
                }
                if (current + (pagesize * 10) < count) {
                    current = current + (pagesize * 10);
                    document.getElementById("current_button").value=Math.trunc(current/pagesize)+1;
                }
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
            }
            function page_backward() {
                for(var i = current; i < current + pagesize; i++) {
                    row_off(i);
                }
                if (current - pagesize >= 0) {
                    current = current - pagesize;
                } else {
                    current = 0;
                }
                document.getElementById("current_button").value=Math.trunc(current/pagesize)+1;
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
            }
            function page_backward_ten() {
                for(var i = current; i < current + pagesize; i++) {
                    row_off(i);
                }
                if (current - (pagesize * 10) >= 0) {
                    current = current - (pagesize * 10);
                } else {
                    current = 0;
                }
                document.getElementById("current_button").value=Math.trunc(current/pagesize)+1;
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
            }
            function first_page() {
                for(var i = current; i < current + pagesize; i++) {
                    row_off(i);
                }
                current = 0;
                document.getElementById("current_button").value=Math.trunc(current/pagesize)+1;
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
            }
            function last_page() {
                for(var i = current; i < current + pagesize; i++) {
                    row_off(i);
                }
                current = count - pagesize;
                document.getElementById("current_button").value=Math.trunc(current/pagesize)+1;
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
            }
            function setup_work() {
                loops++;
                //if(loops > 50) { // Shouldn't be infinite, adjust number accordingly
                //    clearInterval(refreshMe)
                //}
                count = document.getElementById("content").getElementsByTagName("tr").length;
                first_date = document.getElementById("row_0").getElementsByTagName("td")[0].innerHTML;
                last_date = document.getElementById("content").getElementsByTagName("tr")[count-1].getElementsByTagName("td")[0].innerHTML;
                document.getElementById("end_button").value=Math.trunc(count/pagesize);
                document.getElementById("start_button").title=first_date;
                document.getElementById("end_button").title=last_date;
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
                r = document.getElementById("hidden_time");
                if(r !== null) {
                    if(r.style.display !== "none") {
                        r.style.display = "none";
                    }
                    t = document.getElementById("timespent");
                    if(t !== null) {
                        //t.innerHTML = r.innerHTML;
                        t.innerHTML = pagesize;
                    }
                }
                last_page();
                //t = document.getElementById("timespent");
                //if(t !== null) {
                //    t.innerHTML = (t1 - t0).toFixed(3);
                //}
            }
            function refill_the_table() {
                t0 = performance.now();
                w = document.getElementById("pagegen_word");
                if(w !== null) {
                    w.innerHTML = "refreshed";
                }
                //t = document.getElementById("content");
                //oldbody = t.getElementsByTagName("tbody")[0];
                //newbody = document.createElement("tbody");
                
                // Do file_tail in javascript to get new content.
                // Then format it like the PHP version.
                // Fille the new table body with the results.

                //t.replaceChild(newbody, oldbody);
                spent = performance.now() - t0;
                w = document.getElementById("timespent");
                if(w !== null) {
                    w.innerHTML = (spent/1000).toFixed(3);
                }
            }
            function setup() {
                setup_work();
                refreshMe = setTimeout(refill_the_table, 10000);
                //refreshMe = setInterval(setup_work, 10000);
            }
        </script>
        <style>
            html, body { table-layout: fixed; max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            table { table-layout: fixed; max-width: 99%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
            <?php if( $isMobile ) { ?>
            th,td { font-size: 40px; }
            input { font-size: 60px; }
            <?php } ?>
        </style>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="setup();">
        <table id="navbar" width="99%" align="center">
            <tr>
                <?php if( ! $isMobile ) { ?>
                <td align="left" width="35%">
                <?php } else { ?>
                <td align="center" width="100%">
                <?php } ?>
                    <input id="start_button" type="button" value="1" onclick="first_page();">
                    <input id="back_ten_button" type="button" value="<<" onclick="page_backward_ten();">
                    <input id="back_button" type="button" value="back" onclick="page_backward();">
                    <input id="current_button" type="button" value="1">
                    <input id="forward_button" type="button" value="forward" onclick="page_forward();">
                    <input id="forward_ten_button" type="button" value=">>" onclick="page_forward_ten();">
                    <input id="end_button" type="button" value="end" onclick="last_page();">
                </td>
                <?php if( ! $isMobile ) { ?>
                <td align="center" width="10%">
                    &nbsp;
                </td>
                <td align="center" width="10%">
                    <a href="mudlist.html">Mudlist</a>
                </td>
                <td align="center" width="10%">
                    <a href="https://themud.org/chanhist.php#Channel=all">Other Logs</a>
                </td>
                <td align="right" width="35%" onmouseover="pagegen.style.color='#00FF00'; timespent.style.color='#00FF00';" onmouseout="pagegen.style.color='#1F1F1F'; timespent.style.color='#1F1F1F';">
                    &nbsp;
                    <a href="javascript:;" onmousedown="toggleDiv('source');">
                        <span id="pagegen" style="color: #1F1F1F">
                            Page 
                            <span id="pagegen_word">generated</span>
                            in 
                            <span id="timespent" style="color: #1F1F1F">
                                00.0000
                            </span>
                             seconds.
                        </span>
                    </a>
                </td>
                <?php } ?>
            </tr>
        </table>
        <table id="content" width="99%" align="center">
            <thead>
            <tr>
                <?php if( ! $isMobile ) { ?>
                <th id="dateheader" align="left" width="80px" style="color: #DDDDDD; min-width: 80px;">Date</th>
                <th id="timeheader" align="left" width="60px" style="color: #DDDDDD; min-width: 40px;">Time</th>
                <th id="channelheader" align="left" width="80px" style="color: #DDDDDD; min-width: 100px;">Channel</th>
                <th id="speakerheader" align="left" width="200px" style="color: #DDDDDD; min-width: 200px;">Speaker</th>
                <?php } else { ?>
                <th id="timeheader" align="left" width="120mm" style="color: #DDDDDD; min-width: 170mm;">Time</th>
                <th id="speakerheader" align="left" width="250mm" style="color: #DDDDDD; min-width: 250mm;">Speaker</th>
                <?php } ?>
                <th align="left">&nbsp;</th>
            </tr>
            </thead>
            <tbody>
            <?php
            foreach ($file_lines as $line) {
                $line_data = explode( "\t", $line );
                if ( sizeof( $line_data ) < 5 ) {
                    continue;
                }
                $date = substr( $line_data[0], 0, 10 ); // YYYY.MM.DD
                $bgColor = get_background_color( $line_data[1], $bg );
                $who = explode( "@", $line_data[2] );
                $is_emote = $line_data[3];
                $message = implode("\t", array_slice($line_data, 4));
                ?>
                <tr id="row_<?php echo $bg;?>" style="display:none">
                    <?php if( ! $isMobile ) { ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $date; ?></td>
                    <?php } ?>

                    <?php $time = get_time_color( $line_data[0] ); ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $time; ?></td>

                    <?php if( ! $isMobile ) { ?>
                    <?php $channel = get_channel_color( $line_data[1] ); ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $channel; ?></td>
                    <?php } ?>

                    <?php $speaker = get_speaker_color( $who[0], $who[1] ); ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $speaker; ?></td>

                    <?php
                    //$tmp_msg = preg_replace("/\x1b\[[0-9]+(;[0-9]+)*m/", "", $line_data[3]);
                    //$message = htmlentities($tmp_msg,0,'UTF-8');
                    $message = htmlentities($message, ENT_QUOTES|ENT_SUBSTITUTE, 'UTF-8');
                    $message = handle_colors($message);

                    # Putty:  ((((https?|ftp):\/\/)|www\.)(([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)|(([a-zA-Z0-9\-]+\.)*[a-zA-Z0-9\-]+\.(aero|asia|biz|cat|com|coop|info|int|jobs|mobi|museum|name|net|org|post|pro|tel|travel|xxx|edu|gov|mil|[a-zA-Z][a-zA-Z]))|([a-z]+[0-9]*))(:[0-9]+)?((\/|\?)[^ "]*[^ ,;\.:">)])?)|(spotify:[^ ]+:[^ ]+)
                    # Mine:   ((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)
                    #$message = preg_replace("/(https?\:\/\/[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+)/", '<a href="$1" target="I3-link">$1</a>', $message);
                    $message = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/', '<a href="$1" target="I3-link">$1</a>', $message);
                    $message = preg_replace('/YouTube\s+(<span.*?>)\s*\[([^\]]*)\]/', 'YouTube $1 <a href="https://youtu.be/$2" target="I3-link">[$2]</a>', $message);
                    $message = preg_replace('/IMDB\s+(<span.*?>)\s*\[([^\]]*)\]/', 'IMDB $1 <a href="https://www.imdb.com/title/$2/" target="I3-link">[$2]</a>', $message);
                    $message = preg_replace('/Steam\s+(<span.*?>)\s*\[([^\]]*)\]/', 'Steam $1 <a href="http://store.steampowered.com/app/$2/" target="I3-link">[$2]</a>', $message);
                    $message = preg_replace('/Dailymotion\s+(<span.*?>)\s*\[([^\]]*)\]/', 'Dailymotion $1 <a href="https://www.dailymotion.com/video/$2" target="I3-link">[$2]</a>', $message);
                    ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><span style="font-family: monospace; white-space: pre-wrap;"><?php echo $message; ?></span></td>
                </tr>
                <?php 
                $bg++;
            }
            ?>
            </tbody>
        </table>
        <?php if( ! $isMobile ) { ?>
        <div id="source" style="display: none; vertical-align: bottom; background-color: white;"> <?php echo numbered_source(__FILE__); ?> </div>
        <div id="dump" style="display: none; vertical-align: bottom; background-color: white;"> <?php echo json_encode($colorMap, JSON_PRETTY_PRINT); ?> </div>
        <?php $time_end = microtime(true); $time_spent = $time_end - $time_start; ?>
        <div id="hidden_time" style="display: none;"> <?php printf( "%7.3f", $time_spent); ?> </div>
        <?php } ?>
    </body>
</html>
