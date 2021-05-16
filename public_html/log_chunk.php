<?php
$PG_DB      = "i3log";
$PG_USER    = "wiley";
$PG_PASS    = "tardis69";

$time_start = microtime(true);

function pcmd($command) {
    $data = "";
    $fp = popen("$command", "r");
    do {
        $data .= fread($fp, 8192);
    } while(!feof($fp));
    pclose($fp);
    return $data;
}

function db_connect() {
    global $PG_DB;
    global $PG_USER;
    global $PG_PASS;

    $db = null;
    try {
        $db = new PDO( "pgsql:dbname=$PG_DB;user=$PG_USER;password=$PG_PASS", null, null, array(
            //PDO::ATTR_PERSISTENT        => true, 
            PDO::ATTR_ERRMODE           => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_EMULATE_PREPARES  => false,
        ));
    } catch(PDOException $e) {
        echo $e->getMessage();
    }
    return $db;
}

// select date_part('epoch', now())::integer;
// select to_timestamp(the_time);

function fetch_page_data_by_time($db, $query_start_time = NULL, $query_end_time = NULL, $limit_count = NULL) {
    // the_date YYYY-MM-DD
    // the_time HH:MM:SS
    // channel <word+>
    // speaker <word+@word+>
    // message <text>
    // hour_html <html_prefix>
    // channel_html <html_prefix>
    // speaker_html <html_prefix>
    // unix_time <integer>
    $sql1 = "
             SELECT date(local) AS the_date,
                    to_char(local, 'HH24:MI:SS') AS the_time,
                    channel,
                    speaker || '@' || mud AS speaker,
                    message,
                    hour_html, channel_html, speaker_html,
                    date_part('epoch', local)::integer AS unix_time
               FROM page_view
              WHERE local >= to_timestamp(:query_start_time)
                AND local <= to_timestamp(:query_end_time)
           ORDER BY local ASC
              LIMIT :limit_count
    ";
    $sql2 = "
             SELECT * FROM (
                 SELECT date(local) AS the_date,
                        to_char(local, 'HH24:MI:SS') AS the_time,
                        channel,
                        speaker || '@' || mud AS speaker,
                        message,
                        hour_html, channel_html, speaker_html,
                        date_part('epoch', local)::integer AS unix_time
                   FROM page_view
               ORDER BY local DESC
                  LIMIT :limit_count
            ) s ORDER BY unix_time ASC
    ";
    $use_times = 1;
    $sql = $sql1;
    if( $query_start_time === NULL && $query_end_time === NULL ) {
        $use_times = 0;
        $sql = $sql2;
    } else {
        if( $query_start_time === NULL || !is_numeric($query_start_time) || $query_start_time < 1 ) {
            $query_start_time = time() - (60 * 5);
        }
        if( $query_end_time === NULL || !is_numeric($query_end_time) || $query_end_time < 1 || $query_end_time < $query_start_time ) {
            $query_end_time = time();
        }
    }
    if( $limit_count === NULL || !is_numeric($limit_count) || $limit_count > 100 ) {
        $limit_count = 100;
    } else if( $limit_count < 1 ) {
        $limit_count = 1;
    }
    try {
        //$db->beginTransaction();
        $sth = $db->prepare($sql);
        if($use_times == 1) {
            $sth->bindParam(':query_start_time', $query_start_time);
            $sth->bindParam(':query_end_time', $query_end_time);
        }
        $sth->bindParam(':limit_count', $limit_count);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        $seen_urls = [];
        while($row = $sth->fetch()) {
            $message = $row['message'];
            //$message = htmlentities($message, ENT_QUOTES|ENT_SUBSTITUTE, 'UTF-8');
            $old_message = $message;
            $message = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/i', '<a href="$1" target="I3-link">$1</a>', $message);
            $row['message'] = $message;
            $result[] = $row;

            $urls = [];
            if(preg_match('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/i', $old_message, $urls)) {
                // URLbot, rise from the dead!
                // Usage: /home/wiley/bin/untiny [wiley|ansi|html|debug] URL [channel] [speaker]
                //echo "URL MATCH: ";
                //print_r($urls);
                $url = $urls[0]; // The whole match
                $channel = $row['channel'];
                $speaker = $row['speaker'];
                // Check for already existing urls here, to avoid repeats...
                //$urlbot = pcmd("/home/wiley/bin/untiny wiley '$url' '$channel' '$speaker'");
                $urlbot = '<SPAN class="flash_tag" style="color: red;">COMING SOON!</SPAN>';

                $urlbot = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/i', '<a href="$1" target="I3-link">$1</a>', $urlbot);
                $urlbot = preg_replace('/YouTube\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'YouTube $1 <a href="https://youtu.be/$2" target="I3-link">[$2]</a>', $urlbot);
                $urlbot = preg_replace('/IMDB\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'IMDB $1 <a href="https://www.imdb.com/title/$2/" target="I3-link">[$2]</a>', $urlbot);
                $urlbot = preg_replace('/Steam\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'Steam $1 <a href="http://store.steampowered.com/app/$2/" target="I3-link">[$2]</a>', $urlbot);
                $urlbot = preg_replace('/Dailymotion\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'Dailymotion $1 <a href="https://www.dailymotion.com/video/$2" target="I3-link">[$2]</a>', $urlbot);

                $newrow = [];
                $newrow['the_date'] = $row['the_date'];
                $newrow['the_time'] = $row['the_time'];
                $newrow['channel'] = 'url';
                $newrow['speaker'] = 'URLbot';
                $newrow['message'] = $urlbot;
                $newrow['hour_html'] = $row['hour_html'];
                $newrow['channel_html'] = "<SPAN style=\"background-color: #0000bb; color: #ffff55\">";
                $newrow['speaker_html'] = "<SPAN style=\"background-color: #0000bb; color: #ffff55\">";
                $newrow['unix_time'] = $row['unix_time'];

                $result[] = $newrow;
            }
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    return $result;
}

$time_from = NULL;
$time_to = NULL;
$row_limit = NULL;

if(array_key_exists('from', $_GET)) {
    $time_from = $_GET['from'];
}
if(array_key_exists('to', $_GET)) {
    $time_to = $_GET['to'];
}
if(array_key_exists('limit', $_GET)) {
    $row_limit = $_GET['limit'];
}

$db = db_connect();
$data = fetch_page_data_by_time($db, $time_from, $time_to, $row_limit);
$time_end = microtime(true);
$time_spent = $time_end - $time_start;
$json = json_encode($data, JSON_PRETTY_PRINT);

header("Content-Type: application/json; charset=UTF-8");
echo $json;

?>
