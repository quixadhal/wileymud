<?php 
$time_start = microtime(true);

function pcmd($command) {
    $data = "";
    $fp = popen("$command", "r");
    do {
        $data .= fread($fp, 8192);
    } while(!feof($fp));
    pclose($fp);
    echo $data;
}

function is_local_ip() {
    $visitor_ip = $_SERVER['REMOTE_ADDR'];
    $varr = explode(".", $visitor_ip);
    if($varr[0] == "192" && $varr[1] == "168")
        return 1;
    return 0;
}

function numbered_source($filename)
{
    ini_set('highlight.string',  '#DD0000'); // DD0000
    ini_set('highlight.comment', '#0000BB'); // FF8000
    ini_set('highlight.keyword', '#00CC00'); // 007700
    ini_set('highlight.bg',      '#111111'); // FFFFFF
    ini_set('highlight.default', '#00DDDD'); // 0000BB
    ini_set('highlight.html',    '#CCCCCC'); // 000000
    $lines = implode(range(1, count(file($filename))), '<br />');
    $content = highlight_file($filename, true);
    $style = '
    <style type="text/css"> 
        .num { 
        float: left; 
        color: gray; 
        background-color: #111111;
        font-size: 13px;    
        font-family: monospace; 
        text-align: right; 
        margin-right: 6pt; 
        padding-right: 6pt; 
        border-right: 1px solid gray;} 

        body {margin: 0px; margin-left: 5px;} 
        td {vertical-align: top; white-space: normal;} 
        code {white-space: nowrap;} 
    </style>
    '; 
    return "$style\n<div style=\"background-color: black;\"><table><tr><td class=\"num\">\n$lines\n</td><td>\n$content\n</td></tr></table></div>"; 
}

function random_image($dir) {
    $old_dir = getcwd();
    chdir($dir);

    $jpg_list = glob("*.jpg");
    $png_list = glob("*.png");
    $file_list = array_merge($jpg_list, $png_list);
    $pick = array_rand($file_list);

    chdir($old_dir);
    return $file_list[$pick];
}

$isLocal = is_local_ip();

$graphics = array();
$graphics['server_case'] = $isLocal ? "gfx/server_case.png" : "https://i.imgur.com/TFmF5Yg.png";
//$graphics['server_case'] = $isLocal ? "gfx/server_case.png" : "https://lh6.googleusercontent.com/-w6XwIBerDjw/UdooiSE-NUI/AAAAAAAAAPI/wGjTt7QiEmA/s800/server_case.jpg";
//$graphics['server_case'] = $isLocal ? "gfx/server_case.jpg" : "http://i302.photobucket.com/albums/nn96/quixadhal/shadowlord/server_case_zpsdcdc0b79.jpg";
$graphics['speedtest_kalamazoo'] = $isLocal ? "gfx/speedtest_kalamazoo_4478672602.png" : "http://www.speedtest.net/result/4478672602.png";
$graphics['speedtest_seattle'] = $isLocal ? "gfx/speedtest_seattle_10284037752.png" : "https://www.speedtest.net/result/10284037752.png";
$graphics['speedtest_bellevue'] = $isLocal ? "gfx/speedtest_bellevue_10600322876.png" : "https://www.speedtest.net/result/10600322876.png";
$graphics['speedtest_current'] = $isLocal ? "gfx/speedtest_bellevue_10600322876.png" : "https://www.speedtest.net/result/10600322876.png";
$graphics['speedtest_wifi'] = $isLocal ? "gfx/speedtest_bellevue_10600322876.png" : "https://www.speedtest.net/result/10600322876.png";

$SPEEDTEST_FILE         = "/home/wiley/public_html/speedtest.json";
$SPEEDTEST_WIFI_FILE    = "/home/wiley/public_html/speedtest_wifi.json";
$MUDLIST_FILE           = "/home/wiley/public_html/mudlist.json";
$BACKGROUND_DIR         = "/home/wiley/public_html/gfx/wallpaper/";

