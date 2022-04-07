<?php
require_once 'site_global.php';

// This function also uses the CSS classes,
// .source-line-number and .source-code.
function numbered_source($filename)
{
    ini_set('highlight.string',  '#DD0000'); // DD0000
    ini_set('highlight.comment', '#0000BB'); // FF8000
    ini_set('highlight.keyword', '#00CC00'); // 007700
    ini_set('highlight.bg',      '#111111'); // FFFFFF
    ini_set('highlight.default', '#00DDDD'); // 0000BB
    ini_set('highlight.html',    '#CCCCCC'); // 000000
    $lines = implode('<br />', range(1, count(file($filename))));
    $content = highlight_file($filename, true);
    return "<table><tr><td class=\"source-line-number\">\n$lines\n</td><td class=\"source-code\">\n$content\n</td></tr></table>"; 
}

$PAGE_SOURCE_FILE       = "$FILE_HOME/page_source_css.php";
$PAGE_SOURCE_TIME       = filemtime($PAGE_SOURCE_FILE);
$PAGE_SOURCE_CSS        = "$URL_HOME/page_source_css.php?version=$PAGE_SOURCE_TIME";
?>
