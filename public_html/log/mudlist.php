<?php
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'page_source.php';
require_once 'random_background.php';
require_once 'navbar.php';

$MUDLIST_FILE       = "$FILE_HOME/mudlist_css.php";
$MUDLIST_TIME       = filemtime($MUDLIST_FILE);
$MUDLIST_CSS        = "$URL_HOME/mudlist_css.php?version=$MUDLIST_TIME";
?>

<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <meta http-equiv="refresh" content="1800" />
        <title>I3 MUD List</title>
        <!-- Global site tag (gtag.js) - Google Analytics -->
        <script async src="https://www.googletagmanager.com/gtag/js?id=UA-163395867-1"></script>
        <script>
          window.dataLayer = window.dataLayer || [];
          function gtag(){dataLayer.push(arguments);}
          gtag('js', new Date());

          gtag('config', 'UA-163395867-1');
        </script>
        <link rel="stylesheet" href="<?php echo $SITE_GLOBAL_CSS;?>">
        <link rel="stylesheet" href="<?php echo $PAGE_SOURCE_CSS;?>">
        <link rel="stylesheet" href="<?php echo $BACKGROUND_CSS;?>">
        <link rel="stylesheet" href="<?php echo $NAVBAR_CSS;?>">
        <link rel="stylesheet" href="<?php echo $MUDLIST_CSS;?>">

        <script src="<?php echo $JQ;?>"></script>
        <script src="<?php echo $JSCOOKIE;?>"></script>
        <script src="<?php echo $JSRANDOM;?>"></script>
        <script src="<?php echo $JSMD5;?>"></script>
        <script src="<?php echo $MOMENT;?>"></script>
        <script src="<?php echo $MOMENT_TZ;?>"></script>
        <script src="<?php echo $SITE_GLOBAL_JS;?>"></script>
        <script src="<?php echo $BACKGROUND_JS;?>"></script>
        <script src="<?php echo $NAVBAR_JS;?>"></script>
        <script language="javascript">
            var timeSpent;
            var backgroundTimer;

            function on_scroll() {
                var body = document.body;
                var html = document.documentElement;
                var bt = document.getElementById("navbar-button-top");
                var bb = document.getElementById("navbar-button-bottom");
                var doc_height = Math.max( body.scrollHeight, body.offsetHeight,
                    html.clientHeight, html.scrollHeight, html.offsetHeight );

                // This is used so that if we add more content and
                // nothing scrolls the page, we know we COULD
                // scroll further, if the user were at the bottom
                // and thus done reading it.
                WasAtBottom = false;
                if( window.innerHeight >= doc_height ) {
                    // The page fits entirely on the screen, no scrolling possible.
                    dim(bb);
                    dim(bt);
                } else if( (window.innerHeight + window.pageYOffset) >=
                    (document.body.offsetHeight) ) {
                    // We are at the bottom of the page.
                    dim(bb);
                    brighten(bt);
                    WasAtBottom = true;
                    NoScroll = false;
                } else if( window.pageYOffset <= 1 ) {
                    // We are at the top of the page.
                    brighten(bb);
                    dim(bt);
                } else {
                    // We're somewhere in the middle.
                    brighten(bb);
                    brighten(bt);
                }
            }
            $(document).ready(function() {
                hideDiv('page-source');
                $('#page-load-time').html(timeSpent);
                showDiv('page-load-time');
                dim(document.getElementById('navbar-button-mudlist'));
                syncBackgroundToggleIcon();
                randomizeBackground();
                updateRefreshTime();
                backgroundTimer = setInterval(randomizeBackground, 1000 * 60 * 5);
                on_scroll(); // Call once, in case the page cannot scroll
                $(window).on("scroll", function() {
                    on_scroll();
                });
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
            <img class="nav-img" id="navbar-button-mudlist" title="List of MUDs" src="<?php echo $MUDLIST_ICON; ?>" />
            <img class="nav-img glowing" id="navbar-button-themudorg" title="I3 Log Page" src="<?php echo $LOG_ICON; ?>" onclick="window.location.href='<?php echo $LOG_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" onclick="window.location.href='<?php echo $PIE_URL; ?>';" />
            <img class="nav-img glowing spinning" id="navbar-button-forum" title="Dead Forums" src="<?php echo $FORUM_ICON; ?>" onclick="window.location.href='<?php echo $FORUM_URL; ?>';" />
            <img class="nav-small-img glowing" id="navbar-button-background" title="Make boring." src="<?php echo $BG_ON_ICON; ?>" onclick="toggleBackground();" />
            <img class="nav-small-img glowing" id="navbar-button-question" title="???!" src="<?php echo $QUESTION_ICON; ?>" onclick="window.location.href='<?php echo $QUESTION_URL; ?>';" />
        </div>
        <div id="navbar-center">
            <table id="wileymud-table">
                <tr>
                    <td rowspan="2" align="right" width="<?php echo $WILEY_BANNER_WIDTH; ?>">
                        <img class="nav-banner-img glowing" id="navbar-button-wileymud" width="<?php echo $WILEY_BANNER_WIDTH; ?>" title="<?php echo "$WILEY_IP $WILEY_PORT"; ?>" src="<?php echo $WILEY_BANNER_ICON; ?>" />
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
            <img class="nav-img glowing" id="navbar-button-top" title="Top of page" src="<?php echo $TOP_ICON; ?>" onclick="scroll_to('player-header-table');" />
            <img class="nav-img glowing" id="navbar-button-bottom" title="Bottom of page" src="<?php echo $BOTTOM_ICON; ?>" onclick="scroll_to('content-bottom');" />
            <span id="refresh-time">--:-- ---</span>
            <img class="nav-img glowing" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" onclick="window.location.href='<?php echo $SERVER_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='<?php echo $GITHUB_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='<?php echo $DISCORD_URL; ?>';" />
        </div>
        <div id="fake-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>



        <table id="player-header-table">
            <tr id="player-header-row">
                <th class="player-table-idle">Idle</th>
                <th class="player-table-rank">Rank</th>
                <th class="player-table-name">Name</th>
            </tr>
        </table>
        <table id="player-table">
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
            <tr>
                <td class="player-table-idle"> <?php echo $idle; ?> </td>
                <td class="player-table-rank"> <?php echo $player["rank"]; ?> </td>
                <td class="player-table-name"> <?php echo $name; ?> </td>
            </tr>
            <?php
                $counter++;
            }
            ?>
            <tr><td colspan="3">&nbsp;</td></tr>
        </table>
        <table id="status-header-table">
            <tr id="status-header-row">
                <th class="status-table-players">Players</th>
                <th class="status-table-gods">Gods</th>
                <th class="status-table-uptime">Uptime</th>
            </tr>
        </table>
        <table id="status-table">
            <tr>
                <td class="status-table-players"><?php echo $mudlist["players"]["mortals"]; ?></td>
                <td class="status-table-gods"><?php echo $mudlist["players"]["gods"]; ?></td>
                <td class="status-table-uptime"><?php echo $mudlist["uptime"]; ?> since <?php echo $mudlist["boot"]; ?></td>
            </tr>
            <tr><td colspan="3">&nbsp;</td></tr>
        </table>



        <table id="content-header-table">
            <tr id="content-header-row">
                <th class="content-left-login">Login Screen</th>
                <th class="content-left-gap">&nbsp;</th>
                <th class="content-left-info">Info</th>
                <th class="content-right-login">Login Screen</th>
                <th class="content-right-gap">&nbsp;</th>
                <th class="content-right-info">Info</th>
            </tr>
        </table>
        <table id="content-table">
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
                        $opacity = "opacity: 0.25; background: rgba(255,0,0,0.25);";
                    } else {
                        $opacity = "opacity: 1.0;";
                        $online_counter++;
                    }
                    if(!($counter % 2)) {
                        // Even rows means we open the TR
                        echo "<tr>\n";
                    }
                    $file = "gfx/mud/" . $mud["md5"] . ".png";
                    $filename = "$FILE_HOME/$file";
                    $fileurl = "$URL_HOME/$file";
                    $updatetime = 650336715; // The beginning of time for WileyMUD
                    $forever_stamp = date("Y-m-d", $updatetime);
                    $today_stamp = date("Y-m-d", time());
                    $yesterday_stamp = date("Y-m-d", time() - 86400);
                    if(!file_exists($filename)) {
                        // No image file, pick a random one!
                        $home = getcwd();
                        chdir("$FILE_HOME/gfx/mud");
                        $random_file = array_rand(array_flip(glob("__NOT_FOUND_*.png")));
                        chdir($home);
                        $file = "gfx/mud/" . $random_file;
                        $filename = "$FILE_HOME/$file";
                        $fileurl = "$URL_HOME/$file";
                        if($mud["online"] == 1) {
                            $updatetime = time();
                        }
                    } else {
                        $image_counter++;
                        if($mud["online"] == 1) {
                            $verified_counter++;
                            $updatetime = time();
                        } else {
                            $updatetime = filemtime($filename);
                        }
                    }
                    $update_stamp = date("Y-m-d", $updatetime);
                    if( $update_stamp === $today_stamp ) {
                        $hour_stamp = date("H", $updatetime);
                        $now_hour = date("H");
                        if( $now_hour === $hour_stamp ) {
                            $update_stamp = "Online!";
                        } else {
                            $update_stamp = date("g:i a", $updatetime);
                            $update_stamp = "Last seen today, at $update_stamp";
                        }
                    } else if( $update_stamp === $yesterday_stamp ) {
                            $update_stamp = date("g:i a", $updatetime);
                            $update_stamp = "Last seen yesterday, at $update_stamp";
                    } else if( $update_stamp === $forever_stamp ) {
                        $update_stamp = "&nbsp;";
                    } else {
                        $update_stamp = "Last seen on $update_stamp";
                    }

                    $geo_file = "gfx/mud/" . $mud["md5"] . ".json";
                    $geo_filename = "$FILE_HOME/$geo_file";
                    $country_code = "";
                    $latitude = "";
                    $longitude = "";
                    $map_url = "";
                    if(file_exists($geo_filename)) {
                        $geo_text = file_get_contents($geo_filename);
                        $geo_ip = json_decode($geo_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);
                        if(array_key_exists('geoip', $geo_ip)) {
                            if(array_key_exists('country_code', $geo_ip['geoip'])) {
                                $country_code = strtolower($geo_ip['geoip']['country_code']);
                            }
                            if (array_key_exists('latitude', $geo_ip['geoip']) &&
                                array_key_exists('longitude', $geo_ip['geoip'])) {
                                $latitude = $geo_ip['geoip']['latitude'];
                                $longitude = $geo_ip['geoip']['longitude'];
                                $map_url = "https://www.google.com/maps/place//@$latitude,$longitude,17z";
                            }
                        }
                    }
                    ?>
                    <td class="content-left-login" style="<?php echo $opacity; ?>">
                        <div class="gallery-item">
                            <a href="<?php echo $fileurl; ?>" data-lightbox>
                                <img border="0" width="192" height="120" src="<?php echo $fileurl; ?>" />
                            </a>
                        </div>
                    </td>
                    <td class="content-left-gap" style="<?php echo $opacity; ?>">&nbsp;</td>
                    <td class="content-left-info" style="<?php echo $opacity; ?>">
                        <a target="I3 mudlist" href="http://<?php echo $mud["ipaddress"]; ?>/">
                            <?php echo $mud["name"]; ?>
                        </a><br />
                        <?php echo $mud["type"]; ?><br />
                        <?php echo $mud["mudlib"]; ?><br />
                        <?php
                            if($country_code !== "") {
                                $flag_filename = "$FILE_HOME/gfx/flags/$country_code.png";
                                $flag_url = "$URL_HOME/gfx/flags/$country_code.png";
                                if(file_exists($flag_filename)) {
                                    if($map_url !== "") {
                                        echo "<a href=\"$map_url\">";
                                    }
                                    echo "<img height=\"$FONT_SIZE\" src=\"$flag_url\" />";
                                    if($map_url !== "") {
                                        echo "</a>";
                                    }
                                }
                            }
                        ?>
                        <a href="telnet://<?php echo $mud["ipaddress"]; ?>:<?php echo $mud["port"]; ?>/">
                            <?php echo $mud["ipaddress"] . "&nbsp;" . $mud["port"]; ?>
                        </a>
                        <br />
                        <?php echo "$update_stamp"; ?><br />
                    </td>
                    <?php
                    if($counter % 2) {
                        // Odd rows means we close the TR
                        echo "</tr>\n";
                        $row_counter++;
                    }
                    $counter++;
                }
                if($counter % 2) {
                    // The counter is odd after running out of data, close the TR
                    //echo "</tr>\n";
                    // or enter a fake one to give some background space...
                    echo "<td colspan=\"3\">&nbsp;</td></tr>\n";
                }
                ?>
            </div>
            <tr>
                <td colspan="6" id="summary-column">
                    <?php echo $total_muds; ?> total muds listed 
                    (
                    <?php echo $online_counter; ?> connected,
                    <?php echo $verified_counter; ?> verified
                    )
                </td>
            </tr>
            <tr id="content-bottom">
                <?php $time_end = microtime(true); $time_spent = $time_end - $time_start; ?>
                <td colspan="6" id="time-column">
                    <?php echo sprintf("%9.4f", $time_spent); ?> seconds
                </td>
            </tr>
        </table>
        <!-- Not sure why, but the lightbox stuff MUST be down here -->
        <link rel="stylesheet" href="<?php echo $LIGHTBOX_CSS;?>">
        <script src="<?php echo $LIGHTBOX_JS;?>"></script>
        <script>
            const btn = document.querySelector('.trigger-lightbox');
            btn.addEventListener('click', () => new lightbox('#modal'));
        </script>
    </body>
</html>
