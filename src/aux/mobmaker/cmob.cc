#ifndef cmob_cpp 
#define cmob_cpp 

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include "cmob.h"

extern FILE *file_mob_in;
extern FILE *file_mob_out;

#define NUMFLAGS 23 
#define MAX_STR_LENGTH 1500
#define MAX_SKILLS 10

// SECTION 1: FLAGS
char *races[] = 
	{
	"HALFBREED",
	"HUMAN",
	"ELVEN",
	"DWARF",
	"HALFLING",
	"GNOME",
	"REPTILE",
	"SPECIAL",
	"LYCANTHROPE",
	"DRAGON",
	"UNDEAD",
	"ORC",
	"INSECT",
	"ARACHNID",
	"DINOSAUR",
	"FISH",
	"BIRD",
	"GIANT",
	"PREDATOR",
	"PARASITE",
	"SLIME",
	"DEMON",
	"SNAKE",
	"HERBIV",
	"TREE",
	"VEGGIE",
	"ELEMENT",
	"PLANAR",
	"DEVIL",
	"GHOST",
	"GOBLIN",
	"TROLL",
	"VEGMAN",
	"MFLAYER",
	"PRIMATE",
	"ANIMAL",
	"FAERY",
	"PLANT",
	"\0"
	};

char *sexes[] = 
	{
	"NEUTRAL",
	"MALE",
	"FEMALE",
	"NEUTRAL",
	"MALE",
	"FEMALE",
	"\0"
	};

int cmob_flags_initialized = 0;

struct 
{
  char *flag;
  long bit;
  int FA;  // flags affected (0 = act_flags, 1 = affection_flags)
} flags_translation[NUMFLAGS];

struct 
{
  char *flag;
  long bit;
  int FA;  // flags affected (0 = act_flags, 1 = affection_flags)
} classes[6];

struct
{
  char *flag;
  long bit;
} imm_types[7];

struct
{
  char *flag;
  long bit;
} positions[4];

void initialize_cmob_flags_translation(void)
{
  if(cmob_flags_initialized == 1) return;  // already done
  cmob_flags_initialized = 1;
  register int i = 0;

  // act_flags
  flags_translation[i].flag = strdup("SPECIALPROC");
  flags_translation[i].bit = 1;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("SENTINEL");
  flags_translation[i].bit = 2;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("SCAVENGER");
  flags_translation[i].bit = 4;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("NICE_THIEF");
  flags_translation[i].bit = 16;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("AGGRESSIVE");
  flags_translation[i].bit = 32;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("STAY_ZONE");
  flags_translation[i].bit = 64;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("WIMPY");
  flags_translation[i].bit = 128;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("HATED");
  flags_translation[i].bit = 256;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("IMMORTAL");
  flags_translation[i].bit = 2048;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("USES_ITEMS");
  flags_translation[i].bit = 131072;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("FIGHTER");
  flags_translation[i].bit = 262144;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("FOOD_PROVIDER");
  flags_translation[i].bit = 524288;
  flags_translation[i].FA = 0;
  flags_translation[++i].flag = strdup("MOUNTABLE");
  flags_translation[i].bit = 2097152;
  flags_translation[i].FA = 0;

 // affection flags: 

  flags_translation[++i].flag = strdup("INVISIBLE");
  flags_translation[i].bit = 2;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("DETECT_I");
  flags_translation[i].bit = 8;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("SENSE_LIFE");
  flags_translation[i].bit = 32;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("SANCTUARY");
  flags_translation[i].bit = 128;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("FLYING");
  flags_translation[i].bit = 2048;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("INFRAVISION");
  flags_translation[i].bit = 32768;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("GILLS");
  flags_translation[i].bit = 65536;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("SNEAK");
  flags_translation[i].bit = 524288;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("HIDE");
  flags_translation[i].bit = 1048576;
  flags_translation[i].FA = 1;
  flags_translation[++i].flag = strdup("CHARM");
  flags_translation[i].bit = 4194304;
  flags_translation[i].FA = 1;

  // Immunities:
  i = 0;
  imm_types[i].flag = strdup("FIRE");
  imm_types[i].bit = 1;
  imm_types[++i].flag = strdup("COLD");
  imm_types[i].bit = 2;
  imm_types[++i].flag = strdup("ELEC");
  imm_types[i].bit = 4;
  imm_types[++i].flag = strdup("ENERGY");
  imm_types[i].bit = 8;
  imm_types[++i].flag = strdup("BLUNT");
  imm_types[i].bit = 16;
  imm_types[++i].flag = strdup("PIERCE");
  imm_types[i].bit = 32;
  imm_types[++i].flag = strdup("SLASH");
  imm_types[i].bit = 64;

  // positions
  i = 0;
  positions[i].flag = strdup("SLEEPING");
  positions[i].bit = 4;
  positions[++i].flag = strdup("RESTING");
  positions[i].bit = 5;
  positions[++i].flag = strdup("SITTING");
  positions[i].bit = 6;
  positions[++i].flag = strdup("STANDING");
  positions[i].bit = 8;


  i = 0;
	classes[i].flag = strdup("MAGE");
	classes[i].bit = 1;
	classes[++i].flag = strdup("CLERIC");
	classes[i].bit = 2;
	classes[++i].flag = strdup("WARRIOR");
	classes[i].bit = 4;
	classes[++i].flag = strdup("THIEF");
	classes[i].bit = 8;
	classes[++i].flag = strdup("RANGER");
	classes[i].bit = 16;
	classes[++i].flag = strdup("DRUID");
	classes[i].bit = 32;
}

