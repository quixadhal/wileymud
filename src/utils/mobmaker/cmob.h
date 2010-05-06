#ifndef CMOB_H
#define CMOB_H
#include <stdio.h>

#define NUMFLAGS 23 
#define MAX_STR_LENGTH 1500
#define MAX_SKILLS 10

struct dice {
	long num;
	long side;
	long bonus;
	};

class cmob
{
private:
  long virtual_number;  // virtual mob number
  char *name_list; 		// List of names whichh the mob can
				// be accessed for player actions.
  char *short_description;	// String when the monster takes aaction.
  char *long_description;	// String displayed when in the same room.
  char *description;		// SString displayed when the mob is looked
				// at.
  long act_flags;	// Action flags:

  long affection_flags; // describes what a monster is affected by:
  long alignment;		// 1000-350 = good
				// 349-(-349) = neutral
				// (-350)-(-1000) = evil
  long race;			// MOb's race (see following)
  long Class;				// mob's class (undocumented)
  long sex;				// mob's gender:
  long height;				// mob's height
  long weight;				// mob's weight

  dice gold;				// mob's gold

  dice XP;				// experience worth

  long level;				// mob's level

  dice HP;				// HPS:

  long AC;				// mob's Armour Class
  long thaco;				// mob's thaco
  long attacks_per_round;		// attacks per round.
  
  dice damage;				// Damage Roll
  
  long type;		// always 1.
  long immunities, resistance, susceptibility; // see following chart
  dice str, stradd, dex, con, intel, wis; // stats
  long sav1, sav2, sav3, sav4, sav5; // saving throws
  long position;  // mob's position when loaded into the game
  long def_position;
  long sound;  // 
  char *localsound;
  char *distantsound;
 
  long skill_count;
  long skill_num[MAX_SKILLS];
  long skill_learned[MAX_SKILLS];
  long skill_recognized[MAX_SKILLS];


  // Member functions
  char *fread_string(void);
  void set_bit(long &, long);
  void toggle_bit(long &, long);
  long test_bit(long, long);
  void disp_irs(long);
  void disppos(long);

  char *edit_tmp_file_and_load(void);
  int setp(long &, const char *);
  int toggle_irs(long &, const char *);

public:
  cmob(void);
  cmob(const char *);
  ~cmob(void);

  // File Access:
  int load(void);
  int write(const char *);
  int write(void);

  // Integer Variables:
  void set_number(long);
  void set_str(dice);
  void set_stradd(dice);
  void set_dex(dice);
  void set_con(dice);
  void set_intel(dice);
  void set_wis(dice);
  void set_sav1(long);
  void set_sav2(long);
  void set_sav3(long);
  void set_sav4(long);
  void set_sav5(long);
  void set_alignment(long);
  void set_height(long);
  void set_weight(long);
  void set_gold(dice);
  void set_XP(dice);
  void set_level(long);
  void set_AC(long);
  void set_thaco(long);
  void set_attacks_per_round(long);
  void set_HP(dice);
  void set_damage(dice);

  long get_number(void);
  dice *get_str(void);
  dice *get_stradd(void);
  dice *get_dex(void);
  dice *get_con(void);
  dice *get_intel(void);
  dice *get_wis(void);
  long get_sav1(void);
  long get_sav2(void);
  long get_sav3(void);
  long get_sav4(void);
  long get_sav5(void);
  long get_alignment(void);
  long get_height(void);
  long get_weight(void);
  dice *get_gold(void);
  dice *get_XP(void);
  long get_level(void);
  long get_AC(void);
  long get_thaco(void);
  long get_attacks_per_round(void);
  dice *get_HP(void);
  dice *get_damage(void);

  long get_skill_count(void);

  // Display strings:
  void display_name_list(void);
  void display_short_description(void);
  void display_long_description(void);
  void display_description(void);
  void display_race(void);
  void display_sex(void);
  void display_localsound(void);
  void display_distantsound(void);
  void display_avail_sexes(void);
  void display_avail_races(void);
  void display_avail_positions(void);
  void display_avail_classes(void);
  void display_class(void);

  // set strings
  void set_name_list(void);
  void set_short_description(void);
  void set_long_description(void);
  void set_description(void);
  int set_race(const char *);
  int set_sex(const char *);
  int set_position(const char *);
  int set_def_position(const char *);
  int set_class(const char *);

  // flags:
  void cmob::display_avail_flags(void); 
  int toggle_flag(const char *);
  void display_act_flags(void);
  void display_affection_flags(void);

  // Immunities, Resistance, Susceptibility:
  void display_immunities(void);
  void display_resistance(void);
  void display_susceptibility(void);
  int toggle_immunities(const char *);
  int toggle_resistance(const char *);
  int toggle_susceptibility(const char *);
  void display_avail_irs(void);

  // Positions
  void display_position(void);
  void display_def_position(void);

  // areyou?
  int areyou(const char *);

  // sounds:
  int aresounds(void);
  void setsound(void);

  // skills:
  long get_skill_num(long n);
  long get_skill_learned(long n);
  long get_skill_recognized(long n);
  void remove_skill(int);
  void add_skill(int, int, int);


};

#endif

