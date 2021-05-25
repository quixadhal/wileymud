<?php
require_once 'site_global.php';
require_once 'navbar.php';
header("Content-Type: application/json; charset=UTF-8");
?>
function updateRefreshTime() {
    var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
    var yourLocale = (navigator.languages && navigator.languages.length) ?
        navigator.languages[0] : navigator.language;

    var momentObj = moment().tz(yourTimeZone);
    //var momentStr = momentObj.format('YYYY-MM-DD HH:mm:ss z');
    var momentStr = momentObj.format('HH:mm z');
    var momentHour = momentObj.hour();

    var rt = document.getElementById("refresh-time");
    //yt.innerHTML = "[" + yourLocale + " " + yourTimeZone + "] " + momentStr;
    rt.innerHTML = momentStr;
    rt.style.color = hour_map[momentHour];
}