// Section 2: Constructors

cmob::cmob()
{
  if(!cmob_flags_initialized) initialize_cmob_flags_translation();
  virtual_number = 0;
  name_list = NULL;
  short_description = long_description = description = 
    localsound = distantsound = NULL;
  act_flags = affection_flags = alignment = 0;
  race = sex = 1;
	Class = 4;
  weight = 250;	 
  height = 198;
  gold.num = gold.side = gold.bonus = 0;
  XP.num = XP.side = XP.bonus = 1;
  level = 1;
  HP.num = HP.side = HP.bonus = 1; 
  AC = 6;
  thaco = 10;
  attacks_per_round = 1;
  damage.num = damage.side = damage.bonus = 3; 
  type = 1;
  immunities = resistance = susceptibility = 0;
  str.num = dex.num = con.num = intel.num = wis.num = 3;
  str.side = dex.side = con.side = intel.side = wis.side = 6;
  str.bonus = dex.bonus = con.bonus = intel.bonus = wis.bonus = 0;
  stradd.num = stradd.side = stradd.bonus = 0;

  sav1 = sav2 = sav3 = sav4 = sav5 = 10;
  def_position = position = 8;
  sound = 0;
  localsound = distantsound = NULL;
}
 
cmob::cmob(const char *input_file_name)
{
  cmob();
  file_mob_in = fopen(input_file_name, "r");
  if(!file_mob_in)
  { cerr << "Fatal Error opening file: " << input_file_name << endl;
    exit(-14);
  }
}

cmob::~cmob(void)
{
  // Well, lots of things may or may not have been allocated, so lets
  // free stuff up.
  if(name_list)
    free(name_list);
  if(short_description)
  	free(short_description);
  if(long_description)
	free(long_description);
  if(localsound)
	free(localsound);
  if(distantsound)
	free(distantsound);
}
  
// Section 3: Integer Variables:

long cmob::get_skill_count(void)
{  return skill_count; }

void cmob::set_number(long num)
{  if(num > -1) virtual_number = num; }
long cmob::get_number(void)
{  return virtual_number; }

void cmob::set_sav1(long n)
{  sav1 = n; }
long cmob::get_sav1(void)
{ return sav1; }

void cmob::set_sav2(long n)
{  sav2 = n; }
long cmob::get_sav2(void)
{  return sav2; }

void cmob::set_sav3(long n)
{  sav3 = n; }
long cmob::get_sav3(void)
{  return sav3; }

void cmob::set_sav4(long n)
{  sav4 = n; }
long cmob::get_sav4(void)
{  return sav4; }

void cmob::set_sav5(long n)
{  sav5 = n; }
long cmob::get_sav5(void)
{  return sav5; }

void cmob::set_alignment(long n)
{ if(n>=-1000 && n<=1000) alignment = n; }
long cmob::get_alignment(void)
{ return alignment; }

void cmob::set_height(long n)
{ if(n>0) height = n; }
long cmob::get_height(void)
{ return height; }

void cmob::set_weight(long n)
{ if(n>0) weight = n; }
long cmob::get_weight(void)
{ return weight; }

