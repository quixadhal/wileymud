#ifndef _BUG_H
#define _BUG_H

#undef DIKU_CRUD
#ifdef DIKU_CRUD
#define bug(Str, ...) \
        abug(__FILE__, __FUNCTION__, __LINE__, \
	     GREATER_GOD, 0, NULL, NULL, Str, ## __VA_ARGS__)
#define log(Str, ...) \
        abug(NULL, NULL, 0, GREATER_GOD, 0, NULL, NULL, Str, ## __VA_ARGS__)
#define dlog(Str, ...) \
	abug(NULL, NULL, 0, GREATER_GOD, 0, NULL, NULL, Str, ## __VA_ARGS__)
#define fatal(Str, ...) \
	{\
        abug(__FILE__, __FUNCTION__, __LINE__, \
	     GREATER_GOD, 0, NULL, NULL, Str, ## __VA_ARGS__);\
	exit(-1);\
	}
#define oops(Str, ...) \
	{\
        abug(NULL, NULL, 0, GREATER_GOD, 0, NULL, NULL, Str, ## __VA_ARGS__);\
	exit(-1);\
	}
#else
#define bug(Str, ...) \
        abug(__FILE__, __FUNCTION__, __LINE__, \
	     0, NULL, Str, ## __VA_ARGS__)
#define log(Str, ...) \
        abug(NULL, NULL, 0, 0, NULL, Str, ## __VA_ARGS__)
#define dlog(Str, ...) \
	abug(NULL, NULL, 0, 0, NULL, Str, ## __VA_ARGS__)
#define fatal(Str, ...) \
	{\
        abug(__FILE__, __FUNCTION__, __LINE__, \
	     0, NULL, Str, ## __VA_ARGS__);\
	exit(-1);\
	}
#define oops(Str, ...) \
	{\
        abug(NULL, NULL, 0, 0, NULL, Str, ## __VA_ARGS__);\
	exit(-1);\
	}
#endif

void abug(char *File, char *Func, int Line,
#ifdef DIKU_CRUD
          unsigned int Level,
#endif
          unsigned int Type, char *BugFile,
#ifdef DIKU_CRUD
          struct char_data *ch,
#endif
          char *Str, ...);

#endif
