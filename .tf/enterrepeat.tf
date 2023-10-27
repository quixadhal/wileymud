/loaded enterrepeat.tf

/def -i enter = \
    /if (!ismacro("autoenter")) \
        /echo *AUTOMAGIC ENTER ENABLED*%;\
        /def -i -F -mregexp -h"send ^$$" autoenter = \
        $$(/recall -i - -3)%;\
    /else \
        /echo *AUTOMAGIC ENTER DISABLED*%;\
        /undef autoenter%;\
    /endif