void cmob::set_level(long n)
{ if(n>0) level = n; }
long cmob::get_level(void)
{ return level; }

void cmob::set_AC(long n)
{ AC = n; }
long cmob::get_AC(void)
{ return AC; }

void cmob::set_thaco(long n)
{ thaco = n; }
long cmob::get_thaco(void)
{ return thaco; }

void cmob::set_attacks_per_round(long n)
{ if(n>0) attacks_per_round = n; }
long cmob::get_attacks_per_round(void)
{ return attacks_per_round; }

// section 4: dice variables

void copydice(dice& a, dice b)
{  a.num = b.num;  a.side = b.side;  a.bonus = b.bonus; }

void cmob::set_gold(dice d)
{  copydice(gold, d); }
dice *cmob::get_gold(void)
{  return &gold;  }

void cmob::set_XP(dice d)
{  copydice(XP, d); }
dice *cmob::get_XP(void)
{  return &XP;  }

void cmob::set_HP(dice d)
{  copydice(HP, d); }
dice *cmob::get_HP(void)
{  return &HP;  }

void cmob::set_damage(dice d)
{  copydice(damage, d);  }
dice *cmob::get_damage(void)
{  return &damage;  }

void cmob::set_str(dice d)
{  copydice(str, d); }
dice *cmob::get_str(void)
{  return &str; }

void cmob::set_stradd(dice d)
{  copydice(stradd, d); }
dice *cmob::get_stradd(void)
{  return &stradd; }

void cmob::set_dex(dice d)
{  copydice(dex, d);  }
dice *cmob::get_dex(void)
{  return &dex; }

void cmob::set_con(dice d)
{  copydice(con, d); }
dice *cmob::get_con(void)
{  return &con; }

void cmob::set_intel(dice d)
{  copydice(intel, d); }
dice *cmob::get_intel(void)
{  return &intel; }

void cmob::set_wis(dice d)
{  copydice(wis, d); }
dice *cmob::get_wis(void)
{  return &wis; }


// Section 5: File Access
char *cmob::fread_string(void)
{
  char buf[MAX_STR_LENGTH], tmp[MAX_STR_LENGTH];
  char *ack;
  char *rslt;
  register char *point;
  int flag;

  for(register int i = 0; i < MAX_STR_LENGTH; i++)
  {  buf[i] = 0; tmp[i] = 0; }

  do {
    if(!fgets(tmp, MAX_STR_LENGTH, file_mob_in))
    {  cerr << "File read error.\n" << "String so far:\n" << buf; 
       exit(-1);
    }
    ack = tmp;
    if(strlen(ack) + strlen(buf) + 1 > MAX_STR_LENGTH)
    {  ack[MAX_STR_LENGTH - strlen(buf) - 2] = '\0';
       cerr << "String too long in mob #" << virtual_number << endl;
    }
    strcat(buf, ack);

    for(point = buf+strlen(buf)-2; point >= buf && 
				((*point == ' ') || (*point == '\n') || (*point == '\r')
				 || (*point == '\0'));
		point--);
    if(flag=(*point == '~'))
      if(*(buf+strlen(buf)-3) == '\n')
      {
        *(buf+strlen(buf)-2) = '\0';
        *(buf+strlen(buf)-1) = '\0';
      } else 
        *(buf+strlen(buf)-2) = '\0';
    else {
      *(buf+strlen(buf)+1) = '\0';
      *(buf+strlen(buf)) = '\0';
    }
  } while(!flag);
  // do the allocate boogie:
  if(strlen(buf) > 0)
  {
    return strdup(buf);
  }
  return 0;
}


