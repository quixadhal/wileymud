<?php
require_once 'site_global.php';
require_once 'navbar.php';
require_once 'log_navigation.php';
header("Content-type: text/css");
?>
input, select, textarea {
    border-color: <?php echo $INPUT_BORDER;?> !important;
    background-color: <?php echo $INPUT_BACKGROUND;?> !important;
    color: <?php echo $INPUT;?> !important;
}
input:focus, textarea:focus {
    border-color: <?php echo $SELECTED_INPUT_BORDER;?> !important;
    background-color: <?php echo $SELECTED_INPUT_BACKGROUND;?> !important;
    color: <?php echo $SELECTED_INPUT;?> !important;
}
#datepicker {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $SMALL_FONT_SIZE; ?>;
    text-align: center;
}
