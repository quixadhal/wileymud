#ifndef _UTILS_H
#define _UTILS_H

#ifndef __LINE__					       /* You should compile this with an ANSI compiler so */
#define __LINE__ 0					       /* that the debugging will work correctly, but I */
#endif							       /* won't insist upon it ;^) */

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(a) (((a)<0)?(-(a)):(a))

#define FALSE 0
#define TRUE 1

void                                    spin(FILE * fp);
char                                   *my_strdup(char *Str);
FILE                                   *open_file(char *Filename, char *Mode);
char                                   *get_mem(long Count, long Size);
char                                   *get_more_mem(char *Memory, long Count, long Size);
char                                   *get_line(FILE * fp, long *Line, long *Pos, int Skip);
int                                     verify_pos(FILE * fp, long Pos, int Check);
char                                   *get_tilde_string(FILE * fp, long *Line, long *Pos);
char                                   *remap_name(char *Old);
keyword                                *make_keyword_list(char *String);
char                                   *timestamp(void);
void                                    sscanf_dice(char *str, int *x, int *y, int *z);
char                                   *ordinal(int x);
char   *md5_hex(const char *str);
char   *json_escape(char *thing);
int     scprintf(char *buf, size_t limit, const char *Str, ...) __attribute__ ( ( format( printf, 3, 4 ) ) );

#endif