int cmob::load(void)
{
  if(!file_mob_in)
    { cerr << "ERROR!  Attempt to load from unopened file!\n";
      exit(-1);
    }
  
  int i;
  long tmp, tmp2, tmp3, tmp4, tmp5, tmp6;
  char buf[100];
  char letter;

  cmob();

  // Line One:
  fscanf(file_mob_in, "#%i", &i);
  virtual_number = i;
  fscanf(file_mob_in, "\n");
  
  // Line Two:
  name_list = fread_string();
  short_description = fread_string();
  long_description = fread_string();
  description = fread_string();

  // Numeric Data:
  attacks_per_round = 0;
  fscanf(file_mob_in, "%d ", &tmp);
  act_flags = tmp;

  fscanf(file_mob_in, " %d ", &tmp);
  affection_flags = tmp;

  fscanf(file_mob_in, " %d ", &tmp);
  alignment = tmp;

  Class = 1;

  fscanf(file_mob_in, " %c ", &letter);

  switch(letter) {
    case 'W':
    case 'M':
    case 'S':
    {
      if((letter == 'W') || (letter == 'M'))
      {
        fscanf(file_mob_in, " %D ", &tmp);
        attacks_per_round = tmp;
      }
      fscanf(file_mob_in, "\n");
   
      // The new easy monsters:
      str.num = intel.num = wis.num = dex.num = con.num = 14;
      str.side = intel.side = wis.side = dex.side = con.side = 1;
      str.bonus = intel.bonus = wis.bonus = dex.bonus = con.bonus = 0;

      fscanf(file_mob_in, " %D ", &tmp);
      level = tmp;
 
      fscanf(file_mob_in, " %D ", &tmp);
      thaco = tmp;

      fscanf(file_mob_in, " %D ", &tmp);
      AC = tmp;
 
      fscanf(file_mob_in, " %Dd%D+%D ", &tmp, &tmp2, &tmp3);
      HP.num = tmp;  HP.side = tmp2;  HP.bonus = tmp3;

      fscanf(file_mob_in, " %Dd%D+%D ", &tmp, &tmp2, &tmp3);
      damage.num = tmp;  damage.side = tmp2;  damage.bonus = tmp3;

      fscanf(file_mob_in, " %D ", &tmp);
      if (tmp == -1)
      {
	fscanf(file_mob_in, " %D ", &tmp);
      	gold.num = tmp;
        gold.side = 1;
        gold.bonus = 0;
      	fscanf(file_mob_in, " %D ", &tmp);
	XP.num = tmp;
     	XP.side = 1;
	XP.bonus = 0;
 	fscanf(file_mob_in, " %D \n", &tmp);
	race = tmp;
      } else {
	gold.num = tmp;
        gold.side = 1;
        gold.bonus = 1;
	fscanf(file_mob_in, " %D \n", &tmp);
	XP.num = tmp;
	XP.side = 1;
	XP.bonus = 0;
      }

      fscanf(file_mob_in, " %D ", &tmp);
      position = tmp;
      fscanf(file_mob_in, " %D ", &tmp); // this would be def_position
      def_position = tmp;

      fscanf(file_mob_in, " %D ", &tmp); 
      if(tmp<3)
      {
	sex = tmp;
	immunities = resistance = susceptibility = 0;
      } else if(tmp<6) {
	sex = tmp-3;
	fscanf(file_mob_in, " %D ", &tmp);
	immunities = tmp;
	fscanf(file_mob_in, " %D ", &tmp);
	resistance = tmp;
	fscanf(file_mob_in, " %D ", &tmp);
	susceptibility = tmp;
      } else {
	sex = immunities = resistance = susceptibility = 0;
      }
      fscanf(file_mob_in, "\n");

      height = 198;
      weight = 250;

      // read in the sound string:
      if(letter == 'W')
      {
	localsound = fread_string();
	distantsound = fread_string();
      } else {
	localsound = distantsound = 0;
      }
    } break;

    case 'D':
      cerr << "No Support for type D Mobile in number " << virtual_number
	   << endl;
      exit(-2);
      break;

    case 'C': {
      register int x;
      int lvl;
      
      fscanf(file_mob_in, " %d %d %d %d %d", &tmp, &tmp2, &tmp3, &tmp4
					   , &tmp5);
      race = tmp;
      Class = tmp2;
      sex = tmp3;
      height = tmp4;
      weight = tmp5;
  
      fscanf(file_mob_in, " %dd%d+%d ",&tmp, &tmp2, &tmp3);
      gold.num = tmp; gold.side = tmp2; gold.bonus = tmp3;
      fscanf(file_mob_in, "\n");
      fscanf(file_mob_in, " %dd%d+%d ", &tmp,&tmp2,&tmp3);
      XP.num = tmp; XP.side = tmp2; XP.bonus = tmp3;

      fscanf(file_mob_in, " %d ", &tmp);
      level = tmp;
      
      fscanf(file_mob_in, " %dd%d+%d ", &tmp,&tmp2,&tmp3);
      HP.num = tmp;  HP.side = tmp2;  HP.bonus = tmp3;

      fscanf(file_mob_in, " %d %d %d \n", &tmp,&tmp2,&tmp3);
      AC = tmp;
      thaco = tmp2;
      attacks_per_round = tmp3 < 0 ? 1 : tmp3;

      for(x = 0; x < attacks_per_round; x++)
      {
	fscanf(file_mob_in, " %dd%d+%d %d \n",&tmp,&tmp2,&tmp3,&tmp4);
	damage.num = tmp; damage.side = tmp2; damage.bonus = tmp3;
	type = 1;
      }
     
      fscanf(file_mob_in, " %d %d %d \n",&tmp,&tmp2,&tmp3);
      immunities = tmp;
      resistance = tmp2;
      susceptibility = tmp3;

      fscanf(file_mob_in, " %dd%d+%d ",&tmp,&tmp2,&tmp3);
      str.num = tmp; str.side = tmp2; str.bonus = tmp3;
      fscanf(file_mob_in, " %dd%d+%d ",&tmp,&tmp2,&tmp3);
      stradd.num = tmp; stradd.side = tmp2; stradd.bonus = tmp3;
      fscanf(file_mob_in, " %dd%d+%d ",&tmp,&tmp2,&tmp3);
      dex.num = tmp; dex.side = tmp2; dex.bonus = tmp3;
      fscanf(file_mob_in, " %dd%d+%d ",&tmp,&tmp2,&tmp3);
      con.num = tmp; con.side = tmp2; con.bonus = tmp3;
      fscanf(file_mob_in, " %dd%d+%d ",&tmp,&tmp2,&tmp3);
      intel.num = tmp; intel.side = tmp2; intel.bonus = tmp3;
      fscanf(file_mob_in, " %dd%d+%d ",&tmp,&tmp2,&tmp3);
      wis.num = tmp; wis.side = tmp2; wis.bonus = tmp3;

      fscanf(file_mob_in, "\n");
     
      fscanf(file_mob_in, " %d %d %d %d %d\n",&tmp,&tmp2,&tmp3,&tmp4,&tmp5);
      sav1 = tmp; sav2=tmp2; sav3=tmp3; sav4=tmp4; sav5=tmp5;

      fscanf(file_mob_in, "\n");
      
      fscanf(file_mob_in, " %d %d %d %d\n",&tmp,&tmp2,&tmp3,&tmp4);
      position = tmp;  def_position = tmp2;
      if(sound = tmp3)
      {
         localsound = fread_string();
         distantsound = fread_string();
      }
      if(skill_count = tmp4) 
      {
        if(tmp4 > MAX_SKILLS)
					{  cerr << "ERROR!  Too many skills in mob " << virtual_number
									<< endl;
	   				 exit(-4);
        	}
        for(x=0; x<tmp4; x++)
	  if((fscanf(file_mob_in, " %d %d %d\n",&tmp,&tmp2,&tmp3))==3)
	  { skill_num[x] = tmp; skill_learned[x] = tmp2;
	    skill_recognized[x] = tmp3;
          }
      }
    }
    break;
  default:
    fprintf(stderr,"Unknown mob type in mob %d\n",virtual_number);
    exit(-5);
    break;
  }
}
  
