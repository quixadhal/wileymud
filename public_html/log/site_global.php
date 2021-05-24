<?php
function is_local_ip() {
    $visitor_ip = $_SERVER['REMOTE_ADDR'];
    if($visitor_ip == '104.156.100.167') // Hard coded DNS entry
        return 1;
    $varr = explode(".", $visitor_ip);
    if($varr[0] == "192" && $varr[1] == "168")
        return 1;
    return 0;
}
function pcmd($command) {
    $data = "";
    $fp = popen("$command", "r");
    do {
        $data .= fread($fp, 8192);
    } while(!feof($fp));
    pclose($fp);
    echo $data;
}

$isLocal                = false;
if(is_local_ip()) {
    $isLocal = true;
}

$URL_HOME               = "http://wileymud.themud.org/~wiley";
//$URL_HOME               = "http://104.156.100.167/~wiley";
$FILE_HOME              = "/home/wiley/public_html";

$JQ                     = "$URL_HOME/log/jquery-3.3.1.min.js";
$JQUI_CSS               = "$URL_HOME/log/jquery-ui-1.12.1.custom/jquery-ui.min.css";
$JQUI_THEME             = "$URL_HOME/log/jquery-ui-1.12.1.custom/jquery-ui.theme.min.css";
$JQUI                   = "$URL_HOME/log/jquery-ui-1.12.1.custom/jquery-ui.min.js";
$JSCOOKIE               = "$URL_HOME/log/js.cookie.min.js";
$JSRANDOM               = "$URL_HOME/log/js.random.js";
$JSMD5                  = "$URL_HOME/log/js.md5.js";
$MOMENT                 = "$URL_HOME/log/moment/moment-with-locales.min.js";
$MOMENT_TZ              = "$URL_HOME/log/moment/moment-timezone-with-data.min.js";

$SITE_GLOBAL_FILE       = "$FILE_HOME/log/site_global.css";
$SITE_GLOBAL_TIME       = filemtime($SITE_GLOBAL_FILE);
$SITE_GLOBAL_CSS        = "$URL_HOME/log/site_global.css?version=$SITE_GLOBAL_TIME";
$SITE_GLOBAL_JS         = "$URL_HOME/log/site_global_js.php";

$BGCOLOR                = "black";
$TEXT                   = "#d0d0d0";
$UNVISITED              = "#ffffbf";
$VISITED                = "#ffa040";
$DELETED                = "#ff0000";
$EVEN                   = "rgba(31,31,31,0.7)";
$ODD                    = "rgba(0,0,0,0.7)";

$SCALE                  = 1.0;      // An easy way to adjust the overall size of everything.
$ICON_BASE              = 64;
$FONT_BASE              = 16;   // 24pt 39px, 18pt 30px, 14pt 24px, 10pt 17px, 1.7 seems close

$ICON_SIZE              = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
$FONT_SIZE              = sprintf("%dpt", (int)($FONT_BASE * $SCALE));
$SMALL_FONT_SIZE        = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.90));
$TINY_FONT_SIZE         = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.70));

$MUDLIST_FILE           = "$FILE_HOME/mudlist.json";
$mudlist_text           = file_get_contents($MUDLIST_FILE);
$mudlist                = json_decode($mudlist_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
$WILEY_BUILD_NUMBER     = $mudlist["version"]["build"];
$WILEY_BUILD_DATE       = $mudlist["version"]["date"];
$WILEY_TIME             = $mudlist["time"];
$WILEY_BANNER_ICON      = "$URL_HOME/gfx/wileymud3.png";
$WILEY_BANNER_WIDTH     = 334;
$WILEY_BANNER_HEIGHT    = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
//$WILEY_BANNER_HEIGHT    = 48;
$WILEY_URL              = "telnet://wileymud.themud.org:3000";

?>
