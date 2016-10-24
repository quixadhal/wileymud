<?php
global $time_start;
$time_start = microtime(true);

global $isLocal;
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

$CHAT_COLOR_FILE    = "/home/bloodlines/lib/secure/save/chat.o";
$CHAT_TEXT_FILE     = "/home/wiley/lib/i3/i3.allchan.log";
$CHAT_DATA_FILE     = "/home/wiley/lib/i3/i3.speakers.json";

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

        "ds"          => "%^YELLOW%^",
        "dchat"	      => "%^CYAN%^",
        "intergossip" => "%^GREEN%^",
        "intercre"    => "%^ORANGE%^",
        "pyom"        => "%^FLASH%^%^LIGHTGREEN%^",
        "free_speech" => "%^PINK%^",
        "url"         => "%^WHITE%^",

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

function get_chatter_colors() {
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
    $colors['default'] = "%^GREY%^";
    //foreach ($pinkfishMap as $k => $v) {
    //    echo "pinkfishMap[$k] = ".htmlentities($v)."<br>";
    //}
    for($i = 0; $i < sizeof($mapping); $i++ ) {
        $map = explode(":", $mapping[$i]);
        if(sizeof($map) < 2) {
            //echo "WARNING: Invalid entry for color map \"";
            //print_r($map);
            //echo "\".<br>";
            continue;
        }
        $mapname = substr($map[0], 1, -1); // Strip quotes
        $mapcolor = substr($map[1], 1, -1); // Strip quotes
        if(array_key_exists($mapcolor, $pinkfishMap)) {
            $colors[$mapname] = $pinkfishMap[$mapcolor];
        }

        //echo "colors[$mapname] = ".htmlentities($colors[$mapname])."<br>";
    }

    $dump = json_encode( $colors );
    file_put_contents($CHAT_DATA_FILE, $dump);
    return $colors;
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
    return trim($output);
}

function get_time_color($time) {
    global $hourColors;

    $hour = substr( $time, 11, 2 );
    $min  = substr( $time, 14, 2 );
    $sec  = substr( $time, 17, 2 );
    $fancyLad = $hourColors[$hour] . "$hour:$min.$sec" . "</SPAN>";
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

    $whoLow = strtolower($who);
    $speakerColor = $colorMap["default"];
    if( array_key_exists( $whoLow, $colorMap )) {
        $speakerColor = $colorMap[$whoLow];
    } else {
        // Add a new entry!
        $colorMap[$whoLow] = $speakerColor;
        $chatCounter = count( $colorMap ) - 1;
        $colorIndex = $chatCounter % count( $pinkfishMap );
        $speakerColor = $pinkfishMap[$colorIndex];
        $colorMap[$whoLow] = $speakerColor;
        // Save data here...
        $dump = json_encode( $colorMap );
        file_put_contents($CHAT_DATA_FILE, $dump);
    }
    $fancyLad = "$speakerColor" . $who . "@" . $where . "</SPAN>";
    return $fancyLad;
}

$isLocal = is_local_ip();

init_pinkfish_map();
$hourColors = get_hour_colors();
$channelColors = get_channel_colors();
$colorMap = get_chatter_colors();

//2016.04.13-06.14,33000	intergossip	Cratylus@Dead Souls Dev	the problem with listening to music in the client office

