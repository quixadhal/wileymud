<?php 
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'page_source.php';
require_once 'random_background.php';
require_once 'navbar.php';
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
        <script src="<?php echo $BACKGROUND_JS;?>""></script>
        <script src="<?php echo $NAVBAR_JS;?>""></script>

        <script language="javascript">
            var backgroundTimer;
            $(document).ready(function() {
                hideDiv('page-source');
                //showDiv('page-load-time');
                dim(document.getElementById('navbar-button-server'));
                randomizeBackground();
                updateRefreshTime();
                backgroundTimer = setInterval(randomizeBackground, 1000 * 60 * 5);
                // clearInterval(backgroundTimer);
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
            <table id="wileymud-table">
                <tr>
                    <td rowspan="2" align="right" width="<?php echo $WILEY_BANNER_WIDTH; ?>">
                        <img class="nav-banner-img glowing" id="navbar-button-wileymud" width="<?php echo $WILEY_BANNER_WIDTH; ?>" title="WileyMUD" src="<?php echo $WILEY_BANNER_ICON; ?>" />
                    </td>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-version" align="right">
                        Version:
                    </td>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-version" align="left">
                        <?php echo $WILEY_BUILD_NUMBER; ?>
                    </td>
                </tr>
                <tr>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-build-date" align="right">
                        Build Date:
                    </td>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-build-date" align="left">
                        <?php echo $WILEY_BUILD_DATE; ?>
                    </td>
                </tr>
            </table>
<!--
            <img class="nav-banner-img glowing" id="navbar-button-wileymud" width="<?php echo $WILEY_BANNER_WIDTH; ?>" title="WileyMUD" src="<?php echo $WILEY_BANNER_ICON; ?>" />
            <span id="wileymud-version">&nbsp;&nbsp;&nbsp;Version:&nbsp;<?php echo $WILEY_BUILD_NUMBER; ?></span>
            <span id="wileymud-build-date">Build Date:&nbsp;<?php echo $WILEY_BUILD_DATE; ?></span>
-->
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
