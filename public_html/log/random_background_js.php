<?php
require_once 'site_global.php';
require_once 'random_background.php';
require_once 'log_navigation.php';
header("Content-Type: application/json; charset=UTF-8");
?>
// This depends on <script src="<?php echo $JSRANDOM;?>"></script>
// This also depends on <script src="<?php echo $JSCOOKIE;?>"></script>

var BackgroundImageList = <?php echo json_encode($background_image_list); ?>;
var SpecialImageList = <?php echo json_encode($special_image_list); ?>;
var BoringImageList = <?php echo json_encode($boring_image_list); ?>;
var TodayDirExists = <?php echo $today_dir_exists ? "1" : "0"; ?>;
var VisitorIP = "<?php echo $VISITOR_IP; ?>";
var OnePixelBG = "<?php echo $ONE_PIXEL_ICON; ?>";
var BG_CUTE = "<?php echo $BG_CUTE_ICON; ?>";
var BG_ON = "<?php echo $BG_ON_ICON; ?>";
var BG_OFF = "<?php echo $BG_OFF_ICON; ?>";

var Random = new MersenneTwister();
var NoBackground = Cookies.get('nobackground') || false;
var BoringBackground = Cookies.get('boringbackground') || false;

function toggleBackground() {
    NoBackground = Cookies.get('nobackground') || false;
    Cookies.remove("boringbackground");
    if(NoBackground) {
        // Turn it on now.
        Cookies.remove("nobackground");
        NoBackground = false;
        $("#navbar-button-background").attr("src", BG_ON);
        $("#navbar-button-background").attr("title", "Make boring.");
    } else {
        Cookies.set("nobackground", true);
        NoBackground = true;
        $("#navbar-button-background").attr("src", BG_OFF);
        $("#navbar-button-background").attr("title", "Make kawaii!");
    }
    randomizeBackground();
}

function syncBackgroundToggleIcon() {
    NoBackground = Cookies.get('nobackground') || false;
    Cookies.remove("boringbackground");
    if(NoBackground) {
        $("#navbar-button-background").attr("src", BG_OFF);
        $("#navbar-button-background").attr("title", "Make kawaii!");
    } else {
        $("#navbar-button-background").attr("src", BG_ON);
        $("#navbar-button-background").attr("title", "Make boring.");
    }
}

function new_syncBackgroundToggleIcon() {
    NoBackground = Cookies.get('nobackground') || false;
    BoringBackground = Cookies.get('boringbackground') || false;
    if(NoBackground) {
        $("#navbar-button-background").attr("src", BG_OFF);
        $("#navbar-button-background").attr("title", "Make colorful.");
    } else if(BoringBackground) {
        $("#navbar-button-background").attr("src", BG_ON);
        $("#navbar-button-background").attr("title", "Make kawaii!");
    } else {
        $("#navbar-button-background").attr("src", BG_CUTE);
        $("#navbar-button-background").attr("title", "Make boring.");
    }
}

function new_toggleBackground() {
    NoBackground = Cookies.get('nobackground') || false;
    BoringBackground = Cookies.get('boringbackground') || false;
    if(NoBackground) {
        // We have no background, but want generic, safe, boring backgrounds.
        Cookies.remove("nobackground");
        NoBackground = false;
        Cookies.set("boringbackground", true);
        BoringBackground = true;
        $("#navbar-button-background").attr("src", BG_ON);
        $("#navbar-button-background").attr("title", "Make kawaii!");
    } else if(BoringBackground) {
        // We have backgrounds, but want super cute backgrounds!
        Cookies.remove("nobackground");
        NoBackground = false;
        Cookies.remove("boringbackground");
        BoringBackground = false;
        $("#navbar-button-background").attr("src", BG_CUTE);
        $("#navbar-button-background").attr("title", "Make boring.");
    } else {
        // We are cute, but we want no backgrounds at all.
        Cookies.set("nobackground", true);
        NoBackground = true;
        Cookies.set("boringbackground", true);
        BoringBackground = true;
        $("#navbar-button-background").attr("src", BG_OFF);
        $("#navbar-button-background").attr("title", "Make colorful.");
    }
    randomizeBackground();
}

function randomizeBackground() {
    if(NoBackground) {
        $("#background-img").attr("src", OnePixelBG);
    } else {
        if(BoringBackground) {
            // We do NOT want to do special day overrides if you are in boring mode.
            var bg_choice = Math.floor(BoringImageList.length * Random.random());
            var new_bg = "<?php echo "$BORING_DIR_URL/"; ?>" + BoringImageList[bg_choice];
            $("#background-img").attr("src", new_bg);
        } else if(TodayDirExists) {
            var bg_choice = Math.floor(SpecialImageList.length * Random.random());
            var new_bg = "<?php echo "$SPECIAL_DIR_URL/"; ?>" + SpecialImageList[bg_choice];
            $("#background-img").attr("src", new_bg);
        } else {
            var bg_choice = Math.floor(BackgroundImageList.length * Random.random());
            var new_bg = "<?php echo "$BACKGROUND_DIR_URL/"; ?>" + BackgroundImageList[bg_choice];
            $("#background-img").attr("src", new_bg);
        }
    }
}
