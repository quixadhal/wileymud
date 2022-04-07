<?php
$time_start = microtime(true);
require_once 'site_global.php';
require_once 'page_source.php';
require_once 'random_background.php';
require_once 'navbar.php';

$PIE_FILE               = "$FILE_HOME/pie_css.php";
$PIE_TIME               = filemtime($PIE_FILE);
$PIE_CSS                = "$URL_HOME/pie_css.php?version=$PIE_TIME";

$PIE_JSON               = "$FILE_HOME/data/pie.json";
$pie_text               = file_get_contents($PIE_JSON);
$pie_data               = json_decode($pie_text, true, 512, JSON_INVALID_UTF8_SUBSTITUTE);

?>

<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="pragma" content="no-cache" />
        <meta http-equiv="refresh" content="1800" />
        <title>Everyone Loves PIE!</title>
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
        <link rel="stylesheet" href="<?php echo $PIE_CSS;?>">

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
            var ContentTimer;
            var pieData = <?php echo $pie_text; ?>;

            var en = [];
            var jp = [];

            en['today']     = 'Today-chan';
            jp['today']     = '今日ちゃん';
            en['yesterday'] = 'Yesterday-chan';
            jp['yesterday'] = '昨日ちゃん';
            en['week']      = 'Week-chan';
            jp['week']      = '週間ちゃん';
            en['month']     = 'Month-chan';
            jp['month']     = '月ちゃん';
            en['year']      = 'Year-chan';
            jp['year']      = '年ちゃん';
            en['all']       = 'All Time-chan';
            jp['all']       = '常にちゃん';

            function hover_on(x) {
                // Change text of this to en
                // button-chan-XXX where you want en[XXX]
                var s = x.id.substr(12);
                x.innerHTML = en[s];
            }
            function hover_off(x) {
                // Change text of this to jp
                // button-chan-XXX where you want jp[XXX]
                var s = x.id.substr(12);
                x.innerHTML = jp[s];
            }
            function click_select(x) {
                // Set ALL buttons to inactive.
                // Then set this to active.
                // Set all graphs to display:none
                // Then set the one this matches to display:block
                // button-XXX to match to graph-XXX
                // button-chan-XXX to match to graph-chan-XXX
                var s = x.id.substr(7);
                var t = s.substr(0,5);
                if(t != 'chan-') {
                    t = '';
                }

                hideDiv('graph-today');
                hideDiv('graph-yesterday');
                hideDiv('graph-week');
                hideDiv('graph-month');
                hideDiv('graph-year');
                hideDiv('graph-all');
                hideDiv('graph-chan-today');
                hideDiv('graph-chan-yesterday');
                hideDiv('graph-chan-week');
                hideDiv('graph-chan-month');
                hideDiv('graph-chan-year');
                hideDiv('graph-chan-all');
                showDiv('graph-' + s);

                removeClass('button-today', 'button-active');
                removeClass('button-yesterday', 'button-active');
                removeClass('button-week', 'button-active');
                removeClass('button-month', 'button-active');
                removeClass('button-year', 'button-active');
                removeClass('button-all', 'button-active');
                removeClass('button-chan-today', 'button-chan-active');
                removeClass('button-chan-yesterday', 'button-chan-active');
                removeClass('button-chan-week', 'button-chan-active');
                removeClass('button-chan-month', 'button-chan-active');
                removeClass('button-chan-year', 'button-chan-active');
                removeClass('button-chan-all', 'button-chan-active');
                addClass('button-today', 'button-inactive');
                addClass('button-yesterday', 'button-inactive');
                addClass('button-week', 'button-inactive');
                addClass('button-month', 'button-inactive');
                addClass('button-year', 'button-inactive');
                addClass('button-all', 'button-inactive');
                addClass('button-chan-today', 'button-chan-inactive');
                addClass('button-chan-yesterday', 'button-chan-inactive');
                addClass('button-chan-week', 'button-chan-inactive');
                addClass('button-chan-month', 'button-chan-inactive');
                addClass('button-chan-year', 'button-chan-inactive');
                addClass('button-chan-all', 'button-chan-inactive');

                removeClass('button-' + s, 'button-' + t + 'inactive');
                addClass('button-' + s, 'button-' + t + 'active');
            }

            function updateContent() {
                clearTimeout(ContentTimer);
                click_select(document.getElementById('button-today'));
            }

            $(document).ready(function() {
                $('#page-load-time').html(timeSpent);
                showDiv('page-load-time');
                dim(document.getElementById('navbar-button-pie'));
                syncBackgroundToggleIcon();
                randomizeBackground();
                updateRefreshTime();
                backgroundTimer = setInterval(randomizeBackground, 1000 * 60 * 5);
                ContentTimer = setTimeout(updateContent, 100);
            });
        </script>
        <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
        <script type="text/javascript">
            google.charts.load('current', {'packages':['corechart']});
            google.charts.setOnLoadCallback(drawTheCharts);
            var options;

            function drawSetup() {
                options = {
                    is3D                : 1,
                    title               : "Today's Jibber-Jabber",
                    titleColor          : '#e0e0e0',
                    //backgroundColor     : '#101010',
                    backgroundColor     : 'none',
                    legendTextStyle     : {
                        color           : '#e0e0e0',
                    },
                    pieSliceTextStyle   : {
                        color           : '#000000',
                        fontName        : 'Arial',
                        fontSize        : '<?php echo $FONT_SIZE; ?>',
                    },
                    chartArea           : {
                        top             : '5%',
                        left            : '5%',
                        height          : '95%',
                        width           : '95%',
                    },
                    colors              : [
                        '#ff3f3f', '#ffaf3f', '#ffff3f',
                        '#afff3f', '#3fff3f', '#3fffaf',
                        '#3fffff', '#3fafff', '#3f3fff',
                        '#af3fff', '#ff3fff', '#ff3faf',
                    ],
                };
            }

            function drawTodaySpeakers() {
                if(pieData.speakers.today.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Speaker', 'Count'] ];
                    for(var i = 0; i < pieData.speakers.today.length; i++) {
                        total += pieData.speakers.today[i].count;
                        dataTableArray.push([ pieData.speakers.today[i].speaker,
                                              pieData.speakers.today[i].count ]);
                    }
                    options['title'] = "Today's " + total + " Jibber-Jabbers (" + pieData.dates.today + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-today').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-today'));
                    chart.draw(data, options);
                    document.getElementById('graph-today').style.display='none';
                    if(pieData.quotes.today) {
                        document.getElementById('quote-today').innerHTML= pieData.quotes.today.speaker + " said &#x300C;" + pieData.quotes.today.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-today').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-today').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-today').innerHTML="I got nothin'.";
                }
            }

            function drawTodayChannels() {
                if(pieData.channels.today.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Channel', 'Count'] ];
                    for(var i = 0; i < pieData.channels.today.length; i++) {
                        total += pieData.channels.today[i].count;
                        dataTableArray.push([ pieData.channels.today[i].channel,
                                              pieData.channels.today[i].count ]);
                    }
                    options['title'] = "Today's " + total + " Jibber-Jabbers (" + pieData.dates.today + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-chan-today').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-chan-today'));
                    chart.draw(data, options);
                    document.getElementById('graph-chan-today').style.display='none';
                    if(pieData.quotes.today) {
                        document.getElementById('quote-chan-today').innerHTML= pieData.quotes.today.speaker + " said &#x300C;" + pieData.quotes.today.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-chan-today').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-chan-today').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-chan-today').innerHTML="I got nothin'.";
                }
            }

            function drawYesterdaySpeakers() {
                if(pieData.speakers.yesterday.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Speaker', 'Count'] ];
                    for(var i = 0; i < pieData.speakers.yesterday.length; i++) {
                        total += pieData.speakers.yesterday[i].count;
                        dataTableArray.push([ pieData.speakers.yesterday[i].speaker,
                                              pieData.speakers.yesterday[i].count ]);
                    }
                    options['title'] = "Yesterday's " + total + " Bits of Rubbish (" + pieData.dates.yesterday + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-yesterday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-yesterday'));
                    chart.draw(data, options);
                    document.getElementById('graph-yesterday').style.display='none';
                    if(pieData.quotes.yesterday) {
                        document.getElementById('quote-yesterday').innerHTML= pieData.quotes.yesterday.speaker + " said &#x300C;" + pieData.quotes.yesterday.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-yesterday').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-yesterday').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-yesterday').innerHTML="I got nothin'.";
                }
            }

            function drawYesterdayChannels() {
                if(pieData.channels.yesterday.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Channel', 'Count'] ];
                    for(var i = 0; i < pieData.channels.yesterday.length; i++) {
                        total += pieData.channels.yesterday[i].count;
                        dataTableArray.push([ pieData.channels.yesterday[i].channel,
                                              pieData.channels.yesterday[i].count ]);
                    }
                    options['title'] = "Yesterday's " + total + " Bits of Rubbish (" + pieData.dates.yesterday + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-chan-yesterday').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-chan-yesterday'));
                    chart.draw(data, options);
                    document.getElementById('graph-chan-yesterday').style.display='none';
                    if(pieData.quotes.yesterday) {
                        document.getElementById('quote-chan-yesterday').innerHTML= pieData.quotes.yesterday.speaker + " said &#x300C;" + pieData.quotes.yesterday.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-chan-yesterday').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-chan-yesterday').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-chan-yesterday').innerHTML="I got nothin'.";
                }
            }

            function drawWeekSpeakers() {
                if(pieData.speakers.week.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Speaker', 'Count'] ];
                    for(var i = 0; i < pieData.speakers.week.length; i++) {
                        total += pieData.speakers.week[i].count;
                        dataTableArray.push([ pieData.speakers.week[i].speaker,
                                              pieData.speakers.week[i].count ]);
                    }
                    options['title'] = "The Week of " + total + " Idiotic Ramglings (" + pieData.dates.week + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-week').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-week'));
                    chart.draw(data, options);
                    document.getElementById('graph-week').style.display='none';
                    if(pieData.quotes.week) {
                        document.getElementById('quote-week').innerHTML= pieData.quotes.week.speaker + " said &#x300C;" + pieData.quotes.week.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-week').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-week').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-week').innerHTML="I got nothin'.";
                }
            }

            function drawWeekChannels() {
                if(pieData.channels.week.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Channel', 'Count'] ];
                    for(var i = 0; i < pieData.channels.week.length; i++) {
                        total += pieData.channels.week[i].count;
                        dataTableArray.push([ pieData.channels.week[i].channel,
                                              pieData.channels.week[i].count ]);
                    }
                    options['title'] = "The Week of " + total + " Idiotic Ramglings (" + pieData.dates.week + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-chan-week').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-chan-week'));
                    chart.draw(data, options);
                    document.getElementById('graph-chan-week').style.display='none';
                    if(pieData.quotes.week) {
                        document.getElementById('quote-chan-week').innerHTML= pieData.quotes.week.speaker + " said &#x300C;" + pieData.quotes.week.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-chan-week').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-chan-week').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-chan-week').innerHTML="I got nothin'.";
                }
            }

            function drawMonthSpeakers() {
                if(pieData.speakers.month.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Speaker', 'Count'] ];
                    for(var i = 0; i < pieData.speakers.month.length; i++) {
                        total += pieData.speakers.month[i].count;
                        dataTableArray.push([ pieData.speakers.month[i].speaker,
                                              pieData.speakers.month[i].count ]);
                    }
                    options['title'] = "A Month with " + total + " Nonsensical Opinions? (" + pieData.dates.month + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-month').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-month'));
                    chart.draw(data, options);
                    document.getElementById('graph-month').style.display='none';
                    if(pieData.quotes.month) {
                        document.getElementById('quote-month').innerHTML= pieData.quotes.month.speaker + " said &#x300C;" + pieData.quotes.month.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-month').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-month').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-month').innerHTML="I got nothin'.";
                }
            }

            function drawMonthChannels() {
                if(pieData.channels.month.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Channel', 'Count'] ];
                    for(var i = 0; i < pieData.channels.month.length; i++) {
                        total += pieData.channels.month[i].count;
                        dataTableArray.push([ pieData.channels.month[i].channel,
                                              pieData.channels.month[i].count ]);
                    }
                    options['title'] = "A Month with " + total + " Nonsensical Opinions? (" + pieData.dates.month + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-chan-month').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-chan-month'));
                    chart.draw(data, options);
                    document.getElementById('graph-chan-month').style.display='none';
                    if(pieData.quotes.month) {
                        document.getElementById('quote-chan-month').innerHTML= pieData.quotes.month.speaker + " said &#x300C;" + pieData.quotes.month.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-chan-month').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-chan-month').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-chan-month').innerHTML="I got nothin'.";
                }
            }

            function drawYearSpeakers() {
                if(pieData.speakers.year.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Speaker', 'Count'] ];
                    for(var i = 0; i < pieData.speakers.year.length; i++) {
                        total += pieData.speakers.year[i].count;
                        dataTableArray.push([ pieData.speakers.year[i].speaker,
                                              pieData.speakers.year[i].count ]);
                    }
                    options['title'] = "What a Horrible Year it's Been... " + total + " Open Sores. (" + pieData.dates.year + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-year').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-year'));
                    chart.draw(data, options);
                    document.getElementById('graph-year').style.display='none';
                    if(pieData.quotes.year) {
                        document.getElementById('quote-year').innerHTML= pieData.quotes.year.speaker + " said &#x300C;" + pieData.quotes.year.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-year').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-year').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-year').innerHTML="I got nothin'.";
                }
            }

            function drawYearChannels() {
                if(pieData.channels.year.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Channel', 'Count'] ];
                    for(var i = 0; i < pieData.channels.year.length; i++) {
                        total += pieData.channels.year[i].count;
                        dataTableArray.push([ pieData.channels.year[i].channel,
                                              pieData.channels.year[i].count ]);
                    }
                    options['title'] = "What a Horrible Year it's Been... " + total + " Open Sores. (" + pieData.dates.year + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-chan-year').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-chan-year'));
                    chart.draw(data, options);
                    document.getElementById('graph-chan-year').style.display='none';
                    if(pieData.quotes.year) {
                        document.getElementById('quote-chan-year').innerHTML= pieData.quotes.year.speaker + " said &#x300C;" + pieData.quotes.year.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-chan-year').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-chan-year').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-chan-year').innerHTML="I got nothin'.";
                }
            }

            function drawAllSpeakers() {
                if(pieData.speakers.all.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Speaker', 'Count'] ];
                    for(var i = 0; i < pieData.speakers.all.length; i++) {
                        total += pieData.speakers.all[i].count;
                        dataTableArray.push([ pieData.speakers.all[i].speaker,
                                              pieData.speakers.all[i].count ]);
                    }
                    options['title'] = "What Have You DONE??? " + total + " Wastes of Time! (" + pieData.dates.all + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-all').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-all'));
                    chart.draw(data, options);
                    document.getElementById('graph-all').style.display='none';
                    if(pieData.quotes.all) {
                        document.getElementById('quote-all').innerHTML= pieData.quotes.all.speaker + " said &#x300C;" + pieData.quotes.all.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-all').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-all').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-all').innerHTML="I got nothin'.";
                }
            }

            function drawAllChannels() {
                if(pieData.channels.all.length > 0) {
                    var total = 0;
                    var dataTableArray = [ ['Channel', 'Count'] ];
                    for(var i = 0; i < pieData.channels.all.length; i++) {
                        total += pieData.channels.all[i].count;
                        dataTableArray.push([ pieData.channels.all[i].channel,
                                              pieData.channels.all[i].count ]);
                    }
                    options['title'] = "What Have You DONE??? " + total + " Wastes of Time! (" + pieData.dates.all + ")";
                    var data = google.visualization.arrayToDataTable(dataTableArray);
                    document.getElementById('graph-chan-all').style.display='block';
                    var chart = new google.visualization.PieChart(document.getElementById('pie-chan-all'));
                    chart.draw(data, options);
                    document.getElementById('graph-chan-all').style.display='none';
                    if(pieData.quotes.all) {
                        document.getElementById('quote-chan-all').innerHTML= pieData.quotes.all.speaker + " said &#x300C;" + pieData.quotes.all.message + "&#x300D;";
                    } else {
                        document.getElementById('quote-chan-all').innerHTML="I got nothin'.";
                    }
                } else {
                    document.getElementById('pie-chan-all').innerHTML='<img class="not-available" src="<?php echo $NOT_AVAILABLE_ICON; ?>" />';
                    document.getElementById('quote-chan-all').innerHTML="I got nothin'.";
                }
            }

            function drawTheCharts() {
                drawSetup();

                drawTodaySpeakers();
                drawYesterdaySpeakers();
                drawWeekSpeakers();
                drawMonthSpeakers();
                drawYearSpeakers();
                drawAllSpeakers();

                drawTodayChannels();
                drawYesterdayChannels();
                drawWeekChannels();
                drawMonthChannels();
                drawYearChannels();
                drawAllChannels();

                document.getElementById('graph-today').style.display='block';
            }
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
            <img class="nav-img" id="navbar-button-pie" title="Everyone loves PIE!" src="<?php echo $PIE_ICON; ?>" />
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
            <span id="refresh-time">--:-- ---</span>
            <img class="nav-img glowing" id="navbar-button-server" title="Crusty Server Statistics" src="<?php echo $SERVER_ICON; ?>" onclick="window.location.href='<?php echo $SERVER_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-github" title="All of this in Github" src="<?php echo $GITHUB_ICON; ?>" onclick="window.location.href='<?php echo $GITHUB_URL; ?>';" />
            <img class="nav-img glowing" id="navbar-button-discord" title="The I3 Discord" src="<?php echo $DISCORD_ICON; ?>" onclick="window.location.href='<?php echo $DISCORD_URL; ?>';" />
        </div>
        <div id="fake-navbar">
            <img class="nav-img" title="???!" src="<?php echo $QUESTION_ICON; ?>" />
        </div>

        <table id="button-table">
            <tr>
                <td id="button-today"           class="button-active"   onclick="click_select(this);"> Today </td>
                <td id="button-yesterday"       class="button-inactive" onclick="click_select(this);"> Yesterday </td>
                <td id="button-week"            class="button-inactive" onclick="click_select(this);"> Last Week </td>
                <td id="button-month"           class="button-inactive" onclick="click_select(this);"> The Month </td>
                <td id="button-year"            class="button-inactive" onclick="click_select(this);"> A Year Ago </td>
                <td id="button-all"             class="button-inactive" onclick="click_select(this);"> Of All Time </td>
            </tr>
            <tr>
                <td id="button-chan-today"      class="button-chan-inactive" onclick="click_select(this);" onmouseover="hover_on(this);" onmouseout="hover_off(this);" > 今日ちゃん </td>
                <td id="button-chan-yesterday"  class="button-chan-inactive" onclick="click_select(this);" onmouseover="hover_on(this);" onmouseout="hover_off(this);" > 昨日ちゃん </td>
                <td id="button-chan-week"       class="button-chan-inactive" onclick="click_select(this);" onmouseover="hover_on(this);" onmouseout="hover_off(this);" > 週間ちゃん </td>
                <td id="button-chan-month"      class="button-chan-inactive" onclick="click_select(this);" onmouseover="hover_on(this);" onmouseout="hover_off(this);" > 月ちゃん </td>
                <td id="button-chan-year"       class="button-chan-inactive" onclick="click_select(this);" onmouseover="hover_on(this);" onmouseout="hover_off(this);" > 年ちゃん </td>
                <td id="button-chan-all"        class="button-chan-inactive" onclick="click_select(this);" onmouseover="hover_on(this);" onmouseout="hover_off(this);" > 常にちゃん </td>
            </tr>
            <tr>
                <td class="row-gap" colspan="6">&nbsp;</td>
            </tr>
        </table>

        <div id="graph-today" class="graph-div">
            <div id="pie-today" class="pie-div">
            </div>
            <div id="quote-today" class="quote-div">
                Clever today &#x300C;quote&#x300D; goes here.
            </div>
        </div>

        <div id="graph-chan-today" class="graph-div">
            <div id="pie-chan-today" class="pie-div">
            </div>
            <div id="quote-chan-today" class="quote-div">
                Clever today-chan &#x300C;quote&#x300D; goes here.
            </div>
        </div>

        <div id="graph-yesterday" class="graph-div">
            <div id="pie-yesterday" class="pie-div">
            </div>
            <div id="quote-yesterday" class="quote-div">
                Clever yesterday &#x300C;quote&#x300D; goes here.
            </div>
        </div>
        <div id="graph-chan-yesterday" class="graph-div">
            <div id="pie-chan-yesterday" class="pie-div">
            </div>
            <div id="quote-chan-yesterday" class="quote-div">
                Clever yesterday-chan &#x300C;quote&#x300D; goes here.
            </div>
        </div>

        <div id="graph-week" class="graph-div">
            <div id="pie-week" class="pie-div">
            </div>
            <div id="quote-week" class="quote-div">
                Clever week &#x300C;quote&#x300D; goes here.
            </div>
        </div>
        <div id="graph-chan-week" class="graph-div">
            <div id="pie-chan-week" class="pie-div">
            </div>
            <div id="quote-chan-week" class="quote-div">
                Clever week-chan &#x300C;quote&#x300D; goes here.
            </div>
        </div>

        <div id="graph-month" class="graph-div">
            <div id="pie-month" class="pie-div">
            </div>
            <div id="quote-month" class="quote-div">
                Clever month &#x300C;quote&#x300D; goes here.
            </div>
        </div>
        <div id="graph-chan-month" class="graph-div">
            <div id="pie-chan-month" class="pie-div">
            </div>
            <div id="quote-chan-month" class="quote-div">
                Clever month-chan &#x300C;quote&#x300D; goes here.
            </div>
        </div>

        <div id="graph-year" class="graph-div">
            <div id="pie-year" class="pie-div">
            </div>
            <div id="quote-year" class="quote-div">
                Clever year &#x300C;quote&#x300D; goes here.
            </div>
        </div>
        <div id="graph-chan-year" class="graph-div">
            <div id="pie-chan-year" class="pie-div">
            </div>
            <div id="quote-chan-year" class="quote-div">
                Clever year-chan &#x300C;quote&#x300D; goes here.
            </div>
        </div>

        <div id="graph-all" class="graph-div">
            <div id="pie-all" class="pie-div">
            </div>
            <div id="quote-all" class="quote-div">
                Clever all &#x300C;quote&#x300D; goes here.
            </div>
        </div>
        <div id="graph-chan-all" class="graph-div">
            <div id="pie-chan-all" class="pie-div">
            </div>
            <div id="quote-chan-all" class="quote-div">
                Clever all-chan &#x300C;quote&#x300D; goes here.
            </div>
        </div>
    </body>
</html>
