/*
 * ************************************************************************
 * *  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
 * *  Usage: Procedures interpreting user command                            *
 * ************************************************************************* 
 */

void                             command_interpreter(struct char_data *ch, char *argument);
int                              search_block(char *arg, char **list, bool exact);
int                              old_search_block(char *argument, int begin, int length, char **list, int mode);
char                             lower(char c);
void                             argument_interpreter(char *argument, char *first_arg, char *second_arg);
char                            *one_argument(char *argument, char *first_arg);
void                             only_argument(char *argument, char *first_arg);
int                              fill_word(char *argument);
void                             half_chop(char *string, char *arg1, char *arg2);
void                             nanny(struct descriptor_data *d, char *arg);
int                              is_abbrev(char *arg1, char *arg2);

struct command_info {
  void                             (*command_pointer) (struct char_data * ch, char *argument, int cmd);
  byte                             minimum_position;
  byte                             minimum_level;
};

#define MENU \
"\n\r" \
"                      You are within Quixadhal's DeadMUD!\n\r" \
"\n\r" \
"Your choices are controlled.  You are allowed to\n\r" \
"\n\r" \
"    0) Run away.\n\r" \
"    1) Start the Bloodbath.\n\r" \
"    2) Describe yourself.\n\r" \
"    3) Read the imaginativly original background story!\n\r" \
"    4) Be paranoid and change your password.\n\r" \
"    5) Get a list of potential victims.\n\r" \
"or  6) Commit SUICIDE and prove that you're a WIMP!\n\r" \
"\n\r" \
"Choose carefully [0123456]: "

#define SEXMENU \
"What gender are you [MF]? "

#define RACEMENU \
"Choose a race:  (D)warf, (E)lf, (H)uman, hal(F)ling, or (G)nome\n\r" \
"For help, try (?)\n\r" \
"\n\r" \
"Race [DEHFG?]:  "

#define RACEHELP \
"You may choose to claim one of the following races as your heritage:\n\r" \
"\n\r" \
"Dwarves:   Short, stocky little bipeds with beards and an attitude.\n\r" \
"           They tend to move slower than others, but are very strong\n\r" \
"           and can survive almost anything.  They can also see in the\n\r" \
"           darkness of the deepest caverns.\n\r" \
"Elves:     Tall, thin, somewhat arrogent bipeds who move with the grace\n\r" \
"           and speed of the wind.  They are also very intellectual, and\n\r" \
"           study the mysteries of the universe.  Because of their aloof\n\r" \
"           and superior attitude, most other races can't stand them.\n\r" \
"           They can see on the darkest nights.\n\r" \
"Humans:    The generic human is just plain boring.  He has no special\n\r" \
"           talents or abilities.  In fact, he can do just about anything\n\r" \
"           if he tries, which is seldom.\n\r" \
"Halflings: Little people, far shorter than the dwarves.  They tend to\n\r" \
"           anything they get their hands on!  Since they are so quick\n\r" \
"           and nimble, this is quite alot.  They don't care about magic\n\r" \
"           and most other races don't care much about them.\n\r" \
"Gnomes:    Smaller versions of the dwarves.  They are not nearly as\n\r" \
"           tough, but they are quite clever.  They also have a much more\n\r" \
"           pleasant disposition, and so are well liked by others.  They\n\r" \
"           can also see in the dark.\n\r" \
"\n\r" \
"Race [DEHFG?]:  "

#define GREETINGS \
"Welcome to the dismal lands of\n\r" \
"                  ________                     .___ _____   ____ ___________   \n\r" \
"     (_\\_|_|_/_)  \\______ \\   ____ _____     __| _//     \\ |    |   \\______ \\  \n\r" \
"  ____=_(o o)      |    |  \\_/ __ \\\\__  \\   / __ |/  \\ /  \\|    |   /|    |  \\ \n\r" \
" /m00se _\\ /       |    `   \\  ___/ / __ \\_/ /_/ /    Y    \\    |  / |    `   \\\n\r" \
"'+____+/  O       /_______  /\\___  >____  /\\____ \\____|__  /______/ /_______  /\n\r" \
" |\\   |\\                  \\/     \\/     \\/      \\/       \\/                 \\/ \n\r" \
" ^ ^  ^ ^\n\r" \
" Being tested, abused, spammed and crashed on port 4000 of yakko.cs.wmich.edu.\n\r" \
"\n\r" \
"Quixadhal says, \"If you think this is bad, wait 'till you see the REAL game!\"\n\r" \
"\n\r"

