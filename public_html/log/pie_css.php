<?php
require_once 'site_global.php';
header("Content-type: text/css");
?>
#button-table {
    font-family: 'Lato', sans-serif;
    font-size: <?php echo $FONT_SIZE; ?>;
    overflow-x: hidden;
    min-width: 80%;
    max-width: 80%;
    width: 80%;
    margin-left: 10%;
    margin-right: 10%;
}
.row-gap {
    background-color: #000000;
    opacity: 0.50;
    align: center;
    text-align: center;
}
.button-active {
    background-color: #8f4f2f;
    opacity: 1.0;
    align: center;
    text-align: center;
}
.button-inactive:not(:hover) {
    background-color: #2f0000;
    opacity: 0.6;
    align: center;
    text-align: center;
}
.button-inactive:hover {
    background-color: #4f0000;
    opacity: 1.0;
    align: center;
    text-align: center;
}
.button-chan-active {
    background-color: #2f4f8f;
    opacity: 1.0;
    align: center;
    text-align: center;
}
.button-chan-inactive:not(:hover) {
    background-color: #00002f;
    opacity: 0.6;
    align: center;
    text-align: center;
}
.button-chan-inactive:hover {
    background-color: #00004f;
    opacity: 1.0;
    align: center;
    text-align: center;
}
.graph-div {
    display: none;
    position: absolute;
    width: 80%;
    height: 80%;
    left: 50%;
    transform: translateX(-50%);
}
.pie-div {
    background: rgba(0,0,0,0.50);
    width: 100%;
    height: 90%;
}
.quote-div {
    font-family: 'Lato', sans-serif;
    font-size: <?php echo $FONT_SIZE; ?>;
    background: rgba(0,0,0,0.50);
    color: #FF9F9F;
    width: 100%;
    text-align: center;
}
.not-available {
    opacity: 0.5;
    width: 100%;
    height: 100%;
    object-fit: contain;
}
