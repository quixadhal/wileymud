<?php
require_once 'site_global.php';

$playlist_list = file("$FILE_HOME/data/autoplaylist.txt", FILE_SKIP_EMPTY_LINES);
$url = sprintf("Location: %s", $playlist_list[array_rand($playlist_list)]);
header($url);
die("You are no fun.");
?>
