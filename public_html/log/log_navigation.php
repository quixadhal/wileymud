<?php
require_once 'site_global.php';
require_once 'navbar.php';

function load_date_counts($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        ksort($data, SORT_NATURAL);
        //print "\nDATE_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

$DATE_CACHE     = "$FILE_HOME/data/date_counts.json";
$date_counts    = load_date_counts($DATE_CACHE);
$first_date     = $date_counts[array_key_first($date_counts)]['the_date'];
$last_date      = $date_counts[array_key_last($date_counts)]['the_date'];

$tz_obj = new DateTimeZone("America/Los_Angeles");
$date_obj = new DateTime('NOW', $tz_obj);
$date_obj->setTime(0,0);    // Set the clock at midnight of the current day.

$today = $date_obj->format('Y-m-d');
$month_day = $date_obj->format('m-d');
$date_obj->modify('-1 day');
$yesterday = $date_obj->format('Y-m-d');
$date_obj->modify('-2 day');
$tomorrow = $date_obj->format('Y-m-d');

$no_data = false;
if( $last_date !== $today ) {
    $no_data = true;
}

$LOG_NAV_DATES_JS       = "$URL_HOME/data/log_navigation_dates.js";
$LOG_NAV_JS             = "$URL_HOME/log_navigation_js.php";
$LOG_NAV_CSS            = "$URL_HOME/log_navigation_css.php";
?>
