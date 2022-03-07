<?php
$PG_DB      = "i3log";
$PG_USER    = "wiley";
$PG_PASS    = "tardis69";
$PG_CHARSET = "en_US.UTF-8";

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
    global $PG_CHARSET;

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
    /*
    try {
        $sth = $db->prepare("SET CLIENT_ENCODING TO 'UTF8';");
        $sth->execute();
    } catch(PDOException $e) {
        echo $e->getMessage();
    }
     */
    return $db;
}

// select date_part('epoch', now())::integer;
// select to_timestamp(the_time);

function fetch_page_data_by_time($db, $query_start_time = NULL, $query_end_time = NULL, $query_date = NULL, $limit_count = NULL, $offset_point = NULL, $urlbot_mode = FALSE, $search_term = NULL) {
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
    $sql3 = "
             SELECT date(local) AS the_date,
                    to_char(local, 'HH24:MI:SS') AS the_time,
                    channel,
                    speaker || '@' || mud AS speaker,
                    message,
                    hour_html, channel_html, speaker_html,
                    date_part('epoch', local)::integer AS unix_time
               FROM page_view
              WHERE date(local) = date(:query_date)
           ORDER BY local ASC
              LIMIT :limit_count
    ";

    $sql1s = "
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
                AND position(:search_term IN lower(message)) > 0
           ORDER BY local ASC
              LIMIT :limit_count
    ";
    $sql2s = "
             SELECT * FROM (
                 SELECT date(local) AS the_date,
                        to_char(local, 'HH24:MI:SS') AS the_time,
                        channel,
                        speaker || '@' || mud AS speaker,
                        message,
                        hour_html, channel_html, speaker_html,
                        date_part('epoch', local)::integer AS unix_time
                   FROM page_view
                  WHERE position(:search_term IN lower(message)) > 0
               ORDER BY local DESC
                  LIMIT :limit_count
            ) s ORDER BY unix_time ASC
    ";
    $sql3s = "
             SELECT date(local) AS the_date,
                    to_char(local, 'HH24:MI:SS') AS the_time,
                    channel,
                    speaker || '@' || mud AS speaker,
                    message,
                    hour_html, channel_html, speaker_html,
                    date_part('epoch', local)::integer AS unix_time
               FROM page_view
              WHERE date(local) = date(:query_date)
                AND position(:search_term IN lower(message)) > 0
           ORDER BY local ASC
              LIMIT :limit_count
    ";

    $use_times = 1;
    $use_date = 0;
    $use_search = 0;
    $sql = $sql1;
    $sqls = $sql1s;
    if( $query_start_time === NULL && $query_end_time === NULL && $query_date === NULL) {
        $use_times = 0;
        $use_date = 0;
        $sql = $sql2;
        $sqls = $sql2s;
    } elseif ($query_date !== NULL) {
        $use_times = 0;
        $use_date = 1;
        $sql = $sql3;
        $sqls = $sql3s;
    } else {
        if( $query_start_time === NULL || !is_numeric($query_start_time) || $query_start_time < 1 ) {
            $query_start_time = time() - (60 * 5);
        }
        if( $query_end_time === NULL || !is_numeric($query_end_time) || $query_end_time < 1 || $query_end_time < $query_start_time ) {
            $query_end_time = time();
        }
    }
    if( $limit_count === NULL || !is_numeric($limit_count) || $limit_count > 2000 ) {
        $limit_count = 2000;
    } else if( $limit_count < 1 ) {
        $limit_count = 1;
    }
    if( $search_term !== NULL ) {
        $use_search = 1;
    }
    try {
        //$db->beginTransaction();
        if($use_search == 1) {
            $sth = $db->prepare($sqls);
        } else {
            $sth = $db->prepare($sql);
        }
        if($use_times == 1) {
            $sth->bindParam(':query_start_time', $query_start_time);
            $sth->bindParam(':query_end_time', $query_end_time);
        } elseif($use_date == 1) {
            $sth->bindParam(':query_date', $query_date);
        }
        if($use_search == 1) {
            $sth->bindParam(':search_term', $search_term);
        }
        $sth->bindParam(':limit_count', $limit_count);
        $sth->execute();
        $sth->setFetchMode(PDO::FETCH_ASSOC);
        $result = array();
        $new_result = array();
        while($row = $sth->fetch()) {
            if($row['channel'] == 'url' && $row['speaker'] == 'URLbot@Disk World') {
                // Skip OLD urlbot stuff...
                continue;
            }
            $message_orig = $row['message'];
            //$message = htmlentities($message, ENT_QUOTES|ENT_SUBSTITUTE, 'UTF-8');
            if( $row['speaker'] == 'Cron@WileyMUD' || $row['speaker'] == 'Cron') {
                $message = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~\:\@])*)/i', '<a href="$1" target="_self">$1</a>', $message_orig);
            } else {
                $message = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~\:\@])*)/i', '<a href="$1" target="I3-link">$1</a>', $message_orig);
            }
            if($message != $message_orig) {
                $row['message'] = $message;
                $row['message_orig'] = $message_orig;
            }
            $result[] = $row;
        }

        $sql =  "
                SELECT message
                  FROM urls
                 WHERE message IS NOT NULL AND url = :the_url
              ORDER BY created ASC
                 LIMIT 1
                ";

        foreach($result as $row) {
            //$new_result[] = $row;
            if(!$urlbot_mode) {
                $new_result[] = $row;
            }
            if(array_key_exists('message_orig', $row)) {
                // We found a URL before
                $urls = [];
                if(preg_match('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)/i', $row['message_orig'], $urls)) {
                    // Set up a new entry...
                    $newrow = [];
                    $newrow['the_date'] = $row['the_date'];
                    $newrow['the_time'] = $row['the_time'];
                    $newrow['channel'] = 'url';
                    $newrow['speaker'] = 'URLbot';
                    $newrow['hour_html'] = $row['hour_html'];
                    $newrow['channel_html'] = "<SPAN style=\"background-color: #0000bb; color: #ffff55\">";
                    $newrow['speaker_html'] = "<SPAN style=\"background-color: #0000bb; color: #ffff55\">";
                    $newrow['unix_time'] = $row['unix_time'];

                    // extract the url...
                    $url = $urls[0]; // The whole match

                    // Now, see if we have this in the database.
                    $sth = $db->prepare($sql);
                    $sth->bindParam(':the_url', $url);
                    $sth->execute();
                    $sth->setFetchMode(PDO::FETCH_ASSOC);

                    $urlbot = '';
                    if($known_url = $sth->fetch()) {
                        // If we got a result back, use it!
                        $urlbot = $known_url['message'];
                        $urlbot = htmlentities($urlbot, ENT_QUOTES|ENT_SUBSTITUTE, 'UTF-8');
                        //$urlbot = preg_replace('/<(.*\s+on\s+.*)>/i', '&lt;$1&gt;', $urlbot);
                        $urlbot = preg_replace('/((?:http|https|ftp)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~\:])*)/i', '<a href="$1" target="I3-link">$1</a>', $urlbot);
                        $urlbot = preg_replace('/YouTube\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'YouTube $1 <a href="https://youtu.be/$2" target="I3-link">[$2]</a>', $urlbot);
                        $urlbot = preg_replace('/IMDB\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'IMDB $1 <a href="https://www.imdb.com/title/$2/" target="I3-link">[$2]</a>', $urlbot);
                        $urlbot = preg_replace('/Steam\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'Steam $1 <a href="http://store.steampowered.com/app/$2/" target="I3-link">[$2]</a>', $urlbot);
                        $urlbot = preg_replace('/Dailymotion\s+(<span.*?>)\s*\[([^\]]*)\]/i', 'Dailymotion $1 <a href="https://www.dailymotion.com/video/$2" target="I3-link">[$2]</a>', $urlbot);
                    } else {
                        // Oh well, maybe it'll populate later...
                        //$urlbot = pcmd("/home/wiley/bin/untiny wiley '$url' '$channel' '$speaker'");
                        $urlbot = '<SPAN class="flash_tag" style="color: red;">COMING SOON!</SPAN>';
                        //$urlbot = pcmd("/home/wiley/bin/untiny cache '$url' '$channel' '$speaker'");
                    }
                    $newrow['message'] = $urlbot;
                    // Now we have to splice the new row between the one we just looked at and the next one
                    $new_result[] = $newrow;
                }
            }
        }
        //$db->commit();
    } catch (Exception $e) {
        //$db->rollback();
        throw $e;
    }
    $actual_count = count($new_result);
    if($actual_count > $limit_count) {
        // The URLbot entries bloated us up, we need to trim the newest
        // ones off and get them again next time...
        array_splice($new_result, ($limit_count - $actual_count));
    }
    return $new_result;
}

$time_from = NULL;
$time_to = NULL;
$the_date = NULL;
$row_limit = NULL;
$urlbot_mode = FALSE;
$row_offset = NULL;
$search_term = NULL;

if(array_key_exists('from', $_GET)) {
    $time_from = $_GET['from'];
}
if(array_key_exists('to', $_GET)) {
    $time_to = $_GET['to'];
}
if(array_key_exists('date', $_GET)) {
    $the_date = $_GET['date'];
}
if(array_key_exists('limit', $_GET)) {
    $row_limit = $_GET['limit'];
}
if(array_key_exists('urlbot', $_GET)) {
    $urlbot_mode = TRUE;
}
if(array_key_exists('offset', $_GET)) {
    $offset = $_GET['offset'];
}
if(array_key_exists('search', $_GET)) {
    $search_term = $_GET['search'];
}

$db = db_connect();
$data = fetch_page_data_by_time($db, $time_from, $time_to, $the_date, $row_limit, $row_offset, $urlbot_mode, $search_term);
$time_end = microtime(true);
$time_spent = $time_end - $time_start;
$json = json_encode($data, JSON_UNESCAPED_UNICODE | JSON_INVALID_UTF8_SUBSTITUTE | JSON_PRETTY_PRINT);

header("Content-Type: application/json; charset=UTF-8");
header("Content-Language: en, jp");
echo $json;

?>
