<?php
$time_start = microtime(true);

global $PG_DB;
global $PG_USERNAME;
global $PG_PASSWORD;

$MUDLIST_FILE   = "/home/wiley/public_html/mudlist.json";
$PG_DB          = "i3log";
$PG_USERNAME    = "wiley";
$PG_PASSWORD    = "tardis69";
$URL_HOME       = "http://wileymud.themud.org/~wiley";
$BACKGROUND_DIR = "/home/wiley/public_html/gfx/wallpaper/";

function random_image($dir) {
    $old_dir = getcwd();
    chdir($dir);

    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $file_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($file_list);

    chdir($old_dir);
    return $file_list[$pick];
}

function db_connect() {
    global $PG_DB;
    global $PG_USERNAME;
    global $PG_PASSWORD;

    $db = null;
    try {
        //$db = new PDO( "pgsql:host=localhost;port=5432;dbname=$PG_DB;user=$PG_USERNAME;password=$PG_PASSWORD", null, null, array(
        $db = new PDO( "pgsql:dbname=$PG_DB;user=$PG_USERNAME;password=$PG_PASSWORD", null, null, array(
            //PDO::ATTR_PERSISTENT        => true, 
            PDO::ATTR_ERRMODE           => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_EMULATE_PREPARES  => false,
        ));
    } catch(PDOException $e) {
        echo $e->getMessage();
    }
    return $db;
}

