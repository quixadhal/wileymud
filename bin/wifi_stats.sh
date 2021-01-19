#!/bin/bash

#<pre><?php pcmd("/usr/bin/nmcli -f 'DEVICE,CHAN,BARS,SIGNAL,RATE,SSID' dev wifi | /usr/bin/egrep '(\s+SSID|\s+Dread_.748)'"); ?></pre>
#<pre>Wifi Connection in use: <?php pcmd("/sbin/iwconfig wlp1s0 | grep ESSID"); ?></pre>

if [ $UID -eq 0 ]; then
    /sbin/iwconfig wlp1s0 \
        | grep ESSID \
        >/home/wiley/public_html/iwconfig.txt 2>/dev/null
    /usr/bin/nmcli -f 'DEVICE,CHAN,BARS,SIGNAL,RATE,SSID' dev wifi \
        | /usr/bin/egrep '(\s+SSID|\s+Dread_.748)' \
        >/home/wiley/public_html/nmcli.txt 2>/dev/null
    chown wiley.users /home/wiley/public_html/iwconfig.txt
    chmod 644 /home/wiley/public_html/iwconfig.txt
    chown wiley.users /home/wiley/public_html/nmcli.txt
    chmod 644 /home/wiley/public_html/nmcli.txt
fi

