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
$NAVHOME_GFX    = "$URL_HOME/gfx/navhome.png";
$NAVTOP_GFX     = "$URL_HOME/gfx/nav/green/top.png";
$NAVBOTTOM_GFX  = "$URL_HOME/gfx/nav/green/bottom.png";

$UNVISITED      = "#ffffbf";
//$UNVISITED      = "#ffa040";
$VISITED        = "#00FF00";
$DELETED        = "#FF0000";

$output_list = array();
$url_list = array();
$playlist_list = file("/home/wiley/public_html/autoplaylist_titles.txt", FILE_SKIP_EMPTY_LINES);
$random_choice = $playlist_list[array_rand($playlist_list)];
$random_id = substr($random_choice, 0, 11);
$new_id = "nothing";

if(array_key_exists('v', $_GET)) {
    $candidate = $_GET['v'];
    $found = false;

    foreach($playlist_list as $k => $v) {
        if(substr($v, 0, 11) == $candidate) {
            $random_id = $candidate;
            $found = true;
            break;
        }
    }
    if(!$found) {
        // The candidate wasn't in our playlist... add it?
        if(is_local_ip()) {
            // First, append the new entry to the raw url list...
            $fp = fopen("/home/wiley/public_html/autoplaylist.txt", "a");
            if($fp) {
                fprintf($fp, "https://www.youtube.com/watch?v=%s\n", $candidate);
                fflush($fp);
                fclose($fp);
                // Then use our external tool to push it to the title list...
                exec("/home/wiley/bin/yt-titles");
                exec("/usr/bin/sync");
                // Reload our playlist...
                $playlist_list = file("/home/wiley/public_html/autoplaylist_titles.txt", FILE_SKIP_EMPTY_LINES);
                $found = false;
                // And check again to see if it's there now.
                foreach($playlist_list as $k => $v) {
                    if(substr($v, 0, 11) == $candidate) {
                        $random_id = $candidate;
                        $found = true;
                        break;
                    }
                }
                if(!$found) {
                    // Well, we tried... something went wrong...
                    $random_choice = $playlist_list[array_rand($playlist_list)];
                    $random_id = substr($random_choice, 0, 11);
                } else {
                    $new_id = $random_id;
                }
            }
        }
    }
}

$random_url = "https://www.youtube.com/watch?v=" . $random_id;
$random_embed = "https://www.youtube.com/embed/" . $random_id . "?showinfo=0&autoplay=1&autohide=0&controls=1&mute=1";
// YouTube is stupid and will only autoplay if muted until you click something...

