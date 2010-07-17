/* Codebase Macros - SocketMUD
 *
 * This should cover the following derivatives:
 *
 * SocketMUD: Versions 2.1 and up.
 *
 * Please send any needed updates to imc@imc2.org
 */

#ifndef __IMC2CFG_H__
#define __IMC2CFG_H__

#if !defined(IMCSTANDALONE)

#define CH_IMCDATA(ch)           ((ch)->imcchardata)
#define CH_IMCLEVEL(ch)          ((ch)->level)
#define CH_IMCNAME(ch)           ((ch)->name)
#define CH_IMCTITLE(ch)          ( "User" )
#define CH_IMCRANK(ch)           ( "User" )
#define first_descriptor dsock_list

#else

typedef unsigned char bool;

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#define CH_IMCDATA(ch)           ((ch)->imcchardata)
#define CH_IMCLEVEL(ch)          ((ch)->level)
#define CH_IMCNAME(ch)           ((ch)->name)
#define CH_IMCSEX(ch)            ((ch)->sex)
#define CH_IMCTITLE(ch)          ( "User" )
#define CH_IMCRANK(ch)           ( "User" )

typedef enum
{
   SEX_NEUTRAL, SEX_MALE, SEX_FEMALE
} genders;

#define STATE_PLAYING 1
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

typedef struct user_data D_MOBILE;
typedef struct conn_data D_SOCKET;

struct user_data
{
   struct imcchar_data *imcchardata;
   char *name;
   int level;
   short sex;
};

struct conn_data
{
   D_SOCKET *next;
   D_SOCKET *prev;
   D_MOBILE *original;
   D_MOBILE *player;
   short state;
};

D_SOCKET *first_descriptor;
D_SOCKET *last_descriptor;
#endif

#endif
