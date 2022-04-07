<?php
require_once 'site_global.php';

$background_image_list = array();

function random_image($dir) {
    global $background_image_list;
    $old_dir = getcwd();
    chdir($dir);

    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $background_image_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($background_image_list);

    chdir($old_dir);
    return $background_image_list[$pick];
}

$PLAYLIST_FILE      = "$FILE_HOME/data/autoplaylist.txt";
$TITLES_FILE        = "$FILE_HOME/data/autoplaylist_titles.txt";
$DOWNLOAD_FILE      = "$FILE_HOME/data/video_list.tar.xz";

$BACKGROUND_DIR     = "$FILE_HOME/gfx/wallpaper/";
$BACKGROUND         = random_image($BACKGROUND_DIR);
$BACKGROUND_DIR_URL = "$URL_HOME/gfx/wallpaper";
$BACKGROUND_URL     = "$BACKGROUND_DIR_URL/$BACKGROUND";
$BACKGROUND_IMG     = "<img class=\"overlay-bg\" src=\"$BACKGROUND_URL\" />";

$SCALE              = 1.0;
$ICON_BASE          = 64;
$FONT_BASE          = 18;
$PLAYER_BASE_WIDTH  = 960;
$PLAYER_BASE_HEIGHT = 540;

$ICON_SIZE          = sprintf("%dpx", (int)($ICON_BASE * $SCALE));
$LEFT_PAD_SIZE      = sprintf("%dpx", (int)($ICON_BASE * 1.5 * $SCALE));
$FONT_SIZE          = sprintf("%dpt", (int)($FONT_BASE * $SCALE));
$PLAYER_WIDTH       = sprintf("%dpx", (int)($PLAYER_BASE_WIDTH * $SCALE));
$PLAYER_HEIGHT      = sprintf("%dpx", (int)($PLAYER_BASE_HEIGHT * $SCALE));
$BANNER_HEIGHT      = sprintf("%dpx", (int)($FONT_BASE * 4.45 * $SCALE));

$NAVHOME_GFX        = "$URL_HOME/gfx/nav/home.png";
$NAVTOP_GFX         = "$URL_HOME/gfx/nav/top.png";
$NAVBOTTOM_GFX      = "$URL_HOME/gfx/nav/bottom.png";
$QUESTION_GFX       = "$URL_HOME/gfx/bar/question_girl3.png";
$DOWNLOAD_GFX       = "$URL_HOME/gfx/download.png";
$DOWNLOAD_URL       = "$URL_HOME/data/video_list.tar.xz";

$BGCOLOR            = "black";
$TEXT               = "#d0d0d0";
$UNVISITED          = "#ffffbf";
//$VISITED            = "#ffa040";
$VISITED            = "#00FF00";
$DELETED            = "#FF0000";

$output_list = array();
$url_list = array();
$playlist_list = file($TITLES_FILE, FILE_SKIP_EMPTY_LINES);
$random_choice = $playlist_list[array_rand($playlist_list)];
$random_id = substr($random_choice, 0, 11);
$new_id = "nothing";
$allowed = false;

