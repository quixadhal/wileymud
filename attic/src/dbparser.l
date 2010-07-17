%{
#define TOK_zero		0
#define TOK_inherit		1
#define TOK_pound		2
#define TOK_end			3
#define TOK_name		4
#define TOK_desc		5
#define TOK_sdesc		6
#define TOK_sect		7
#define TOK_zone		8
#define TOK_flags		9
#define TOK_type		10
#define TOK_exit		11
#define TOK_key			12
#define TOK_goto		13
#define TOK_dark		14
#define TOK_no_mob		15
#define TOK_indoors		16
#define TOK_no_fight		17
#define TOK_no_steal		18
#define TOK_no_summon		19
#define TOK_no_magic		20
#define TOK_private		21
#define TOK_sound1		22
#define TOK_sound2		23
#define TOK_isdoor		24
#define TOK_closed		25
#define TOK_locked		26
#define TOK_secret		27
#define TOK_trapped		28
#define TOK_pickproof		29
#define TOK_inside		30
#define TOK_city		31
#define TOK_field		32
#define TOK_forest		33
#define TOK_hills		34
#define TOK_mountain		35
#define TOK_swim		36
#define TOK_no_swim		37
#define TOK_air			38
#define TOK_underwater		39
#define TOK_north		40
#define TOK_east		41
#define TOK_south		42
#define TOK_west		43	
#define TOK_up			44
#define TOK_down		45
#define TOK_tilde		46
#define TOK_ID			47
#define TOK_cr			48
#define TOK_int			49
#define TOK_elof		50
#define TOK_tele_time		51
#define TOK_tele_targ		52
#define TOK_tele_look		53
#define TOK_no			54
#define TOK_yes			55
#define TOK_off			56
#define TOK_on			57
#define TOK_ex_desc		58

int ROOMcompile = 0;
int LINEcount = 1;
int NESTlevel = 0;
int MYtoken;

struct RET_flags {
  int TOKEN_type;
  int BIT_flag;
} GLOBAL_flags;


int LIST_sector[] = 
{ 
  TOK_inside,
  TOK_city,
  TOK_field,
  TOK_forest,
  TOK_hills,
  TOK_mountain,
  TOK_swim,
  TOK_no_swim,
  TOK_air,
  TOK_underwater,
  -1,
};

char * Sector_names[] = 
{
  "inside",
  "city",
  "field",
  "forest",
  "hills",
  "mountain",
  "swim",
  "no swim",
  "air",
  "underwater",
  0,
};

int LIST_water[] =
{
  TOK_swim,
  TOK_no_swim,
  TOK_air,
  TOK_underwater,
  -1,
};

int LIST_direction[] =
{
  TOK_north,
  TOK_east,
  TOK_south,
  TOK_west,
  TOK_up,
  TOK_down,
  -1,
};

int LIST_reply[] =
{
  TOK_yes,
  TOK_no,
  -1,
};

int LIST_exit_flags[] =
{
  TOK_isdoor,
  TOK_closed,
  TOK_locked,
  TOK_secret,
  TOK_trapped,
  TOK_pickproof,
  -1,
};

char * EXIT_FLAGS_NAMES[] = 
{
  "Isdoor",
  "Closed",
  "Locked",
  "Secret",
  "Trapped",
  "PickProof",
  "\n",
};
int EXIT_FLAGS[] = {1,2,4,8,16,32,-1};

%}
DIGIT   [0-9]
ID      [a-zA-Z][a-zA-Z0-9//]*
%%
{DIGIT}+  		{ return(TOK_int); }
[;;].*			{ printf("%s\n",yytext); }
[::].*			{ }
@			{ return(TOK_inherit); }
#			{ NESTlevel = 1;return(TOK_pound); }
"end"			{ NESTlevel--;return(TOK_end); }
"name "|"name"		{ return(TOK_name); }
"desc "|"desc"|"desc"\n		{ return(TOK_desc); }
"sdesc "|"sdesc"	{ return(TOK_sdesc); }
"sect"			{ return(TOK_sect); }
"zone"			{ return(TOK_zone); }
"flags"			{ return(TOK_flags); }
"type"			{ return(TOK_type); }
"exit"			{ NESTlevel++;return(TOK_exit); }
"key"			{ return(TOK_key); }
"goto"			{ return(TOK_goto); }
"dark"			{ return(TOK_dark); }
"no_mob"		{ return(TOK_no_mob); }
"indoors"		{ return(TOK_indoors); }
"no_fight"		{ return(TOK_no_fight); }
"no_steal"		{ return(TOK_no_steal); }
"no_summon"		{ return(TOK_no_summon) ; }
"no_magic"		{ return(TOK_no_magic); }
"private"		{ return(TOK_private); }
"sound1"		{ LINEcount++;return(TOK_sound1); }
"sound2"		{ LINEcount++;return(TOK_sound2); }
"isdoor"		{ return(TOK_isdoor); }
"closed"		{ return(TOK_closed); }
"locked"		{ return(TOK_locked); }
"secret"		{ return(TOK_secret); }
"trapped"		{ return(TOK_trapped); }
"pickproof"		{ return(TOK_pickproof); }
"inside"		{ return(TOK_inside); }
"city"			{ return(TOK_city); }
"field"			{ return(TOK_field); }
"forest"		{ return(TOK_forest); }
"hills"			{ return(TOK_hills); }
"mountain"		{ return(TOK_mountain); }
"swim"			{ return(TOK_swim); }
"no_swim"		{ return(TOK_no_swim); }
"air"			{ return(TOK_air); }
"underwater"		{ return(TOK_underwater); }
"north"|N|n		{ return(TOK_north); }
"south"|S|s		{ return(TOK_south); }
"east"|E|e		{ return(TOK_east); }
"west"|W|w		{ return(TOK_west); }
"up"|U|u		{ return(TOK_up); }
"down"|D|d		{ return(TOK_down); }
~			{ return(TOK_tilde); }
"->"			{ return(TOK_elof); }
"tele_time"		{ return(TOK_tele_time); }
"tele_targ"		{ return(TOK_tele_targ); }
"tele_look"		{ return(TOK_tele_look); }
"NO"|"no"		{ return(TOK_no); }
"YES"|"yes"		{ return(TOK_yes); }
"OFF"|"off"		{ return(TOK_off); }
"ON"|"on"		{ return(TOK_on); }
"ex_desc"		{ return(TOK_ex_desc); }
{ID}            	{ return(TOK_ID); }
\n	        	{ LINEcount++;}  
[ \t]			{ }
.                       { }
%%
