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

$URL_HOME           = "http://wileymud.themud.org/~wiley";
$FILE_HOME          = "/home/wiley/public_html";
?>