?>
<html>
    <head>
        <style>
            a {
                text-decoration:none;
                color: <?php echo $UNVISITED; ?>;
            }
            a:visited {
                color: <?php echo $UNVISITED; ?>;
            }
            a:hover {
                text-decoration:underline;
            }
            a:active, a:focus {
                outline: 0;
                border: none;
                -moz-outline-style: none;
            }
            @keyframes blinking {
                0% {
                    opacity: 0;
                }
                49% {
                    opacity: 0;
                }
                50% {
                    opacity: 1;
                }
                100% {
                    opacity: 1;
                }
            }
            .flash_tag {
                animation: blinking 1.5s infinite;
            }
            html, body {
                font-family: 'Lato', sans-serif;
                padding: 0px;
                margin: 0px;
            }
            body::after {
                z-index: -1;
                opacity: 0.20;
                content: "";
                background: url(<?php echo $BACKGROUND_URL; ?>);
                top: 0;
                left: 0;
                bottom: 0;
                right: 0;
                position: fixed;
                background-size: cover;
            }
            #player-div {
                z-index: 1;
                opacity: 0.65;
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
            #headline {
                width: 100%;
                height: 80px;
            }
            #new-addition {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                top: 0;
                left: 0;
                width: 100%;
                height: 80px;
                background-color: #0000FF;
                color: #FFFFFF;
                text-align: center;
                display: none;
            }
            #banner {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                top: 0;
                left: 0;
                width: 100%;
                height: 80px;
                background-color: #FF0000;
                color: #FFFFFF;
                text-align: center;
                display: none;
            }
            #nav-home {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                border: none;
                top: 50%;
                left: 10;
                width: 48px;
                transform: translateY(-50%);
            }
            #delete-button {
                z-index: 2;
                opacity: 0.70;
                background-color: #0000FF;
                color: #FFFFFF;
            }
        </style>
        <title> Playlist </title>
        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JSCOOKIE;?>""></script>
        <script type="text/javascript">
            function toggleDiv(divID) {
                element = document.getElementById(divID);
                if(element !== undefined && element !== null) {
                    if(element.style.display == 'none') {
                        element.style.display = 'block';
                    } else {
                        element.style.display = 'none';
                    }
                }
            }
            function showDiv(divID) {
                element = document.getElementById(divID);
                if(element !== undefined && element !== null) {
                    element.style.display = 'block';
                }
            }
            function hideDiv(divID) {
                element = document.getElementById(divID);
                if(element !== undefined && element !== null) {
                    element.style.display = 'none';
                }
            }
            function scroll_to(id) {
                document.getElementById(id).scrollIntoView({behavior: 'smooth'});
            }
        </script>
        <script type="text/javascript">
            var boxes_checked = 0;
            var box_ids = [];
            var current_id = "<?php echo $random_id; ?>";
            //var top_id = "<?php echo substr($playlist_list[0], 0, 11); ?>";
            var top_id = "headline";
            var bottom_id = "<?php echo substr($playlist_list[array_key_last($playlist_list)], 0, 11); ?>";
            var jar = [];
            var disabled = [];

            function show_new_addition() {
                showDiv("new-addition");
                setTimeout(function() {
                    hideDiv("new-addition");
                }, 10000);
            }

            function update_headline() {
                $('#headline-h1').text( " " + (<?php echo count($playlist_list); ?> - disabled.length) + " not-yet-deleted videos.  You've seen " + jar.length + " of them.");
            }

            function update_delete_stuff() {
                if(boxes_checked > 1) {
                        $('#banner-warning').text("You've marked " + boxes_checked + " videos for deletion!");
                    <?php if(is_local_ip()) { ?>
                        $('#delete-button').val("DELETE " + boxes_checked + " VIDEOS!");
                        $('#delete-button').css("background-color", "#FF0000");
                        $('#delete-button').css("color", "#FFFF00");
                        $('#delete-button').removeAttr("disabled");
                    <?php } ?>
                        showDiv("banner");
                } else if(boxes_checked > 0) {
                        $('#banner-warning').text("You've marked " + boxes_checked + " video for deletion!");
                    <?php if(is_local_ip()) { ?>
                        $('#delete-button').val("DELETE " + boxes_checked + " VIDEO!");
                        $('#delete-button').css("background-color", "#FF0000");
                        $('#delete-button').css("color", "#FFFF00");
                        $('#delete-button').removeAttr("disabled");
                    <?php } ?>
                        showDiv("banner");
                } else {
                    <?php if(is_local_ip()) { ?>
                    $('#delete-button').val("Nothing to DELETE...yet!");
                    $('#delete-button').css("background-color", "#0000FF");
                    $('#delete-button').css("color", "#FFFFFF");
                    $('#delete-button').attr("disabled", true);
                    <?php } ?>
                    hideDiv("banner");
                }
            }

            function mark_seen(id) {
                var idx = jar.indexOf(id);
                if(idx == -1) {
                    // If not seen before, save it!
                    jar.push(id);
                    var cookie = JSON.stringify(jar);
                    Cookies.set('jar', cookie, { expires: 30 });
                }
            }

            function has_seen(id) {
                var idx = jar.indexOf(id);
                if(idx == -1) {
                    return false;
                } else {
                    return true;
                }
            }

            function red_box(obj) {
                var name = $(obj).attr("name");
                var idx = box_ids.indexOf(name);
                if(idx == -1) {
                    // Add to array to make RED
                    box_ids.push(name);
                }
                return name;
            }

            function green_box(obj) {
                var name = $(obj).attr("name");
                var idx = box_ids.indexOf(name);
                if(idx != -1) {
                    // Remove from array to make RED
                    box_ids.splice(idx, 1);
                }
                return name;
            }

            function color_a_link(id) {
                var element = document.getElementById(id);
                if(element !== undefined && element !== null) {
                    var idx = box_ids.indexOf(id);
                    if(idx != -1) {
                        // If the box has been checked, it's dead.
                        element.style.color = "<?php echo $DELETED; ?>";
                    } else {
                        if(has_seen(id)) {
                            element.style.color = "<?php echo $VISITED; ?>";
                        } else {
                            element.style.color = "<?php echo $UNVISITED; ?>";
                        }
                    }
                } else {
                    return false;
                }
                return true;
            }

            function check_box(obj) {
                var name = $(obj).attr("name");
                if($(obj).is(":checked")) {
                    boxes_checked++;
                    update_delete_stuff();
                    red_box(obj);
                } else {
                    boxes_checked--;
                    update_delete_stuff();
                    green_box(obj);
                }
                color_a_link(name);
            }

            function color_links() {
                //var jar = Cookies.get();
                //for (const [name,url] of Object.entries(jar)) {
                //    color_a_link(name);
                //}
                var cookie = Cookies.get('jar');
                if(cookie != undefined && cookie != null) {
                    jar = JSON.parse(cookie);
                }

                var stale = [];
                for (const name of jar) {
                    if(color_a_link(name) == false) {
                        // This ID is not in our playlist, remove the entry.
                        stale.push(name);
                    }
                }
                // Now we can iterate over the stale cookies and remove them.
                for (const name of stale) {
                    var idx = jar.indexOf(name);
                    jar.splice(idx, 1);
                }
                stale = [];

                disabled = document.querySelectorAll('input:disabled');
                for (const obj of disabled) {
                    var name = red_box(obj);
                    color_a_link(name);
                }

                update_headline();
                update_delete_stuff();
            }

            function play_link(id) {
                // We want to mark the clicked link as visited
                // To do this, since we aren't visiting it in the browser
                // and aren't allowed to modify the browser's history, we'll
                // store it in a cookie and color the links as loaded if
                // the id matches...
                url = "https://www.youtube.com/watch?v=" + id;
                embed = "https://www.youtube.com/embed/" + id + "?showinfo=0&autoplay=1&autohide=0&controls=1";
                mark_seen(id);
                // Refresh link color
                color_a_link(id);
                // Unblink the old link, and blink the new one.
                document.getElementById(current_id).classList.remove("flash_tag");
                document.getElementById(id).classList.add("flash_tag");
                current_id = id;
                update_headline();
                // And then stuff it into the player
                $('#iframe-player').attr('src', embed);
                return false;
            }

            $(document).ready(function() {
                setTimeout(function() {
                    var id = "<?php echo $random_id;?>";
                    var url = "<?php echo $random_url;?>";
                    //location.hash = "#<?php echo $random_id; ?>";
                    mark_seen(id);
                    update_headline();
                    document.getElementById(id).style.color = "<?php echo $VISITED; ?>";
                    document.getElementById(id).classList.add("flash_tag");
                    document.getElementById(id).scrollIntoView({behavior: 'smooth'});
                    if("<?php echo $new_id; ?>" != "nothing") {
                        $('#new-addition-msg').text("You've added <?php echo $new_id; ?>, as a new video!");
                        show_new_addition();
                    }
                }, 500);
                color_links();
            });
        </script>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="<?php echo $UNVISITED; ?>" vlink="<?php echo $UNVISITED; ?>">
        <div id="player-div">
            <iframe id="iframe-player"
                frameborder="0"
                allow="autoplay; fullscreen"
                src="<?php echo $random_embed; ?>">
            </iframe>
        </div>
        <div id="new-addition">
            <h1 id="new-addition-msg" class="flash_tag"> You've added a new video! </h1>
        </div>
        <div id="banner">
            <h1 id="banner-warning" class="flash_tag"> You've marked something for deletion! </h1>
        </div>
        <div id="nav-home">
            <img height="48" width="48" src="<?php echo $NAVTOP_GFX; ?>" onclick="scroll_to(top_id);" />
            <img height="48" width="48" src="<?php echo $NAVHOME_GFX; ?>" onclick="scroll_to(current_id);" />
            <img height="48" width="48" src="<?php echo $NAVBOTTOM_GFX; ?>" onclick="scroll_to(bottom_id);" />
        </div>
        <div id="content">
            <div id="headline">
                <h1 id="headline-h1">&nbsp;<?php echo count($playlist_list) - count($_POST); ?> not-yet-deleted videos.</h1>
                <hr />
            </div>
            <form id="to_delete" action="" method="post" >
            <pre>
          <font color="red">DELETE</font>     ----ID----- TITLE
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
                $is_top = 0;
                $is_bottom = 0;
                $aname_class = "middle";
                if($counter == 0) {
                    $is_top = 1;
                    $is_bottom = 0;
                    $aname_class = "top";
                }
                if($counter == (count($playlist_list) - 1)) {
                    $is_top = 0;
                    $is_bottom = 1;
                    $aname_class = "bottom";
                }
                if($is_top || $is_bottom || (round($percent * 100.0) != round($last_percent * 100.0))) {
                    $last_percent = $percent;
                    if($is_bottom) {
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
<a name="<?php echo $id; ?>" class="<?php echo $aname_class; ?>"></a><font color="red"><input onchange="check_box(this);" style="color: red;" disabled form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>"><?php printf("&nbsp;%s&nbsp;%s",$percent_string,$id);?>&nbsp;<a id="<?php echo $id;?>" style="color: red;" target="__autoplaylist_titles.txt" onclick="return play_link('<?php echo $id; ?>');" href="<?php echo $url;?>"><?php echo $title; ?></a></font>
            <?php
                } else {
                    // We write it out and present the form element
                    $output_list[] = $entry;
                    $url_list[] = $url;
            ?>
<a name="<?php echo $id; ?>" class="<?php echo $aname_class; ?>"></a><input onchange="check_box(this);" form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>"><?php printf("&nbsp;%s&nbsp;%s",$percent_string,$id);?>&nbsp;<a id="<?php echo $id;?>" target="__autoplaylist_titles.txt" onclick="return play_link('<?php echo $id; ?>');" href="<?php echo $url;?>"><?php echo $title; ?></a>
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
                    file_put_contents("/home/wiley/public_html/autoplaylist.txt", implode("\n", $url_list)."\n");
                }
            ?>
                <input id="delete-button" disabled form="to_delete" type="submit" value="DELETE ALL CHECKED!">
            <?php
            } else {
            ?>
                <h1 style="color:red;">Visitors from [<?php echo $_SERVER['REMOTE_ADDR']; ?>] cannot delete entries.</h1>
            <?php
            }
            ?>
            </form>
        </div>
    </body>
</html>
