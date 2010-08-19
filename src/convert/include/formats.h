#ifndef _FORMATS_H
#define _FORMATS_H

#define IF_COUNT	1
#define IF_ALL		((1<<IF_COUNT)-1)
#define OF_COUNT	9
#define OF_ALL		((1<<OF_COUNT)-1)

typedef struct s_name {
  char *Name;
  char *Type;
} t_name;

#ifndef _FORMATS_C
extern t_name *InputData;
extern t_name *OutputData;
#endif

char *if_name(unsigned long InputFormat);
char *of_name(unsigned long InputFormat);
char *if_type(unsigned long InputFormat);
char *of_type(unsigned long InputFormat);
unsigned long if_mask(char *InputName);
unsigned long of_mask(char *OutputName);

#endif
