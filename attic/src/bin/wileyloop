#!/bin/bash
# 42 is our magic number for not rebooting,
# otherwise we assume it crashed or we said reboot.

PORT=3000
MUDDIR=/home/wiley
PATH=$MUDDIR/bin:/bin:/usr/bin:/sbin:/usr/sbin
DAEMON=$MUDDIR/bin/wileymud
SCRIPT=$MUDDIR/bin/wileyloop
PIDFILE=$MUDDIR/etc/wileymud.pid
STARTDIR=$MUDDIR/bin

cd $STARTDIR
while [ -x $DAEMON ]; do
  LOGFILE=`/bin/date "+$MUDDIR/lib/log/runlog.%y%m%d-%H%M%S"`
  touch $LOGFILE
  chmod 640 $LOGFILE
  $DAEMON -P $PIDFILE -L $LOGFILE $PORT
  STATUS=$?
  rm -f $PIDFILE
  sync
  bzip2 -9q $LOGFILE
  sync
  if [ $STATUS = 42 ]; then
    exit
  fi
  echo "Status was $STATUS"
  sleep 240
done

