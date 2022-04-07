<?php
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'random_background.php';

?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <title>Random Wallpaper Slideshow</title>

        <link rel="stylesheet" href="<?php echo $SITE_GLOBAL_CSS;?>">
        <link rel="stylesheet" href="<?php echo $BACKGROUND_CSS;?>">

        <style>
            .body {
                overflow-x: hidden;
                overflow-y: hidden;
            }
            ::-webkit-scrollbar {
                display: none;
            }
            ::-webkit-scrollbar-track {
                display: none;
            }
            ::-webkit-scrollbar-thumb {
                display: none;
            }
            #background-div {
                opacity: 1.00;
            }
            #filename-div {
                display: none;
                position: absolute;
                top: 10px;
                left: 10px;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $BIG_FONT_SIZE; ?>;
                white-space: nowrap;
                width: 40ch;
                box-sizing: content-box;
                background-color: rgba(0,0,0,0.6);
                text-align: center;
                border-radius: 20px;
                -webkit-border-radius: 20px;
                -moz-border-radius: 20px;
            }
            #clock-div {
                display: none;
                position: absolute;
                top: 10px;
                right: 10px;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $BIG_FONT_SIZE; ?>;
                white-space: nowrap;
                width: 11ch;
                box-sizing: content-box;
                background-color: rgba(0,0,0,0.8);
                text-align: center;
                border-radius: 20px;
                -webkit-border-radius: 20px;
                -moz-border-radius: 20px;
            }
            #help-div {
                display: none;
                position: absolute;
                top: 20%;
                left: 20%;
                font-family: 'Lato', sans-serif;
                font-size: <?php echo $BIG_FONT_SIZE; ?>;
                width: 60%;
                padding: 20px 20px 20px 20px;
                box-sizing: content-box;
                background-color: rgba(0,0,0,0.8);
                text-align: left;
                text-indent: 5ch;
                border-radius: 20px;
                -webkit-border-radius: 20px;
                -moz-border-radius: 20px;
            }
        </style>

        <script src="<?php echo $JQ;?>"></script>
        <script src="<?php echo $JSCOOKIE;?>"></script>
        <script src="<?php echo $JSRANDOM;?>"></script>
        <script src="<?php echo $JSMD5;?>"></script>
        <script src="<?php echo $MOMENT;?>"></script>
        <script src="<?php echo $MOMENT_TZ;?>"></script>
        <script src="<?php echo $SITE_GLOBAL_JS;?>"></script>
        <script src="<?php echo $BACKGROUND_JS;?>"></script>
        <script src="<?php echo $NAVBAR_JS;?>"></script>

        <script language="javascript">
            var autoSkipTime = 1000 * 60 * 0.5;
            var autoHelpTime = 1000 * 60 * 0.25;
            var autoClockTime = 1000 * 60 & 0.1;

            var backgroundTimer;
            var helpTimer;
            var clockTimer;

            var isPlaying = true;
            var currentBackground = $("#background-img").attr("src");

            function newBackground() {
                randomizeBackground();
                currentBackground = $("#background-img").attr("src");
                clearInterval(backgroundTimer);
                var currentFilename = currentBackground.substr(currentBackground.lastIndexOf("/") + 1);
                $("#filename-a").attr("href", currentBackground);
                $("#filename-a").text(currentFilename);
                updateRefreshTime();
                clearInterval(backgroundTimer);
                if (isPlaying) {
                    backgroundTimer = setInterval(newBackground, autoSkipTime);
                }
            }

            function copyTextToClipboard(str) {
                var e = document.createElement('textarea');
                e.value = str;
                // This bit pushes the element off screen so we don't see it.
                e.setAttribute('readonly', '');
                e.style = {position: 'absolute', left: '-9999px'};
                document.body.appendChild(e);
                e.select();
                document.execCommand('copy');
                document.body.removeChild(e);
            }

            function hideHelp() {
                clearTimeout(helpTimer);
                hideDiv('help-div');
            }

            $(document).ready(function() {
                newBackground();
                hideDiv('filename-div');
                showDiv('clock-div');
                showDiv('help-div');
                helpTimer = setTimeout(hideHelp, autoHelpTime);
                clockTimer = setInterval(updateRefreshTime, autoClockTime);
            });

            $(document).click( function(event) {
                var cd = document.getElementById("clock-div");
                var fd = document.getElementById("filename-div");
                if(cd.contains(event.target)) {
                    if (isPlaying) {
                        // Pause
                        isPlaying = false;
                        clearInterval(backgroundTimer);
                        // Display filename and other stuff
                        showDiv('filename-div');
                    } else {
                        isPlaying = true;
                        backgroundTimer = setInterval(newBackground, autoSkipTime);
                        // Hide filename and other stuff
                        hideDiv('filename-div');
                    }
                } else if(fd.contains(event.target)) {
                    var url = $("#filename-a").attr("href");
                    //var text = $("#filename-a").text();
                    copyTextToClipboard(url);
                }
            });

            $(document).keypress( function(event) {
                if (event.which === 32 || event.which === 13) { // space or enter
                    newBackground();
                } else if (event.which === 67 || event.which === 99) { // 'C' or 'c'
                    var url = $("#filename-a").attr("href");
                    copyTextToClipboard(url);
                } else if (event.which === 72 || event.which === 104 || event.which === 63) { // 'H', 'h', or '?'
                    clearTimeout(helpTimer);
                    toggleDiv('help-div');
                } else if (event.which === 80 || event.which === 112) { // 'P' or 'p'
                    if (isPlaying) {
                        // Pause
                        isPlaying = false;
                        clearInterval(backgroundTimer);
                        // Display filename and other stuff
                        showDiv('filename-div');
                    } else {
                        isPlaying = true;
                        backgroundTimer = setInterval(newBackground, autoSkipTime);
                        // Hide filename and other stuff
                        hideDiv('filename-div');
                    }
                }
            });
        </script>
    </head>
    <body bgcolor="<?php echo $BGCOLOR; ?>" text="<?php echo $TEXT; ?>" link="<?php echo $UNVISITED; ?>" vlink="<?php echo $VISITED; ?>">
        <div id="background-div">
            <img id="background-img" src="<?php echo $BACKGROUND_URL; ?>" />
        </div>
        <div id="filename-div">
            <a id="filename-a" href="<?php echo $BACKGROUND_URL; ?>"><?php echo $BACKGROUND_URL; ?></a>
        </div>
        <div id="clock-div">
            <span id="refresh-time">--:-- ---</span>
        </div>
        <div id="help-div">
            <p>
                Click on the clock button in the upper right
                to pause the slideshow, or resume it.  The
                keyboard shortcut 'p' also does this.
            </p>
            <p>
                When paused, the image name is displayed in the
                upper left.  Click on that to copy the URL of the
                image to your clipboard.  The keyboard shortcut
                'c' does this too.
            </p>
            <p>
                You can hit 'spacebar' or 'return' to advance to
                the next image manually, while paused or not.
            </p>
            <p>
                The 'h' or '?' keys hide, or reveal, this helpful help!
            </p>
        </div>
    </body>
</html>