function fetch_dates($db) {
    $sql = "
        SELECT date('now') AS today,
               date(now() - '1 day'::interval) as yesterday,
               date(now() - '1 week'::interval) as week,
               date(now() - '1 month'::interval) as month,
               date(now() - '1 year'::interval) as year,
               min(date(local)) as all
          FROM i3log
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = $sth->fetch();
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

function fetch_today_speakers($db) {
    $sql = "
        SELECT speaker, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
           AND local BETWEEN date_trunc('day',now()) AND now()
      GROUP BY speaker
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        //$sth->bindParam(':time_interval', $time_interval);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            //$result[$row['pinkfish']] = $row;
            $result[] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function fetch_today_channels($db) {
    $sql = "
        SELECT channel, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
           AND local BETWEEN date_trunc('day',now()) AND now()
      GROUP BY channel
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        //$sth->bindParam(':time_interval', $time_interval);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            //$result[$row['pinkfish']] = $row;
            $result[] = $row;
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

function fetch_yesterday_speakers($db) {
    $sql = "
        SELECT speaker, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
           AND local BETWEEN date_trunc('day',now() - '1 day'::interval)
           AND date_trunc('day', now()) - '1 microsecond'::interval
      GROUP BY speaker
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

function fetch_yesterday_channels($db) {
    $sql = "
        SELECT channel, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
           AND local BETWEEN date_trunc('day',now() - '1 day'::interval)
           AND date_trunc('day', now()) - '1 microsecond'::interval
      GROUP BY channel
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

function fetch_historical_speakers($db, $period) {
    if( $period === NULL ) {
        $period = '1 week';
    }
    $sql = "
        SELECT speaker, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
           AND local BETWEEN date_trunc('day',now() - :period ::interval)
           AND now()
      GROUP BY speaker
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->bindParam(':period', $period);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

function fetch_historical_channels($db, $period) {
    if( $period === NULL ) {
        $period = '1 week';
    }
    $sql = "
        SELECT channel, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
           AND local BETWEEN date_trunc('day',now() - :period ::interval)
           AND now()
      GROUP BY channel
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->bindParam(':period', $period);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

function fetch_all_speakers($db) {
    $sql = "
        SELECT speaker, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
      GROUP BY speaker
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

function fetch_all_channels($db) {
    $sql = "
        SELECT channel, count(*)
          FROM i3log
         WHERE speaker <> 'URLbot'
      GROUP BY channel
      ORDER BY count DESC
         LIMIT 12
    ;";
    try {
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        while($row = $sth->fetch()) {
            $result[] = $row;
        }
    } catch (Exception $e) {
        throw $e;
    }
    return $result;
}

$mudlist_text = file_get_contents($MUDLIST_FILE);
$mudlist = json_decode($mudlist_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);

if( json_last_error() != JSON_ERROR_NONE ) {
    echo "<hr>".json_last_error_msg()."<br><hr>";
}

$WILEY_BUILD_NUMBER     = $mudlist["version"]["build"];
$WILEY_BUILD_DATE       = $mudlist["version"]["date"];
$WILEY_TIME             = $mudlist["time"];
$BACKGROUND             = random_image($BACKGROUND_DIR);
$OVERLAY_ICON           = "$URL_HOME/gfx/NA.png";
// contain leaves space, cover clips edges.
$BACKGROUND_IMG         = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover;\" src=\"$URL_HOME/gfx/wallpaper/$BACKGROUND\" />";
$OVERLAY_IMG            = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 999; left: 50%; transform: translateX(-50%); object-fit: contain; opacity: 0.5;\" src=\"$OVERLAY_ICON\" />";

$TODAY_BACKGROUND           = random_image($BACKGROUND_DIR);
$TODAY_BACKGROUND_IMG       = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover; opacity: 0.2;\" src=\"$URL_HOME/gfx/wallpaper/$TODAY_BACKGROUND\" />";
$YESTERDAY_BACKGROUND       = random_image($BACKGROUND_DIR);
$YESTERDAY_BACKGROUND_IMG   = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover; opacity: 0.2;\" src=\"$URL_HOME/gfx/wallpaper/$YESTERDAY_BACKGROUND\" />";
$WEEK_BACKGROUND            = random_image($BACKGROUND_DIR);
$WEEK_BACKGROUND_IMG        = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover; opacity: 0.2;\" src=\"$URL_HOME/gfx/wallpaper/$WEEK_BACKGROUND\" />";
$MONTH_BACKGROUND           = random_image($BACKGROUND_DIR);
$MONTH_BACKGROUND_IMG       = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover; opacity: 0.2;\" src=\"$URL_HOME/gfx/wallpaper/$MONTH_BACKGROUND\" />";
$YEAR_BACKGROUND            = random_image($BACKGROUND_DIR);
$YEAR_BACKGROUND_IMG        = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover; opacity: 0.2;\" src=\"$URL_HOME/gfx/wallpaper/$YEAR_BACKGROUND\" />";
$ALL_BACKGROUND             = random_image($BACKGROUND_DIR);
$ALL_BACKGROUND_IMG         = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 998; left: 50%; transform: translateX(-50%); object-fit: cover; opacity: 0.2;\" src=\"$URL_HOME/gfx/wallpaper/$ALL_BACKGROUND\" />";

$db = db_connect();
$date_info = fetch_dates($db);

$active_style   = "background-color: #8f4f2f; opacity: 1.0;";
$hover_style    = "background-color: #4f0000; opacity: 1.0;";
$inactive_style = "background-color: #2f0000; opacity: 0.6;";

$activechan_style   = "background-color: #2f4f8f; opacity: 1.0; font-size: 60%";
$hoverchan_style    = "background-color: #00004f; opacity: 1.0; font-size: 60%";
$inactivechan_style = "background-color: #00002f; opacity: 0.6; font-size: 60%";
?>

<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <meta http-equiv="refresh" content="1800" />
        <!-- Global site tag (gtag.js) - Google Analytics -->
        <script async src="https://www.googletagmanager.com/gtag/js?id=UA-163395867-1"></script>
        <script>
          window.dataLayer = window.dataLayer || [];
          function gtag(){dataLayer.push(arguments);}
          gtag('js', new Date());

          gtag('config', 'UA-163395867-1');
        </script>
        <style>
            html, body { table-layout: fixed; max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis;}
            table { table-layout: fixed; max-width: 99%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
            a:active, a:focus { outline: 0; border: none; -moz-outline-style: none; }
            #navbar { position: fixed; top: 0; background-color: black; opacity: 1.0; z-index: 2; }
            #content { padding-top: 48px; }
        </style>
        <title>Welcome to WileyMUD!</title>
        <script type="text/javascript">
            var current_button;
            var current_graph;

            function get_today() {
                current_button = document.getElementById("btnToday");
                current_graph = document.getElementById("graphToday");
                current_graph.style.display = "block";
            }
            function hover_on(x) {
                if(current_button != x) {
                    var s = x.id.substr(0,3);
                    if(s == "btn") {
                        x.style = "<?php echo $hover_style; ?>";
                    } else {
                        x.style = "<?php echo $hoverchan_style; ?>";
                    }
                }
            }
            function hover_off(x) {
                if(current_button != x) {
                    var s = x.id.substr(0,3);
                    if(s == "btn") {
                        x.style = "<?php echo $inactive_style; ?>";
                    } else {
                        x.style = "<?php echo $inactivechan_style; ?>";
                    }
                }
            }
            function click_select(x) {
                if(current_button != x) {
                    var s = x.id.substr(0,3);
                    var g;
                    if(s == "btn") {
                        x.style = "<?php echo $active_style; ?>";
                        g = document.getElementById("graph" + x.id.substr(3));
                    } else {
                        x.style = "<?php echo $activechan_style; ?>";
                        g = document.getElementById("graphChannels" + x.id.substr(3));
                    }
                    s = current_button.id.substr(0,3);
                    if(s == "btn") {
                        current_button.style = "<?php echo $inactive_style; ?>";
                    } else {
                        current_button.style = "<?php echo $inactivechan_style; ?>";
                    }
                    current_button = x;
                    current_graph.style.display = "none";
                    current_graph = g;
                    g.style.display = "block";
                }
            }
        </script>
        <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
        <script type="text/javascript">
            google.charts.load('current', {'packages':['corechart']});
            google.charts.setOnLoadCallback(drawTheCharts);
            var options;

            function drawSetup() {
                options = {
                    is3D                : 1,
                    title               : "Today's Jibber-Jabber",
                    titleColor          : '#e0e0e0',
                    //backgroundColor     : '#101010',
                    backgroundColor     : 'none',
                    legendTextStyle     : {
                        color           : '#e0e0e0',
                    },
                    pieSliceTextStyle   : {
                        color           : '#000000',
                        fontName        : 'Arial',
                        fontSize        : '14',
                    },
                    chartArea           : {
                        top             : '5%',
                        left            : '5%',
                        height          : '95%',
                        width           : '95%',
                    },
                    colors              : [
                        '#ff3f3f', '#ffaf3f', '#ffff3f',
                        '#afff3f', '#3fff3f', '#3fffaf',
                        '#3fffff', '#3fafff', '#3f3fff',
                        '#af3fff', '#ff3fff', '#ff3faf',
                    ],
                };
            }

            function drawTodaySpeakers() {
                <?php
                $result = fetch_today_speakers($db);
                if(count($result) > 0) {
                ?>
                    options['title'] = "Today's Jibber-Jabber (<?php echo $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['speaker'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphToday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_today'));
                    chart.draw(data, options);
                    document.getElementById('graphToday').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_today').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawTodayChannels() {
                <?php
                $result = fetch_today_channels($db);
                if(count($result) > 0) {
                ?>
                    options['title'] = "Today's Jibber-Jabber (<?php echo $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['channel'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsToday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_today'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsToday').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_today').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawYesterdaySpeakers() {
                <?php
                $result = fetch_yesterday_speakers($db);
                if(count($result) > 0) {
                ?>
                    options['title'] = "Yesterday's Rubbish (<?php echo $date_info['yesterday'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['speaker'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphYesterday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_yesterday'));
                    chart.draw(data, options);
                    document.getElementById('graphYesterday').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_yesterday').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawYesterdayChannels() {
                <?php
                $result = fetch_yesterday_channels($db);
                if(count($result) > 0) {
                ?>
                    options['title'] = "Yesterday's Rubbish (<?php echo $date_info['yesterday'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['channel'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsYesterday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_yesterday'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsYesterday').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_yesterday').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawWeekSpeakers() {
                <?php
                $result = fetch_historical_speakers($db, '1 week');
                if(count($result) > 0) {
                ?>
                    options['title'] = "The Week of Stupidity (<?php echo $date_info['week'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['speaker'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphWeek').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_week'));
                    chart.draw(data, options);
                    document.getElementById('graphWeek').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_week').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawWeekChannels() {
                <?php
                $result = fetch_historical_channels($db, '1 week');
                if(count($result) > 0) {
                ?>
                    options['title'] = "The Week of Stupidity (<?php echo $date_info['week'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['channel'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsWeek').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_week'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsWeek').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_week').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawMonthSpeakers() {
                <?php
                $result = fetch_historical_speakers($db, '1 month');
                if(count($result) > 0) {
                ?>
                    options['title'] = "A Whole Month of Nonsense? (<?php echo $date_info['month'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['speaker'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphMonth').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_month'));
                    chart.draw(data, options);
                    document.getElementById('graphMonth').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_month').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawMonthChannels() {
                <?php
                $result = fetch_historical_channels($db, '1 month');
                if(count($result) > 0) {
                ?>
                    options['title'] = "A Whole Month of Nonsense? (<?php echo $date_info['month'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['channel'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsMonth').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_month'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsMonth').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_month').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawYearSpeakers() {
                <?php
                $result = fetch_historical_speakers($db, '1 year');
                if(count($result) > 0) {
                ?>
                    options['title'] = "What a Horrible Year it's Been... (<?php echo $date_info['year'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['speaker'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphYear').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_year'));
                    chart.draw(data, options);
                    document.getElementById('graphYear').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_year').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawYearChannels() {
                <?php
                $result = fetch_historical_channels($db, '1 year');
                if(count($result) > 0) {
                ?>
                    options['title'] = "What a Horrible Year it's Been... (<?php echo $date_info['year'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['channel'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsYear').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_year'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsYear').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_year').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawAllSpeakers() {
                <?php
                $result = fetch_all_speakers($db);
                if(count($result) > 0) {
                ?>
                    options['title'] = "What Have You DONE??? (<?php echo $date_info['all'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['speaker'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphAll').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_all'));
                    chart.draw(data, options);
                    document.getElementById('graphAll').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_all').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawAllChannels() {
                <?php
                $result = fetch_all_channels($db);
                if(count($result) > 0) {
                ?>
                    options['title'] = "What Have You DONE??? (<?php echo $date_info['all'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ '%s', %d ],\n", $r['channel'], $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsAll').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_all'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsAll').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_all').innerHTML='<?php echo $BACKGROUND_IMG; echo $OVERLAY_IMG;?>';
                <?php } ?>
            }

            function drawTheCharts() {
                drawSetup();

                drawTodaySpeakers();
                drawYesterdaySpeakers();
                drawWeekSpeakers();
                drawMonthSpeakers();
                drawYearSpeakers();
                drawAllSpeakers();

                drawTodayChannels();
                drawYesterdayChannels();
                drawWeekChannels();
                drawMonthChannels();
                drawYearChannels();
                drawAllChannels();

                document.getElementById('graphToday').style.display='block';
            }
        </script>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="get_today()">
    <table id="navbar" width="99%" cellspacing="0" cellpadding="1" align="center">
        <tr>
            <td align="left" width="25%">
                <a href="http://wileymud.themud.org/~wiley/mudlist.php" alt="Mudlist" title="Mudlist">
                    <img src="http://wileymud.themud.org/~wiley/gfx/mud.png" width="48" height="48" border="0" />
                </a>
                <a href="http://wileymud.themud.org/~wiley/logpages" alt="Logs" title="Logs">
                    <img src="http://wileymud.themud.org/~wiley/gfx/log.png" width="48" height="48" border="0" />
                </a>
                <a href="https://discord.gg/kUduSsJ" alt="Discord" title="Discord">
                    <img src="http://wileymud.themud.org/~wiley/gfx/discord.png" width="48" height="48" border="0" />
                </a>
            </td>
            <td align="right" width="24%">
                <a href="telnet://wileymud.themud.org:3000" alt="WileyMUD" title="WileyMUD">
                    <img src="http://wileymud.themud.org/~wiley/gfx/wileymud3.png" width="247" height="48" border="0" />
                </a>
            </td>
            <td align="center" width="">
                &nbsp;
            </td>
            <td valign="top" align="left" width="24%">
                    <table border="0" cellspacing="0" cellpadding="0">
                        <tr>
                            <td align="right">
                                <span style="color: #bb0000"> Version: </span>
                            </td>
                            <td align="center" width="10">
                                &nbsp;
                            </td>
                            <td align="left">
                                <span style="color: #bb0000"> <?php echo $WILEY_BUILD_NUMBER; ?> </span>
                            </td>
                        </tr>
                        <tr>
                            <td align="right">
                                <span style="color: #bb0000"> Build Date: </span>
                            </td>
                            <td align="center" width="10">
                                &nbsp;
                            </td>
                            <td align="left">
                                <span style="color: #bb0000"> <?php echo $WILEY_BUILD_DATE; ?> </span>
                            </td>
                        </tr>
                    </table>
            </td>
            <td align="right" width="25%">
                <span style="vertical-align: top; color: #ccaa00"> <?php echo $WILEY_TIME; ?> </span>
                <a href="http://wileymud.themud.org/~wiley/server.php" alt="Server" title="Server">
                    <img src="http://wileymud.themud.org/~wiley/gfx/server_icon.png" width="48" height="48" border="0" />
                </a>
                    <img src="http://wileymud.themud.org/~wiley/gfx/pie_chart.png" width="48" height="48" border="0" style="opacity: 0.2; background: rgba(255,0,0,0.25);" />
            </td>
        </tr>
    </table>

<div id="content" align="center">
    <table border="0" cellspacing="0" cellpadding="1" width="80%">
        <tr>
            <td id="btnToday" style="<?php echo $active_style; ?>;" align="center" width="23%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                Today
            </td>
            <td id="chnToday" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                今日ちゃん
            </td>
            <td id="btnYesterday" style="<?php echo $inactive_style; ?>;" align="center" width="24%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                Yesterday
            </td>
            <td id="chnYesterday" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                昨日ちゃん
            </td>
            <td id="btnWeek" style="<?php echo $inactive_style; ?>;" align="center" width="23%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                Last Week
            </td>
            <td id="chnWeek" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                週間ちゃん
            </td>
        </tr>
        <tr>
            <td id="btnMonth" style="<?php echo $inactive_style; ?>;" align="center" width="23%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                The Month
            </td>
            <td id="chnMonth" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                月ちゃん
            </td>
            <td id="btnYear" style="<?php echo $inactive_style; ?>;" align="center" width="24%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                A Year Ago
            </td>
            <td id="chnYear" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                年ちゃん
            </td>
            <td id="btnAll" style="<?php echo $inactive_style; ?>;" align="center" width="23%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                Of All Time
            </td>
            <td id="chnAll" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                常にちゃん
            </td>
        </tr>
        <tr bgcolor="#000000">
            <td id="graphHole" colspan="3" align="center">
                <div id="graphToday" style="display: none;">
                    <?php echo $TODAY_BACKGROUND_IMG; ?>
                    <div id="pie_today" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphYesterday" style="display: none;">
                    <?php echo $YESTERDAY_BACKGROUND_IMG; ?>
                    <div id="pie_yesterday" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphWeek" style="display: none;">
                    <?php echo $WEEK_BACKGROUND_IMG; ?>
                    <div id="pie_week" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphMonth" style="display: none;">
                    <?php echo $MONTH_BACKGROUND_IMG; ?>
                    <div id="pie_month" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphYear" style="display: none;">
                    <?php echo $YEAR_BACKGROUND_IMG; ?>
                    <div id="pie_year" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphAll" style="display: none;">
                    <?php echo $ALL_BACKGROUND_IMG; ?>
                    <div id="pie_all" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphChannelsToday" style="display: none;">
                    <?php echo $TODAY_BACKGROUND_IMG; ?>
                    <div id="piechan_today" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphChannelsYesterday" style="display: none;">
                    <?php echo $YESTERDAY_BACKGROUND_IMG; ?>
                    <div id="piechan_yesterday" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphChannelsWeek" style="display: none;">
                    <?php echo $WEEK_BACKGROUND_IMG; ?>
                    <div id="piechan_week" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphChannelsMonth" style="display: none;">
                    <?php echo $MONTH_BACKGROUND_IMG; ?>
                    <div id="piechan_month" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphChannelsYear" style="display: none;">
                    <?php echo $YEAR_BACKGROUND_IMG; ?>
                    <div id="piechan_year" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
                <div id="graphChannelsAll" style="display: none;">
                    <?php echo $ALL_BACKGROUND_IMG; ?>
                    <div id="piechan_all" style="width: 700px; height: 500px; position: fixed; z-index: 999; left: 50%; transform: translateX(-50%);"></div>
                </div>
            </td>
        </tr>
        <tr bgcolor="#000000">
            <?php $time_end = microtime(true); $time_spent = $time_end - $time_start; ?>
            <td align="right" colspan="6"><font size="-1" color="#1f1f1f"><?php echo sprintf("%9.4f", $time_spent); ?> seconds</font></td>
        </tr>
    </table>
</div>
</body>
</html>
