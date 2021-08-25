#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "include/main.h"
#define _FORMATS_C
#include "include/formats.h"

t_name InputData[IF_COUNT] = {{"wiley", "WileyMUD III"}};

t_name OutputData[OF_COUNT] = {{"index", "Line Indicies of VNUM headers"},
                               {"report", "Text Report"},
                               {"map", "PGM bitmap"},
                               {"isles", "The Isles DikuMUD"},
                               {"nightmare", "Nightmare IVr2.5 LpMUD"},
                               {"fr", "Final Realms 2.1b4 LpMUD"},
                               {"afk", "AFKMUD (Diku Derivative)"},
                               {"smaug", "Smaug (Diku Derivative)"},
                               {"json", "JSON Data Dump"},
                               {"newmap", "New Map Data Dump"},
                               {"ds", "Dead Souls 3.0 LpMUD"}};

char *if_name(unsigned long InputFormat)
{
    register int i;
    static char tmp[MAX_STRING_LEN];

    bzero(tmp, MAX_STRING_LEN);
    for (i = 0; i < IF_COUNT; i++)
        if (InputFormat & (1 << i))
        {
            strcat(tmp, InputData[i].Name);
            strcat(tmp, " ");
        }
    if (strlen(tmp))
        tmp[strlen(tmp) - 1] = '\0';
    return tmp;
}

char *of_name(unsigned long OutputFormat)
{
    register int i;
    static char tmp[MAX_STRING_LEN];

    bzero(tmp, MAX_STRING_LEN);
    for (i = 0; i < OF_COUNT; i++)
        if (OutputFormat & (1 << i))
        {
            strcat(tmp, OutputData[i].Name);
            strcat(tmp, " ");
        }
    if (strlen(tmp))
        tmp[strlen(tmp) - 1] = '\0';
    return tmp;
}

char *if_type(unsigned long InputFormat)
{
    register int i;
    static char tmp[MAX_STRING_LEN];

    bzero(tmp, MAX_STRING_LEN);
    for (i = 0; i < IF_COUNT; i++)
        if (InputFormat & (1 << i))
        {
            strcat(tmp, InputData[i].Type);
            strcat(tmp, "\n");
        }
    if (strlen(tmp))
        tmp[strlen(tmp) - 1] = '\0';
    return tmp;
}

char *of_type(unsigned long OutputFormat)
{
    register int i;
    static char tmp[MAX_STRING_LEN];

    bzero(tmp, MAX_STRING_LEN);
    for (i = 0; i < OF_COUNT; i++)
        if (OutputFormat & (1 << i))
        {
            strcat(tmp, OutputData[i].Type);
            strcat(tmp, "\n");
        }
    if (strlen(tmp))
        tmp[strlen(tmp) - 1] = '\0';
    return tmp;
}

unsigned long if_mask(char *InputName)
{
    register int i;

    for (i = 0; i < IF_COUNT; i++)
        if (!strcasecmp(InputName, InputData[i].Name))
            return (unsigned long)(1 << i);
    return 0;
}

unsigned long of_mask(char *OutputName)
{
    register int i;

    for (i = 0; i < OF_COUNT; i++)
        if (!strcasecmp(OutputName, OutputData[i].Name))
            return (unsigned long)(1 << i);
    return 0;
}
