<?php
require_once 'site_global.php';
require_once 'random_background.php';
header("Content-Type: application/json; charset=UTF-8");

echo "var BackgroundImageList = [\n";
echo "\"" . implode("\",\n\"", $background_image_list) . "\"\n";
echo "];\n";
?>
// This depends on <script src="<?php echo $JSRANDOM;?>""></script>

var Random = new MersenneTwister();
function randomizeBackground() {
    var bg_choice = Math.floor(BackgroundImageList.length * Random.random());
    var new_bg = "<?php echo "$BACKGROUND_DIR_URL/"; ?>" + BackgroundImageList[bg_choice];
    $("#background-img").attr("src", new_bg);
}
