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
function https() {
    return isset($_SERVER['HTTPS']) ? 'https' : 'http';
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

$URL_HOME                   = https() . "://wileymud.themud.org/log";
$FILE_HOME                  = "/home/www/log";

$JQ                         = "$URL_HOME/inc/jquery-3.3.1.min.js";
$JQUI_CSS                   = "$URL_HOME/inc/jquery-ui-1.12.1.custom/jquery-ui.min.css";
$JQUI_THEME                 = "$URL_HOME/inc/jquery-ui-1.12.1.custom/jquery-ui.theme.min.css";
$JQUI                       = "$URL_HOME/inc/jquery-ui-1.12.1.custom/jquery-ui.min.js";
$JSCOOKIE                   = "$URL_HOME/inc/js.cookie.min.js";
$JSRANDOM                   = "$URL_HOME/inc/js.random.js";
$JSMD5                      = "$URL_HOME/inc/js.md5.js";
$MOMENT                     = "$URL_HOME/inc/moment/moment-with-locales.min.js";
$MOMENT_TZ                  = "$URL_HOME/inc/moment/moment-timezone-with-data.min.js";
$LIGHTBOX_JS                = "$URL_HOME/inc/lightbox/lightbox.min.js";
$LIGHTBOX_CSS               = "$URL_HOME/inc/lightbox/lightbox.min.css";

$SITE_GLOBAL_FILE           = "$FILE_HOME/site_global_css.php";
$SITE_GLOBAL_TIME           = filemtime($SITE_GLOBAL_FILE);
$SITE_GLOBAL_CSS            = "$URL_HOME/site_global_css.php?version=$SITE_GLOBAL_TIME";
$SITE_GLOBAL_JS             = "$URL_HOME/site_global_js.php";

$BGCOLOR                    = "black";
$TEXT                       = "#d0d0d0";
$UNVISITED                  = "#ffffbf";
$VISITED                    = "#ffa040";
$DELETED                    = "#ff0000";
$EVEN                       = "rgba(31,31,31,0.7)";
$ODD                        = "rgba(0,0,0,0.7)";
$INPUT                      = "#d0d0d0";
$INPUT_BORDER               = "#101010";
$INPUT_BACKGROUND           = "#101010";
$SELECTED_INPUT             = "#f0f0f0";
$SELECTED_INPUT_BORDER      = "#101010";
$SELECTED_INPUT_BACKGROUND  = "#303030";


$SCALE                      = 1.0;      // An easy way to adjust the overall size of everything.
$ICON_BASE                  = 64;
$FONT_BASE                  = 14;   // 24pt 39px, 18pt 30px, 14pt 24px, 10pt 17px, 1.7 seems close

$ICON_SIZE                  = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
$SMALL_ICON_SIZE            = sprintf("%dpx", (int)($ICON_BASE * $SCALE * 0.75));
$FONT_SIZE                  = sprintf("%dpt", (int)($FONT_BASE * $SCALE));
$SMALL_FONT_SIZE            = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.90));
$TINY_FONT_SIZE             = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 0.70));
$BIG_FONT_SIZE              = sprintf("%dpt", (int)($FONT_BASE * $SCALE * 1.20));

$MUDLIST_JSON               = "$FILE_HOME/data/mudlist.json";
$mudlist_text               = file_get_contents($MUDLIST_JSON);
$mudlist                    = json_decode($mudlist_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
$WILEY_BUILD_NUMBER         = $mudlist["version"]["build"];
$WILEY_BUILD_DATE           = $mudlist["version"]["date"];
$WILEY_TIME                 = $mudlist["time"];
$WILEY_BANNER_ICON          = "$URL_HOME/gfx/wileymud3.png";
$WILEY_BANNER_WIDTH         = 334;
$WILEY_BANNER_HEIGHT        = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
//$WILEY_BANNER_HEIGHT      = 48;
$WILEY_URL                  = "telnet://wileymud.themud.org:3000";
$WILEY_IP                   = "wileymud.themud.org";
$WILEY_PORT                 = 3000;

$NOT_AVAILABLE_ICON         = "$URL_HOME/gfx/NA.png";
$ONE_PIXEL_ICON             = "$URL_HOME/gfx/one_black_pixel.png";

$LOG_PAGE_FILE              = "$FILE_HOME/log_page_css.php";
$LOG_PAGE_TIME              = filemtime($LOG_PAGE_FILE);
$LOG_PAGE_CSS               = "$URL_HOME/log_page_css.php?version=$LOG_PAGE_TIME";
$LOG_PAGE_JS                = "$URL_HOME/log_page_js.php";
$VISITOR_IP                 = $_SERVER['REMOTE_ADDR'];

?>