if(is_local_ip()) {
    $allowed = true;
}

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
        if($allowed) {
            // First, append the new entry to the raw url list...
            $fp = fopen($PLAYLIST_FILE, "a");
            if($fp) {
                fprintf($fp, "https://www.youtube.com/watch?v=%s\n", $candidate);
                fflush($fp);
                fclose($fp);
                // Then use our external tool to push it to the title list...
                exec("/home/wiley/bin/yt-titles");
                exec("/usr/bin/sync");
                // Reload our playlist...
                $playlist_list = file($TITLES_FILE, FILE_SKIP_EMPTY_LINES);
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
        } else {
            // Not allowed to add a new video, maybe tell the user.
            $new_id = $candidate;
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
            table {
                table-layout: fixed;
                max-width: 99%;
                overflow-x: hidden;
            }
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
            .unblurred {
                font-family: monospace;
                white-space: pre-wrap;
            }
            .blurry:not(:hover) {
                filter: blur(3px);
                font-family: monospace;
                white-space: pre-wrap;
            }
            .blurry:hover {
                font-family: monospace;
                white-space: pre-wrap;
            }
            .glowing:not(:hover) {
                filter: brightness(1);
            }
            .glowing:hover {
                filter: brightness(1.75);
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
            #background-div {
                padding: 0px;
                margin: 0px;
                z-index: -1;
                opacity: 0.20;
                top: 0;
                left: 0;
                bottom: 0;
                right: 0;
                position: fixed;
                height: 100%;
                width: 100%;
            }
            #background-img {
                height: 100%;
                width: 100%;
                object-fit: cover;
            }
            #player-div {
                z-index: 1;
                opacity: 0.65;
                position: fixed;
                top: 0;
                right: 0;
            }
            #iframe-player {
                position: fixed;
                top: 0;
                right: 0;
                width: <?php echo $PLAYER_WIDTH; ?>;
                height: <?php echo $PLAYER_HEIGHT; ?>;
            }
            #content {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
            }
            #content-table {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
            }
            .padding-column {
                text-align: center;
                min-width: <?php echo $LEFT_PAD_SIZE; ?>;
                width: <?php echo $LEFT_PAD_SIZE; ?>;
                max-width: <?php echo $LEFT_PAD_SIZE; ?>;
            }
            .checkbox-column {
                text-align: center;
                min-width: 5%;
                width: 5%;
            }
            .percent-column {
                text-align: center;
                min-width: 5%;
                width: 5%;
            }
            .id-column {
                text-align: center;
                min-width: 10%;
                width: 10%;
            }
            .title-column {
                text-align: left;
                overflow-x: hidden;
                padding-left: 1em;
                white-space: nowrap;
            }
            #headline {
                width: 100%;
                height: <?php echo $BANNER_HEIGHT; ?>;
            }
            #new-addition {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                top: 30%;
                left: 0;
                width: 100%;
                height: <?php echo $BANNER_HEIGHT; ?>;
                background-color: #0000FF;
                color: #FFFFFF;
                text-align: center;
                transform: translateY(-50%);
                display: none;
            }
            #banner {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                top: 0;
                left: 0;
                width: 100%;
                height: <?php echo $BANNER_HEIGHT; ?>;
                background-color: #FF0000;
                color: #FFFFFF;
                text-align: center;
                display: none;
            }
            #question-mark {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                border: none;
                top: 25%;
                left: 10;
                width: <?php echo $ICON_SIZE; ?>;
                transform: translateY(-50%);
            }
            #download-button {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                border: none;
                top: 75%;
                left: 10;
                width: <?php echo $ICON_SIZE; ?>;
                transform: translateY(-50%);
            }
            #nav-home {
                z-index: 2;
                opacity: 0.70;
                position: fixed;
                border: none;
                top: 50%;
                left: 10;
                width: <?php echo $ICON_SIZE; ?>;
                transform: translateY(-50%);
            }
            #delete-button {
                z-index: 2;
                opacity: 0.70;
                background-color: #0000FF;
                color: #FFFFFF;
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
            }
        </style>
        <title> Playlist </title>
        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JSCOOKIE;?>""></script>
        <script src="<?php echo $JSRANDOM;?>""></script>
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
                document.getElementById(id).scrollIntoView({behavior: 'smooth', block: "center"});
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
            //var ms = new Date().getMilliseconds();
            var Random = new MersenneTwister();
<?php
            echo "var BackgroundImageList = [\n";
            echo "\"" . implode("\",\n\"", $background_image_list) . "\"\n";
            echo "];\n";