$speedtest_text = file_get_contents($SPEEDTEST_FILE);
$speedtest = json_decode($speedtest_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
if( json_last_error() != JSON_ERROR_NONE ) {
    echo "<hr>".json_last_error_msg()."<br><hr>";
}
$graphics['speedtest_current'] = $speedtest["result"]["url"] . ".png";

$speedtest["unix_timestamp"] = strtotime($speedtest["timestamp"]);
$speedtest["the_time"] = strftime("%Y-%m-%d %H:%M:%S %Z", $speedtest["unix_timestamp"]);

$speedtest_wifi_text = file_get_contents($SPEEDTEST_WIFI_FILE);
$speedtest_wifi = json_decode($speedtest_wifi_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
if( json_last_error() != JSON_ERROR_NONE ) {
    echo "<hr>".json_last_error_msg()."<br><hr>";
}
$graphics['speedtest_wifi'] = $speedtest_wifi["result"]["url"] . ".png";

$speedtest_wifi["unix_timestamp"] = strtotime($speedtest_wifi["timestamp"]);
$speedtest_wifi["the_time"] = strftime("%Y-%m-%d %H:%M:%S %Z", $speedtest_wifi["unix_timestamp"]);

$mudlist_text = file_get_contents($MUDLIST_FILE);
$mudlist = json_decode($mudlist_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
if( json_last_error() != JSON_ERROR_NONE ) {
    echo "<hr>".json_last_error_msg()."<br><hr>";
}
$WILEY_BUILD_NUMBER     = $mudlist["version"]["build"];
$WILEY_BUILD_DATE       = $mudlist["version"]["date"];
$WILEY_TIME             = $mudlist["time"];

$GFX_HOME       = $isLocal ? "gfx" : "http://wileymud.themud.org/~wiley/gfx";
$BACKGROUND     = random_image($BACKGROUND_DIR);
$BACKGROUND_IMG = "<img class=\"overlay-bg\" src=\"$GFX_HOME/wallpaper/$BACKGROUND\" />";

?>

<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <meta http-equiv="refresh" content="1800" />
        <!-- Global site tag (gtag.js) - Google Analytics -->
        <script async src="https://www.googletagmanager.com/gtag/js?id=UA-163395867-1"></script>
        <script>
            window.dataLayer = window.dataLayer || [];
            function gtag(){dataLayer.push(arguments);}
            gtag('js', new Date());
            gtag('config', 'UA-163395867-1');
        </script>
        <script language="javascript">
            function toggleDiv(divID) {
                if(document.getElementById(divID).style.display == 'none') {
                    document.getElementById(divID).style.display = 'block';
                } else {
                    document.getElementById(divID).style.display = 'none';
                }
            }
        </script>
        <style>
            html, body { table-layout: fixed; max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis;}
            table { table-layout: fixed; max-width: 99%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
            a:active, a:focus { outline: 0; border: none; -moz-outline-style: none; }
            input, select, textarea { border-color: #101010; background-color: #101010; color: #d0d0d0; }
            input:focus, textarea:focus { border-color: #101010; background-color: #303030; color: #f0f0f0; }
            #xnavbar { position: fixed; top: 0; background-color: black; opacity: 1.0; z-index: 2; }
            #navbar { position: fixed; top: 0; height: 58px; background-color: black; }
            #xcontent-header { padding-top: 48px; }
            #content-header { position: fixed; top: 58px; width: 100%; background-color: black; }
            #content { padding-top: 48px; }
            .overlay-fixed { position: fixed; top: 48px; left: 0px; width: 100%; height: 100%; z-index: 999; opacity: 0.3; pointer-events: none; }
            .overlay-bg { position: fixed; top: 58px; z-index: 998; opacity: 0.15; pointer-events: none; object-fit: cover; width: 100%; height: 100%; left: 50%; transform: translateX(-50%); }
            .unblurred { font-family: monospace; white-space: pre-wrap; }
            .blurry:not(:hover) { filter: blur(3px); font-family: monospace; white-space: pre-wrap; }
            .blurry:hover { font-family: monospace; white-space: pre-wrap; }
            .glowing:not(:hover) { filter: brightness(1); }
            .glowing:hover { filter: brightness(1.75); }
        </style>
        <title>Old Crusty MUD Server</title>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040">
        <?php echo $BACKGROUND_IMG; ?>
        <table id="navbar" width="99%" cellspacing="0" cellpadding="1" align="center">
            <tr>
                <td align="left" width="25%">
                    <a href="http://wileymud.themud.org/~wiley/mudlist.php" alt="Mudlist" title="Mudlist">
                        <img class="glowing" src="<?php echo $GFX_HOME; ?>/mud.png" width="48" height="48" border="0" />
                    </a>
                    <a href="http://wileymud.themud.org/~wiley/logpages" alt="Logs" title="Logs">
                        <img class="glowing" src="<?php echo $GFX_HOME; ?>/log.png" width="48" height="48" border="0" />
                    </a>
                    <a href="http://wileymud.themud.org/~wiley/pie.php" alt="PIE!" title="PIE!">
                        <img class="glowing" src="<?php echo $GFX_HOME; ?>/pie_chart.png" width="48" height="48" border="0" />
                    </a>
                </td>
                <td align="right" width="24%">
                    <a href="telnet://wileymud.themud.org:3000" alt="WileyMUD" title="WileyMUD">
                        <img class="glowing" src="<?php echo $GFX_HOME; ?>/wileymud3.png" width="247" height="48" border="0" />
                    </a>
                </td>
                <td align="center" width="">
                    &nbsp;
                </td>
                <td valign="top" align="left" width="24%">
                        <table border="0" cellspacing="0" cellpadding="0">
                            <tr>
                                <td align="right">
                                    <span style="color: #bb0000"> Version: </span>
                                </td>
                                <td align="center" width="10">
                                    &nbsp;
                                </td>
                                <td align="left">
                                    <span style="color: #bb0000"> <?php echo $WILEY_BUILD_NUMBER; ?> </span>
                                </td>
                            </tr>
                            <tr>
                                <td align="right">
                                    <span style="color: #bb0000"> Build Date: </span>
                                </td>
                                <td align="center" width="10">
                                    &nbsp;
                                </td>
                                <td align="left">
                                    <span style="color: #bb0000"> <?php echo $WILEY_BUILD_DATE; ?> </span>
                                </td>
                            </tr>
                        </table>
                </td>
                <td align="right" width="25%">
                    <span style="vertical-align: top; color: #ccaa00"> <?php echo $WILEY_TIME; ?> </span>
                        <img src="<?php echo $GFX_HOME; ?>/server_icon.png" width="48" height="48" border="0" style="opacity: 0.2; background: rgba(255,0,0,0.25);" />
                    <a href="https://github.com/quixadhal/wileymud" alt="Source" title="Source">
                        <img class="glowing" src="<?php echo $GFX_HOME; ?>/github1600.png" width="48" height="48" border="0" />
                    </a>
                    <a href="https://discord.gg/kUduSsJ" alt="Discord" title="Discord">
                        <img class="glowing" src="<?php echo $GFX_HOME; ?>/discord.png" width="48" height="48" border="0" />
                    </a>
                </td>
            </tr>
        </table>

    <div id="content" align="center">
        <table id="content" width="99%" align="center">
            <tr>
                <td width="75%" align="left">
                    <h1>Old Crusty MUD Server</h1>
                    <hr />
                    <h3>Hardware purchased:</h3>
                    <table id="prices" width="100%" align="left">
                        <tr>
                            <td width="100" align="right">54.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16813157335">ASRock B75M-DGS LGA 1155 Intel B75 Micro ATX Intel Motherboard</a></td>
                            <td width="180" align="left">2012-11-21</td>
                        </tr>
                        <tr>
                            <td width="100" align="right">49.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16811147153">Rosewill CHALLENGER Black Gaming ATX Mid Tower Case</a></td>
                            <td width="180" align="left">2012-11-21</td>
                        </tr>
                        <tr>
                            <td width="100" align="right">36.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16817139026">Corsair CX430 ATX Active PFC Power Supply</a></td>
                            <td width="180" align="left">2012-11-21</td>
                        </tr>
                        <tr>
                            <td width="100" align="right">49.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16820231489">G.Skill Ripjaw X Series 16GB (2 x 8GB) DDR3 SDRAM (PC3 12800)</a></td>
                            <td width="180" align="left">2012-11-21</td>
                        </tr>
                        <tr>
                            <td width="100" align="right">15.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16827106289">Lite-On SATA DVD Burner</a> (A burner was cheaper than a non-burner!)</td>
                            <td width="180" align="left">2012-11-21</td>
                        </tr>
                        <tr>
                            <td width="100" align="right">64.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16819116406">Intel Pentium G630 Sandy Bridge 2.7GHz Dual Core CPU (LGA 1155)</a></td>
                            <td width="180" align="left">2012-11-21</td>
                        </tr>
                        <tr>
                            <td width="100" align="right"><s>74.99</s></td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16822236070">WD Cavier Green 1TB SATA hard drive</a> RECYCLED </td>
                            <td width="180" align="left">2012-11-21 RECYCLED</td>
                        </tr>
                        <tr>
                            <td width="100" align="right"><s>119.99</s></td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16822136749">WD My Book 3TB Desktop USB 3.0 External Hard Drive</a> (now internal) RECYCLED </td>
                            <td width="180" align="left">2015-06-11 RECYCLED</td>
                        </tr>
                        <tr>
                            <td width="100" align="right"><s>147.99</s></td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="https://www.amazon.com/gp/product/B00IRRDHUI/">Crucial M550 256GB SATA 2.5" 7mm (with 9.5mm adapter) Internal Solid State Drive</a> RECYCLED </td>
                            <td width="180" align="left">2018-01-17 RECYCLED</td>
                        </tr>
                        <tr>
                            <td width="100" align="right">34.99</td>
                            <td width="10"> &nbsp; </td>
                            <td align="left"><a href="https://www.amazon.com/gp/product/B016K0896K/">TP-Link AC1300 PCIe WiFi PCIe Card(Archer T6E)- 2.4G/5G Dual Band Wireless PCI Express Adapter</a></td>
                            <td width="180" align="left">2020-12-24 NEW!</td>
                        </tr>
                    </table>
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <h3>Total purchase price: $304.70 as of December 24, 2020</h3>
                    <hr />
                </td>
                <td align="left" >
                    <img src="<?php echo $graphics['server_case']; ?>" border="0" width="300" height="304" />
                </td>
            </tr>
            <tr>
                <td colspan="2" align="left">
                    <hr />
                    <h3>Internet Performance:</h3>
                    <center>
                        <table border="0" cellspacing="0" cellpadding="5">
                            <tr>
                                <td align="center"> <img src="<?php echo $graphics['speedtest_current']; ?>" border="0" width="300" height="135" /> </td>
                                <td align="center"> <img src="<?php echo $graphics['speedtest_wifi']; ?>" border="0" width="300" height="135" /> </td>
                                <td align="center"> <img src="<?php echo $graphics['speedtest_kalamazoo']; ?>" border="0" width="300" height="135" /> </td>
                            </tr>
                            <tr>
                                <td align="center"> Bellevue &nbsp;(wired) </td>
                                <td align="center"> Bellevue &nbsp;(wifi) </td>
                                <td align="center"> Kalamazoo </td>
                            </tr>
                        </table>
                    </center>
                </td>
            </tr>
            <tr>
                <td colspan="2" align="left">
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('uptime');">Uptime:</a></h3>
                    <div id="uptime" style="display: none;">
                    <pre><?php pcmd("/bin/cat /proc/version"); ?></pre>
                    <pre><?php pcmd("/usr/bin/uptime"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('network');">Network Information:</a></h3>
                    <div id="network" style="display: none;">
                        <pre>Blacklist entries: <?php pcmd("/usr/bin/wc -l /etc/iptables/ipset.blacklist"); ?></pre>
                        <pre><?php pcmd("/usr/bin/nmcli -f 'DEVICE,CHAN,BARS,SIGNAL,RATE,SSID' dev wifi | /usr/bin/egrep '(\s+SSID|\s+Dread_.748)'"); ?></pre>
                        <pre>Wifi Connection in use: <?php pcmd("/sbin/iwconfig wlp1s0 | grep ESSID"); ?></pre>
                        <pre>
Wired Speedtest performed on  <?php printf("%s\n", $speedtest["the_time"]); ?>
                interface     <?php printf("%s:%s\n", $speedtest["interface"]["internalIp"], $speedtest["interface"]["name"]); ?>
                target node   <?php printf("%s (%s)\n", $speedtest["server"]["name"], $speedtest["server"]["host"]); ?>
                ping          <?php printf("%-.3f ms\n", $speedtest["ping"]["latency"]); ?>
                download      <?php printf("%-.2f Mbps\n", ($speedtest["download"]["bandwidth"] * 8.0 / 1000000.0)); ?>
                upload        <?php printf("%-.2f Mbps\n", ($speedtest["upload"]["bandwidth"] * 8.0 / 1000000.0)); ?>
                        </pre>
                        <pre>
Wi-fi Speedtest performed on  <?php printf("%s\n", $speedtest_wifi["the_time"]); ?>
                interface     <?php printf("%s:%s\n", $speedtest_wifi["interface"]["internalIp"], $speedtest_wifi["interface"]["name"]); ?>
                target node   <?php printf("%s (%s)\n", $speedtest_wifi["server"]["name"], $speedtest_wifi["server"]["host"]); ?>
                ping          <?php printf("%-.3f ms\n", $speedtest_wifi["ping"]["latency"]); ?>
                download      <?php printf("%-.2f Mbps\n", ($speedtest_wifi["download"]["bandwidth"] * 8.0 / 1000000.0)); ?>
                upload        <?php printf("%-.2f Mbps\n", ($speedtest_wifi["upload"]["bandwidth"] * 8.0 / 1000000.0)); ?>
                        </pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('mem');">Memory Information:</a></h3>
                    <div id="mem" style="display: none;">
                        <pre><?php pcmd("/usr/bin/free --mega -h"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('cpuinfo');">CPU Information:</a></h3>
                    <div id="cpuinfo" style="display: none;">
                        <pre><?php pcmd("/bin/cat /proc/cpuinfo"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('disk');">Disk Information:</a></h3>
                    <div id="disk" style="display: none;">
                        <pre><?php pcmd("/bin/dmesg | /bin/grep 'WDC' | /bin/grep 'ata'"); ?></pre>
                        <pre><?php pcmd("/bin/df -h | /bin/grep -v 'udev' | /bin/grep -v 'tmpfs' | /bin/grep -v 'by-uuid'"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('temp');">Temperature Sensor Output:</a></h3>
                    <div id="temp" style="display: none;">
                        <pre><?php pcmd("/usr/bin/sensors"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('hacklog');">HACKLOG:</a></h3>
                    <div id="hacklog" style="display: none;">
                        <pre><?php pcmd("/bin/cat /home/wiley/HACKLOG"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('source');">Page Source:</a></h3>
                    <div id="source" style="display: none;">
                        <?php echo numbered_source(__FILE__); ?>
                    </div>
                </td>
            </tr>
        </table>
        </div>
    </body>
</html>
