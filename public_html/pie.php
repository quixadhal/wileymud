<?php
$time_start = microtime(true);
global $fp;
global $DEBUG;
global $FRESH;
global $CACHE;
$DEBUG          = 0;
$DO_ALL_TIME    = 1;

if( $DEBUG ) {
    $DEBUG_LOG = "/home/wiley/public_html/pie/debug.log";

    if( file_exists($DEBUG_LOG) ) {
        chmod($DEBUG_LOG, 0664);
    }
    $fp = fopen($DEBUG_LOG, "w");
    fprintf($fp, "BEGIN ----------\n");
}

$MUDLIST_FILE   = "/home/wiley/public_html/mudlist.json";
$PG_DB          = "i3log";
$PG_USERNAME    = "wiley";
$PG_PASSWORD    = "tardis69";
$URL_HOME       = "http://wileymud.themud.org/~wiley";
$BACKGROUND_DIR = "/home/wiley/public_html/gfx/wallpaper/";
$PIE_DIR        = "/home/wiley/public_html/pie";

$FRESH['dates']     = 67;
$FRESH['quotes']    = 1701;
$FRESH['today']     = 67;
$FRESH['yesterday'] = 859;
$FRESH['week']      = 5405;
$FRESH['month']     = 21601;
$FRESH['year']      = 43203;
$FRESH['all']       = 86407;
$CACHE['dates']     = "$PIE_DIR/dates.json";
$CACHE['quotes']    = "$PIE_DIR/quotes.json";
$CACHE['today']     = "$PIE_DIR/today.json";
$CACHE['yesterday'] = "$PIE_DIR/yesterday.json";
$CACHE['week']      = "$PIE_DIR/week.json";
$CACHE['month']     = "$PIE_DIR/month.json";
$CACHE['year']      = "$PIE_DIR/year.json";
$CACHE['all']       = "$PIE_DIR/all.json";

function numbered_source($filename) {
    ini_set('highlight.string',  '#DD0000'); // DD0000
    ini_set('highlight.comment', '#0000BB'); // FF8000
    ini_set('highlight.keyword', '#00CC00'); // 007700
    ini_set('highlight.bg',      '#111111'); // FFFFFF
    ini_set('highlight.default', '#00DDDD'); // 0000BB
    ini_set('highlight.html',    '#CCCCCC'); // 000000
    $lines = implode(range(1, count(file($filename))), '<br />');
    $content = highlight_file($filename, true);
    $style = '
    <style type="text/css"> 
        .num { 
        float: left; 
        color: gray; 
        background-color: #111111;
        font-size: 13px;    
        font-family: monospace; 
        text-align: right; 
        margin-right: 6pt; 
        padding-right: 6pt; 
        border-right: 1px solid gray;} 

        code {white-space: nowrap;} 
    </style>
    '; 
    return "$style\n<div style=\"background-color: black;\"><table><tr><td class=\"num\">\n$lines\n</td><td>\n$content\n</td></tr></table></div>"; 
}

function random_image($dir) {
    global $fp;
    global $DEBUG;

    if( $DEBUG ) $local_start = microtime(true);
    $old_dir = getcwd();
    chdir($dir);

    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $file_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($file_list);

    chdir($old_dir);
    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "random_image(): %9.4f\n", $local_spent);
    }
    return $file_list[$pick];
}

function db_connect($PG_DB, $PG_USERNAME, $PG_PASSWORD) {
    global $fp;
    global $DEBUG;

    if( $DEBUG ) $local_start = microtime(true);
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
    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "db_connect(): %9.4f\n", $local_spent);
    }
    return $db;
}

function check_cache($thing, $cache, $freshness) {
    global $fp;
    global $DEBUG;

    if( $DEBUG ) $local_start = microtime(true);

    if( file_exists($cache) ) {
        $stats = stat($cache);
        $cache_age = time() - $stats['mtime'];
        if( $DEBUG ) {
            fprintf($fp, "Cache(%s) is %d seconds old, threshold is %d\n", $thing, $cache_age, $freshness);
        }
        if($cache_age < $freshness) {
            $json_data = file_get_contents($cache);
            $result = json_decode($json_data, 1);
            if( $DEBUG ) {
                $local_end = microtime(true); $local_spent = $local_end - $local_start;
                fprintf($fp, "    check_cache(%s): %9.4f [CACHED]\n", $thing, $local_spent);
            }
            return $result;
        }
    }
    if( $DEBUG ) {
        fprintf($fp, "    check_cache(%s) is not valid.\n", $thing);
    }
    return null;
}

