<?php
require_once 'site_global.php';

$PINKFISH_JS            = "$URL_HOME/pinkfish_colors_js.php";
$PINKFISH_CACHE         = "$FILE_HOME/data/pinkfish.json";
$pinkfish_map           = array();

function reverseSortByKeyLength($a, $b) {
    return strlen($b) - strlen($a);
}

function load_pinkfish_map($filename) {
    if( file_exists($filename) ) {
        $json_data = file_get_contents($filename);
        $data = json_decode($json_data, 1);
        uksort($data, "reverseSortByKeyLength");
        //print "\nPINKFISH_CACHE\n";
        //print_r($data);
        //print "\n";
        return $data;
    }
    print("No $filename exists.\n");
    return null;
}

function handle_colors( $pinkfish_map, $message ) {
    foreach ($pinkfish_map as $pf) {
        $pattern = '/'.preg_quote($pf['pinkfish']).'/';
        $repl = $pf['html'];
        $message = preg_replace($pattern, $repl, $message);
    }
    return $message;
}

// Load the current mapping into a variable we can use elsewhere.
$pinkfish_map           = load_pinkfish_map($PINKFISH_CACHE);

?>
