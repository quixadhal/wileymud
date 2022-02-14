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
var TodayDirExists = <?php echo $today_dir_exists ? "1" : "0"; ?>;
var VisitorIP = "<?php echo $VISITOR_IP; ?>";
var OnePixelBG = "<?php echo $ONE_PIXEL_ICON; ?>";

if(VisitorIP === "142.113.220.26") {
    Cookies.set("nobackground", true);
    //Cookies.remove("nobackground");
}

var NoBackground = Cookies.get('nobackground') || false;

var Random = new MersenneTwister();
function randomizeBackground() {
    if(NoBackground) {
        $("#background-img").attr("src", OnePixelBG);
    } else {
        if(TodayDirExists) {
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
