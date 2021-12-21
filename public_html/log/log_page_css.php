<?php
require_once 'site_global.php';
header("Content-type: text/css");
?>
#debug-message {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow-x: hidden;
}

#content-header {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow-x: hidden;
    position: fixed;
    background-color: black;
    min-width: 100%;
    width: 100%;
    top: <?php echo $ICON_SIZE; ?>;
    left: 0;
    z-index: 3;
}
.content-date-header {
    text-align: left;
    min-width: 11ch;
    width: 11ch; 
}
.content-time-header {
    text-align: left;
    min-width: 9ch;
    width: 9ch; 
}
.content-channel-header {
    text-align: left;
    overflow-x: hidden;
    white-space: nowrap;
    min-width: 16ch;
    width: 16ch; 
}
.content-speaker-header {
    text-align: left;
    min-width: 30ch;
    width: 30ch; 
}
.content-message-header {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $TINY_FONT_SIZE; ?>;
    text-align: right;
    overflow-x: hidden;
    white-space: nowrap;
    padding-left: 1ch;
    padding-right: 1ch;
}

#content-table {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow-x: hidden;
    min-width: 100%;
    width: 100%;
}
.content-date-column {
    text-align: left;
    min-width: 11ch;
    width: 11ch; 
}
.content-time-column {
    text-align: left;
    min-width: 9ch;
    width: 9ch; 
}
.content-channel-column {
    text-align: left;
    overflow-x: hidden;
    white-space: nowrap;
    min-width: 16ch;
    width: 16ch; 
}
.content-speaker-column {
    text-align: left;
    min-width: 30ch;
    width: 30ch; 
}
.content-message-column {
    font-family: "DejaVu Sans Mono", Consolas, "Lucida Console", Monaco, Courier, monospace;
    text-align: left;
    overflow-x: hidden;
    white-space: normal;
    padding-left: 1ch;
}
.content-alert-column {
    text-align: center;
    overflow-x: hidden;
    white-space: normal;
}
#content-table tr:nth-child(even) {
    background-color: <?php echo $EVEN; ?>;
}
#content-table tr:nth-child(odd) {
    background-color: <?php echo $ODD; ?>;
}

#page-load-time {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $TINY_FONT_SIZE; ?>;
    position: fixed;
    bottom: 0px;
    right: 0px;
    z-index: 2;
    background-color: black;
}