//$file_lines = explode( "\n", file_get_contents( $CHAT_TEXT_FILE ) );
$file_lines = explode( "\n", file_tail( $CHAT_TEXT_FILE, 3000 ) );
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
            var current = 0;
            var pagesize = 30;
            var count = 0;

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
            function setup() {
                count = document.getElementById("content").getElementsByTagName("tr").length;
                first_date = document.getElementById("row_0").getElementsByTagName("td")[0].innerHTML;
                last_date = document.getElementById("content").getElementsByTagName("tr")[count-1].getElementsByTagName("td")[0].innerHTML;
                document.getElementById("end_button").value=Math.trunc(count/pagesize);
                document.getElementById("start_button").title=first_date;
                document.getElementById("end_button").title=last_date;
                for(var i = current; i < current + pagesize; i++) {
                    row_on(i);
                }
                last_page();
            }
        </script>
        <style>
            html, body { max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
        </style>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="setup();">
        <table id="navbar" width="100%">
            <tr>
                <td align="left" width="50%">
                    <input id="start_button" type="button" value="1" onclick="first_page();">
                    <input id="back_ten_button" type="button" value="<<" onclick="page_backward_ten();">
                    <input id="back_button" type="button" value="back" onclick="page_backward();">
                    <input id="current_button" type="button" value="1">
                    <input id="forward_button" type="button" value="forward" onclick="page_forward();">
                    <input id="forward_ten_button" type="button" value=">>" onclick="page_forward_ten();">
                    <input id="end_button" type="button" value="end" onclick="last_page();">
                </td>
                <td align="right" width="50%" onmouseover="pagegen.style.color='#00FF00'; timespent.style.color='#00FF00';" onmouseout="pagegen.style.color='#1F1F1F'; timespent.style.color='#1F1F1F';">
                    <a href="javascript:;" onmousedown="toggleDiv('dump');">
                        <span id="pagegen" style="color: #1F1F1F">
                            &nbsp;Page generated in 
                            <span id="timespent" style="color: #1F1F1F">
                                <?php $time_end = microtime(true); $time_spent = $time_end - $time_start; printf( "%7.3f", $time_spent); ?>
                            </span>
                             seconds.
                        </span>
                    </a>
                </td>
            </tr>
        </table>
        <table id="content" width="100%">
            <tr>
                <th id="dateheader" align="left" width="80px" style="color: #DDDDDD; min-width: 80px;">Date</th>
                <th id="timeheader" align="left" width="40px" style="color: #DDDDDD; min-width: 40px;">Time</th>
                <th id="channelheader" align="left" width="100px" style="color: #DDDDDD; min-width: 100px;">Channel</th>
                <th id="speakerheader" align="left" width="200px" style="color: #DDDDDD; min-width: 200px;">Speaker</th>
                <th align="left">&nbsp;</th>
            </tr>
            <?php
            foreach ($file_lines as $line) {
                $bgColor = ($bg % 2) ? "#000000" : "#1F1F1F";
                $line_data = explode( "\t", $line );
                if ( sizeof( $line_data ) != 4 ) {
                    continue;
                }
                $who = explode( "@", $line_data[2] );
                $date = substr( $line_data[0], 0, 10 ); // YYYY.MM.DD
                ?>
                <tr id="row_<?php echo $bg;?>" style="display:none">
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $date; ?></td>

                    <?php $time = get_time_color( $line_data[0] ); ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $time; ?></td>

                    <?php $channel = get_channel_color( $line_data[1] ); ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $channel; ?></td>

                    <?php $speaker = get_speaker_color( $who[0], $who[1] ); ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><?php echo $speaker; ?></td>

                    <?php
                    $tmp_msg = preg_replace("/\x1b\[[0-9]+(;[0-9]+)*m/", "", $line_data[3]);
                    $message = htmlentities($tmp_msg,0,'UTF-8');
                    $message = preg_replace( '/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/', '<a href="$1" target="I3-link">$1</a>', $message);
                    ?>
                    <td bgcolor="<?php echo $bgColor; ?>"><font face="monospace"><?php echo $message; ?></font></td>
                </tr>
                <?php 
                $bg++;
            }
            ?>
        </table>
        <div id="source" style="display: none; vertical-align: bottom; background-color: white;"> <?php echo numbered_source(__FILE__); ?> </div>
        <div id="dump" style="display: none; vertical-align: bottom; background-color: white;"> <?php echo json_encode($colorMap, JSON_PRETTY_PRINT); ?> </div>
    </body>
</html>