int cmob::write(const char *output_file_name)
{
  file_mob_out = fopen(output_file_name, "w");
  return this->write();
}

int cmob::write()
{
  if(!file_mob_out) 
  {
    cerr << "ERROR!  Can't access output file.\n";
    return 0;
  }
  fprintf(file_mob_out,"#%d\n",virtual_number);
  fprintf(file_mob_out,"%s~\n",name_list);
  fprintf(file_mob_out,"%s~\n",short_description);
  fprintf(file_mob_out,"%s\n~\n",long_description);
  fprintf(file_mob_out,"%s\n~\n",description);

  fprintf(file_mob_out,"%d %d %d C\n",act_flags,affection_flags,alignment);

  fprintf(file_mob_out,"%d %d %d %d %d %dd%d+%d\n",
		race, Class, sex, height, weight,
		gold.num, gold.side, gold.bonus);

  fprintf(file_mob_out, "%dd%d+%d %d %dd%d+%d %d %d %d\n",
		XP.num, XP.side, XP.bonus,
		level,
		HP.num, HP.side, HP.bonus,
		AC, thaco, attacks_per_round);

  for(register int i = 0; i < attacks_per_round; i++)
	fprintf(file_mob_out, "%dd%d+%d %d\n",
		damage.num, damage.side, damage.bonus, type);
  fprintf(file_mob_out, "%d %d %d\n", 
	        immunities, resistance, susceptibility);

  // the stats:
  fprintf(file_mob_out, "%dd%d+%d %dd%d+%d %dd%d+%d %dd%d+%d %dd%d+%d %dd%d+%d\n",
	str.num, str.side, str.bonus,
	stradd.num, stradd.side, stradd.bonus,
	dex.num, dex.side, dex.bonus,
	con.num, con.side, con.bonus,
	intel.num, intel.side, intel.bonus,
	wis.num, wis.side, wis.bonus);

  fprintf(file_mob_out, "%d %d %d %d %d\n",
	sav1, sav2, sav3, sav4, sav5);

  fprintf(file_mob_out, "%d %d %d %d\n",
	position, def_position, sound, skill_count);

  if(sound)
  {
    fprintf(file_mob_out,"%s~\n",localsound);
    fprintf(file_mob_out,"%s~\n",distantsound);
  }

  for(
#ifdef HOME
register int 
#endif
       i = 0; i < skill_count; i++)
  {
    fprintf(file_mob_out, "%d %d %d\n",
	skill_num[i], skill_learned[i], skill_recognized[i]);
  }
}



