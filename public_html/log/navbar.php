<?php
require_once 'site_global.php';

$BEGIN_ICON             = "$URL_HOME/gfx/nav/begin.png";
$PREV_ICON              = "$URL_HOME/gfx/nav/previous.png";
$NEXT_ICON              = "$URL_HOME/gfx/nav/next.png";
$END_ICON               = "$URL_HOME/gfx/nav/end.png";

$TOP_ICON               = "$URL_HOME/gfx/nav/green/top.png";
$UP_ICON                = "$URL_HOME/gfx/nav/green/up.png";
$DOWN_ICON              = "$URL_HOME/gfx/nav/green/down.png";
$BOTTOM_ICON            = "$URL_HOME/gfx/nav/green/bottom.png";

$MUDLIST_ICON           = "$URL_HOME/gfx/mud.png";
$OTHER_LOG_ICON         = "$URL_HOME/gfx/other_logs.png";
$LOG_ICON               = "$URL_HOME/gfx/log.png";
$PIE_ICON               = "$URL_HOME/gfx/pie_chart.png";
$QUESTION_ICON          = "$URL_HOME/gfx/question_girl3.png";
$SERVER_ICON            = "$URL_HOME/gfx/server_icon.png";
$GITHUB_ICON            = "$URL_HOME/gfx/github1600.png";
$DISCORD_ICON           = "$URL_HOME/gfx/discord.png";

$NAVBAR_FILE            = "$FILE_HOME/log/navbar_css.php";
$NAVBAR_TIME            = filemtime($NAVBAR_FILE);
$NAVBAR_CSS             = "$URL_HOME/log/navbar_css.php?version=$NAVBAR_TIME";
$NAVBAR_JS              = "$URL_HOME/log/navbar_js.php";

$MUDLIST_URL            = "$URL_HOME/mudlist.php";
$LOG_URL                = "$URL_HOME/log/";
$OTHER_LOG_URL          = "https://themud.org/chanhist.php#Channel=all";
$PIE_URL                = "$URL_HOME/pie.php";
$QUESTION_URL           = "$URL_HOME/random_video.php";
$SERVER_URL             = "$URL_HOME/log/server.php";
$GITHUB_URL             = "https://github.com/quixadhal/wileymud";
$DISCORD_URL            = "https://discord.gg/kUduSsJ";
?>
