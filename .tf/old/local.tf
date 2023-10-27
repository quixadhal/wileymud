/loaded local.tf

/redef on
/def -i COMPRESS_SUFFIX=.gz
;/def -ib'^[[A' = /DOKEY RECALLB
;/def -ib'^[[B' = /DOKEY RECALLF
;/def -ib'^P'   = /DOKEY SOCKETB
;/def -ib'^N'   = /DOKEY SOCKETF
;/def -ib'^[b'  = /DOKEY RECALLB
;/def -ib'^[f'  = /DOKEY RECALLF
/def -ib'^I'   = /DOKEY SEARCHB
/def -ib'^[OA' = /DOKEY RECALLB
/def -ib'^[OB' = /DOKEY RECALLF

; more sensible pager

/def -i pager = \
    /purge -i -b" "%; \
    /dokey page
/def -i -arh -hMORE = \
  /def -i -b" " = /pager

; a more verbose time command

/def -i date = /time %%C %%r %%Z

/visual on
/wrap 0

/echo Resources are now kept in ~/.tf/tfrc. \
To include most of the "fun" commands, use "/require everything.tf".

