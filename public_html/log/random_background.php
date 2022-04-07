<?php
require_once 'site_global.php';
require_once 'log_navigation.php';

$BACKGROUND_URL         = "$URL_HOME/gfx/one_black_pixel.png";
$BACKGROUND_DIR         = "$FILE_HOME/gfx/wallpaper/";
$SPECIAL_DIR            = "$FILE_HOME/gfx/wallpaper/$month_day/";
$BORING_DIR             = "$FILE_HOME/gfx/wallpaper/scenic/";
$BACKGROUND_DIR_URL     = "$URL_HOME/gfx/wallpaper";
$SPECIAL_DIR_URL        = "$URL_HOME/gfx/wallpaper/$month_day";
$BORING_DIR_URL         = "$URL_HOME/gfx/wallpaper/scenic";
$BACKGROUND_FILE        = "$FILE_HOME/random_background_css.php";
$BACKGROUND_TIME        = filemtime($BACKGROUND_FILE);
$BACKGROUND_CSS         = "$URL_HOME/random_background_css.php?version=$BACKGROUND_TIME";
$BACKGROUND_JS          = "$URL_HOME/random_background_js.php";
$background_image_list  = array();
$special_image_list     = array();
$boring_image_list      = array();
$today_dir_exists       = false;

function random_background($dir) {
    global $background_image_list;
    global $special_image_list;
    global $boring_image_list;
    global $month_day;
    global $today_dir_exists;
    $old_dir = getcwd();
    $today_dir = "$dir/$month_day";
    $boring_dir = "$dir/scenic";

    chdir($boring_dir);
    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $boring_image_list = array_merge($jpg_list, $png_list);

    chdir($dir);
    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $background_image_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($background_image_list);

    if(is_dir($today_dir)) {
        // This allows us to create special subdirectories
        // which will override the normal set for a specific
        // date each year (mm-dd format), so we can have
        // holiday specific images.
        $today_dir_exists = true;
        chdir($today_dir);
        $jpg_list = glob("*.jpg");
        $png_list = glob("*.png");
        $special_image_list = array_merge($jpg_list, $png_list);
        $pick = array_rand($special_image_list);
    }
    chdir($old_dir);
    return $background_image_list[$pick];
}

// We don't USE the result here, but we populate the array by calling it.
// That, in turn, lets us push that array contents into javascript later.
random_background($BACKGROUND_DIR);
?>
