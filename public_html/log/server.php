<?php 
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'page_source.php';
require_once 'random_background.php';
require_once 'navbar.php';

$SERVER_CASE            = "$URL_HOME/gfx/server_case.png";
$SERVER_GUTS            = "$URL_HOME/gfx/server_guts.jpg";

$SPEEDTEST_FILE         = "$FILE_HOME/speedtest.json";
$SPEEDTEST_WIFI_FILE    = "$FILE_HOME/speedtest_wifi.json";
$SPEEDTEST_AVG_FILE     = "$FILE_HOME/speedtest_avg.json";

$speedtest_text = file_get_contents($SPEEDTEST_FILE);
$speedtest = json_decode($speedtest_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
$speedtest["unix_timestamp"] = strtotime($speedtest["timestamp"]);
$speedtest["the_time"] = strftime("%Y-%m-%d %H:%M:%S %Z", $speedtest["unix_timestamp"]);
$speedtest["speedtest_current"] = $speedtest["result"]["url"] . ".png";

$speedtest_wifi_text = file_get_contents($SPEEDTEST_WIFI_FILE);
$speedtest_wifi = json_decode($speedtest_wifi_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
$speedtest_wifi["unix_timestamp"] = strtotime($speedtest_wifi["timestamp"]);
$speedtest_wifi["the_time"] = strftime("%Y-%m-%d %H:%M:%S %Z", $speedtest_wifi["unix_timestamp"]);
$speedtest_wifi["speedtest_wifi"] = $speedtest_wifi["result"]["url"] . ".png";

$speedtest_avg_text = file_get_contents($SPEEDTEST_AVG_FILE);
$speedtest_avg = json_decode($speedtest_avg_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);

?>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <link rel="stylesheet" href="<?php echo $SITE_GLOBAL_CSS;?>">
        <link rel="stylesheet" href="<?php echo $PAGE_SOURCE_CSS;?>">
        <link rel="stylesheet" href="<?php echo $BACKGROUND_CSS;?>">
        <link rel="stylesheet" href="<?php echo $NAVBAR_CSS;?>">
        <style>
            #greeting {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                width: 100%;
                max-width: 100%;
                min-width: 100%;
                background-color: <?php echo $ODD; ?>;
                text-align: left;
                align: left;
            }
            #content-table {
                font-family: Consolas, "Lucida Console", Monaco, Courier, monospace;
                font-size: <?php echo $FONT_SIZE; ?>;
                overflow-x: hidden;
                min-width: 100%;
                width: 100%;
            }
            #content-table tr:nth-child(even) {
                background-color: <?php echo $EVEN; ?>
            }
            #content-table tr:nth-child(odd) {
                background-color: <?php echo $ODD; ?>
            }
            .content-price-column {
                text-align: right;
                width: 8ch; 
                max-width: 8ch;
                min-width: 8ch;
                padding-right: 2ch;
            }
            .content-message-column {
                text-align: left;
                white-space: nowrap;
                overflow-x: hidden;
                padding-left: 1ch;
                padding-right: 1ch;
            }
            .content-date-column {
                text-align: left;
                width: 11ch; 
                max-width: 11ch;
                min-width: 11ch;
                padding-left: 2ch;
            }
            .content-status-column {
                text-align: left;
                width: 9ch; 
                max-width: 9ch;
                min-width: 9ch;
                padding-left: 2ch;
            }
            .content-image-column {
                text-align: center;
                vertical-align: top;
                width: 25%; 
                max-width: 25%;
                min-width: 25%;
            }

            #server-case {
                display: block;
                text-align: center;
                align: center;
                vertical-align: center;
            }
            #server-case img {
                border: none;
                max-width: 300px;
                max-height: 300px;
                overflow: hidden;
                object-fit: cover;
            }
            #server-guts {
                display: none;
                text-align: center;
                align: center;
                vertical-align: center;
            }
            #server-guts img {
                border: none;
                max-width: 300px;
                max-height: 300px;
                overflow: hidden;
                object-fit: cover;
            }
        </style>

        <script src="<?php echo $JQ;?>""></script>
        <script src="<?php echo $JSCOOKIE;?>""></script>
        <script src="<?php echo $JSRANDOM;?>""></script>
        <script src="<?php echo $JSMD5;?>""></script>
        <script src="<?php echo $MOMENT;?>""></script>
        <script src="<?php echo $MOMENT_TZ;?>""></script>
        <script src="<?php echo $SITE_GLOBAL_JS;?>""></script>
        <script src="<?php echo $BACKGROUND_JS;?>""></script>
        <script src="<?php echo $NAVBAR_JS;?>""></script>

        <script language="javascript">
            var backgroundTimer;
            $(document).ready(function() {
                hideDiv('page-source');
                //showDiv('page-load-time');
                dim(document.getElementById('navbar-button-server'));
                randomizeBackground();
                updateRefreshTime();
                backgroundTimer = setInterval(randomizeBackground, 1000 * 60 * 5);
                // clearInterval(backgroundTimer);
            });
        </script>
    </head>
    <body bgcolor="<?php echo $BGCOLOR; ?>" text="<?php echo $TEXT; ?>" link="<?php echo $UNVISITED; ?>" vlink="<?php echo $VISITED; ?>">
        <div id="background-div">
            <img id="background-img" src="<?php echo $BACKGROUND_URL; ?>" />
        </div>
        <div id="back-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
        <div id="navbar-left">
            <img class="nav-img glowing" id="navbar-button-mudlist" title="List of MUDs" src="<?php echo $MUDLIST_ICON; ?>" onclick="window.location.href='<?php echo $MUDLIST_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-themudorg" title="I3 Log Page" src="<?php echo $LOG_ICON; ?>" onclick="window.location.href='<?php echo $LOG_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" onclick="window.location.href='<?php echo $PIE_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-question" title="???!" src="<?php echo $QUESTION_ICON; ?>" onclick="window.location.href='<?php echo $QUESTION_URL; ?>';" />
        </div>
        <div id="navbar-center">
            <table id="wileymud-table">
                <tr>
                    <td rowspan="2" align="right" width="<?php echo $WILEY_BANNER_WIDTH; ?>">
                        <img class="nav-banner-img glowing" id="navbar-button-wileymud" width="<?php echo $WILEY_BANNER_WIDTH; ?>" title="WileyMUD" src="<?php echo $WILEY_BANNER_ICON; ?>" />
                    </td>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-version" align="right">
                        Version:
                    </td>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-version" align="left">
                        <?php echo $WILEY_BUILD_NUMBER; ?>
                    </td>
                </tr>
                <tr>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-build-date" align="right">
                        Build Date:
                    </td>
                    <td class="wileymud-gap">&nbsp;</td>
                    <td class="wileymud-build-date" align="left">
                        <?php echo $WILEY_BUILD_DATE; ?>
                    </td>
                </tr>
            </table>
        </div>
        <div id="navbar-right">
            <span id="refresh-time">--:-- ---</span>
            <img class="nav-img" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" />
            <img class="nav-img glowing" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='<?php echo $GITHUB_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='<?php echo $DISCORD_URL; ?>';" />
        </div>
        <div id="fake-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>
        <div id="greeting">
            <h1>Old Crusty MUD Server</h1>
            <hr width="75%" align="left" />
            <h3>Hardware purchased:</h3>
        </div>
        <table id="content-table">
            <tr>
                <td class="content-price-column">54.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16813157335">ASRock B75M-DGS LGA 1155 Intel B75 Micro ATX Intel Motherboard</a></td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">&nbsp;</td>
                <td class="content-image-column" rowspan="0">
                    <div id="server-case">
                        <img onmousedown="hideDiv('server-case'); showDiv('server-guts');" src="<?php echo $SERVER_CASE; ?>" />
                    </div>
                    <div id="server-guts">
                        <img onmousedown="hideDiv('server-guts'); showDiv('server-case');" src="<?php echo $SERVER_GUTS; ?>" />
                    </div>
                </td>
            </tr>
            <tr>
                <td class="content-price-column">49.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16811147153">Rosewill CHALLENGER Black Gaming ATX Mid Tower Case</a></td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-price-column">36.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16817139026">Corsair CX430 ATX Active PFC Power Supply</a></td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-price-column">49.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16820231489">G.Skill Ripjaw X Series 16GB (2 x 8GB) DDR3 SDRAM (PC3 12800)</a></td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-price-column">15.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16827106289">Lite-On SATA DVD Burner</a> (A burner was cheaper than a non-burner!)</td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-price-column">64.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16819116406">Intel Pentium G630 Sandy Bridge 2.7GHz Dual Core CPU (LGA 1155)</a></td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-price-column">74.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16822236070">WD Cavier Green 1TB SATA hard drive</a></td>
                <td class="content-date-column">2012-11-21</td>
                <td class="content-status-column">RECYCLED</td>
            </tr>
            <tr>
                <td class="content-price-column">119.99</td>
                <td class="content-message-column"><a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16822136749">WD My Book 3TB Desktop USB 3.0 External Hard Drive</a> (now internal)</td>
                <td class="content-date-column">2015-06-11</td>
                <td class="content-status-column">RECYCLED</td>
            </tr>
            <tr>
                <td class="content-price-column">147.99</td>
                <td class="content-message-column"><a href="https://www.amazon.com/gp/product/B00IRRDHUI/">Crucial M550 256GB SATA 2.5" 7mm (with 9.5mm adapter) Internal Solid State Drive</a></td>
                <td class="content-date-column">2018-01-17</td>
                <td class="content-status-column">RECYCLED</td>
            </tr>
            <tr>
                <td class="content-price-column">34.99</td>
                <td class="content-message-column"><a href="https://www.amazon.com/gp/product/B016K0896K/">TP-Link AC1300 PCIe WiFi PCIe Card(Archer T6E)- 2.4G/5G Dual Band Wireless PCI Express Adapter</a></td>
                <td class="content-date-column">2020-12-24</td>
                <td class="content-status-column">NEW!</td>
            </tr>
            <tr>
                <td class="content-price-column">----.--</td>
                <td class="content-message-column">&nbsp;</td>
                <td class="content-date-column">----------</td>
                <td class="content-status-column">&nbsp;</td>
            </tr>
            <tr>
                <td class="content-price-column">304.70</td>
                <td class="content-message-column">Total purchase price</td>
                <td class="content-date-column">2020-12-24</td>
                <td class="content-status-column">TOTAL</td>
            </tr>
        </table>

        <div id="page-source">
            <?php echo numbered_source(__FILE__); ?>
        </div>
    </body>
</html>
