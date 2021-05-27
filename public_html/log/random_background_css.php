<?php
require_once 'site_global.php';
require_once 'random_background.php';
header("Content-type: text/css");
?>
#background-div {
    padding: 0px;
    margin: 0px;
    z-index: -1;
    opacity: 0.50;
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    position: fixed;
    height: 100%;
    width: 100%;
}
#background-img {
    height: 100%;
    width: 100%;
    border: none;
    object-fit: cover;
}
