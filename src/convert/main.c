/*
 * Welcome to the WileyMUD conversion project!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include "include/getopt.h"
#include <unistd.h>

#include "include/bug.h"
#include "include/formats.h"
#include "include/structs.h"
#define _MAIN_C
#include "include/main.h"

#include "include/utils.h"
#include "include/make_index.h"
#include "include/parse_wiley.h"
#include "include/dump_map.h"
#include "include/dump_report.h"
#include "include/dump_isles.h"
#include "include/dump_fr.h"
#include "include/dump_afk.h"
#include "include/dump_smaug.h"
#include "include/dump_nightmare.h"
//#include "include/dump_ds.h"

char *ProgName;
int Debug = 0;
int Quiet = 0;
int Verbose = 0;
unsigned long InputFormat = 0;
unsigned long OutputFormat = 0;
char *InputDir = "./world";
char *OutputDir = "./output";

int IncludeShortInLong = 0;
int ObviousExits = 1;
int HardReturns = 1;
int OneBigDomain = 0;
int PitchBlack = 0;

void usage (void);
void help (void);
void parse_options (int argc, char **argv);
void config_status(void);

#define USAGE \
"usage:  %s [-dhqv] [-s source] [-r result] -i format -o format [-o format ...]\n"

void usage (void) {
  fprintf(stderr, USAGE, basename(ProgName));
  exit (-1);
}

void help (void) {
  register int i;

  printf("Quixadhal's WileyMUD III conversion program:  %s\n", VERSION);
  printf("\n");
  printf(USAGE, basename(ProgName));
  printf("long options:\n");
  printf("    --debug             - Enable debugging spam.\n");
  printf("    --help              - This helpful help!\n");
  printf("    --quiet             - Run with minimal output (disabled debug).\n");
  printf("    --verbose           - Output blow-by-blow progress reports.\n");
  printf("    --input,\n");
  printf("    --input-format      - Select an appropriate input format to convert from.\n");
  printf("                          Supported formats:\n");
  for(i= 0; i< IF_COUNT; i++)
    printf("                          %-16.16s: %s\n", if_name(1<<i), if_type(1<<i));
  printf("    --output,\n");
  printf("    --output-format     - Select an appropriate output format to dump to.\n");
  printf("                          Supported formats:\n");
  for(i= 0; i< OF_COUNT; i++)
    printf("                          %-16.16s: %s\n", of_name(1<<i), of_type(1<<i));
  printf("                          %-16.16s: All supported formats\n", "all");
  printf("    --result,\n");
  printf("    --destination,\n");
  printf("    --output-directory  - Sets the destination directory to dump to.\n");
  printf("                          Actual data will be in a subdirectory of this.\n");
  printf("                          Default is ./output.\n");
  printf("    --source,\n");
  printf("    --input-directory   - Sets the source directory to read from.\n");
  printf("                          Default is ./world.\n");
  printf("Game options (prefix with \"no-\" to disable):\n");
  printf("    --include-short-in-long       - Includes the short-description of a\n");
  printf("                                    room in the long-description field.\n");
  printf("                                    DEFAULT:  OFF\n");
  printf("    --obvious-exits               - Causes exits to be obvious by default.\n");
  printf("                                    DEFAULT:  ON\n");
  printf("    --hard-returns                - Force formatting to match original,\n");
  printf("                                    otherwise free-form is allowed.\n");
  printf("                                    DEFAULT:  ON\n");
  printf("    --one-big-domain              - Force all zones in source file to be put\n");
  printf("                                    into a single domain.\n");
  printf("                                    DEFAULT:  OFF\n");
  printf("    --pitch-black                 - Make all \"dark\" rooms absolutely dark,\n");
  printf("                                    instead of just indoor-dark.\n");
  printf("                                    DEFAULT:  OFF\n");
  exit (-1);
}

void parse_options (int argc, char **argv) {
  int c;
  /* int digit_optind = 0; */
  unsigned long int mask;

  if (argc < 2)
    usage();
  while (1) {
    /* int this_option_optind = optind ? optind : 1; */
    int option_index = 0;
    static struct option long_options[] = {
      {"debug", 0, 0, 'd'},
      {"help", 0, 0, 'h'},
      {"input", 1, 0, 'i'},
      {"input-format", 1, 0, 'i'},
      {"output", 1, 0, 'o'},
      {"output-format", 1, 0, 'o'},
      {"quiet", 0, 0, 'q'},
      {"result", 1, 0, 'r'},
      {"destination", 1, 0, 'r'},
      {"output-directory", 1, 0, 'r'},
      {"source", 1, 0, 's'},
      {"input-directory", 1, 0, 's'},
      {"verbose", 0, 0, 'v'},
      {"include-short-in-long", 0, 0, 1},
      {"no-include-short-in-long", 0, 0, 1},
      {"obvious-exits", 0, 0, 2},
      {"no-obvious-exits", 0, 0, 2},
      {"hard-returns", 0, 0, 3},
      {"no-hard-returns", 0, 0, 3},
      {"one-big-domain", 0, 0, 4},
      {"no-one-big-domain", 0, 0, 4},
      {"pitch-black", 0, 0, 5},
      {"no-pitch-black", 0, 0, 5},
      {0, 0, 0, 0}
    };

    c = getopt_long(argc, argv, "dhqvi:o:s:r:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 0:
        printf("option %s", long_options[option_index].name);
        if (optarg)
          printf(" with arg %s", optarg);
        printf("\n");
        break;
      case 1:
        if(!strncasecmp(long_options[option_index].name, "no-", 3))
          IncludeShortInLong= 0;
        else
          IncludeShortInLong= 1;
        break;
      case 2:
        if(!strncasecmp(long_options[option_index].name, "no-", 3))
          ObviousExits= 0;
        else
          ObviousExits= 1;
        break;
      case 3:
        if(!strncasecmp(long_options[option_index].name, "no-", 3))
          HardReturns= 0;
        else
          HardReturns= 1;
        break;
      case 4:
        if(!strncasecmp(long_options[option_index].name, "no-", 3))
          OneBigDomain= 0;
        else
          OneBigDomain= 1;
        break;
      case 5:
        if(!strncasecmp(long_options[option_index].name, "no-", 3))
          PitchBlack= 0;
        else
          PitchBlack= 1;
        break;
      case 'd':
        if (Quiet)
          fprintf(stderr, "Debugging is not compatible with Quiet mode.\n");
        else
          Verbose= Debug= 1;
        break;
      case 'h':
        help();
        break;
      case 'i':
        if(!(mask= if_mask(optarg))) {
          fprintf(stderr, "Unknown input format: \"%s\"\n", optarg);
          fprintf(stderr, "Valid choices are: %s\n", if_name(IF_ALL));
          exit(-1);
        } else
          InputFormat = mask;
        break;
      case 'o':
        if(!(strcasecmp(optarg, "all")))
          OutputFormat = OF_ALL;
        else if(!(mask= of_mask(optarg))) {
          fprintf(stderr, "Unknown output format: \"%s\"\n", optarg);
          fprintf(stderr, "Valid choices are: %s\n", of_name(OF_ALL));
          exit(-1);
        } else
          OutputFormat |= mask;
        break;
      case 'q':
        if (Verbose)
          fprintf(stderr, "Verbose flag set, quiet option ignored.\n");
        else
          Quiet= 1;
        if (Debug) {
          Verbose= Debug= 0;
          fprintf(stderr, "Debugging flag is now off.\n");
        }
        break;
      case 'r':
        printf("option r with value `%s'\n", optarg);
        break;
      case 's':
        printf("option s with value `%s'\n", optarg);
        break;
      case 'v':
        if (Quiet)
          fprintf(stderr, "Quiet flag set, verbose option ignored.\n");
        else
          Verbose= 1;
        break;
      case '?':
        break;
      default:
        fprintf(stderr, "?? getopt returned character code %o ??\n", c);
    }
  }

  if (optind < argc) {
    fprintf(stderr, "non-option ARGV-elements: ");
    while (optind < argc)
      fprintf(stderr, "%s ", argv[optind++]);
    fprintf(stderr, "\n");
  }
}

