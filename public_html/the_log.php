<?php
$time_start = microtime(true);
$background_image_list = array();
$allowed = false;

function is_local_ip() {
    $visitor_ip = $_SERVER['REMOTE_ADDR'];
    if($visitor_ip == '104.156.100.167') // Hard coded DNS entry
        return 1;
    $varr = explode(".", $visitor_ip);
    if($varr[0] == "192" && $varr[1] == "168")
        return 1;
    return 0;
}

if(is_local_ip()) {
    $allowed = true;
}

function numbered_source($filename)
{
    ini_set('highlight.string',  '#DD0000'); // DD0000
    ini_set('highlight.comment', '#0000BB'); // FF8000
    ini_set('highlight.keyword', '#00CC00'); // 007700
    ini_set('highlight.bg',      '#111111'); // FFFFFF
    ini_set('highlight.default', '#00DDDD'); // 0000BB
    ini_set('highlight.html',    '#CCCCCC'); // 000000
    $lines = implode(range(1, count(file($filename))), '<br />');
    $content = highlight_file($filename, true);
    return "<table><tr><td class=\"source-line-number\">\n$lines\n</td><td class=\"source-code\">\n$content\n</td></tr></table>"; 
}

function random_image($dir) {
    global $background_image_list;
    $old_dir = getcwd();
    chdir($dir);

    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $background_image_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($background_image_list);

    chdir($old_dir);
    return $background_image_list[$pick];
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

function handle_colors( $pinkfish_map, $message ) {
    foreach ($pinkfish_map as $pf) {
        $pattern = '/'.preg_quote($pf['pinkfish']).'/';
        $repl = $pf['html'];
        $message = preg_replace($pattern, $repl, $message);
    }
    return $message;
}


$URL_HOME           = "http://wileymud.themud.org/~wiley";
$FILE_HOME          = "/home/wiley/public_html";
$LOG_HOME           = "$URL_HOME/logpages";

$DATA_URL           = "$URL_HOME/log_chunk.php";
$BACKGROUND_DIR     = "$FILE_HOME/gfx/wallpaper/";
$BACKGROUND         = random_image($BACKGROUND_DIR);
$BACKGROUND_DIR_URL = "$URL_HOME/gfx/wallpaper";
$BACKGROUND_URL     = "$BACKGROUND_DIR_URL/$BACKGROUND";
$BACKGROUND_IMG     = "<img class=\"overlay-bg\" src=\"$BACKGROUND_URL\" />";
$JQ                 = "$URL_HOME/jquery.js";
$JSCOOKIE           = "$URL_HOME/js.cookie.min.js";
$JSRANDOM           = "$URL_HOME/js.random.js";
$JSMD5              = "$URL_HOME/js.md5.js";

$JQUI_CSS       = "$LOG_HOME/jquery/jquery-ui.css";
$JQUI_THEME     = "$LOG_HOME/jquery/jquery-ui.theme.css";
$JQUI           = "$LOG_HOME/jquery/jquery-ui.js";
$MOMENT         = "$LOG_HOME/moment.js";
$MOMENT_TZ      = "$LOG_HOME/moment-timezone.js";

$SCALE              = 1.0;
$ICON_BASE          = 64;
$FONT_BASE          = 18;
$RESULT_LIMIT       = 100;

$ICON_SIZE          = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
$FONT_SIZE          = sprintf("%dpt", (int)($FONT_BASE * $SCALE));
$TINY_FONT_SIZE     = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.75));

$BGCOLOR            = "black";
$TEXT               = "#d0d0d0";
$UNVISITED          = "#ffffbf";
$VISITED            = "#ffa040";
$DELETED            = "#ff0000";
$EVEN               = "rgba(31,31,31,0.7)";
$ODD                = "rgba(0,0,0,0.7)";

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
$PIE_ICON       = "$URL_HOME/gfx/pie_chart.png";
$QUESTION_ICON  = "$URL_HOME/gfx/question_girl3.png";
$DISCORD_ICON   = "$URL_HOME/gfx/discord.png";
$SERVER_ICON    = "$URL_HOME/gfx/server_icon.png";
$GITHUB_ICON    = "$URL_HOME/gfx/github1600.png";