// Section 6: bits
void cmob::set_bit(long &bitfield, long bit)
{  bitfield |= bit; }

void cmob::toggle_bit(long &bitfield, long bit)
{  bitfield ^= bit; }


// Section 7: strings
void cmob::display_name_list(void)
{  if(name_list) cout << name_list; }

void cmob::display_short_description(void)
{ if(short_description) cout << short_description; }

void cmob::display_long_description(void)
{ if(long_description) cout << long_description; }

void cmob::display_description(void)
{ if(description) cout << description; }

void cmob::display_race(void)
{ printf("%s",races[race]); }

void cmob::display_sex(void)
{ printf("%s",sexes[sex]); }

void cmob::display_localsound(void)
{ if(localsound) cout << localsound; }

void cmob::display_distantsound(void)
{ if(distantsound) cout << distantsound; }

void cmob::display_avail_classes(void)
{
		printf("Available classes:\n");
		for(register int i = 0; i < 6; i++)
		{
				printf("%-15s", classes[i].flag);
				if(!((i+1)%4)) printf("\n");
		}
		printf("\n");
}

void cmob::display_avail_races(void)
{
  printf("Available Races:\n");
  for(register int i = 0; races[i][0]; i++)
  {
    printf("%-15s", races[i]);
    if(!((i+1)%4)) printf("\n");
  }
  printf("\n");
}

void cmob::display_avail_sexes(void)
{
  printf("Available Sexes:\n");
  printf("%-15s%-15s%-15s\n","MALE","FEMALE","NEUTRAL");
}

void cmob::display_avail_positions(void)
{
  printf("Available positions:\n");
  printf("%-15s%-15s%-15s%-15s\n",
	positions[0].flag, positions[1].flag,
	positions[2].flag, positions[3].flag);
}


void cmob::set_name_list(void)
{
  char buffer[250];
  cout << endl;
  cout << "Please enter a list of names, seperated by spaces:\n> ";
  cin.getline(buffer, 250);

  if(!buffer) return;

  if(name_list)
    free(name_list); 

  name_list = strdup(buffer);
}

void cmob::set_short_description(void)
{
  char buffer[250];
  cout << endl;
  cout << "Please enter a short description for your mob:\n> ";
  cin.getline(buffer, 250);

  if(!buffer) return;

  if(name_list)
    free(short_description); 

  short_description = strdup(buffer);
}

void cmob::set_long_description(void)
{
  char buffer[250];
  cout << endl;
  cout << "Please enter a long description for your mob:\n> ";
  cin.getline(buffer, 250);

  if(!buffer) return;

  if(name_list)
    free(long_description); 

  long_description = strdup(buffer);
}

