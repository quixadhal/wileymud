#ifndef _MAIN_H
#define _MAIN_H

#define VERSION		"Version 0.7, Released 96.08.02"

#define MAX_LINE_LEN 256
#define MAX_PAGE_LEN 8192

#define ROOM_FILE	"tinyworld.wld"			       /* room definitions */
#define MOB_FILE	"tinyworld.mob"			       /* monster prototypes */
#define OBJ_FILE	"tinyworld.obj"			       /* object prototypes */
#define ZONE_FILE	"tinyworld.zon"			       /* zone defs & command tables */
#define SHOP_FILE	"tinyworld.shp"			       /* shop file */

#define INDEX_SUBDIR	"index"
#define REPORT_SUBDIR	"report"
#define MAP_SUBDIR	"map"
#define ISLES_SUBDIR	"isles"
#define AFK_SUBDIR	"afk"
#define FR_SUBDIR	"fr"
#define SMAUG_SUBDIR	"smaug"
#define NIGHTMARE_SUBDIR	"nightmare"
#define DS_SUBDIR	"ds"

#define MAP_FILE	"map.out"
#define PPM_FILE	"map-%03d.ppm"

#ifndef _MAIN_C
extern char                            *Progname;
extern int                              Debug;
extern int                              Quiet;
extern int                              Verbose;
extern unsigned long                    InputFormat;
extern unsigned long                    OutputFormat;
extern char                            *InputDir;
extern char                            *OutputDir;

extern int                              IncludeShortInLong;
extern int                              ObviousExits;
extern int                              HardReturns;
extern int                              OneBigDomain;
extern int                              PitchBlack;
#endif

#endif
