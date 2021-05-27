<?php
require_once 'site_global.php';
require_once 'page_source.php';
header("Content-type: text/css");
?>
.source-line-number {
    float: left;
    color: gray;
    background-color: #111111;
    opacity: 0.90;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: 14px;
    text-align: right;
    margin: 0px;
    padding: 0px;
    border-spacing: 0px;
    margin-right: 0pt;
    padding-right: 6pt;
    border-spacing-right: 0px;
    border-right: 1px solid gray;
    vertical-align: top;
    white-space: normal;
}
.source-code {
    background-color: black;
    opacity: 0.70;
    width: 100%;
    font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
    font-size: 14px;
    margin: 0px;
    padding: 0px;
    border-spacing: 0px;
    margin-left: 0pt;
    padding-left: 6px;
    border-spacing-left: 6px;
    vertical-align: top;
    white-space: nowrap;
}
#page-source {
    display: none;
}
