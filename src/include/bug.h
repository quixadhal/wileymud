#ifndef _BUG_H
#define _BUG_H

#define DIKU_CRUD
#ifndef DIKU_CRUD
#define GREATER_GOD 0
#endif

#define bug(Str...) \
        abug(__FILE__, __PRETTY_FUNCTION__, __LINE__, \
	     GREATER_GOD, 0, NULL, NULL, Str ##)
#define log(Str...) \
        abug(NULL, NULL, 0, GREATER_GOD, 0, NULL, NULL, Str ##)
#define dlog(Str...) \
	abug(NULL, NULL, 0, GREATER_GOD, 0, NULL, NULL, Str ##)

void abug(char *File, char *Func, int Line, UINT Level, UINT Type, char *BugFile, struct char_data *ch, char *Str,...);

#endif
