<?php
$time_start = microtime(true);
$background_image_list = array();
$allowed = false;
$do_extra_ajax = 0;

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

$DATA_URL           = "$URL_HOME/log_chunk.php";
$BACKGROUND_DIR     = "$FILE_HOME/gfx/wallpaper/";
$BACKGROUND         = random_image($BACKGROUND_DIR);
$BACKGROUND_DIR_URL = "$URL_HOME/gfx/wallpaper";
$BACKGROUND_URL     = "$BACKGROUND_DIR_URL/$BACKGROUND";
$BACKGROUND_IMG     = "<img class=\"overlay-bg\" src=\"$BACKGROUND_URL\" />";
$PINKFISH_CACHE     = "$FILE_HOME/logpages/pinkfish.json";

$JQ                 = "$URL_HOME/jquery.js";
$JQUI_CSS           = "$URL_HOME/jquery/jquery-ui.css";
$JQUI_THEME         = "$URL_HOME/jquery/jquery-ui.theme.css";
$JQUI               = "$URL_HOME/jquery/jquery-ui.js";
$JSCOOKIE           = "$URL_HOME/js.cookie.min.js";
$JSRANDOM           = "$URL_HOME/js.random.js";
$JSMD5              = "$URL_HOME/js.md5.js";
$MOMENT             = "$URL_HOME/moment.js";
$MOMENT_TZ          = "$URL_HOME/moment-timezone.js";

$BEGIN_ICON         = "$URL_HOME/gfx/nav/begin.png";
$PREV_ICON          = "$URL_HOME/gfx/nav/previous.png";
$NEXT_ICON          = "$URL_HOME/gfx/nav/next.png";
$END_ICON           = "$URL_HOME/gfx/nav/end.png";
$UP_ICON            = "$URL_HOME/gfx/nav/green/up.png";
$DOWN_ICON          = "$URL_HOME/gfx/nav/green/down.png";
$TOP_ICON           = "$URL_HOME/gfx/nav/green/top.png";
$BOTTOM_ICON        = "$URL_HOME/gfx/nav/green/bottom.png";

$MUDLIST_ICON       = "$URL_HOME/gfx/mud.png";
$LOG_ICON           = "$URL_HOME/gfx/other_logs.png";
$PIE_ICON           = "$URL_HOME/gfx/pie_chart.png";
$QUESTION_ICON      = "$URL_HOME/gfx/question_girl3.png";
$DISCORD_ICON       = "$URL_HOME/gfx/discord.png";
$SERVER_ICON        = "$URL_HOME/gfx/server_icon.png";
$GITHUB_ICON        = "$URL_HOME/gfx/github1600.png";

$SCALE              = 1.0;
$ICON_BASE          = 64;
$FONT_BASE          = 16;   // 24pt 39px, 18pt 30px, 14pt 24px, 10pt 17px, 1.7 seems close
$RESULT_LIMIT       = 100;  // Fetch no more than this many per request
$DISPLAY_LIMIT      = 1000; // Keep no more than this many in the table

$ICON_SIZE          = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
$FONT_SIZE          = sprintf("%dpt", (int)($FONT_BASE * $SCALE));
$SMALL_FONT_SIZE    = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.90));
$TINY_FONT_SIZE     = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.70));

$BGCOLOR            = "black";
$TEXT               = "#d0d0d0";
$UNVISITED          = "#ffffbf";
$VISITED            = "#ffa040";
$DELETED            = "#ff0000";
$EVEN               = "rgba(31,31,31,0.7)";
$ODD                = "rgba(0,0,0,0.7)";

$pinkfish_map = load_pinkfish_map($PINKFISH_CACHE);

$the_date = NULL;
if(array_key_exists('date', $_GET)) {
    $the_date = $_GET['date'];
}