function write_cache($thing, $result) {
    global $fp;
    global $CACHE;
    global $DEBUG;

    $cache      = $CACHE[$thing];
    $json_data  = json_encode($result);
    if( file_exists($cache) ) {
        chmod($cache, 0664);
        //chown($cache, 'wiley');
    }
    $jfp = fopen($cache, "w");
    fprintf($jfp, "%s\n", $json_data);
    fclose($jfp);
    chmod($cache, 0664);
    //chown($cache, 'wiley');
    if( $DEBUG ) {
        fprintf($fp, "    write_cache(%s)\n", $thing);
    }
}

function fetch_dates($db) {
    global $fp;
    global $DEBUG;

    if( $DEBUG ) $local_start = microtime(true);
    $sql = "
        SELECT date(now()) AS today,
               date(now() - '1 day'::interval) AS yesterday,
               date(now() - '1 week'::interval) AS week,
               date(now() - '1 month'::interval) AS month,
               date(now() - '1 year'::interval) AS year,
               date(min(local)) AS all
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

    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "    fetch_dates(): %9.4f\n", $local_spent);
    }
    return $result;
}

function thing_and_clause($thing) {
    $and = " ";

    switch ($thing) {
        default:
        case 'today':
            $and .= " AND local BETWEEN date_trunc('day',now()) AND now()";
            break;
        case 'yesterday':
            $and .= " AND local BETWEEN date_trunc('day',now() - '1 day'::interval)";
            $and .= " AND date_trunc('day', now()) - '1 microsecond'::interval";
            break;
        case 'week':
            $and .= " AND local BETWEEN date_trunc('day',now() - '1 week'::interval)";
            $and .= " AND now()";
            break;
        case 'month':
            $and .= " AND local BETWEEN date_trunc('day',now() - '1 month'::interval)";
            $and .= " AND now()";
            break;
        case 'year':
            $and .= " AND local BETWEEN date_trunc('day',now() - '1 year'::interval)";
            $and .= " AND now()";
            break;
        case 'all':
            //$first = $date_info['now'];
            //$sql .= " AND local BETWEEN date_trunc('day',$first)";
            //$sql .= " AND now()";
            break;
    }

    return $and;
}

function fetch_quote($db, $thing) {
    global $fp;
    global $DEBUG;

    if( $DEBUG ) $local_start = microtime(true);
    $and  = thing_and_clause($thing);
    // URLbot is intentionally not marked as a bot, as when viewing the logs
    // we typically want to filter out normal bot messages, but still see the
    // url expansions from what people post.
    $sql  = "SELECT speaker, message FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot";
    $sql .= $and;
    $sql .= " OFFSET (random() * (SELECT COUNT(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot";
    $sql .= " $and ))::INTEGER LIMIT 1";

    try {
        $sth = $db->prepare($sql);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = $sth->fetch();
        if(isset($result['message']))
            $result['epoch'] = time();
        if( $DEBUG ) {
            fprintf($fp, "quote for (%s): \"%s - %s\"\n", $thing, $result['speaker'], $result['message']);
        }
    } catch (Exception $e) {
        throw $e;
    }

    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "    fetch_quote(%s): %9.4f\n", $thing, $local_spent);
    }
    return $result;
}

function fetch_stuff($db, $thing, $kind) {
    global $fp;
    global $DEBUG;

    if( $DEBUG ) $local_start = microtime(true);
    if($kind !== 'channel') {
        $kind = 'speaker';
    }

    // URLbot is intentionally not marked as a bot, as when viewing the logs
    // we typically want to filter out normal bot messages, but still see the
    // url expansions from what people post.
    //$sql  = "SELECT $kind, count(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot";
    if($kind === 'speaker') {
        $sql  = "SELECT lower(username) AS speaker, count(*) FROM i3log WHERE lower(username) <> 'urlbot' AND NOT is_bot";
        $sql .= thing_and_clause($thing);
        $sql .= " GROUP BY lower(username) ORDER BY count DESC LIMIT 12;";
    } else {
        $sql  = "SELECT $kind, count(*) FROM i3log WHERE speaker <> 'URLbot' AND NOT is_bot";
        $sql .= thing_and_clause($thing);
        $sql .= " GROUP BY $kind ORDER BY count DESC LIMIT 12;";
    }

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

    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "    fetch_stuff(%s,%s): %9.4f\n", $thing, $kind, $local_spent);
    }
    return $result;
}