char *cmob::edit_tmp_file_and_load(void)
{
  char buffer[30*80];
  int curpos = 0;
  char tmp[100]; 
  char *tmpfilename;
  FILE *tmpfile;
 
  tmpfilename = tempnam(".","MBG");
  //shell to vi
	sprintf(tmp, "vi %s",tmpfilename);
  system(tmp);
  //returned from vi

  tmpfile = fopen(tmpfilename, "r");
  while(!feof(tmpfile))
  {
    buffer[curpos] = fgetc(tmpfile);
    curpos++;
    if(curpos>=30*80)
    { cerr << "Buffer exceeded in edit_tmp_file\n";
      return NULL;
    }
  }
  buffer[curpos-1] = '\0';

  fclose(tmpfile);
 
  unlink(tmpfilename);
  free(tmpfilename);
  return strdup(buffer);
}

void cmob::set_description(void)
{
  char *tmp = edit_tmp_file_and_load();
  if(!tmp) return;
  if(description)
    free(description);
  description = tmp;
}

int cmob::set_race(const char *newrace)
{
  for(register int i = 0; races[i][0]; i++)
  {
    if(!strncasecmp(races[i], newrace, 6))
    {  race = i;  return 0; }
  }
	return 1;
}

int cmob::set_sex(const char *newsex)
{
  for(register int i = 0; i<3; i++)
  {  if(!strncasecmp(newsex, sexes[i], 1))
     {  sex = i; 
        if(immunities || resistance || susceptibility)
	  			sex += 3;
        return 0;
     }
   }
	return 1;
}

int cmob::setp(long &pvar, const char *newpos)
{
  for(register int i = 0;  i<4; i++)
  {
    if(!strncasecmp(positions[i].flag, newpos, 2))
    {  pvar = positions[i].bit;  return 0; }
  }
	return 1;
}

int cmob::set_class(const char *n)
{
		for(register int i = 0; i<6; i++)
		{
				if(!strncasecmp(classes[i].flag, n, 1))
				{
						Class = classes[i].bit;
						return 0;
				}
		}
		return 1;
}

int cmob::set_position(const char *n)
{
  return setp(position, n);
}

int cmob::set_def_position(const char *n)
{
  return setp(def_position, n);
}


// Section 8: Flag Functions
long cmob::test_bit(long field, long bit)
{  return (field & bit); } 

int cmob::toggle_flag(const char *in)
{
  int i;
  for(i = 0; i < NUMFLAGS; i++)
  {
    if(!strncasecmp(in, flags_translation[i].flag, 4))
	break;
  }
  if(i == NUMFLAGS)
			return 1;

  switch(flags_translation[i].FA)
  {
    case 0: act_flags ^= flags_translation[i].bit; break;
    case 1: affection_flags ^= flags_translation[i].bit; break;
  }
	return 0;
}

void cmob::display_avail_flags(void)
{
  printf("\nACTION FLAGS:\n");
  int i = 0;
  while(flags_translation[i].FA == 0)
  {
			printf("%-15s",flags_translation[i].flag);
			if(!((i+1)%4)) printf("\n");
			i++;
  }

  printf("\n\nAffection Flags:\n");
  while(i<NUMFLAGS)
  {
			printf("%-15s",flags_translation[i].flag);
			if(!((i+1)%4)) printf("\n");
			i++;
  }
	printf("\n");
}

void cmob::display_act_flags(void)
{
  int i = 0;  int counter = 0;
  while(flags_translation[i].FA == 0)
  {
    if(test_bit(act_flags, flags_translation[i].bit))
    {
      printf("%-15s", flags_translation[i].flag);
      counter++;
      if(!((counter+1)%4))
	printf("\n");
    }
    i++;
  }
}

void cmob::display_affection_flags(void)
{
  int i = 0;  int counter = 0;
  while(flags_translation[i].FA == 0) i++;

  while(i<NUMFLAGS)
  {
    if(test_bit(affection_flags, flags_translation[i].bit))
    {
      printf("%-15s", flags_translation[i].flag);
      counter++;
      if(!((counter+1)%4))
	printf("\n");
    }
    i++;
  }
}



// Section 9: IRS -- Immuns, Resistance, Sucept...
void cmob::display_avail_irs(void)
{
		for(register int i = 0; i < 7; i++)
		{
				printf("%-10s",imm_types[i].flag);
				if(!((i+1)%4)) printf("\n");
		}
	printf("\n");
}

