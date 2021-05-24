<?php 
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'page_source.php';
require_once 'random_background.php';
require_once 'navbar.php';
$BACKGROUND_URL         = "$URL_HOME/gfx/one_black_pixel.png";
?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <link rel="stylesheet" href="<?php echo $SITE_GLOBAL_CSS;?>">
        <link rel="stylesheet" href="<?php echo $PAGE_SOURCE_CSS;?>">
        <link rel="stylesheet" href="<?php echo $BACKGROUND_CSS;?>">
        <link rel="stylesheet" href="<?php echo $NAVBAR_CSS;?>">

        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JSCOOKIE;?>""></script>
        <script src="<?php echo $JSRANDOM;?>""></script>
        <script src="<?php echo $JSMD5;?>""></script>
        <script src="<?php echo $MOMENT;?>""></script>
        <script src="<?php echo $MOMENT_TZ;?>""></script>
        <script src="<?php echo $SITE_GLOBAL_JS;?>""></script>

        <script language="javascript">
            var Random = new MersenneTwister();
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
?>
            function randomizeBackground() {
                var bg_choice = Math.floor(BackgroundImageList.length * Random.random());
                var new_bg = "<?php echo "$BACKGROUND_DIR_URL/"; ?>" + BackgroundImageList[bg_choice];
                $("#background-img").attr("src", new_bg);
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
            $(document).ready(function() {
                hideDiv('page-source');
                //showDiv('page-load-time');
                dim(document.getElementById('navbar-button-server'));
                randomizeBackground();
                updateRefreshTime();
            });
        </script>
    </head>
    <body bgcolor="<?php echo $BGCOLOR; ?>" text="<?php echo $TEXT; ?>" link="<?php echo $UNVISITED; ?>" vlink="<?php echo $VISITED; ?>">
        <div id="background-div">
            <img id="background-img" src="<?php echo $BACKGROUND_URL; ?>" />
        </div>
        <div id="back-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
        <div id="navbar-left">
            <img class="nav-img glowing" id="navbar-button-mudlist" title="List of MUDs" src="<?php echo $MUDLIST_ICON; ?>" onclick="window.location.href='<?php echo $MUDLIST_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-themudorg" title="I3 Log Page" src="<?php echo $LOG_ICON; ?>" onclick="window.location.href='<?php echo $LOG_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" onclick="window.location.href='<?php echo $PIE_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-question" title="???!" src="<?php echo $QUESTION_ICON; ?>" onclick="window.location.href='<?php echo $QUESTION_URL; ?>';" />
        </div>
        <div id="navbar-center">
            <img class="nav-img glowing" id="navbar-button-top" title="Top of page" src="<?php echo $TOP_ICON; ?>" onclick="scroll_to('content-top');" />
            <img class="nav-img glowing" id="navbar-button-bottom" title="Bottom of page" src="<?php echo $BOTTOM_ICON; ?>" onclick="scroll_to('content-bottom');" />
        </div>
        <div id="navbar-right">
            <span id="refresh-time">--:-- ---</span>
            <img class="nav-img" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" />
            <img class="nav-img glowing" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='<?php echo $GITHUB_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='<?php echo $DISCORD_URL; ?>';" />
        </div>
        <div id="fake-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
        <div id="page-source">
            <?php echo numbered_source(__FILE__); ?>
        </div>
    </body>
</html>
