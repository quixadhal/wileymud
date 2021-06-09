<?php
require_once 'site_global.php';
header("Content-type: text/css");
?>
#player-header-table {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    border: none;
    border-spacing: none;
    padding: none;
    width: 80%;
    text-align: center;
    margin-left: 10%;
    margin-right: 10%;
}
#player-header-row {
    background-color: #2f0000;
}
#player-table {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    border: none;
    border-spacing: none;
    padding: none;
    width: 80%;
    text-align: center;
    margin-left: 10%;
    margin-right: 10%;
}
#player-table tr:nth-child(even) {
    background-color: <?php echo $EVEN; ?>
}
#player-table tr:nth-child(odd) {
    background-color: <?php echo $ODD; ?>
}
.player-table-header-row {
    background-color: #2f0000;
}
.player-table-idle {
    text-align: center;
    width: 10ch;
}
.player-table-rank {
    text-align: center;
    width: 10ch;
}
.player-table-name {
    text-align: left;
}

#status-header-table {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    border: none;
    border-spacing: none;
    padding: none;
    width: 80%;
    text-align: center;
    margin-left: 10%;
    margin-right: 10%;
}
#status-header-row {
    background-color: #002f00;
}
#status-table {
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: <?php echo $FONT_SIZE; ?>;
    border: none;
    border-spacing: none;
    padding: none;
    width: 80%;
    align: center;
    text-align: center;
    margin-left: 10%;
    margin-right: 10%;
}
#status-table tr:nth-child(even) {
    background-color: <?php echo $EVEN; ?>
}
#status-table tr:nth-child(odd) {
    background-color: <?php echo $ODD; ?>
}
.status-table-players {
    text-align: center;
    width: 10ch;
}
.status-table-gods {
    text-align: center;
    width: 10ch;
}
.status-table-uptime {
    text-align: left;
}

#content-header-table {
    font-family: 'Lato', sans-serif;
    font-size: <?php echo $FONT_SIZE; ?>;
    border: none;
    border-spacing: none;
    padding: none;
    width: 80%;
    text-align: center;
    margin-left: 10%;
    margin-right: 10%;
}
#content-header-row {
    background-color: #00002f;
}
#content-table {
    font-family: 'Lato', sans-serif;
    font-size: <?php echo $FONT_SIZE; ?>;
    border: none;
    border-spacing: none;
    padding: none;
    width: 80%;
    align: center;
    text-align: center;
    margin-left: 10%;
    margin-right: 10%;
}
#content-table tr:nth-child(even) {
    background-color: <?php echo $EVEN; ?>
}
#content-table tr:nth-child(odd) {
    background-color: <?php echo $ODD; ?>
}
.content-left-login {
    text-align: center;
    width: 194px;
}
.content-left-gap {
    text-align: left;
    width: 50px;
}
.content-left-info {
    text-align: left;
    width: 25%;
}
.content-right-login {
    text-align: center;
    width: 194px;
}
.content-right-gap {
    text-align: left;
    width: 50px;
}
.content-right-info {
    text-align: left;
    width: 25%;
}

#summary-column {
    column-span: 6;
    text-align: center;
}

#time-column {
    column-span: 6;
    text-align: right;
    font-size: <?php echo $TINY_FONT_SIZE; ?>;
    color: #3f3f3f;
}