?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <style>
            html, body {
                font-family: 'Lato', sans-serif;
                padding: 0px;
                margin: 0px;
            }
            table {
                table-layout: fixed;
                max-width: 99%;
                overflow-x: hidden;
                border: 0px;
                padding: 0px;
                border-spacing: 0px;
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
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                white-space: pre-wrap;
            }
            .blurry:not(:hover) {
                filter: blur(3px);
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                white-space: pre-wrap;
            }
            .blurry:hover {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
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
            #debug-message {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
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
                border: none;
                object-fit: cover;
            }

            .nav-img {
                border: none;
                min-height: <?php echo $ICON_SIZE; ?>;
                height: <?php echo $ICON_SIZE; ?>;
                max-height: <?php echo $ICON_SIZE; ?>;
                min-width: <?php echo $ICON_SIZE; ?>;
                width: <?php echo $ICON_SIZE; ?>;
                max-width: <?php echo $ICON_SIZE; ?>;
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
            #back-navbar {
                text-align: left;
                min-width: 100%;
                width: 100%;
                top: 0;
                left: 0;
                z-index: 2;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                position: fixed;
                background-color: black;
            }
            #navbar-left {
                text-align: left;
                min-width: 30%;
                width: 30%; 
                top: 0;
                left: 0;
                z-index: 3;
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
                z-index: 3;
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
                z-index: 3;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                position: fixed;
                background-color: black;
            }
            #refresh-time {
                vertical-align: top;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $SMALL_FONT_SIZE; ?>;
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
                z-index: 3;
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
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $TINY_FONT_SIZE; ?>;
                text-align: right;
                overflow-x: hidden;
                white-space: nowrap;
                padding-left: 1ch;
                padding-right: 1ch;
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
            #content-table tr:nth-child(even) {
                background-color: <?php echo $EVEN; ?>
            }
            #content-table tr:nth-child(odd) {
                background-color: <?php echo $ODD; ?>
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
        </style>
<!--
                position: absolute;
                top: calc(<?php echo $ICON_SIZE; ?> + (<?php echo $FONT_SIZE; ?> * 1.25));
                left: 0;
