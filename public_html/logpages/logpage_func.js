function col_click(ob) {
    var url = window.location.href;
    var id = ob.parentNode.id;
    url = url.replace(/#.*$/, "") + "#" + id;

    // Our navbar screws up anchors a bit... we need to subtract
    // 5 rows so we're not "under" the navbar when the page loads.
    //
    // The problem is, we don't want to do this if we're still on
    // the first scroll set of rows, otherwise it will always push
    // the top few off....
    //
    // How do I find out how many rows are actually visible on
    // YOUR screen???
    //var n = id.match(/\d+/)[0];
    //if (n >= 30) {
    //    n -= 5;
    //    id = "row_" + n;
    //    url = url.replace(/#.*$/, "") + "#" + id;
    //} else {
    //    url = url.replace(/#.*$/, "") + "#content";
    //}
    //console.log("URL: " + url);

    if (window.clipboardData && window.clipboardData.setData ) {
        clipboardData.setData("Text", url);
        //console.log("Clipboard set to " + url);
    } else if (document.queryCommandSupported && document.queryCommandSupported("copy")) {
        var textarea = document.createElement("textarea");
        textarea.textContent = url;
        textarea.style.position = "fixed";
        document.body.appendChild(textarea);
        textarea.select();
        var result;
        try {
            result = document.execCommand("copy");
            //console.log("Copied: " + url);
        } catch (ex) {
            //console.error("Copy failed: ", ex);
        } finally {
            document.body.removeChild(textarea);
            //console.log("Result: " + result);
        }
    } else {
        //console.log("No way to do this!");
    }
}

function debug_local_time() {
    var hour_map = [
        '#555555',
        '#555555',
        '#555555',
        '#555555',
        '#bb0000',
        '#bb0000',
        '#bbbb00',
        '#bbbb00',
        '#ffff55',
        '#ffff55',
        '#00bb00',
        '#00bb00',
        '#55ff55',
        '#55ff55',
        '#bbbbbb',
        '#bbbbbb',
        '#55ffff',
        '#55ffff',
        '#00bbbb',
        '#00bbbb',
        '#5555ff',
        '#5555ff',
        '#0000bb',
        '#0000bb'
    ];
    var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
    var yourLocale = (navigator.languages && navigator.languages.length) ?
        navigator.languages[0] : navigator.language;

    var momentObj = moment().tz(yourTimeZone);
    var momentStr = momentObj.format('YYYY-MM-DD HH:mm:ss z');
    var momentHour = momentObj.hour();

    var yt = document.getElementById("yourTime");
    yt.innerHTML = "[" + yourLocale + " " + yourTimeZone + "] " + momentStr;
    yt.style.color = hour_map[momentHour];
}

function localize_rows() {
    // 0 -> 23
    var hour_map = [
        '#555555',
        '#555555',
        '#555555',
        '#555555',
        '#bb0000',
        '#bb0000',
        '#bbbb00',
        '#bbbb00',
        '#ffff55',
        '#ffff55',
        '#00bb00',
        '#00bb00',
        '#55ff55',
        '#55ff55',
        '#bbbbbb',
        '#bbbbbb',
        '#55ffff',
        '#55ffff',
        '#00bbbb',
        '#00bbbb',
        '#5555ff',
        '#5555ff',
        '#0000bb',
        '#0000bb'
    ];

    var yourTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
    var yourLocale = (navigator.languages && navigator.languages.length) ?
        navigator.languages[0] : navigator.language;

    var rows = $(`[id^=row_]`);
    for( var i = 0; i < rows.length-1; i++ ) {
        var tds = rows[i].getElementsByTagName("td");
        var oldDate = tds[0].innerHTML;
        var tdspan = tds[1].getElementsByTagName("span");
        var oldTime = tdspan[0].innerHTML;

        var oldMoment = moment.tz(oldDate + " " + oldTime, "America/Los_Angeles");
        var newMoment = oldMoment.clone().tz(yourTimeZone);
        //var newMoment = oldMoment.clone().tz("Asia/Tokyo");
        var newDate = newMoment.format('YYYY-MM-DD');
        var newTime = newMoment.format('HH:mm:ss');
        var newHour = newMoment.hour();

        tds[0].innerHTML = newDate;
        tdspan[0].innerHTML = newTime;
        tdspan[0].style.color = hour_map[newHour];
    }
}

function toggleDiv(divID) {
    if(document.getElementById(divID).style.display == 'none') {
        document.getElementById(divID).style.display = 'block';
    } else {
        document.getElementById(divID).style.display = 'none';
    }
}
