<?php
$time_start = microtime(true);

$MUDLIST_FILE = "/home/wiley/public_html/mudlist.json";

$mudlist_text = file_get_contents($MUDLIST_FILE);
$mudlist = json_decode($mudlist_text, true);
if( json_last_error() != JSON_ERROR_NONE ) {
    echo "<hr>".json_last_error_msg()."<br>";
}
$WILEY_BUILD_NUMBER = $mudlist["version"]["build"];
$WILEY_BUILD_DATE = $mudlist["version"]["date"];
$WILEY_TIME = $mudlist["time"];
?>

<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <style>
            html, body { table-layout: fixed; max-width: 100%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis;}
            table { table-layout: fixed; max-width: 99%; overflow-x: hidden; word-wrap: break-word; text-overflow: ellipsis; }
            a { text-decoration:none; }
            a:hover { text-decoration:underline; }
            a:active, a:focus { outline: 0; border: none; -moz-outline-style: none; }
            #navbar { position: fixed; top: 0; background-color: black; opacity: 1.0; z-index: 2; }
            #content-header { padding-top: 48px; }
        </style>
        <title>Welcome to WileyMUD!</title>
    </head>
    <body bgcolor="black" text="#d0d0d0" link="#ffffbf" vlink="#ffa040">
    <table id="navbar" width="99%" cellspacing="0" cellpadding="1" align="center">
        <tr>
            <td align="left" width="25%">
                <a href="http://wileymud.themud.org/~wiley/logpages" alt="Logs" title="Logs">
                    <img src="http://wileymud.themud.org/~wiley/gfx/log.png" width="48" height="48" border="0" />
                </a>
                <a href="https://discord.gg/kUduSsJ" alt="Discord" title="Discord">
                    <img src="http://wileymud.themud.org/~wiley/gfx/discord.png" width="48" height="48" border="0" />
                </a>
            </td>
            <td align="right" width="24%">
                <a href="telnet://wileymud.themud.org:3000" alt="WileyMUD" title="WileyMUD">
                    <img src="http://wileymud.themud.org/~wiley/gfx/wileymud3.png" width="247" height="48" border="0" />
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
                <a href="http://wileymud.themud.org/~wiley/server.php" alt="Server" title="Server">
                    <img src="http://wileymud.themud.org/~wiley/gfx/server_icon.png" width="48" height="48" border="0" />
                </a>
            </td>
        </tr>
    </table>

    <div id="content-header" align="center">
        <table id="players" border="0" cellspacing="0" cellpadding="1" width="80%">
            <tr bgcolor="#2f0000">
                <th align="center" width="100">Idle</th>
                <th align="center" width="100">Rank</th>
                <th align="left" >Name</th>
            </tr>
            <?php
            $counter = 0;
            foreach ($mudlist["player_list"] as $player) {
                $bg_color = ($counter % 2) ? "#000000" : "#1F1F1F";

                $idle = $player["idle"];
                $seconds = $idle % 60;
                $idle = floor($idle / 60);
                $minutes = $idle % 60;
                $idle = floor($idle / 60);
                $hours = $idle % 24;
                $idle = floor($idle / 24);
                $days = $idle;
                if($days > 0) {
                    $idle = sprintf("%dd %d:%02d:%02d", $days, $hours, $minutes, $seconds);
                } else if( $hours > 0) {
                    $idle = sprintf("%d:%02d:%02d", $hours, $minutes, $seconds);
                } else if( $minutes > 0) {
                    $idle = sprintf("%d:%02d", $minutes, $seconds);
                } else {
                    $idle = sprintf("%d seconds", $seconds);
                }

                $pretitle = $player["pretitle"] ? $player["pretitle"] . " " : "";
                $title = $player["title"] ? " " . $player["title"] : "";
                $name = sprintf("%s%s%s", $pretitle, $player["name"], $title);
            ?>
            <tr bgcolor="<?php echo $bg_color; ?>">
                <td align="center"> <?php echo $idle; ?> </td>
                <td align="center"> <?php echo $player["rank"]; ?> </td>
                <td align="left"> <?php echo $name; ?> </td>
            </tr>
            <?php
                $counter++;
            }
            ?>
        </table>
        <br />
        <table id="status" border="0" cellspacing="0" cellpadding="1" width="80%">
            <tr bgcolor="#002f00">
                <th align="center" width="100">Players</th>
                <th align="center" width="100">Gods</th>
                <th align="left" >Boot Time</th>
            </tr>
            <tr bgcolor="#1f1f1f">
                <td align="center" ><?php echo $mudlist["players"]["mortals"]; ?></td>
                <td align="center" ><?php echo $mudlist["players"]["gods"]; ?></td>
                <td align="left" ><?php echo $mudlist["boot"]; ?></td>
            </tr>
        </table>
        <br />
    </div>

