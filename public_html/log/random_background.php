<?php
require_once 'site_global.php';

$BACKGROUND_DIR         = "$FILE_HOME/gfx/wallpaper/";
$BACKGROUND_DIR_URL     = "$URL_HOME/gfx/wallpaper";
$BACKGROUND_FILE        = "$FILE_HOME/log/random_background.css";
$BACKGROUND_TIME        = filemtime($BACKGROUND_FILE);
$BACKGROUND_CSS         = "$URL_HOME/log/random_background.css?version=$BACKGROUND_TIME";
$background_image_list  = array();

function random_background($dir) {
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

// We don't USE the result here, but we populate the array by calling it.
// That, in turn, lets us push that array contents into javascript later.
random_background($BACKGROUND_DIR);
?>
