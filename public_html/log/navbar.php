<?php
require_once 'site_global.php';

$BEGIN_ICON             = "$URL_HOME/gfx/nav/begin.png";
$PREV_ICON              = "$URL_HOME/gfx/nav/previous.png";
$NEXT_ICON              = "$URL_HOME/gfx/nav/next.png";
$END_ICON               = "$URL_HOME/gfx/nav/end.png";

$TOP_ICON               = "$URL_HOME/gfx/nav/up_arrow.png";
$UP_ICON                = "$URL_HOME/gfx/nav/up_arrow.png";
$DOWN_ICON              = "$URL_HOME/gfx/nav/down_arrow.png";
$BOTTOM_ICON            = "$URL_HOME/gfx/nav/down_arrow.png";
$PLAY_ICON              = "$URL_HOME/gfx/nav/play.png";
$PAUSE_RED_ICON         = "$URL_HOME/gfx/nav/pause_orange.png";
$PAUSE_GREY_ICON        = "$URL_HOME/gfx/nav/pause_grey.png";
$HOME_ICON              = "$URL_HOME/gfx/nav/home.png";

$MUDLIST_ICON           = "$URL_HOME/gfx/bar/mud.png";
$OTHER_LOG_ICON         = "$URL_HOME/gfx/bar/other_logs.png";
$LOG_ICON               = "$URL_HOME/gfx/bar/log.png";
$PIE_ICON               = "$URL_HOME/gfx/bar/pie_chart.png";
$QUESTION_ICON          = "$URL_HOME/gfx/bar/question_girl3.png";
$FORUM_ICON             = "$URL_HOME/gfx/bar/skull.png";
$SERVER_ICON            = "$URL_HOME/gfx/bar/server_icon.png";
$GITHUB_ICON            = "$URL_HOME/gfx/bar/github1600.png";
$DISCORD_ICON           = "$URL_HOME/gfx/bar/discord.png";
$BG_CUTE_ICON           = "$URL_HOME/gfx/bar/painting_kawaii.png";
$BG_ON_ICON             = "$URL_HOME/gfx/bar/painting_on.png";
$BG_OFF_ICON            = "$URL_HOME/gfx/bar/painting_off.png";

$NAVBAR_FILE            = "$FILE_HOME/navbar_css.php";
$NAVBAR_TIME            = filemtime($NAVBAR_FILE);
$NAVBAR_CSS             = "$URL_HOME/navbar_css.php?version=$NAVBAR_TIME";
$NAVBAR_JS              = "$URL_HOME/navbar_js.php";

$MUDLIST_URL            = "$URL_HOME/mudlist.php";
$LOG_URL                = "$URL_HOME/";
$OTHER_LOG_URL          = "https://themud.org/chanhist.php#Channel=all";
$PIE_URL                = "$URL_HOME/pie.php";
$QUESTION_URL           = "$URL_HOME/random_video.php";
if($isLocal) {
    $QUESTION_URL       = "$URL_HOME/all_videos.php";
}
$FORUM_URL              = "$URL_HOME/lpmuds.net/forum/index.html";
$SERVER_URL             = "$URL_HOME/server.php";
$GITHUB_URL             = "https://github.com/quixadhal/wileymud";
$DISCORD_URL            = "https://discord.gg/kUduSsJ";
?>
