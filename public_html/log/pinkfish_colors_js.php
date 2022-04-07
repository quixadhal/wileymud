<?php
require_once 'site_global.php';
require_once 'pinkfish_colors.php';
header("Content-Type: application/json; charset=UTF-8");

echo "var PinkfishMap = {\n";
foreach ($pinkfish_map as $pf) {
    echo "'" . $pf['pinkfish'] . "' : '" . $pf['html'] . "',\n";
}
echo "};\n";
?>
function handle_colors(s) {
    var result = s;
    for([k,v] of Object.entries(PinkfishMap)) {
        result = result.split(k).join(v);
    }
    //result = result.replace('%^RESET%^', '');
    return result;
}
