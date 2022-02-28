<?php
require_once 'site_global.php';
header("Content-type: text/css");
?>
html, body {
    font-family: 'Lato', sans-serif;
    padding: 0px;
    margin: 0px;
}
body {
    overflow-x: hidden;
    overflow-y: scroll;
}
::-webkit-scrollbar {
    width: 15px;
}
::-webkit-scrollbar-track {
    background: #202020;
    box-shadow: inset 0 0 5px black; 
    border-radius: 7px;
}
::-webkit-scrollbar-thumb {
    background: #808080; 
    border-radius: 7px;
}
::-webkit-scrollbar-thumb:hover {
    background: #d0d080; 
}
table {
    table-layout: fixed;
    max-width: 100%;
    overflow-x: hidden;
    border: 0px;
    padding: 0px;
    border-spacing: 0px;
}
a {
    text-decoration:none;
    color: <?php echo $UNVISITED; ?>;
}
a:visited {
    color: <?php echo $UNVISITED; ?>;
}
a:hover {
    text-decoration:underline;
}
a:active, a:focus {
    outline: 0;
    border: none;
    -moz-outline-style: none;
}
.unblurred {
    white-space: pre-wrap;
}
.blurry:not(:hover) {
    filter: blur(5px);
    white-space: pre-wrap;
}
.blurry:hover {
    white-space: pre-wrap;
}
.glowing:not(:hover) {
    filter: brightness(1);
}
.glowing:hover {
    filter: brightness(1.75);
}
@keyframes blinking {
    0% {
        opacity: 0;
    }
    49% {
        opacity: 0;
    }
    50% {
        opacity: 1;
    }
    100% {
        opacity: 1;
    }
}
.flash_tag {
    animation: blinking 1.5s infinite;
}
@keyframes spinning {
    from {
        transform: rotate(360deg);
    }
    to {
        transform: rotate(0deg);
    }
}
.spinning {
    animation: spinning 240s infinite linear;
}