?>
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
                    <?php if($allowed) { ?>
                        $('#delete-button').val("DELETE " + boxes_checked + " VIDEOS!");
                        $('#delete-button').css("background-color", "#FF0000");
                        $('#delete-button').css("color", "#FFFF00");
                        $('#delete-button').removeAttr("disabled");
                    <?php } ?>
                        showDiv("banner");
                } else if(boxes_checked > 0) {
                        $('#banner-warning').text("You've marked " + boxes_checked + " video for deletion!");
                    <?php if($allowed) { ?>
                        $('#delete-button').val("DELETE " + boxes_checked + " VIDEO!");
                        $('#delete-button').css("background-color", "#FF0000");
                        $('#delete-button').css("color", "#FFFF00");
                        $('#delete-button').removeAttr("disabled");
                    <?php } ?>
                        showDiv("banner");
                } else {
                    <?php if($allowed) { ?>
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
                scroll_to(id);
                current_id = id;
                update_headline();
                // And then stuff it into the player
                $('#iframe-player').attr('src', embed);
                return false;
            }

            function play_new_random() {
                var total = $("input:checkbox").length;
                var choice = Math.floor(total * Random.random());
                var new_id = $($("input:checkbox")[choice]).attr("name");
                if(has_seen(new_id)) {
                    // Two more chances to pick a new unseen video, no loop lagging!
                    choice = Math.floor(total * Random.random());
                    new_id = $($("input:checkbox")[choice]).attr("name");
                }
                if(has_seen(new_id)) {
                    // One more chance to pick a new unseen video, no loop lagging!
                    choice = Math.floor(total * Random.random());
                    new_id = $($("input:checkbox")[choice]).attr("name");
                }
                //$('#new-addition-msg').text("Random number is "+choice+" out of "+total+".");
                //show_new_addition();
                var bg_choice = Math.floor(BackgroundImageList.length * Random.random());
                var new_bg = "<?php echo "$BACKGROUND_DIR_URL/"; ?>" + BackgroundImageList[bg_choice];
                //$('#new-addition-msg').text("Random image is "+new_bg+".");
                //show_new_addition();
                $("#background-img").attr("src", new_bg);
                play_link(new_id);
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
                    scroll_to(id);
                    if("<?php echo $new_id; ?>" != "nothing") {
                        if("<?php echo $allowed; ?>" == "1") {
                            $('#new-addition-msg').text("You've added <?php echo $new_id; ?>, as a new video!");
                        } else {
                            $('#new-addition-msg').text("Sorry, <?php echo $new_id; ?> isn't on the list.");
                        }
                        show_new_addition();
                    } else {
                    }
                }, 500);
                color_links();
            });
        </script>
    </head>
    <body bgcolor="<?php echo $BGCOLOR; ?>" text="<?php echo $TEXT; ?>" link="<?php echo $UNVISITED; ?>" vlink="<?php echo $VISITED; ?>">
        <div id="background-div">
            <img id="background-img" src="<?php echo $BACKGROUND_URL; ?>" />
        </div>
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
        <div id="question-mark">
            <img title="???" class="glowing" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" src="<?php echo $QUESTION_GFX; ?>" onclick="play_new_random();" />
        </div>
        <?php clearstatcache(); if(file_exists($DOWNLOAD_FILE)) { ?>
        <div id="download-button">
            <a title="All the things!" download="autoplaylist.txt" href="<?php echo $DOWNLOAD_URL; ?>">
                <img class="glowing" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" src="<?php echo $DOWNLOAD_GFX; ?>" />
            </a>
        </div>
        <?php } ?>
        <div id="nav-home">
            <img title="Top of page" class="glowing" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" src="<?php echo $NAVTOP_GFX; ?>" onclick="scroll_to(top_id);" />
            <img title="Now playing" class="glowing" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" src="<?php echo $NAVHOME_GFX; ?>" onclick="scroll_to(current_id);" />
            <img title="Bottom of page" class="glowing" height="<?php echo $ICON_SIZE; ?>" width="<?php echo $ICON_SIZE; ?>" src="<?php echo $NAVBOTTOM_GFX; ?>" onclick="scroll_to(bottom_id);" />
        </div>
        <div id="content">
            <div id="headline">
                <h1 id="headline-h1">&nbsp;<?php echo count($playlist_list) - count($_POST); ?> not-yet-deleted videos.</h1>
            </div>
            <hr />
            <form id="to_delete" action="" method="post" >
            <table id="content-table">
                <thead>
                <tr>
                    <th class="padding-column">&nbsp;</th>
                    <th class="checkbox-column"><font color="red">DELETE</font></th>
                    <th class="percent-column">%</th>
                    <th class="id-column">---ID---</th>
                    <th class="title-column">TITLE</th>
                </tr>
                </thead>
                <tbody>
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
                    $red_style = "";
                    $disabled = "";
                    $red_font = "";
                    $red_font_end = "";
                    if(array_key_exists($id, $_POST)) {
                        // We make things red to show it was deleted
                        $red_style = "style=\"color: red;\" ";
                        $disabled = "disabled ";
                        $red_font = "<font color=\"red\"> ";
                        $red_font_end = "</font> ";
                    } else {
                        // We write it out and present the form element
                        $output_list[] = $entry;
                        $url_list[] = $url;
                    }
                    ?>
                    <tr>
                        <td class="padding-column">
                            <a name="<?php echo $id; ?>" class="<?php echo $aname_class; ?>">&nbsp;</a>
                        </td>
                        <td class="checkbox-column">
                            <input onchange="check_box(this);" <?php echo $red_style; echo $disabled; ?> form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>">
                        </td>
                        <td class="percent-column">
                            <?php echo $red_font; echo $percent_string; echo $red_font_end; ?>
                        </td>
                        <td class="id-column">
                            <?php echo $red_font; echo $id; echo $red_font_end; ?>
                        </td>
                        <td class="title-column">
                            <a id="<?php echo $id;?>" <?php echo $red_style; ?> target="__autoplaylist" onclick="return play_link('<?php echo $id; ?>');" href="<?php echo $url;?>"><?php echo $title; ?></a>
                        </td>
                    </tr>
                <?php
                }
                ?>
                </tbody>
            </table>

            <?php
            if($allowed) {
                if(!empty($_POST)) {
                    // If we had anything to delete, write the output back to the file
                    file_put_contents($TITLES_FILE, implode("", $output_list));
                    // Also write the matching raw url list
                    file_put_contents($PLAYLIST_FILE, implode("\n", $url_list)."\n");
                }
            }
            ?>

            <?php if($allowed) { ?>
                <input id="delete-button" disabled form="to_delete" type="submit" value="DELETE ALL CHECKED!">
            <?php } else { ?>
                <h1 style="color:red;">Visitors from [<?php echo $_SERVER['REMOTE_ADDR']; ?>] cannot delete entries.</h1>
            <?php } ?>
            </form>
        </div>
    </body>
</html>