$PINKFISH_CACHE = "$FILE_HOME/logpages/pinkfish.json";
$pinkfish_map = load_pinkfish_map($PINKFISH_CACHE);

?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <style>
            table {
                table-layout: fixed;
                max-width: 99%;
                overflow-x: hidden;
                border: 0px;
                padding: 0px;
                border-spacing: 0px;
            }
            tr:nth-child(even) {
                background-color: <?php echo $EVEN; ?>
            }
            tr:nth-child(odd) {
                background-color: <?php echo $ODD; ?>
            }
            a {
                text-decoration:none;
                color: <?php echo $UNVISITED; ?>;
            }
            a:visited {
                color: <?php echo $UNVISITED; ?>;
            }
            a:hover {
                text-decoration:underline;
            }
            a:active, a:focus {
                outline: 0;
                border: none;
                -moz-outline-style: none;
            }
            .unblurred {
                font-family: monospace;
                white-space: pre-wrap;
            }
            .blurry:not(:hover) {
                filter: blur(3px);
                font-family: monospace;
                white-space: pre-wrap;
            }
            .blurry:hover {
                font-family: monospace;
                white-space: pre-wrap;
            }
            .glowing:not(:hover) {
                filter: brightness(1);
            }
            .glowing:hover {
                filter: brightness(1.75);
            }
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
            html, body {
                font-family: 'Lato', sans-serif;
                padding: 0px;
                margin: 0px;
            }
            #background-div {
                padding: 0px;
                margin: 0px;
                z-index: -1;
                opacity: 0.50;
                top: 0;
                left: 0;
                bottom: 0;
                right: 0;
                position: fixed;
                height: 100%;
                width: 100%;
            }
            #background-img {
                height: 100%;
                width: 100%;
                object-fit: cover;
            }
            .source-line-number {
                float: left;
                color: gray;
                background-color: #111111;
                opacity: 0.90;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: 14px;
                text-align: right;
                margin: 0px;
                padding: 0px;
                border-spacing: 0px;
                margin-right: 0pt;
                padding-right: 6pt;
                border-spacing-right: 0px;
                border-right: 1px solid gray;
                vertical-align: top;
                white-space: normal;
            }
            .source-code {
                background-color: black;
                opacity: 0.70;
                width: 100%;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: 14px;
                margin: 0px;
                padding: 0px;
                border-spacing: 0px;
                margin-left: 0pt;
                padding-left: 6px;
                border-spacing-left: 6px;
                vertical-align: top;
                white-space: nowrap;
            }
            #page-source {
                display: none;
            }
            #page-load-time {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $TINY_FONT_SIZE; ?>;
                position: fixed;
                bottom: 0px;
                right: 0px;
                z-index: 2;
                background-color: black;
            }
            #message {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
            }
            #fake-navbar {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                background-color: black;
                min-width: 100%;
                width: 100%;
                top: 0;
                left: 0;
            }
            #navbar-left {
                text-align: left;
                min-width: 30%;
                width: 30%; 
                top: 0;
                left: 0;
                z-index: 2;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                position: fixed;
                background-color: black;
            }
            #navbar-center {
                text-align: center;
                min-width: 40%;
                width: 40%; 
                top: 0;
                left: 50%;
                transform: translateX(-50%);
                z-index: 2;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                position: fixed;
                background-color: black;
            }
            #navbar-right {
                text-align: right;
                min-width: 30%;
                width: 30%; 
                top: 0;
                right: 0;
                z-index: 2;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                position: fixed;
                background-color: black;
            }
            #refresh-time {
                vertical-align: top;
            }
            #content-header {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                position: fixed;
                background-color: black;
                min-width: 100%;
                width: 100%;
                top: <?php echo $ICON_SIZE; ?>;
                left: 0;
                z-index: 2;
            }
            .content-date-header {
                text-align: left;
                min-width: 11ch;
                width: 11ch; 
            }
            .content-time-header {
                text-align: left;
                min-width: 9ch;
                width: 9ch; 
            }
            .content-channel-header {
                text-align: left;
                min-width: 16ch;
                width: 16ch; 
            }
            .content-speaker-header {
                text-align: left;
                min-width: 30ch;
                width: 30ch; 
            }
            .content-message-header {
                text-align: left;
                overflow-x: hidden;
                white-space: nowrap;
                padding-left: 1ch;
            }
            #content-table {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                min-width: 100%;
                width: 100%;
            }
            .content-date-column {
                text-align: left;
                min-width: 11ch;
                width: 11ch; 
            }
            .content-time-column {
                text-align: left;
                min-width: 9ch;
                width: 9ch; 
            }
            .content-channel-column {
                text-align: left;
                min-width: 16ch;
                width: 16ch; 
            }
            .content-speaker-column {
                text-align: left;
                min-width: 30ch;
                width: 30ch; 
            }
            .content-message-column {
                text-align: left;
                overflow-x: hidden;
                white-space: normal;
                padding-left: 1ch;
            }
        </style>

        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JSCOOKIE;?>""></script>
        <script src="<?php echo $JSRANDOM;?>""></script>
        <script src="<?php echo $JSMD5;?>""></script>
        <script src="<?php echo $MOMENT;?>""></script>
        <script src="<?php echo $MOMENT_TZ;?>""></script>

        <script language="javascript">
            function toggleDiv(divID) {
                element = document.getElementById(divID);
                if(element !== undefined && element !== null) {
                    if(element.style.display == 'none') {
                        element.style.display = 'block';
                    } else {
                        element.style.display = 'none';
                    }
                }
            }
            function showDiv(divID) {
                element = document.getElementById(divID);
                if(element !== undefined && element !== null) {
                    element.style.display = 'block';
                }
            }
            function hideDiv(divID) {
                element = document.getElementById(divID);
                if(element !== undefined && element !== null) {
                    element.style.display = 'none';
                }
            }
            function scroll_to(id) {
                document.getElementById(id).scrollIntoView({behavior: 'smooth', block: "center"});
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
                var bt = document.getElementById("navbar-button-top");
                var bb = document.getElementById("navbar-button-bottom");
                var doc_height = Math.max( body.scrollHeight, body.offsetHeight,
                    html.clientHeight, html.scrollHeight, html.offsetHeight );

                if( window.innerHeight >= doc_height ) {
                    // The page fits entirely on the screen, no scrolling possible.
                    dim(bb);
                    dim(bt);
                } else if( (window.innerHeight + window.pageYOffset) >=
                    (document.body.offsetHeight) ) {
                    // We are at the bottom of the page.
                    dim(bb);
                    brighten(bt);
                } else if( window.pageYOffset <= 1 ) {
                    // We are at the top of the page.
                    brighten(bb);
                    dim(bt);
                } else {
                    // We're somewhere in the middle.
                    brighten(bb);
                    brighten(bt);
                }
            }
            function htmlentites(s) {
                var result = s.replace(/[\u00A0-\u9999<>\&]/g, function(i) {
                    return '&#'+i.charCodeAt(0)+';';
                });
                return result;
            }
        </script>
        <script language="javascript">
            var Random = new MersenneTwister();
            var Timer;
            var Ticks = 0;
            var dataUrlBase = "<?php echo $DATA_URL; ?>";
            var LastRow = 0; // unix timestamp of most recent data
            var MD5s = [];
            var RowCount = 0; // number of rows we have now
            var RowLimit = <?php echo $RESULT_LIMIT; ?>;
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
<?php
            echo "var BackgroundImageList = [\n";
            echo "\"" . implode("\",\n\"", $background_image_list) . "\"\n";
            echo "];\n";

            echo "var PinkfishMap = {\n";
            foreach ($pinkfish_map as $pf) {
                echo "'" . $pf['pinkfish'] . "' : '" . $pf['html'] . "',\n";
            }
            echo "};\n";
