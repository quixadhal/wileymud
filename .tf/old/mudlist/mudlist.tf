/set redef=1
/set matching=2
/set catch_ctrls=2
/set oldslash=0
/set wrap=off
/set quitdone=1

/def -t"^(.*)Welcome(.*)$" blah = /undef blah%;/repeat -01 1 /send @mudlist *
/def -t"^####" ack = /undef ack%;/def -t"^####" ack = /send QUIT

/addtiny mudlist toybox.infomagic.com 4801
/world mudlist
