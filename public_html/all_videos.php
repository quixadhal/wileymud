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

$URL_HOME       = "http://wileymud.themud.org/~wiley";
$BACKGROUND_DIR = "/home/wiley/public_html/gfx/wallpaper/";
$BACKGROUND     = random_image($BACKGROUND_DIR);
$BACKGROUND_URL = "$URL_HOME/gfx/wallpaper/$BACKGROUND";
$BACKGROUND_IMG = "<img class=\"overlay-bg\" src=\"$BACKGROUND_URL\" />";
$JQ             = "$URL_HOME/jquery.js";
$JSCOOKIE       = "$URL_HOME/js.cookie.min.js";

$VISITED        = "#00FF00";

$playlist_list = file("/home/wiley/public_html/autoplaylist_titles.txt", FILE_SKIP_EMPTY_LINES);
$output_list = array();
$url_list = array();
$random_choice = $playlist_list[array_rand($playlist_list)];
$random_id = substr($random_choice, 0, 11);
$random_url = "https://www.youtube.com/watch?v=" . $random_id;
$random_embed = "https://www.youtube.com/embed/" . $random_id . "?showinfo=0&autoplay=1&autohide=0&controls=1&mute=1";
// YouTube is stupid and will only autoplay if muted until you click something...

?>
<html>
    <head>
        <style>
            a { text-decoration:none; color: #ffffbf; }
            a:visited { color: #ffa040; }
            a:hover { text-decoration:underline; }
            a:active, a:focus {
                outline: 0;
                border: none;
                -moz-outline-style: none;
            }
            html, body {
                font-family: 'Lato', sans-serif;
                padding: 0px;
                margin: 0px;
            }
            body::after {
                content: "";
                background: url(<?php echo $BACKGROUND_URL; ?>);
                opacity: 0.15;
                top: 0;
                left: 0;
                bottom: 0;
                right: 0;
                position: fixed;
                z-index: -2;
                background-size: cover;
            }
            #player-div {
                z-index: 1;
                opacity: 0.5;
                position: fixed;
                top: 0;
                right: 0;
                width: 640;
                height: 360;
            }
            #iframe-player {
                position: fixed;
                top: 0;
                right: 0;
                width: 640;
                height: 360;
            }
            #content {
            }
        </style>
        <title> Playlist </title>
        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JSCOOKIE;?>""></script>
        <script type="text/javascript">
            function color_links() {
                var jar = Cookies.get();
                for (const [key,value] of Object.entries(jar)) {
                    var element = document.getElementById(key);
                    if(element !== undefined && element !== null) {
                        element.style.color = "<?php echo $VISITED; ?>";
                    }
                }
            }
            function play_link(id) {
                // We want to mark the clicked link as visited
                // To do this, since we aren't visiting it in the browser
                // and aren't allowed to modify the browser's history, we'll
                // store it in a cookie and color the links as loaded if
                // the id matches...
                url = "https://www.youtube.com/watch?v=" + id;
                embed = "https://www.youtube.com/embed/" + id + "?showinfo=0&autoplay=1&autohide=0&controls=1";
                Cookies.set(id, url, { expires: 30 });
                // Refresh link color
                document.getElementById(id).style.color = "<?php echo $VISITED; ?>";
                // And then stuff it into the player
                $('#iframe-player').attr('src', embed);
                return false;
            }
            $(document).ready(function() {
                setTimeout(function() {
                    var id = "<?php echo $random_id;?>";
                    var url = "<?php echo $random_url;?>";
                    location.hash = "#<?php echo $random_id; ?>";
                    Cookies.set(id, url, { expires: 30 });
                    document.getElementById(id).style.color = "<?php echo $VISITED; ?>";
                }, 500);
                color_links();
            });
        </script>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040">
        <div id="player-div">
            <iframe id="iframe-player"
                frameborder="0"
                allow="autoplay; fullscreen"
                src="<?php echo $random_embed; ?>">
            </iframe>
        </div>
        <div id="content">
            <h1><?php echo count($playlist_list) - count($_POST); ?> not-yet-deleted videos.</h1>
            <form id="to_delete" action="" method="post" >
            <pre>
            <?php
            $counter = 0;
            $last_percent = 0.0;
            $percent_string = sprintf("%3d%%", $last_percent * 100.0);
            foreach ($playlist_list as $entry) {
                $percent = (double)$counter / (double)count($playlist_list);
                $id = substr($entry, 0, 11);
                $title = substr($entry, 35, -1);
                $url = "https://www.youtube.com/watch?v=" . $id;
                $embed = "https://www.youtube.com/embed/" . $id . "?showinfo=0&autoplay=1&autohide=0&controls=1";
                if(($counter == 0) || ($counter == (count($playlist_list) - 1)) || (round($percent * 100.0) != round($last_percent * 100.0))) {
                    $last_percent = $percent;
                    if($counter == (count($playlist_list) - 1)) {
                        $percent_string = sprintf("%3d%%", round($last_percent * 100.0));
                    } else {
                        $percent_string = sprintf("%3d%%", $last_percent * 100.0);
                    }
                } else {
                    $percent_string = "    ";
                }
                $counter++;
                if(array_key_exists($id, $_POST)) {
                    // Display in RED to show it has been deleted
            ?>
<a name="<?php echo $id; ?>"></a><font color="red"><input style="color: red;" disabled form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>"><?php printf("&nbsp;%s&nbsp;%s",$percent_string,$id);?>&nbsp;<a id="<?php echo $id;?>" style="color: red;" target="__autoplaylist_titles.txt" onclick="return play_link('<?php echo $id; ?>');" href="<?php echo $url;?>"><?php echo $title; ?></a></font>
            <?php
                } else {
                    // We write it out and present the form element
                    $output_list[] = $entry;
                    $url_list[] = $url;
            ?>
<a name="<?php echo $id; ?>"></a><input form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>"><?php printf("&nbsp;%s&nbsp;%s",$percent_string,$id);?>&nbsp;<a id="<?php echo $id;?>" target="__autoplaylist_titles.txt" onclick="return play_link('<?php echo $id; ?>');" href="<?php echo $url;?>"><?php echo $title; ?></a>
            <?php
                }
            }
            ?>
            </pre>
            <?php
            if(is_local_ip()) {
                if(!empty($_POST)) {
                    // If we had anything to delete, write the output back to the file
                    file_put_contents("/home/wiley/public_html/autoplaylist_titles.txt", implode("", $output_list));
                    // Also write the matching raw url list
                    file_put_contents("/home/wiley/public_html/autoplaylist.txt", implode("\n", $url_list));
                }
            ?>
                <input form="to_delete" type="submit" value="DELETE ALL CHECKED!">
            <?php
            } else {
            ?>
                <h1 style="color:red;">Visitors [<?php echo $_SERVER['REMOTE_ADDR']; ?>] cannot delete entries.</h1>
            <?php
            }
            ?>
            </form>
        </div>
    </body>
</html>