void config_status(void) {
  printf("Quixadhal's WileyMUD III conversion program:  %s\n", VERSION);
  printf("\n");
  printf("Input format:      %s\n", if_name(InputFormat));
  printf("Input directory:   %s\n", InputDir);
  printf("Output formats:    %s\n", of_name(OutputFormat));
  printf("Output directory:  %s\n", OutputDir);
  printf("\n");
  if(!IncludeShortInLong) printf("DON'T ");
  printf("Include short descriptions in long descriptions.\n");
  if(!ObviousExits) printf("DON'T ");
  printf("Make exits Obvious by default.\n");
  if(!HardReturns) printf("DON'T ");
  printf("Keep original line formatting in descriptions.\n");
  if(!OneBigDomain) printf("DON'T ");
  printf("Put all zones into a single meta-domain.\n");
  if(!PitchBlack) printf("DON'T ");
  printf("Make all dark rooms Absolutely Dark.\n");
  printf("\n");
}

int main(int argc, char **argv) {
  char tmp[MAX_LINE_LEN];
  zones *Zones;
  shops *Shops;
  rooms *Rooms;
  objects *Objects;
  mobs *Mobs;
  forest *Layout;

  if (!(ProgName = (char *) strdup(*argv)))
    log_fatal("No memory!");
  parse_options(argc, argv);
  if(!InputFormat)
    log_fatal("No input format specified.");
  config_status();
  sprintf(tmp, "mkdir -p %s", OutputDir);
  system(tmp);

  sprintf(tmp, "%s/%s", InputDir, ZONE_FILE);
  if(!(Zones= load_zones(tmp)))
    log_fatal("Cannot load zone file!");
  sprintf(tmp, "%s/%s", InputDir, SHOP_FILE);
  if(!(Shops= load_shops(tmp)))
    log_fatal("Cannot load shop file!");
  sprintf(tmp, "%s/%s", InputDir, ROOM_FILE);
  if(!(Rooms= load_rooms(tmp, Zones)))
    log_fatal("Cannot load room file!");
  sprintf(tmp, "%s/%s", InputDir, OBJ_FILE);
  if(!(Objects= load_objects(tmp, Zones)))
    log_fatal("Cannot load object file!");
  sprintf(tmp, "%s/%s", InputDir, MOB_FILE);
  if(!(Mobs= load_mobs(tmp, Zones)))
    log_fatal("Cannot load mob file!");

  if(OutputFormat & of_mask("index")) {
    char ack[256];

    sprintf(tmp, "mkdir -p %s/%s", OutputDir, INDEX_SUBDIR);
    system(tmp);
    if(InputFormat & if_mask("wiley")) {
      fprintf(stderr, "Dumping Format %s from %s\n",
             of_type(of_mask("index")),
             if_type(if_mask("wiley")));
      printf("Dumping Format %s from %s\n",
             of_type(of_mask("index")),
             if_type(if_mask("wiley")));
      sprintf(tmp, "%s/%s/%s.idx", OutputDir, INDEX_SUBDIR, ZONE_FILE);
      sprintf(ack, "%s/%s", InputDir, ZONE_FILE);
      make_index(ack, tmp);
      sprintf(tmp, "%s/%s/%s.idx", OutputDir, INDEX_SUBDIR, MOB_FILE);
      sprintf(ack, "%s/%s", InputDir, MOB_FILE);
      make_index(ack, tmp);
      sprintf(tmp, "%s/%s/%s.idx", OutputDir, INDEX_SUBDIR, OBJ_FILE);
      sprintf(ack, "%s/%s", InputDir, OBJ_FILE);
      make_index(ack, tmp);
      sprintf(tmp, "%s/%s/%s.idx", OutputDir, INDEX_SUBDIR, SHOP_FILE);
      sprintf(ack, "%s/%s", InputDir, SHOP_FILE);
      make_index(ack, tmp);
      sprintf(tmp, "%s/%s/%s.idx", OutputDir, INDEX_SUBDIR, ROOM_FILE);
      sprintf(ack, "%s/%s", InputDir, ROOM_FILE);
      make_index(ack, tmp);
    }
    printf("Index files generated.\n");
  }

  if(OutputFormat & of_mask("report")) {
    sprintf(tmp, "mkdir -p %s/%s", OutputDir, REPORT_SUBDIR);
    system(tmp);
    if(InputFormat & if_mask("wiley")) {
      fprintf(stderr, "Dumping Format %s from %s\n",
             of_type(of_mask("report")),
             if_type(if_mask("wiley")));
      printf("Dumping Format %s from %s\n",
             of_type(of_mask("report")),
             if_type(if_mask("wiley")));
      sprintf(tmp, "%s/%s/%s.out", OutputDir, REPORT_SUBDIR, ZONE_FILE);
      make_zone_report(Zones, Rooms, Objects, Mobs, tmp);
      sprintf(tmp, "%s/%s/%s.out", OutputDir, REPORT_SUBDIR, SHOP_FILE);
      make_shop_report(Zones, Rooms, Objects, Mobs, Shops, tmp);
      sprintf(tmp, "%s/%s/%s.out", OutputDir, REPORT_SUBDIR, ROOM_FILE);
      make_room_report(Zones, Rooms, tmp);
      sprintf(tmp, "%s/%s/%s.out", OutputDir, REPORT_SUBDIR, OBJ_FILE);
      make_obj_report(Zones, Objects, tmp);
      sprintf(tmp, "%s/%s/%s.out", OutputDir, REPORT_SUBDIR, MOB_FILE);
      make_mob_report(Zones, Mobs, tmp);
    }
  }

  if(OutputFormat & of_mask("map")) {
    char ppm_template[256];

    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("map")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("map")),
           if_type(InputFormat));
    sprintf(tmp, "mkdir -p %s/%s", OutputDir, MAP_SUBDIR);
    system(tmp);
    Layout= build_map(Zones, Rooms);
    sprintf(tmp, "%s/%s/%s", OutputDir, MAP_SUBDIR, MAP_FILE);
    sprintf(ppm_template, "%s/%s/%s", OutputDir, MAP_SUBDIR, PPM_FILE);
    make_2d_map(Layout, Zones, tmp, ppm_template);
  }

  if(OutputFormat & of_mask("isles")) {
    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("isles")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("isles")),
           if_type(InputFormat));
    sprintf(tmp, "mkdir -p %s/%s", OutputDir, ISLES_SUBDIR);
    system(tmp);
    dump_as_isles(Zones, Rooms, Shops);
  }

  if(OutputFormat & of_mask("afk")) {
    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("afk")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("afk")),
           if_type(InputFormat));
    sprintf(tmp, "mkdir -p %s/%s", OutputDir, AFK_SUBDIR);
    system(tmp);
    dump_as_afk(Zones, Rooms, Shops, Objects, Mobs);
  }

  if(OutputFormat & of_mask("nightmare")) {
    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("nightmare")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("nightmare")),
           if_type(InputFormat));
    dump_as_nightmare(Zones, Rooms, Shops);
  }

  if(OutputFormat & of_mask("fr")) {
    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("fr")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("fr")),
           if_type(InputFormat));
    dump_as_final_realms(Zones, Rooms, Shops);
  }

  if(OutputFormat & of_mask("smaug")) {
    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("smaug")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("smaug")),
           if_type(InputFormat));
    sprintf(tmp, "mkdir -p %s/%s", OutputDir, SMAUG_SUBDIR);
    system(tmp);
    dump_as_smaug(Zones, Rooms, Shops, Objects, Mobs);
  }

  if(OutputFormat & of_mask("ds")) {
    fprintf(stderr, "Dumping Format %s from %s\n",
           of_type(of_mask("ds")),
           if_type(InputFormat));
    printf("Dumping Format %s from %s\n",
           of_type(of_mask("ds")),
           if_type(InputFormat));
    dump_as_dead_souls(Zones, Rooms, Shops);
  }

  return 0;
}
