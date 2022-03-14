<?php
require_once 'site_global.php';
require_once 'navbar.php';
header("Content-type: text/css");
?>
.nav-banner-img {
    border: none;
    height: <?php echo $ICON_SIZE; ?>;
    max-height: <?php echo $ICON_SIZE; ?>;
    min-height: <?php echo $ICON_SIZE; ?>;
}
.nav-img {
    border: none;
    height: <?php echo $ICON_SIZE; ?>;
    max-height: <?php echo $ICON_SIZE; ?>;
    min-height: <?php echo $ICON_SIZE; ?>;
    width: <?php echo $ICON_SIZE; ?>;
    max-width: <?php echo $ICON_SIZE; ?>;
    min-width: <?php echo $ICON_SIZE; ?>;
}
.nav-small-img {
    border: none;
    height: <?php echo $SMALL_ICON_SIZE; ?>;
    max-height: <?php echo $SMALL_ICON_SIZE; ?>;
    min-height: <?php echo $SMALL_ICON_SIZE; ?>;
    width: <?php echo $SMALL_ICON_SIZE; ?>;
    max-width: <?php echo $SMALL_ICON_SIZE; ?>;
    min-width: <?php echo $SMALL_ICON_SIZE; ?>;
}
#fake-navbar {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow: hidden;
    height: calc(2px + <?php echo $ICON_SIZE; ?>);
    max-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    background-color: black;
    min-width: 100%;
    width: 100%;
    top: 0;
    left: 0;
}
#back-navbar {
    text-align: left;
    min-width: 100%;
    width: 100%;
    top: 0;
    left: 0;
    z-index: 2;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow: hidden;
    height: calc(2px + <?php echo $ICON_SIZE; ?>);
    max-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    min-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    position: fixed;
    background-color: black;
}
#navbar-left {
    text-align: left;
    min-width: 30%;
    width: 30%; 
    top: 0;
    left: 0;
    z-index: 3;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow: hidden;
    height: calc(2px + <?php echo $ICON_SIZE; ?>);
    max-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    min-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    position: fixed;
    background-color: black;
}
#navbar-center {
    text-align: center;
    min-width: 40%;
    width: 40%; 
    top: 0;
    left: 50%;
    transform: translateX(-50%);
    z-index: 3;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow: hidden;
    height: calc(2px + <?php echo $ICON_SIZE; ?>);
    max-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    min-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    position: fixed;
    background-color: black;
}
#navbar-right {
    text-align: right;
    min-width: 30%;
    width: 30%; 
    top: 0;
    right: 0;
    z-index: 3;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow: hidden;
    height: calc(2px + <?php echo $ICON_SIZE; ?>);
    max-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    min-height: calc(2px + <?php echo $ICON_SIZE; ?>);
    position: fixed;
    background-color: black;
}
#refresh-time {
    vertical-align: top;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $SMALL_FONT_SIZE; ?>;
}
.wileymud-version {
    vertical-align: bottom;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $SMALL_FONT_SIZE; ?>;
    color: #bb0000;
}
.wileymud-gap {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $SMALL_FONT_SIZE; ?>;
    color: #bb0000;
    min-width: 2ch;
    width: 2ch;
}
.wileymud-build-date {
    vertical-align: top;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $SMALL_FONT_SIZE; ?>;
    color: #bb0000;
}
