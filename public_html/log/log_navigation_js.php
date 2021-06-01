<?php
require_once 'site_global.php';
require_once 'navbar.php';
require_once 'log_navigation.php';
// NOTE:  You must include log_navigation_dates.js first or this will error!
header("Content-Type: application/json; charset=UTF-8");
?>
var Today = <?php echo $today; ?>;
var Yesterday = <?php echo $yesterday; ?>;
var Tomorrow = <?php echo $tomorrow; ?>;

function dateFromUrl(url) {
    var bits;

    if(typeof(url) === "undefined" || url === null) {
        bits = window.location.href.toString().split('/');
    } else {
        bits = url.split('/');
    }
    var my_date = bits[bits.length-1].substr(0,10);
}

function checkDate(date) {
    var m = date.getMonth();
    var d = date.getDate();
    var y = date.getFullYear();
     
    // First convert the date in to the yyyy-mm-dd format 
    // Take note that we will increment the month count by 1 
    var candidateDate = ("0000" + y).slice(-4) + '-' 
        + ("00" + (m + 1)).slice(-2) + '-' 
        + ("00" + d).slice(-2);
     
    if ($.inArray(candidateDate, validDates) != -1 ) {
        return [true];
    }
    return [false];
}

function gotoNewPage(dateString) {
    // We could also go to the static pages...
    window.location.href = "<?php echo $URL_HOME; ?>/log/?date=" + dateString;
}