#define WELC_MESSG VERSION_STR

#define STORY  \
"     Once upon a time, there was a little mud named Wiley.  He was just\n\r" \
"your typical DikuMUD, and a few people got together to play around with\n\r" \
"some code.  They hacked the code for some length of time, occasionally\n\r" \
"screaming obscenities, or saying things like, \"Huh, huh, huh... This\n\r" \
"bubble sort is COOL!\"  Eventually, they opened it to the mud-starved\n\r" \
"people of WMU and called it Wiley II.  A poor little Sun 3/60 was found\n\r" \
"and managed to keep 30 to 40 people amused with its antics for almost\n\r" \
"two years.  At this point, Cyric the Destroyer took the game down for\n\r" \
"renovations.  There was much anticipation of Wiley III's return, and it\n\r" \
"seemd like we would soon be in for a new experience in mudding!  Alas,\n\r" \
"it was not to be.  For Cyric had lost interest in the project and took\n\r" \
"his Source with him.  The half-finished Wiley III was released to the\n\r" \
"internet via anonymous FTP for a half-hearted month before being buried\n\r" \
"forever.\n\r" \
"    But wait!  An old packrat hacker who went by the subtle name Dread\n\r" \
"Quixadhal, Lord of VI, had snagged a copy and left it buried on his hard\n\r" \
"drive for several years.  After playing with several broken LpMUDs and\n\r" \
"even a different DikuMUD, he dragged the Wiley III source out and forced\n\r" \
"it to learn to like Linux.\n\r" \
"    You have here the result of nearly 4 years of ad-hoc development!\n\r" \
"\n\r" \
"                      Credits to the old team of Wiley II:\n\r" \
"         cyric, harlequin, muth, grimwell, derkhil, elcid and muidnar.\n\r" \
"                          The New Order of Hackers is:\n\r" \
"                    Dread Quixadhal, Muidnar and Malistrum.\n\r" \
"\n\r" \
"                 EMail might be read on mud@yakko.cs.wmich.edu\n\r"

#define SUICIDE_MSG \
"\n\r" \
"                              **** WARNING! ****\n\r" \
"\n\r" \
"        You are about to step into the utter oblivian of SUICIDE!\n\r" \
"        Your name will be forgotten, your possessions and accomplishments\n\r" \
"        will crumble away to dust.  You will become one of the many cowards\n\r" \
"        who lacked the courage to endure the pain and suffering of DeadMUD\n\r" \
"        and its predecessor, WileyMUD II.\n\r" \
"\n\r" \
"If you are positive that you want to do this thing, type \"I want to DIE!\",\n\r" \
"exactly as shown, without the \"'s.  Consider carefully... your very\n\r" \
"reputation depends on it!\n\r" \
"\n\r" \
"Are you certain you want to leave FOREVER? "

#define SUICIDE_DONE \
"\n\r" \
"    You feel a slight shiver run up your spine as your fingers hit the return\n\r" \
"key.  A sense of sudden panic nearly overwhelms you at the thought that\n\r" \
"you may never get to journey through the dark lands of DeadMUD again.  The\n\r" \
"wind seems to chill you a bit more as it begins blowing leaves around your\n\r" \
"feet.  A snapping of twigs hearlds the approaching storm, and a few enormous\n\r" \
"bolts of lightning seem to sound like nails being driven into a heavy wooden\n\r" \
"box.  The sun is setting, and a few flakes of slow hit your face through the\n\r" \
"gusts.  Suddenly, you hear hoofbeats, and a gigantic rider appears before you!\n\r" \
"\n\r" \
"    \"I have come for you.  Are you prepared?\", she asks in a deep resonate\n\r" \
"voice.  Before you can even begin to reply she draws a gigantic bastard sword.\n\r" \
"The dim reddish light of the evening sun seems to avoid the ebony black blade.\n\r" \
"She adds quietly, \"I wish this could be painless for you, but it won't be.\"\n\r" \
"\n\r" \
"    You see her slowly draw back your doom, a slight twitch of her shoulder\n\r" \
"and you hear the whistle of approaching freedom.  A gentle tap, and an odd\n\r" \
"sliding sensation, and you realize the whistling is now air rushing past your\n\r" \
"ears.                        A confused jumble of spinning images...\n\r" \
"            Blood...\n\r" \
"                                Darkness...\n\r" \
"                                                        Eternal."