?>
            function handle_colors(s) {
                var result = s;
                for([k,v] of Object.entries(PinkfishMap)) {
                    result = result.split(k).join(v);
                }
                return result;
            }
            function randomizeBackground() {
                var bg_choice = Math.floor(BackgroundImageList.length * Random.random());
                var new_bg = "<?php echo "$BACKGROUND_DIR_URL/"; ?>" + BackgroundImageList[bg_choice];
                $("#background-img").attr("src", new_bg);
            }
            function shouldBlur(channel, message) {
                if(channel == "free_speech") {
                    return true;
                } else if(channel == "bsg") {
                    return true;
                }
                // } else if(preg_match('/on\sfree_speech\&gt\;/', $message) > 0) {
                // } else if(preg_match('/on\sbsg\&gt\;/', $message) > 0) {
                // } else if(preg_match('/^spoiler:/i', $message) > 0) {
                // } else if(preg_match('/\[(spoiler|redacted)\]/i', $message) > 0) {
                return false;
            }
            function updateRefreshTime() {
                var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
                var yourLocale = (navigator.languages && navigator.languages.length) ?
                    navigator.languages[0] : navigator.language;

                var momentObj = moment().tz(yourTimeZone);
                //var momentStr = momentObj.format('YYYY-MM-DD HH:mm:ss z');
                var momentStr = momentObj.format('HH:mm z');
                var momentHour = momentObj.hour();

                var rt = document.getElementById("refresh-time");
                //yt.innerHTML = "[" + yourLocale + " " + yourTimeZone + "] " + momentStr;
                rt.innerHTML = momentStr;
                rt.style.color = hour_map[momentHour];
            }
            function updateContent() {
                var dataUrl;
                Ticks++;
                if(LastRow > 0) {
                    dataUrl = dataUrlBase + "?from=" + LastRow + "&limit=" + RowLimit;
                } else {
                    dataUrl = dataUrlBase + "?limit=" + RowLimit; // + "&from=1620777600";
                }
                randomizeBackground();
                //$("#message").html("Stuff... " + Ticks);
                $.ajax({
                    url: dataUrl,
                }).then(function(data) {
                    //$("#message").html("Tick " + Ticks);
                    updateRefreshTime();
                    $.each(data, function(rowId, row) {
                        LastRow = row.unix_time;

                        var blur = "unblurred";
                        if(shouldBlur(row.channel, row.message)) {
                            blur = "blurry";
                        }
                        // Remap channel name
                        if( row.channel == "japanese" ) {
                            row.channel = "日本語";
                        }

                        row.message = row.message.split('%^RESET%^').join('');
                        row.message = handle_colors(row.message);
                        // HTML entity and URL processing are done in the AJAX
                        // script that feeds us, as it's just easier there.

                        // Localize time to browser
                        var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
                        var yourLocale = (navigator.languages && navigator.languages.length) ?
                            navigator.languages[0] : navigator.language;
                        var oldMoment = moment.tz(row.the_date + " " + row.the_time, "America/Los_Angeles");
                        var newMoment = oldMoment.clone().tz(yourTimeZone);
                        var newDate = newMoment.format('YYYY-MM-DD');
                        var newTime = newMoment.format('HH:mm:ss');
                        var newHour = newMoment.hour();
                        var newColor = '<span style="color: ' + hour_map[newHour] + '">';

                        var newRow = '<tr id="content-bottom">';
                        newRow = newRow + '<td class="content-date-column">' + newDate + '</td>';
                        newRow = newRow + '<td class="content-time-column">' + newColor + newTime + '</span>' + '</td>';
                        newRow = newRow + '<td class="content-channel-column">' + row.channel_html + row.channel + '</span>' + '</td>';
                        newRow = newRow + '<td class="content-speaker-column">' + row.speaker_html + row.speaker + '</span>' + '</td>';
                        newRow = newRow + '<td class="content-message-column">' + '<span class="' + blur + '">' + row.message + '</span>' + '</td>';
                        newRow = newRow + '</tr>';
                        var newMD5 = hex_md5(newRow);

                        if(MD5s.indexOf(newMD5) == -1) {
                            //$("#message").html("Tick " + Ticks + " Time " + row.the_time + " MD5 " + newMD5);
                            MD5s.push(newMD5);
                            $('#content-table tr:last').removeAttr('id');
                            $('#content-table tr:last').after(newRow);
                            // If we're over our limit, remove a row from the top but one
                            if($('#content-table tr').length > (RowLimit + 1)) {
                                $('#content-table tr:eq(1)').remove();
                            }
                            scroll_to('content-bottom');
                        }
                    });
                });
                if(Ticks < 10) {
                    Timer = setTimeout(updateContent, 1000 * 60);
                } // else clearTimeout(Timer);
            }
            $(document).ready(function() {
                Timer = setTimeout(updateContent, 500);
                hideDiv('page-source');
                showDiv('page-load-time');
                on_scroll(); // Call once, in case the page cannot scroll
                $(window).on("scroll", function() {
                    on_scroll();
                });
            });
        </script>
    </head>
    <body bgcolor="<?php echo $BGCOLOR; ?>" text="<?php echo $TEXT; ?>" link="<?php echo $UNVISITED; ?>" vlink="<?php echo $VISITED; ?>">
        <div id="background-div">
            <img id="background-img" src="<?php echo $BACKGROUND_URL; ?>" />
        </div>