void cmob::disp_irs(long field)
{
  for(register int i = 0; i < 7; i++)
  {
    if(test_bit(field, imm_types[i].bit))
  	printf("%-10s", imm_types[i].flag);
  }
  printf("\n");
}

void cmob::display_immunities(void)
{
  printf("Immunities:\n");
  disp_irs(immunities);
}

void cmob::display_resistance(void)
{
  printf("Resistance:\n");
  disp_irs(resistance);
}

void cmob::display_susceptibility(void)
{
  printf("Susceptibility:\n");
  disp_irs(susceptibility);
}

int cmob::toggle_irs(long &irs, const char *flag)
{
  register int i;
  for(i = 0; i<7; i++)
  {
    if(!strncasecmp(flag, imm_types[i].flag, 2))
      break;
  }
  if(i==7) return 1;
  irs ^= imm_types[i].bit;
  if((sex < 3) && (susceptibility || immunities || resistance))
    sex += 3;
  return 0;
}

int cmob::toggle_immunities(const char *n)
{ return toggle_irs(immunities, n); }

int cmob::toggle_resistance(const char *n)
{ return toggle_irs(resistance, n); }

int cmob::toggle_susceptibility(const char *n)
{ return toggle_irs(susceptibility, n); }


// Section 10: Positions
void cmob::disppos(long field)
{
  for(register int i = 0; i < 4; i++)
    if(field == positions[i].bit)
	printf("%s", positions[i].flag);
}

void cmob::display_class(void)
{
		for(register int i = 0; i<6; i++)
				if(Class == classes[i].bit)
						printf("%s",classes[i].flag);
}

void cmob::display_position(void)
{
  disppos(position);
}

void cmob::display_def_position(void)
{  
   disppos(def_position);
}



// Section 11: The AREYOU function
int cmob::areyou(const char *compstr)
{
  long compl = atol(compstr);
  if(compl != 0)
    return (compl == virtual_number);

  if((compl == 0) && (virtual_number == 0))
    return 1;

  // at this point, we know compstr is not a number.
  int compstrptr = 0;
  int curpos = 0;
  
  while(name_list[curpos])
  {
    while(toupper(name_list[curpos]) == toupper(compstr[compstrptr]))
    {
      if(name_list[curpos] == '\0' && compstr[compstrptr] == '\0')
	return 1;
      if(name_list[curpos] == ' ' && compstr[compstrptr] == '\0')
        return 1;
      curpos++; compstrptr++;
    }
    while(name_list[curpos] != ' ' && name_list[curpos] != '\0')
	    curpos++;
    while(name_list[curpos] == ' ' && name_list[curpos] != '\0')
				curpos++;

    compstrptr = 0;
  }
  return 0;
}

// Section 12: Sounds
int cmob::aresounds(void)
{ return sound; }

void cmob::setsound(void)
{
  cout << "Press any key to begin editing the local sound...";
  getchar();
  char *tmp = edit_tmp_file_and_load();
  if(!tmp) return;
  if(localsound)
    free(localsound);
  localsound = tmp;
  cout << "Local Sound set to:\n" << localsound;
  cout << "Press any key to begin editing the distant sound...";
  getchar();
  tmp = edit_tmp_file_and_load();
  if(!tmp) return;
  if(distantsound)
    free(distantsound);
  distantsound = tmp;
  sound = 1;
}

// Section 13: Skills
long cmob::get_skill_num(long n)
{ return skill_num[n]; }

long cmob::get_skill_learned(long n)
{ return skill_learned[n]; }

long cmob::get_skill_recognized(long n)
{ return skill_recognized[n]; }

void cmob::add_skill(int num, int learned, int recognized)
{
  skill_num[skill_count] = num;
  skill_learned[skill_count] = learned;
  skill_recognized[skill_count] = recognized;
  skill_count++;
}

void cmob::remove_skill(int skillnum)
{
  register int i;
  for(i = 0; i < skill_count; i++)
    if(skill_num[i] == skillnum)
      break;

  if(i == skill_count)
    return;

  // right now, skill_num[i] is the skill to be removed.
  for(register int c = i; i < skill_count-2; i++)
  {
    skill_num[c] = skill_num[c+1];
    skill_learned[c] = skill_learned[c+1];
    skill_recognized[c] = skill_recognized[c+1];
  }
  skill_count--;
  return;
}

#endif

