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

?>
<html>
    <head>
        <style>
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
            a:active, a:focus { outline: 0; border: none; -moz-outline-style: none; }
            body::after { content: ""; background: url(<?php echo $BACKGROUND_URL; ?>); opacity: 0.15; top: 0; left: 0; bottom: 0; right: 0; position: fixed; z-index: -1; background-size: cover; }
        </style>
        <title> Playlist </title>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040">

<?php
    $playlist_list = file("/home/wiley/public_html/autoplaylist_titles.txt", FILE_SKIP_EMPTY_LINES);
    $output_list = array();
    $url_list = array();
?>
        <h1><?php echo count($playlist_list) - count($_POST); ?> not-yet-deleted videos.</h1>
        <form id="to_delete" action="" method="post" >
        <pre>
        <?php foreach ($playlist_list as $entry) {
            $id = substr($entry, 0, 11);
            $title = substr($entry, 35, -1);
            $url = "https://www.youtube.com/watch?v=" . $id;
            if(array_key_exists($id, $_POST)) {
                // Display in RED to show it has been deleted
        ?>
            <font color="red"><input style="color: red;" disabled form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>"><?php echo $id;?>&nbsp;<a style="color: red;" target="__autoplaylist_titles.txt" href="<?php echo $url;?>"><?php echo $title; ?></a></font>
        <?php
            } else {
                // We write it out and present the form element
                $output_list[] = $entry;
                $url_list[] = $url;
        ?>
            <input form="to_delete" type="checkbox" name="<?php echo $id;?>" value="<?php echo $id;?>"><?php echo $id;?>&nbsp;<a target="__autoplaylist_titles.txt" href="<?php echo $url;?>"><?php echo $title; ?></a>
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
    </body>
</html>