<!--
        <div id="message">
            Stuff...
        </div>
-->
        <div id="navbar-left">
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-mudlist" title="List of MUDs" src="<?php echo $MUDLIST_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/mudlist.php';" />
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-themudorg" title="Another I3 Log Page" src="<?php echo $LOG_ICON; ?>" onclick="window.location.href='https://themud.org/chanhist.php#Channel=all';" />
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/pie.php';" />
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-question" title="???!" src="<?php echo $QUESTION_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/random_video.php';" />
        </div>
        <div id="navbar-center">
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-top" title="Top of page" src="<?php echo $TOP_ICON; ?>" onclick="scroll_to('content-top');" />
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-bottom" title="Bottom of page" src="<?php echo $BOTTOM_ICON; ?>" onclick="scroll_to('content-bottom');" />
        </div>
        <div id="navbar-right">
            <span id="refresh-time">--:-- ---</span>
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/server.php';" />
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='https://github.com/quixadhal/wileymud';" />
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='https://discord.gg/kUduSsJ';" />
        </div>
        <div id="fake-navbar">
            <img class="glowing" border="none" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
        <table id="content-header">
            <tr>
                <td class="content-date-header">Date</td>
                <td class="content-time-header">Time</td>
                <td class="content-channel-header">Channel</td>
                <td class="content-speaker-header">Speaker</td>
                <td class="content-message-header">&nbsp;</td>
            </tr>
        </table>
        <table id="content-table">
            <tr id="content-top">
                <td class="content-date-column">&nbsp;</td>
                <td class="content-time-column">&nbsp;</td>
                <td class="content-channel-column">&nbsp;</td>
                <td class="content-speaker-column">&nbsp;</td>
                <td class="content-message-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-date-column">2021-05-12</td>
                <td class="content-time-column">08:20:00</td>
                <td class="content-channel-column">poochan</td>
                <td class="content-speaker-column">pooboy@The Poo Mud</td>
                <td class="content-message-column">A whole bunch of poo.</td>
            </tr>
            <tr>
                <td class="content-date-column">2021-05-12</td>
                <td class="content-time-column">08:20:01</td>
                <td class="content-channel-column">poochan</td>
                <td class="content-speaker-column">pooboy@The Poo Mud</td>
                <td class="content-message-column">More poo.</td>
            </tr>
            <tr id="content-bottom">
                <td class="content-date-column">2021-05-12</td>
                <td class="content-time-column">08:20:02</td>
                <td class="content-channel-column">poochan</td>
                <td class="content-speaker-column">pooboy@The Poo Mud</td>
                <td class="content-message-column">Enough poo for India!</td>
            </tr>
        </table>
<?php
    $time_end = microtime(true);
    $time_spent = $time_end - $time_start;
?>
        <div id="page-load-time">
            <a href="javascript:;" onmousedown="toggleDiv('page-source');">Page:&nbsp;<?php printf("%7.3f&nbsp;seconds",$time_spent);?></a>
        </div>
        <div id="page-source">
            <?php echo numbered_source(__FILE__); ?>
        </div>
    </body>
</html>