-->
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
                    element.classList.remove('glowing');
                    //element.className = "not-glowing";
                }
            }
            function brighten(element) {
                if(element !== null) {
                    element.style.opacity = "1.0";
                    element.classList.add('glowing');
                    //element.className = "glowing";
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
            function secsToHMS(s) {
                var d = Math.floor(s / 86400);
                s -= d * 86400;
                var h = Math.floor(s / 3600);
                s -= h * 3600;
                var m = Math.floor(s / 60);
                s -= m * 60;
                var output = '';
                if(d > 0) {
                    output = output + d + ' d';
                }
                if(h > 0) {
                    var ph = '0' + h;
                    output = output + ph.substr(-2) + ':';
                }
                var pm = '0' + m;
                output = output + pm.substr(-2) + ':';
                var ps = '0' + s;
                output = output + ps.substr(-2);
                return output;
            }
            function server_date() {
                // This is only true when the page first loads
                return "<?php echo date("Y-m-d"); ?>";
            }
            function server_time_midnight() {
                // This is only true when the page first loads
                return <?php $dt = new DateTime(date("Y-m-d")); echo $dt->format("U"); ?>;
            }
        </script>
        <script language="javascript">
            var Random = new MersenneTwister();
            var ContentTimer;
            var CountdownTimer;
            var Ticks = 0;
            var SlowDelay = 1;
            var CurrentTickCountdown = 1;
            var GotDataLastTime = 0;
            var dataUrlBase = "<?php echo $DATA_URL; ?>";
            var LastRow = 0; // unix timestamp of most recent data
            var FirstRow = 0; // unix timestamp of oldest data kept
            var MD5s = [];
            var RowCount = 0; // number of rows we have now
            var RowLimit = <?php echo $RESULT_LIMIT; ?>;
            var DisplayLimit = <?php echo $DISPLAY_LIMIT; ?>;
            var FirstScreen = 1;
            var FirstTimeNewRow = 1;
            var TheDate = "<?php echo $the_date; ?>";
            var DoExtraAjax = <?php echo $do_extra_ajax; ?>;
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
            function updateProcessingTime() {
                clearTimeout(CountdownTimer);
                $('#tick').html(secsToHMS(CurrentTickCountdown));
                CurrentTickCountdown--;
                if(CurrentTickCountdown < 1) {
                    CurrentTickCountdown = 0;
                }
                CountdownTimer = setTimeout(updateProcessingTime, 1000);
            }
            function updateContent() {
                clearTimeout(ContentTimer);
                var startTime = performance.now();
                var dataUrl;
                Ticks++;
                if(LastRow > 0) {
                    dataUrl = dataUrlBase + "?from=" + LastRow + "&limit=" + RowLimit;
                } else if(TheDate !== "") {
                    dataUrl = dataUrlBase + "?limit=" + RowLimit + "&date=" + TheDate;
                } else {
                    //dataUrl = dataUrlBase + "?limit=" + RowLimit; // + "&from=1620777600";
                    //dataUrl = dataUrlBase + "?limit=" + RowLimit + "&date=" + server_date();
                    dataUrl = dataUrlBase + "?from=" + server_time_midnight() + "&limit=" + RowLimit;
                }
                $.ajax({
                    url: dataUrl,
                }).then(function(data) {
                    randomizeBackground();
                    updateRefreshTime();
                    $.each(data, function(rowId, row) {
                        LastRow = row.unix_time;
                        if(FirstRow < 1) {
                            FirstRow = row.unix_time;
                        }

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
                            MD5s.push(newMD5);
                            if(FirstTimeNewRow == 1) {
                                // If this is our first time, remove the placeholder
                                // row that's in the vanilla HTML...
                                $('#content-bottom').remove();
                                FirstTimeNewRow = 0;
                            } else {
                                // Otherwise, just demote it.
                                $('#content-table tr:last').removeAttr('id');
                            }
                            $('#content-table tr:last').after(newRow);
                            // If we're over our limit, remove a row from the top
                            if($('#content-table tr').length > (DisplayLimit + 1)) {
                                $('#content-table tr:eq(1)').remove();
                                //$('#content-table tr:eq(0)').remove();
                                //$('#content-table tr:eq(0)').attr('id', 'content-top');

                                // Now we update the oldest row time to be the new oldest row
                                var row_date = $('#content-table tr:eq(1) td:eq(0)').text();
                                var row_time = $('#content-table tr:eq(1) td:eq(1) span').text();
                                var rowMoment = moment.tz(row_date + " " + row_time, yourTimeZone);
                                FirstRow = rowMoment.unix();
                            }
                            // Note that we got at least some data...
                            GotDataLastTime = 1;
                        }
                    });
                    if(FirstScreen == 1) {
                        // The very first time we load the page, we want
                        // to scroll down to the latest content, but then
                        // we just want to add new things to the bottom and
                        // let the user scroll as they wish.
                        scroll_to('content-bottom');
                        FirstScreen = 0;
                    }
                    if(GotDataLastTime == 1) {
                        // Reset the delay, because we actually got a result back.
                        Ticks = 1;
                        on_scroll();
                        GotDataLastTime = 0;
                    }
                    var endTime = performance.now();
                    var elapsedTime = endTime - startTime;
                    $('#elapsed').html(Math.round(elapsedTime) / 1000.0);
                });

                if(DoExtraAjax) {
                    // At this point, FirstRow and LastRow hold the unix timestamps
                    // of the content we have in our table.  So if we need to
                    // ask for just the urlbot entries between those, we could
                    // do so somewhere around here...

                    dataUrl = dataUrlBase + "?from=" + FirstRow + "&to=" + LastRow + "&urlbot";
                    //console.log("New URL: " + dataUrl);
                    $.ajax({
                        url: dataUrl,
                    }).then(function(data) {
                        //console.log("Entering");
                        $.each(data, function(rowId, row) {
                            //console.log("RowID: " + rowId);
                            // Loop over table rows.
                            $("#content-table > tbody > tr").each( function(index, tr) {
                                var trChannel = $(this).find(".content-channel-column span").text();
                                var trSpeaker = $(this).find(".content-speaker-column span").text();
                                //console.log("trChannel: " + trChannel);
                                //console.log("trSpeaker: " + trSpeaker);
                                if(trChannel == "url" && trSpeaker == "URLbot") {
                                    // It is a URLbot thing
                                    var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
                                    var trDate = $(this).find(".content-date-column").text();
                                    var trTime = $(this).find(".content-time-column span").text();
                                    var trMoment = moment.tz(trDate + " " + trTime, yourTimeZone);
                                    var trUnix= trMoment.unix();

                                    var oldMoment = moment.tz(row.the_date + " " + row.the_time, "America/Los_Angeles");
                                    var newMoment = oldMoment.clone().tz(yourTimeZone);
                                    var newUnix= newMoment.unix();
                                    if(trUnix == newUnix) {
                                        // AND the date and time match...
                                        // So, do what we normally do for urlbot things
                                        var blurThis = "unblurred";
                                        if(shouldBlur(row.channel, row.message)) {
                                            blurThis = "blurry";
                                        }
                                        var trMessage = $(this).find(".content-message-column").html();
                                        var newMessage = row.message.split('%^RESET%^').join('');
                                        newMessage = handle_colors(newMessage);
                                        newMessage = '<span class="' + blurThis + '">' + newMessage + '</span>';

                                        //console.log("OLD Message: " + trMessage);
                                        //console.log("NEW Message: " + newMessage);
                                        $(this).find(".content-message-column").html(newMessage);
                                    }
                                }
                            });
                        });
                    });
                }

                on_scroll();
                //We gradually slow down updates to avoid being spammed by
                //people who might leave the log page open, unattended, for
                //days on end.
                SlowDelay = Math.round(((Ticks * 1.45 / 10.0) + 1.0 + (Random.random() - 0.5)) * 100.0) / 100.0;
                CurrentTickCountdown = Math.round(SlowDelay * 60.0);
                ContentTimer = setTimeout(updateContent, 1000 * CurrentTickCountdown);
            }
            $(document).ready(function() {
                ContentTimer = setTimeout(updateContent, 500);
                CountdownTimer = setTimeout(updateProcessingTime, 500);
                hideDiv('page-source');
                //showDiv('page-load-time');
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
        <div id="debug-message">
            Stuff...
        </div>
-->
        <div id="back-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
        <div id="navbar-left">
            <img class="nav-img glowing" id="navbar-button-mudlist" title="List of MUDs" src="<?php echo $MUDLIST_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/mudlist.php';" />
            <img class="nav-img glowing" id="navbar-button-themudorg" title="Another I3 Log Page" src="<?php echo $LOG_ICON; ?>" onclick="window.location.href='https://themud.org/chanhist.php#Channel=all';" />
            <img class="nav-img glowing" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/pie.php';" />
            <img class="nav-img glowing" id="navbar-button-question" title="???!" src="<?php echo $QUESTION_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/random_video.php';" />
        </div>
        <div id="navbar-center">
            <img class="nav-img glowing" id="navbar-button-top" title="Top of page" src="<?php echo $TOP_ICON; ?>" onclick="scroll_to('content-top');" />
            <img class="nav-img glowing" id="navbar-button-bottom" title="Bottom of page" src="<?php echo $BOTTOM_ICON; ?>" onclick="scroll_to('content-bottom');" />
        </div>
        <div id="navbar-right">
            <span id="refresh-time">--:-- ---</span>
            <img class="nav-img glowing" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" onclick="window.location.href='http://wileymud.themud.org/~wiley/server.php';" />
            <img class="nav-img glowing" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='https://github.com/quixadhal/wileymud';" />
            <img class="nav-img glowing" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='https://discord.gg/kUduSsJ';" />
        </div>
        <div id="fake-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
<?php
    // Yes, this needs to be up here so the content-header can display it, briefly.
    $time_end = microtime(true);
    $time_spent = $time_end - $time_start;
?>
        <table id="content-header">
            <tr>
                <td class="content-date-header">Date</td>
                <td class="content-time-header">Time</td>
                <td class="content-channel-header">Channel</td>
                <td class="content-speaker-header">Speaker</td>
                <td class="content-message-header">
                    <a href="javascript:;" onmousedown="toggleDiv('page-source');">Processing&nbsp;in&nbsp;<span id="tick">--:--</span>&nbsp;--&nbsp;<span id="elapsed"><?php printf("%7.3f",$time_spent);?></span>&nbsp;seconds</a>
                </td>
            </tr>
        </table>
        <table id="content-table">
            <tr id="content-top">
                <td class="content-date-column">&nbsp;</td>
                <td class="content-time-column"><span>&nbsp;</span></td>
                <td class="content-channel-column">&nbsp;</td>
                <td class="content-speaker-column">&nbsp;</td>
                <td class="content-message-column">&nbsp;</td>
            </tr>
            <tr id="content-bottom">
                <td class="content-date-column">2021-05-12</td>
                <td class="content-time-column"><span>08:20:02</span></td>
                <td class="content-channel-column">poochan</td>
                <td class="content-speaker-column">pooboy@The Poo Mud</td>
                <td class="content-message-column">Enough poo for India!</td>
            </tr>
        </table>
<!--
        <div id="page-load-time">
        </div>
-->
        <div id="page-source">
            <?php echo numbered_source(__FILE__); ?>
        </div>
    </body>
</html>