function load_data($db) {
    global $fp;
    global $DEBUG;
    global $FRESH;
    global $CACHE;

    if( $DEBUG ) $local_start = microtime(true);

    $things     = array('today', 'yesterday', 'week', 'month', 'year', 'all');
    $kinds      = array('speaker', 'channel');
    $others     = array();
    $others['speaker'] = 'channel';
    $others['channel'] = 'speaker';

    $data_cache = array();
    $thing      = 'dates';
    $cache      = $CACHE[$thing];
    $freshness  = $FRESH[$thing];
    $result     = check_cache($thing, $cache, $freshness);

    // cache_dates
    if( !isset($result) ) {
        $result = fetch_dates($db);
        write_cache('dates', $result);

        if( $DEBUG ) {
            $local_end = microtime(true); $local_spent = $local_end - $local_start;
            fprintf($fp, "    cache_dates(): %9.4f [NOT CACHED]\n", $local_spent);
        }
    }
    $data_cache[$thing] = $result;
    write_cache($thing, $data_cache[$thing]);

    // cache_quotes
    $thing      = 'quotes';
    $cache      = $CACHE[$thing];
    $freshness  = $FRESH['all'];
    $result     = check_cache($thing, $cache, $freshness);
    if( !isset($result) )
        $result = array();
    foreach ($things as $t) {
        if( isset($result[$t]) ) {
            $cache_age = time() - $result[$t]['epoch'];
            if( $cache_age >= $FRESH[$t] ) {
                $q = fetch_quote($db, $t);
                $result[$t] = $q;
                if( $DEBUG ) fprintf($fp, "    cache_quote(%s): [NOT CACHED]\n", $t);
            } else {
                if( $DEBUG ) fprintf($fp, "    cache_quote(%s): [CACHED]\n", $t);
            }
        } else {
            $q = fetch_quote($db, $t);
            $result[$t] = $q;
            if( $DEBUG ) fprintf($fp, "    cache_quote(%s): [NOT CACHED]\n", $t);
        }
    }
    $data_cache[$thing] = $result;
    write_cache($thing, $data_cache[$thing]);

    // cache_stuff
    foreach ($things as $thing) {
        foreach ($kinds as $kind) {
            $cache      = $CACHE[$thing];
            $freshness  = $FRESH[$thing];
            $result     = check_cache($thing, $cache, $freshness);

            if( isset($result) ) {
                $data_cache[$thing] = $result;
            } else {
                $data_cache[$thing] = array();
                $result = fetch_stuff($db, $thing, $kind);
                $data_cache[$thing][$kind] = $result;
                $result2 = fetch_stuff($db, $thing, $others[$kind]);
                $data_cache[$thing][$others[$kind]] = $result2;
                write_cache($thing, $data_cache[$thing]);
            }
        }
    }

    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "Loading Data: %9.4f\n", $local_spent);
    }
    return $data_cache;
}

if( $DEBUG ) $local_start = microtime(true);
$mudlist_text = file_get_contents($MUDLIST_FILE);
$mudlist = json_decode($mudlist_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
if( json_last_error() != JSON_ERROR_NONE ) {
    echo "<hr>".json_last_error_msg()."<br><hr>";
}
$WILEY_BUILD_NUMBER     = $mudlist["version"]["build"];
$WILEY_BUILD_DATE       = $mudlist["version"]["date"];
$WILEY_TIME             = $mudlist["time"];
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Fetching JSON data: %9.4f\n", $local_spent);
}

$OVERLAY_ICON           = "$URL_HOME/gfx/NA.png";
// contain leaves space, cover clips edges.
$OVERLAY_IMG            = "<img width=\"700\" height=\"500\" style=\"position: fixed; z-index: 999; left: 50%; transform: translateX(-50%); object-fit: contain; opacity: 0.5;\" src=\"$OVERLAY_ICON\" />";
$EMPTY_ICON             = "$URL_HOME/gfx/empty.png";
//$EMPTY_IMG              = "<img width=\"700\" height=\"500\" style=\"left: 50%; transform: translateX(-50%); object-fit: contain;\" src=\"$EMPTY_ICON\" />";
$EMPTY_IMG              = "<img width=\"700\" height=\"460\" src=\"$EMPTY_ICON\" />";

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

