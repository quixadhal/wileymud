<?php
require_once 'site_global.php';
require_once 'navbar.php';
require_once 'log_navigation.php';
// NOTE:  You must include log_navigation_dates.js first or this will error!
header("Content-Type: application/json; charset=UTF-8");
?>
var Today = "<?php echo $today; ?>";
var MonthDay = "<?php echo $month_day; ?>";
var Yesterday = "<?php echo $yesterday; ?>";
var Tomorrow = "<?php echo $tomorrow; ?>";
var noData = <?php echo ($no_data == true) ? 1 : 0; ?>;
var queryParams = query_parameters();
var queryDate = queryParams.date;
var querySearch = queryParams.search;

if(queryDate !== null && queryDate !== undefined) {
    // We were given a date, maybe adjust Yesterday and Tommorrow?
} else {
    // We have none, so use Today
    queryDate = Today;
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
    if(dateString == Today) {
        if(querySearch !== null && querySearch !== undefined && querySearch !== '') {
            window.location.href = "<?php echo $URL_HOME; ?>/?search=" + querySearch;
        } else {
            window.location.href = "<?php echo $URL_HOME; ?>/";
        }
    } else {
        // We could also go to the static pages...
        if(querySearch !== null && querySearch !== undefined && querySearch !== '') {
            window.location.href = "<?php echo $URL_HOME; ?>/?noscroll&pause&date=" + dateString + "&search=" + querySearch;
        } else {
            window.location.href = "<?php echo $URL_HOME; ?>/?noscroll&pause&date=" + dateString;
        }
    }
}

$( function() {
    $( "#datepicker" ).datepicker({
        beforeShowDay   : checkDate,
        onSelect        : gotoNewPage,
        changeYear      : true,
        changeMonth     : true,
        dateFormat      : "yy-mm-dd",
        defaultDate     : queryDate,
     });
});
