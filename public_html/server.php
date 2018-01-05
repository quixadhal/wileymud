<?php 
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

    $isLocal = is_local_ip();

    $graphics = array();
    $graphics['server_case'] = $isLocal ? "gfx/server_case.jpg" : "https://lh6.googleusercontent.com/-w6XwIBerDjw/UdooiSE-NUI/AAAAAAAAAPI/wGjTt7QiEmA/s800/server_case.jpg";
    //$graphics['server_case'] = $isLocal ? "gfx/server_case.jpg" : "http://i302.photobucket.com/albums/nn96/quixadhal/shadowlord/server_case_zpsdcdc0b79.jpg";
    $graphics['speedtest_raw'] = $isLocal ? "gfx/speedtest_raw_4478672602.png" : "http://www.speedtest.net/result/4478672602.png";
    $graphics['speedtest_qos'] = $isLocal ? "gfx/speedtest_qos_4478667111.png" : "http://www.speedtest.net/result/4478667111.png";
    $graphics['speedtest_new'] = $isLocal ? "gfx/wave_g_test2.png" : "gfx/wave_g_test2.png";
?>

<html>
    <head>
        <title>
            lenin inside quixadhal.ddns.net
        </title>
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
        <table style="table-layout:fixed; width:100%; border:0px; white-space:normal;  word-wrap:break-word;">
            <tr>
                <td width="75%" align="left">
                    <h1>lenin inside quixadhal.ddns.net</h1>
                    <hr />
                    <h3>Hardware purchased:</h3>
 54.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16813157335">ASRock B75M-DGS LGA 1155 Intel B75 Micro ATX Intel Motherboard</a><br />
 49.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16811147153">Rosewill CHALLENGER Black Gaming ATX Mid Tower Case</a><br />
 36.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16817139026">Corsair CX430 ATX Active PFC Power Supply</a><br />
 49.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16820231489">G.Skill Ripjaw X Series 16GB (2 x 8GB) DDR3 SDRAM (PC3 12800)</a><br />
 15.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16827106289">Lite-On SATA DVD Burner</a> (A burner was cheaper than a non-burner!)<br />
 64.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16819116406">Intel Pentium G630 Sandy Bridge 2.7GHz Dual Core CPU (LGA 1155)</a><br />
                    <br />
                    <h3>Total purchase price (with discounts): $269.71 on November 21, 2012</h3>
                    <hr />
                    <h3>Hardware recycled:</h3>
 74.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16822236070">WD Cavier Green 1TB SATA hard drive</a><br />
119.99                   <a href="http://www.newegg.com/Product/Product.aspx?Item=N82E16822136749">WD My Book 3TB Desktop USB 3.0 External Hard Drive</a> (now internal)<br />
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
                        NEW:&nbsp;<img src="<?php echo $graphics['speedtest_new']; ?>" border="0" width="300" height="135" />
                        Raw:&nbsp;<img src="<?php echo $graphics['speedtest_raw']; ?>" border="0" width="300" height="135" />
                        QoS:&nbsp;<img src="<?php echo $graphics['speedtest_qos']; ?>" border="0" width="300" height="135" />
                    </center>
                </td>
            </tr>
            <tr>
                <td colspan="2" align="left">
                    <hr />
                    <h3>Uptime:</h3>
                    <pre><?php pcmd("/bin/cat /proc/version"); ?></pre>
                    <pre><?php pcmd("/usr/bin/uptime"); ?></pre>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('mem');">Memory Information:</a></h3>
                    <div id="mem" style="display: none;">
                        <pre><?php pcmd("/usr/bin/free"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('disk');">Disk Information:</a></h3>
                    <div id="disk" style="display: none;">
                        <pre><?php pcmd("/bin/dmesg | /bin/grep 'WDC' | /bin/grep 'ata'"); ?></pre>
                        <pre><?php pcmd("/bin/df | /bin/grep -v 'udev' | /bin/grep -v 'tmpfs' | /bin/grep -v 'by-uuid'"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('temp');">Temperature Sensor Output:</a></h3>
                    <div id="temp" style="display: none;">
                        <pre><?php pcmd("/usr/bin/sensors"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('cpuinfo');">CPU Information:</a></h3>
                    <div id="cpuinfo" style="display: none;">
                        <pre><?php pcmd("/bin/cat /proc/cpuinfo"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('hacklog');">HACKLOG:</a></h3>
                    <div id="hacklog" style="display: none;">
                        <pre><?php pcmd("/bin/cat /home/quixadhal/HACKLOG/HACKLOG.lenin"); ?></pre>
                    </div>
                    <hr />
                    <h3><a href="javascript:;" onmousedown="toggleDiv('source');">Page Source:</a></h3>
                    <div id="source" style="display: none;">
                        <?php echo numbered_source(__FILE__); ?>
                    </div>
                </td>
            </tr>
        </table>
    </body>
</html>