$active_style   = "background-color: #8f4f2f; opacity: 1.0;";
$hover_style    = "background-color: #4f0000; opacity: 1.0;";
$inactive_style = "background-color: #2f0000; opacity: 0.6;";

$activechan_style   = "background-color: #2f4f8f; opacity: 1.0; font-size: 60%";
$hoverchan_style    = "background-color: #00004f; opacity: 1.0; font-size: 60%";
$inactivechan_style = "background-color: #00002f; opacity: 0.6; font-size: 60%";

$db = db_connect($PG_DB, $PG_USERNAME, $PG_PASSWORD);
//$date_info = cache_dates($db);
$all_data = load_data($db);
$date_info = $all_data['dates'];
$things = array('today', 'yesterday', 'week', 'month', 'year', 'all');

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
            .unblurred { font-family: monospace; white-space: pre-wrap; }
            .blurry:not(:hover) { filter: blur(3px); font-family: monospace; white-space: pre-wrap; }
            .blurry:hover { font-family: monospace; white-space: pre-wrap; }
            .glowing:not(:hover) { filter: brightness(1); }
            .glowing:hover { filter: brightness(1.75); }
        </style>
        <title>Welcome to WileyMUD!</title>
        <script type="text/javascript">
            var current_button;
            var current_graph;
            var current_quote;
            var en = [];
            var jp = [];

            en['Today']     = 'Today-chan';
            jp['Today']     = '今日ちゃん';
            en['Yesterday'] = 'Yesterday-chan';
            jp['Yesterday'] = '昨日ちゃん';
            en['Week']      = 'Week-chan';
            jp['Week']      = '週間ちゃん';
            en['Month']     = 'Month-chan';
            jp['Month']     = '月ちゃん';
            en['Year']      = 'Year-chan';
            jp['Year']      = '年ちゃん';
            en['All']       = 'All Time-chan';
            jp['All']       = '常にちゃん';

            function get_today() {
                current_button = document.getElementById("btnToday");
                current_graph = document.getElementById("graphToday");
                current_quote = document.getElementById("graphQuoteToday");
                current_graph.style.display = "block";
                current_quote.style.display = "block";
            }
            function hover_on(x) {
                if(current_button != x) {
                    var s = x.id.substr(0,3);
                    var t = x.id.substr(3);
                    if(s == "btn") {
                        x.style = "<?php echo $hover_style; ?>";
                    } else {
                        x.style = "<?php echo $hoverchan_style; ?>";
                        x.innerHTML = en[t];
                    }
                }
            }
            function hover_off(x) {
                if(current_button != x) {
                    var s = x.id.substr(0,3);
                    var t = x.id.substr(3);
                    if(s == "btn") {
                        x.style = "<?php echo $inactive_style; ?>";
                    } else {
                        x.style = "<?php echo $inactivechan_style; ?>";
                        x.innerHTML = jp[t];
                    }
                }
            }
            function click_select(x) {
                if(current_button != x) {
                    var s = x.id.substr(0,3);
                    var t = x.id.substr(3);
                    var g;
                    var q;
                    if(s == "btn") {
                        x.style = "<?php echo $active_style; ?>";
                        g = document.getElementById("graph" + x.id.substr(3));
                    } else {
                        x.style = "<?php echo $activechan_style; ?>";
                        g = document.getElementById("graphChannels" + x.id.substr(3));
                        x.innerHTML = jp[t];
                    }
                    s = current_button.id.substr(0,3);
                    t = current_button.id.substr(3);
                    if(s == "btn") {
                        current_button.style = "<?php echo $inactive_style; ?>";
                    } else {
                        current_button.style = "<?php echo $inactivechan_style; ?>";
                        current_button.innerHTML = jp[t];
                    }
                    current_button = x;
                    current_graph.style.display = "none";
                    current_graph = g;
                    g.style.display = "block";

                    q = document.getElementById("graphQuote" + x.id.substr(3));
                    current_quote.style.display = "none";
                    current_quote = q;
                    q.style.display = "block";
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

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawTodaySpeakers() {
                <?php
                //$result = cache_stuff($db, 'today', 'speaker');
                $result = $all_data['today']['speaker'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "Today's <?php echo $total; ?> Jibber-Jabbers (<?php echo $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['speaker']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphToday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_today'));
                    chart.draw(data, options);
                    document.getElementById('graphToday').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_today').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteToday').innerHTML="<font size=\"-1\">I got nothin'.</font>";
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawTodaySpeakers(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawTodayChannels() {
                <?php
                //$result = cache_stuff($db, 'today', 'channel');
                $result = $all_data['today']['channel'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "Today's <?php echo $total; ?> Jibber-Jabbers (<?php echo $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['channel']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsToday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_today'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsToday').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_today').innerHTML='<?php echo $OVERLAY_IMG;?>';
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawTodayChannels(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawYesterdaySpeakers() {
                <?php
                //$result = cache_stuff($db, 'yesterday', 'speaker');
                $result = $all_data['yesterday']['speaker'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "Yesterday's <?php echo $total; ?> Bits of Rubbish (<?php echo $date_info['yesterday'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['speaker']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphYesterday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_yesterday'));
                    chart.draw(data, options);
                    document.getElementById('graphYesterday').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_yesterday').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteYesterday').innerHTML="<font size=\"-1\">Nothin' to see here.</font>";
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawYesterdaySpeakers(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawYesterdayChannels() {
                <?php
                //$result = cache_stuff($db, 'yesterday', 'channel');
                $result = $all_data['yesterday']['channel'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "Yesterday's <?php echo $total; ?> Bits of Rubbish (<?php echo $date_info['yesterday'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['channel']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsYesterday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_yesterday'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsYesterday').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_yesterday').innerHTML='<?php echo $OVERLAY_IMG;?>';
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawYesterdayChannels(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawWeekSpeakers() {
                <?php
                //$result = cache_stuff($db, 'week', 'speaker');
                $result = $all_data['week']['speaker'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "The Week of <?php echo $total; ?> Idiotic Ramblings (<?php echo $date_info['week'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['speaker']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphWeek').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_week'));
                    chart.draw(data, options);
                    document.getElementById('graphWeek').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_week').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteWeek').innerHTML="<font size=\"-1\">Move along.</font>";
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawWeekSpeakers(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawWeekChannels() {
                <?php
                //$result = cache_stuff($db, 'week', 'channel');
                $result = $all_data['week']['channel'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "The Week of <?php echo $total; ?> Idiotic Ramblings (<?php echo $date_info['week'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['channel']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsWeek').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_week'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsWeek').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_week').innerHTML='<?php echo $OVERLAY_IMG;?>';
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawWeekChannels(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawMonthSpeakers() {
                <?php
                //$result = cache_stuff($db, 'month', 'speaker');
                $result = $all_data['month']['speaker'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "A Month with <?php echo $total; ?> Nonsensical Opinions? (<?php echo $date_info['month'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['speaker']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphMonth').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_month'));
                    chart.draw(data, options);
                    document.getElementById('graphMonth').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_month').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteMonth').innerHTML="<font size=\"-1\">This is not the quote you're looking for.</font>";
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawMonthSpeakers(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawMonthChannels() {
                <?php
                //$result = cache_stuff($db, 'month', 'channel');
                $result = $all_data['month']['channel'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "A Month with <?php echo $total; ?> Nonsensical Opinions? (<?php echo $date_info['month'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['channel']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsMonth').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_month'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsMonth').style.display='none';
                <?php } else { ?>
                        document.getElementById('piechan_month').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    <?php } ?>
                }
    <?php
    if( $DEBUG ) {
        $local_end = microtime(true); $local_spent = $local_end - $local_start;
        fprintf($fp, "Configuring drawMonthChannels(): %9.4f\n", $local_spent);
    }
    ?>

    <?php if( $DEBUG ) $local_start = microtime(true); ?>
                function drawYearSpeakers() {
                    <?php
                    //$result = cache_stuff($db, 'year', 'speaker');
                    $result = $all_data['year']['speaker'];
                    if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "What a Horrible Year it's Been... <?php echo $total; ?> open sores. (<?php echo $date_info['year'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['speaker']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphYear').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_year'));
                    chart.draw(data, options);
                    document.getElementById('graphYear').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_year').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteYear').innerHTML="<font size=\"-1\">Just give up.</font>";
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawYearSpeakers(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawYearChannels() {
                <?php
                //$result = cache_stuff($db, 'year', 'channel');
                $result = $all_data['year']['channel'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "What a Horrible Year it's Been... <?php echo $total; ?> open sores. (<?php echo $date_info['year'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['channel']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsYear').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_year'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsYear').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_year').innerHTML='<?php echo $OVERLAY_IMG;?>';
                <?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawYearChannels(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawAllSpeakers() {
<?php if( $DO_ALL_TIME ) { ?>
                <?php
                //$result = cache_stuff($db, 'all', 'speaker');
                $result = $all_data['all']['speaker'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "What Have You DONE??? <?php echo $total; ?> Wastes of Time. (<?php echo $date_info['all'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Speaker', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['speaker']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphAll').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie_all'));
                    chart.draw(data, options);
                    document.getElementById('graphAll').style.display='none';
                <?php } else { ?>
                    document.getElementById('pie_all').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteAll').innerHTML="<font size=\"-1\">There really is nothing to see.</font>";
                <?php } ?>
<?php } else { ?>
                    document.getElementById('pie_all').innerHTML='<?php echo $OVERLAY_IMG;?>';
                    document.getElementById('graphQuoteAll').innerHTML="<font size=\"-1\">Nothing really matters anyways.</font>";
<?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawAllSpeakers(): %9.4f\n", $local_spent);
}
?>

<?php if( $DEBUG ) $local_start = microtime(true); ?>
            function drawAllChannels() {
<?php if( $DO_ALL_TIME ) { ?>
                <?php
                //$result = cache_stuff($db, 'all', 'channel');
                $result = $all_data['all']['channel'];
                if(count($result) > 0) {
                    $total = 0;
                    foreach ($result as $r) {
                        $total += $r['count'];
                    }
                ?>
                    options['title'] = "What Have You DONE??? <?php echo $total; ?> Wastes of Time. (<?php echo $date_info['all'] . ' to ' . $date_info['today'];?>)";
                    var data = google.visualization.arrayToDataTable([
                    <?php
                    printf("[ '%s', '%s' ],\n", 'Channel', 'Count');
                    foreach ($result as $r) {
                        printf("[ \"%s\", %d ],\n", addslashes($r['channel']), $r['count']);
                    }
                    ?>
                    ]);
                    document.getElementById('graphChannelsAll').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('piechan_all'));
                    chart.draw(data, options);
                    document.getElementById('graphChannelsAll').style.display='none';
                <?php } else { ?>
                    document.getElementById('piechan_all').innerHTML='<?php echo $OVERLAY_IMG;?>';
                <?php } ?>
<?php } else { ?>
                    document.getElementById('piechan_all').innerHTML='<?php echo $OVERLAY_IMG;?>';
<?php } ?>
            }
<?php
if( $DEBUG ) {
    $local_end = microtime(true); $local_spent = $local_end - $local_start;
    fprintf($fp, "Configuring drawAllChannels(): %9.4f\n", $local_spent);
}
?>

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
        <script type="text/javascript">
            function toggleDiv(divID) {
                if(document.getElementById(divID).style.display == 'none') {
                    document.getElementById(divID).style.display = 'block';
                } else {
                    document.getElementById(divID).style.display = 'none';
                }
            }
        </script>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040" onload="get_today()">
    <table id="navbar" width="99%" cellspacing="0" cellpadding="1" align="center">
        <tr>
            <td align="left" width="25%">
                <a href="http://wileymud.themud.org/~wiley/mudlist.php" alt="Mudlist" title="Mudlist">
                    <img class="glowing" src="http://wileymud.themud.org/~wiley/gfx/mud.png" width="48" height="48" border="0" />
                </a>
                <a href="http://wileymud.themud.org/~wiley/logpages" alt="Logs" title="Logs">
                    <img class="glowing" src="http://wileymud.themud.org/~wiley/gfx/log.png" width="48" height="48" border="0" />
                </a>
                    <img src="http://wileymud.themud.org/~wiley/gfx/pie_chart.png" width="48" height="48" border="0" style="opacity: 0.2; background: rgba(255,0,0,0.25);" />
            </td>
            <td align="right" width="24%">
                <a href="telnet://wileymud.themud.org:3000" alt="WileyMUD" title="WileyMUD">
                    <img class="glowing" src="http://wileymud.themud.org/~wiley/gfx/wileymud3.png" width="247" height="48" border="0" />
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
                    <img class="glowing" src="http://wileymud.themud.org/~wiley/gfx/server_icon.png" width="48" height="48" border="0" />
                </a>
                <a href="https://github.com/quixadhal/wileymud" alt="Source" title="Source">
                    <img class="glowing" src="http://wileymud.themud.org/~wiley/gfx/github1600.png" width="48" height="48" border="0" />
                </a>
                <a href="https://discord.gg/kUduSsJ" alt="Discord" title="Discord">
                    <img class="glowing" src="http://wileymud.themud.org/~wiley/gfx/discord.png" width="48" height="48" border="0" />
                </a>
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
<?php if ($DO_ALL_TIME) { ?>
            <td id="btnAll" style="<?php echo $inactive_style; ?>;" align="center" width="23%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                Of All Time
            </td>
            <td id="chnAll" style="<?php echo $inactivechan_style; ?>;" align="center" width="10%" onmouseover="hover_on(this)" onmouseout="hover_off(this)" onclick="click_select(this)">
                常にちゃん
            </td>
<?php } else { ?>
            <td id="btnAll" style="<?php echo $inactive_style; ?>; text-decoration: line-through; color: #303030;" align="center" width="23%">
                Of All Time
            </td>
            <td id="chnAll" style="<?php echo $inactivechan_style; ?>; text-decoration: line-through; color: #303030;" align="center" width="10%">
                常にちゃん
            </td>
<?php } ?>
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
                <div id="graphQuotePane" style="display: block; height: 510px; text-align: center; transform: translateX(37%);">
                    <?php echo $EMPTY_IMG; ?>
                    <div id="graphQuoteToday" style="background-color: #202020; color: #FF9F9F; display: none;">
                        <font size="-1">
                            <?php
                                $someone    = $all_data['quotes']['today']['speaker'] ? $all_data['quotes']['today']['speaker'] : "Nobody";
                                $something  = $all_data['quotes']['today']['message'] ? $all_data['quotes']['today']['message'] : "a damn thing.";
                                echo "$someone said &#x300C;$something&#x300D;";
                            ?>
                        </font>
                    </div>
                    <div id="graphQuoteYesterday" style="background-color: #202020; color: #FFFF9F; display: none;">
                        <font size="-1">
                            <?php echo $all_data['quotes']['yesterday']['speaker']; ?> said
                            &#x300C;<?php echo $all_data['quotes']['yesterday']['message']; ?>&#x300D;
                        </font>
                    </div>
                    <div id="graphQuoteWeek" style="background-color: #202020; color: #9FFF9F; display: none;">
                        <font size="-1">
                            <?php echo $all_data['quotes']['week']['speaker']; ?> said
                            &#x300C;<?php echo $all_data['quotes']['week']['message']; ?>&#x300D;
                        </font>
                    </div>
                    <div id="graphQuoteMonth" style="background-color: #202020; color: #9F9FFF; display: none;">
                        <font size="-1">
                            <?php echo $all_data['quotes']['month']['speaker']; ?> said
                            &#x300C;<?php echo $all_data['quotes']['month']['message']; ?>&#x300D;
                        </font>
                    </div>
                    <div id="graphQuoteYear" style="background-color: #202020; color: #9FFFFF; display: none;">
                        <font size="-1">
                            <?php echo $all_data['quotes']['year']['speaker']; ?> said
                            &#x300C;<?php echo $all_data['quotes']['year']['message']; ?>&#x300D;
                        </font>
                    </div>
                    <div id="graphQuoteAll" style="background-color: #202020; color: #FFFFFF; display: none;">
                        <font size="-1">
                            <?php echo $all_data['quotes']['all']['speaker']; ?> said
                            &#x300C;<?php echo $all_data['quotes']['all']['message']; ?>&#x300D;
                        </font>
                    </div>
                </div>
            </td>
        </tr>
        <tr bgcolor="#000000">
            <?php $time_end = microtime(true); $time_spent = $time_end - $time_start; ?>
            <td align="right" colspan="6"><font size="-1" color="#1f1f1f"><a href="javascript:;" onmousedown="toggleDiv('source');">Page:</a>&nbsp;<?php echo sprintf("%9.4f", $time_spent); ?> seconds</font></td>
        </tr>
    </table>
</div>
<div id="source" style="display: none;">
    <?php echo numbered_source(__FILE__); ?>
</div>
</body>
</html>
<?php
if( $DEBUG ) {
    fprintf($fp, "Total Script Time: %9.4f seconds.\n", $time_spent);
    fclose($fp);
    chmod($DEBUG_LOG, 0664);
}
?>
