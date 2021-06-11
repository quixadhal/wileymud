<?php
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'page_source.php';
require_once 'random_background.php';
require_once 'pinkfish_colors.php';
require_once 'navbar.php';
require_once 'log_navigation.php';

$do_extra_ajax          = 1;

$DATA_URL               = "$URL_HOME/log/log_chunk.php";
$RESULT_LIMIT           = 200;  // Fetch no more than this many per request
$DISPLAY_LIMIT          = 1000; // Keep no more than this many in the table

$the_date = NULL;
if(array_key_exists('date', $_GET)) {
    $the_date = $_GET['date'];
}
$start_paused = false;
if(array_key_exists('pause', $_GET)) {
    $start_paused = true;
}
$no_initial_scroll = false;
if(array_key_exists('noscroll', $_GET)) {
    $no_initial_scroll = true;
}

?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <title>Intermud-3 Log Page</title>
        <!-- Global site tag (gtag.js) - Google Analytics -->
        <script async src="https://www.googletagmanager.com/gtag/js?id=UA-163395867-1"></script>
        <script>
          window.dataLayer = window.dataLayer || [];
          function gtag(){dataLayer.push(arguments);}
          gtag('js', new Date());

          gtag('config', 'UA-163395867-1');
        </script>
        <link rel="stylesheet" href="<?php echo $SITE_GLOBAL_CSS;?>">
        <link rel="stylesheet" href="<?php echo $JQUI_CSS;?>">
        <link rel="stylesheet" href="<?php echo $JQUI_THEME;?>">
        <link rel="stylesheet" href="<?php echo $PAGE_SOURCE_CSS;?>">
        <link rel="stylesheet" href="<?php echo $BACKGROUND_CSS;?>">
        <link rel="stylesheet" href="<?php echo $NAVBAR_CSS;?>">
        <link rel="stylesheet" href="<?php echo $LOG_NAV_CSS;?>">
        <style>
            #debug-message {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
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
                background-color: <?php echo $EVEN; ?>;
            }
            #content-table tr:nth-child(odd) {
                background-color: <?php echo $ODD; ?>;
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
        </style>
        <script src="<?php echo $JQ;?>"></script>
        <script src="<?php echo $JSCOOKIE;?>"></script>
        <script src="<?php echo $JSRANDOM;?>"></script>
        <script src="<?php echo $JSMD5;?>"></script>
        <script src="<?php echo $MOMENT;?>"></script>
        <script src="<?php echo $MOMENT_TZ;?>"></script>
        <script src="<?php echo $JQUI;?>"></script>
        <script src="<?php echo $SITE_GLOBAL_JS;?>"></script>
        <script src="<?php echo $BACKGROUND_JS;?>"></script>
        <script src="<?php echo $PINKFISH_JS;?>"></script>
        <script src="<?php echo $NAVBAR_JS;?>"></script>
        <script src="<?php echo $LOG_NAV_DATES_JS;?>"></script>
        <script src="<?php echo $LOG_NAV_JS;?>"></script>

        <script language="javascript">
            var WasAtBottom = false;
            var ContentTimer;
            var CountdownTimer;
            var PauseTimer;
            var IsPaused = <?php echo ($start_paused == true) ? "true" : "false"; ?>;
            var NoScroll = <?php echo ($no_initial_scroll == true) ? "true" : "false"; ?>;
            var PauseIconIsRed = true;
            var Ticks = 0;
            var SlowDelay = 1;
            var CurrentTickCountdown = 1;
            var GotDataLastTime = false;
            var dataUrlBase = "<?php echo $DATA_URL; ?>";
            var LastRow = 0; // unix timestamp of most recent data
            var FirstRow = 0; // unix timestamp of oldest data kept
            var MD5s = [];
            var RowCount = 0; // number of rows we have now
            var RowLimit = <?php echo $RESULT_LIMIT; ?>;
            var DisplayLimit = <?php echo $DISPLAY_LIMIT; ?>;
            var FirstScreen = true;
            var FirstTimeNewRow = true;
            var TheDate = "<?php echo $the_date; ?>";
            var DoExtraAjax = <?php echo $do_extra_ajax; ?>;

            function on_scroll() {
                var body = document.body;
                var html = document.documentElement;
                var bt = document.getElementById("navbar-button-top");
                var bb = document.getElementById("navbar-button-bottom");
                var doc_height = Math.max( body.scrollHeight, body.offsetHeight,
                    html.clientHeight, html.scrollHeight, html.offsetHeight );

                // This is used so that if we add more content and
                // nothing scrolls the page, we know we COULD
                // scroll further, if the user were at the bottom
                // and thus done reading it.
                WasAtBottom = false;
                if( window.innerHeight >= doc_height ) {
                    // The page fits entirely on the screen, no scrolling possible.
                    dim(bb);
                    dim(bt);
                } else if( (window.innerHeight + window.pageYOffset) >=
                    (document.body.offsetHeight) ) {
                    // We are at the bottom of the page.
                    dim(bb);
                    brighten(bt);
                    WasAtBottom = true;
                    NoScroll = false;
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
            function isUrlBotRow(tr) {
                var trChannel = tr.find(".content-channel-column span").text();
                var trSpeaker = tr.find(".content-speaker-column span").text();
                if(trChannel == "url" && trSpeaker == "URLbot") {
                    return true;
                }
                return false;
            }
            function doesUrlBotTimeMatch(tr, row) {
                var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
                var trDate = tr.find(".content-date-column").text();
                var trTime = tr.find(".content-time-column span").text();
                var trMoment = moment.tz(trDate + " " + trTime, yourTimeZone);
                var trUnix= trMoment.unix();

                // Server Time Zone
                var oldMoment = moment.tz(row.the_date + " " + row.the_time, "America/Los_Angeles");
                var newMoment = oldMoment.clone().tz(yourTimeZone);
                var newUnix= newMoment.unix();

                if(trUnix == newUnix) {
                    return true;
                }
                return false;
            }
            function isComingSoon(message) {
                return /COMING\s+SOON\!(?:\s+\((?<counter>\d+)\))?/.test(message);
            }
            function incrementComingSoon(message) {
                var trMatch = message.match(/COMING\s+SOON\!(?:\s+\((?<counter>\d+)\))?/);
                if(trMatch !== null) {
                    var trCounter = trMatch.groups["counter"];
                    var trValue = 1;
                    var trNewMessage = "";
                    if(trCounter !== undefined) {
                        trValue = parseInt(trCounter) + 1;
                        // Replace original with new value
                        trNewMessage = message.replace(/COMING\s+SOON\!\s+\(\d+\)/, "COMING SOON! (" + trValue + ")");
                    } else {
                        // Add counter on the end
                        trNewMessage = message.replace(/COMING\s+SOON\!/, "COMING SOON! (" + trValue + ")");
                    }
                    return trNewMessage;
                }
                return message;
            }
            function firstDateInTable() {
                return $("#content-table tr").eq(1).children(".content-date-column").text();
            }
            function lastDateInTable() {
                return $("#content-bottom").children(".content-date-column").text();
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
                    // Server Time Zone
                    var midnightMoment = moment.tz(TheDate, "America/Los_Angeles");
                    var midnightUnix = midnightMoment.unix();
                    dataUrl = dataUrlBase + "?from=" + midnightUnix + "&limit=" + RowLimit;
                    //dataUrl = dataUrlBase + "?limit=" + RowLimit + "&date=" + TheDate;
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
                            if(FirstTimeNewRow == true) {
                                // If this is our first time, remove the placeholder
                                // row that's in the vanilla HTML...
                                $('#content-bottom').remove();
                                FirstTimeNewRow = false;
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
                            GotDataLastTime = true;
                        }
                    });
                    if(FirstScreen == true || WasAtBottom == true) {
                        // The very first time we load the page, we want
                        // to scroll down to the latest content.
                        // Also, if the user was at the bottom (before we
                        // added stuff), they probably will want to see the
                        // new stuff too.  Otherwise, let them keep reading
                        // at their own pace.
                        if(NoScroll == false) {
                            scroll_to('content-bottom');
                        }
                        if(IsPaused == true) {
                            // Prevent future runs of this until we clock play
                            pauseUpdate();
                        }
                        FirstScreen = false;
                    }
                    if(GotDataLastTime == true) {
                        // Reset the delay, because we actually got a result back.
                        Ticks = 1;
                        on_scroll();
                        GotDataLastTime = false;
                    }
                    var endTime = performance.now();
                    var elapsedTime = endTime - startTime;
                    $('#elapsed').html(Math.round(elapsedTime) / 1000.0);
                });

                // At this point, we can check to see if we have any urlbot
                // rows that were not really filled in, and if so, do our extra
                // check to see if they can be updated...
                var FoundComingSoon = false;
                //console.log("Rows: " + $("#content-table > tbody > tr").length);

                $("#content-table > tbody > tr").each( function(index, tr) {
                    if(isUrlBotRow($(this))) {
                        var trMessage = $(this).find(".content-message-column").html();

                        if(isComingSoon(trMessage)) {
                            FoundComingSoon = true;
                            var trNewMessage = incrementComingSoon(trMessage);
                            $(this).find(".content-message-column").html(trNewMessage);
                        }
                    }
                });
                //console.log("FoundComingSoon: " + FoundComingSoon);

                if(DoExtraAjax && FoundComingSoon) {
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
                                if(isUrlBotRow($(this))) {
                                    if(doesUrlBotTimeMatch($(this), row)) {
                                        var trMessage = $(this).find(".content-message-column").html();
                                        var newMessage = row.message.split('%^RESET%^').join('');
                                        var blurThis = "unblurred";
                                        if(shouldBlur(row.channel, row.message)) {
                                            blurThis = "blurry";
                                        }
                                        newMessage = handle_colors(newMessage);
                                        newMessage = '<span class="' + blurThis + '">' + newMessage + '</span>';

                                        if(isComingSoon(trMessage)) {
                                            // The old message is already a placeholder
                                            if(isComingSoon(newMessage)) {
                                                // Still just a placeholder, keep the old one
                                            } else {
                                                $(this).find(".content-message-column").html(newMessage);
                                            }
                                        } else {
                                            // We got a live one!
                                            $(this).find(".content-message-column").html(newMessage);
                                        }
                                    }
                                }
                            });
                        });
                    });
                }

                // Our table has now bee updated... so tweak the calendar
                // widget so it matches our data.
                // Maybe change the color of dates between first and last?
                if(FirstScreen != true) {
                    // Don't do this until we add data once, or we'll get
                    // the nonsense buffer row in the HTML...
                    $("#datepicker").val(firstDateInTable());
                }

                on_scroll();
                //We gradually slow down updates to avoid being spammed by
                //people who might leave the log page open, unattended, for
                //days on end.
                SlowDelay = Math.round(((Ticks * 1.45 / 10.0) + 1.0 + (Random.random() - 0.5)) * 100.0) / 100.0;
                CurrentTickCountdown = Math.round(SlowDelay * 60.0);
                ContentTimer = setTimeout(updateContent, 1000 * CurrentTickCountdown);
            }
            function flashPauseButton() {
                clearTimeout(PauseTimer);
                if(PauseIconIsRed == true) {
                    PauseIconIsRed = false;
                    $("#navbar-button-play").attr("src", "<?php echo $PAUSE_GREY_ICON; ?>");
                } else {
                    PauseIconIsRed = true;
                    $("#navbar-button-play").attr("src", "<?php echo $PAUSE_RED_ICON; ?>");
                }
                PauseTimer = setTimeout(flashPauseButton, 1000);
            }
            function pauseUpdate() {
                clearTimeout(CountdownTimer);
                clearTimeout(ContentTimer);
                clearTimeout(PauseTimer);
                IsPaused = true;
                $('#tick').html("--:--");
                $("#navbar-button-play").attr("src", "<?php echo $PAUSE_RED_ICON; ?>");
                $("#navbar-button-play").attr("title", "Resume updates");
                PauseIconIsRed = true;
                PauseTimer = setTimeout(flashPauseButton, 1000);
            }
            function resumeUpdate() {
                clearTimeout(PauseTimer);
                IsPaused = false;
                SlowDelay = Math.round(((Ticks * 1.45 / 10.0) + 1.0 + (Random.random() - 0.5)) * 100.0) / 100.0;
                CurrentTickCountdown = Math.round(SlowDelay * 60.0);
                ContentTimer = setTimeout(updateContent, 1000 * CurrentTickCountdown);
                updateProcessingTime();
                $("#navbar-button-play").attr("src", "<?php echo $PLAY_ICON; ?>");
                $("#navbar-button-play").attr("title", "Pause updates");
            }
            function clickPlayPause() {
                if(IsPaused == false) {
                    pauseUpdate();
                } else {
                    resumeUpdate();
                    // Don't call updateContent() directly, or people can spam
                    // by clicking pause/play quickly!
                }
            }
            $(document).ready(function() {
                ContentTimer = setTimeout(updateContent, 100);
                CountdownTimer = setTimeout(updateProcessingTime, 100);
                hideDiv('page-source');
                //showDiv('page-load-time');
                $("#datepicker").val(queryDate);
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
            <img class="nav-img glowing" id="navbar-button-mudlist" title="List of MUDs" src="<?php echo $MUDLIST_ICON; ?>" onclick="window.location.href='<?php echo $MUDLIST_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-themudorg" title="Another I3 Log Page" src="<?php echo $OTHER_LOG_ICON; ?>" onclick="window.location.href='<?php echo $OTHER_LOG_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" onclick="window.location.href='<?php echo $PIE_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-question" title="???!" src="<?php echo $QUESTION_ICON; ?>" onclick="window.location.href='<?php echo $QUESTION_URL; ?>';" />
        </div>
        <div id="navbar-center">
            <img class="nav-img glowing" id="navbar-button-play" title="<?php echo ($start_paused == true) ? "Resume updates" : "Pause updates"; ?>" src="<?php echo ($start_paused == true) ? $PAUSE_RED_ICON : $PLAY_ICON; ?>" onclick="clickPlayPause();" />
            <input type="text" id="datepicker" size="10" value="<?php echo $today; ?>" title="Date to begin viewing" />
            <img class="nav-img glowing" id="navbar-button-top" title="Top of page" src="<?php echo $TOP_ICON; ?>" onclick="scroll_to('content-top');" />
            <img class="nav-img glowing" id="navbar-button-bottom" title="Bottom of page" src="<?php echo $BOTTOM_ICON; ?>" onclick="scroll_to('content-bottom');" />
        </div>
        <div id="navbar-right">
            <span id="refresh-time">--:-- ---</span>
            <img class="nav-img glowing" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" onclick="window.location.href='<?php echo $SERVER_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='<?php echo $GITHUB_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='<?php echo $DISCORD_URL; ?>';" />
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
                <td class="content-date-column">1990-08-10</td>
                <td class="content-time-column"><span style="color: #00bbbb;">18:05:15</span></td>
                <td class="content-channel-column">wiley</td>
                <td class="content-speaker-column">SYSTEM@WileyMUD</td>
                <td class="content-message-column">online</td>
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
