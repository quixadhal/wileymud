<?php

function numbered_source($filename)
{
    $lines = implode(range(1, count(file($filename))), '<br />');
    $content = highlight_file($filename, true);
    $style = '
    <style type="text/css"> 
        .num { 
        float: left; 
        color: gray; 
        font-size: 13px;    
        font-family: monospace; 
        text-align: right; 
        margin-right: 6pt; 
        padding-right: 6pt; 
        border-right: 1px solid gray;} 

        body {margin: 0px; margin-left: 5px;} 
        td {vertical-align: top;} 
        code {white-space: nowrap;} 
    </style>
    '; 
    return "$style\n<table><tr><td class=\"num\">\n$lines\n</td><td>\n$content\n</td></tr></table>"; 
}

function is_local_ip() {
    $visitor_ip = $_SERVER['REMOTE_ADDR'];
    $varr = explode(".", $visitor_ip);
    if($varr[0] == "192" && $varr[1] == "168")
        return 1;
    if($varr[0] == "127" && $varr[1] == "0")
        return 1;
    return 0;
}

$isLocal = is_local_ip();
//$isLocal = 0;

header('Cache-control: max-age=0, must-revalidate');

?>
<html>
    <head>
        <meta http-equiv="content-type" content="text/html; charset=UTF-8">
        <meta http-equiv="refresh" content="600">
        <title>WileyMUD III</title>
        <link rel="shortcut icon" href="gfx/fire.ico" />
        <link rev="made" href="mailto:quixadhal@gmail.com">
        <style>
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
        </style>
        <style>
           .menu {
              float:left;
              width:20%;
              height:80%;
            }
            .content {
              float:left;
              width:75%;
              height:80%;
            }
        </style>
        <script language="javascript">
            function toggleDiv(divID) {
                if(document.getElementById(divID).style.display == 'none') {
                    document.getElementById(divID).style.display = 'block';
                } else {
                    document.getElementById(divID).style.display = 'none';
                }
            }
        </script>
    </head>
    <body>
        <iframe name="_menu" class="menu" src="menu.php"></iframe>
        <iframe name="_content" class="content" src="content.php"></iframe>
    </body>
</html>
