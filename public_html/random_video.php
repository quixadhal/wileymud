<?php
    $playlist_list = file("/home/wiley/public_html/playlist.txt", FILE_SKIP_EMPTY_LINES);
    $url = sprintf("Location: %s", $playlist_list[array_rand($playlist_list)]);
    header($url);
    die("You are no fun.");
?>
