<?php
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

    $BACKGROUND_DIR = "/home/wiley/public_html/gfx/wallpaper/";
    $URL_HOME       = "http://wileymud.themud.org/~wiley";
    $BACKGROUND     = random_image($BACKGROUND_DIR);
    $BACKGROUND_HREF= "<a href=\"$URL_HOME/gfx/wallpaper/$BACKGROUND\"> $BACKGROUND </a>";
    $BACKGROUND_IMG = "<img class=\"overlay-bg\" src=\"$URL_HOME/gfx/wallpaper/$BACKGROUND\" />";
    $JQ             = "jquery.js";
    $BACKGROUND_URL = "$URL_HOME/gfx/wallpaper/$BACKGROUND";

    $CSS_FILE       = "random_image.css";
    $CSS_TIME       = filemtime($CSS_FILE);
    $CSS_STUFF      = "random_image.css?version=$CSS_TIME";

    //$url = sprintf("Location: %s", $BACKGROUND_URL);
    //header($url);
    //die("You are no fun.");
?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <script src="<?php echo $JQ;?>""></script>
        <link rel="stylesheet" href="<?php echo $CSS_STUFF;?>">
        <script language="javascript">
	    function setCookie(cname, cvalue, exdays) {
		var d = new Date();
		d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
		var expires = "expires="+d.toUTCString();
		document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
                return cvalue;
	    }
	    function getCookie(cname) {
		var name = cname + "=";
		var ca = document.cookie.split(';');
		for(var i = 0; i < ca.length; i++) {
		    var c = ca[i];
		    while (c.charAt(0) == ' ') {
			c = c.substring(1);
		    }
		    if (c.indexOf(name) == 0) {
			return c.substring(name.length, c.length);
		    }
		}
		return "";
	    }
            function start() {
		var slideshow = getCookie("slideshow");
                var t;
                if( slideshow == "go" ) {
                    document.getElementById("slideshow").style.display = 'block';
                    t = setTimeout( function() {
                        window.location.reload(true);
                    }, 10000); // set timer for refresh
                }
                $(document).keydown( function(event) {
                    if (event.which === 32) { // space
                        window.location.reload(true);
                    } else if (event.which === 13) { // enter
                        window.location.reload(true);
                    } else if (event.which === 83 || event.which === 115) { // 'S' or 's'
                        if(slideshow == "go") {
                            slideshow = setCookie("slideshow", "nope", 365);
                            document.getElementById("slideshow").style.display = 'none';
                            clearTimeout(t);
                        } else {
                            slideshow = setCookie("slideshow", "go", 365);
                            document.getElementById("slideshow").style.display = 'block';
                            document.getElementById("slideshow").style.display = 'block';
                            t = setTimeout( function() {
                                window.location.reload(true);
                            }, 10000); // set timer for refresh
                        }
                    }
                });
            }
        </script>
    </head>
    <body onload="start();">
        <?php echo $BACKGROUND_IMG; ?>
        <div class="filename"> <?php echo $BACKGROUND_HREF; ?></div>
        <div align="right" id="slideshow" class="slideshow"> S </div>
    </body>
</html>