<div id="content" align="center">
    <table border="0" cellspacing="0" cellpadding="1" width="80%">
        <tr bgcolor="#00002f">
            <th align="center" width="194">Login Screen</th>
            <th align="left" width="50">&nbsp;</th>
            <th align="center" width="25%">Info</th>
            <th align="center" width="194">Login Screen</th>
            <th align="left" width="50">&nbsp;</th>
            <th align="center" width="25%">Info</th>
        </tr>
        <div class="gallery">
            <?php
            $counter = 0;
            $row_counter = 0;
            $image_counter = 0;
            $online_counter = 0;
            $verified_counter = 0;
            $total_muds = sizeof($mudlist["mudlist"]);
            $opacity = 1.0;
            foreach ($mudlist["mudlist"] as $mud) {
                if($mud["online"] == 0) {
                    $opacity = "opacity: 0.2; background: rgba(255,0,0,0.25);";
                } else {
                    $opacity = "opacity: 1.0;";
                    $online_counter++;
                }
                $bg_color = ($row_counter % 2) ? "#000000" : "#1F1F1F";
                if(!($counter % 2)) {
                    // Even rows means we open the TR
            ?>
            <tr bgcolor="<?php echo $bg_color; ?>">
            <?php
                }
                $fileurl = "gfx/mud/" . $mud["md5"] . ".png";
                $filename = "/home/wiley/public_html/" . $fileurl;
                if(!file_exists($filename)) {
                    // No image file, pick a random one!
                    $home = getcwd();
                    chdir("/home/wiley/public_html/gfx/mud");
                    $random_file = array_rand(array_flip(glob("__NOT_FOUND_*.png")));
                    chdir($home);
                    $fileurl = "gfx/mud/" . $random_file;
                    $filename = "/home/wiley/public_html/" . $fileurl;
                } else {
                    $image_counter++;
                    if($mud["online"] == 1) {
                        $verified_counter++;
                    }
                }
            ?>
                <td align="center" style="<?php echo $opacity; ?>">
                    <div class="gallery-item">
                        <a href="<?php echo $fileurl; ?>" data-lightbox>
                            <img border="0" width="192" height="120" src="<?php echo $fileurl; ?>" />
                        </a>
                    </div>
                </td>
                <td align="left" style="<?php echo $opacity; ?>">&nbsp;</td>
                <td align="left" style="<?php echo $opacity; ?>">
                    <a target="I3 mudlist" href="http://<?php echo $mud["ipaddress"]; ?>/">
                        <?php echo $mud["name"]; ?>
                    </a><br />
                    <?php echo $mud["type"]; ?><br />
                    <?php echo $mud["mudlib"]; ?><br />
                    <a href="telnet://<?php echo $mud["ipaddress"]; ?>:<?php echo $mud["port"]; ?>/">
                        <?php echo $mud["ipaddress"]; ?> <?php echo $mud["port"]; ?>
                    </a>
                </td>
            <?php
                if($counter % 2) {
                    // Odd rows means we close the TR
            ?>
            </tr>
            <?php
                    $row_counter++;
                }
                $counter++;
            }
            if($counter % 2) {
                // The counter is odd after running out of data, close the TR
            ?>
            </tr>
            <?php
            }
            ?>
        </div>
        <tr bgcolor="#00002f">
            <td align="center" colspan="6">
                <?php echo $total_muds; ?> total muds listed 
                (
                <?php echo $online_counter; ?> connected,
                <?php echo $verified_counter; ?> verified
                )
            </td>
        </tr>
        <tr bgcolor="#000000">
            <?php $time_end = microtime(true); $time_spent = $time_end - $time_start; ?>
            <td align="right" colspan="6"><font size="-1" color="#1f1f1f"><?php echo sprintf("%9.4f", $time_spent); ?> seconds</font></td>
        </tr>
    </table>
</div>
<link href="lightbox/lightbox.min.css" rel="stylesheet">
<script src="lightbox/lightbox.min.js"></script>
<script>
    const btn = document.querySelector('.trigger-lightbox');
    btn.addEventListener('click', () => new lightbox('#modal'));
</script>
</body>
</html>
