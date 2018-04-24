/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

void command_interpreter(struct char_data *ch, char *argument);
int search_block(char *arg, char **list, bool exact);
int old_search_block(char *argument,int begin,int length,char **list,int mode);
char lower( char c );
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
char *one_argument(char *argument,char *first_arg);
void only_argument(char *argument,char *first_arg);
int fill_word(char *argument);
void half_chop(char *string, char *arg1, char *arg2);
void nanny(struct descriptor_data *d, char *arg);
int is_abbrev(char *arg1, char *arg2);

struct command_info
{
	void (*command_pointer) (struct char_data *ch, char *argument, int cmd);
	byte minimum_position;
	byte minimum_level;
};

#define MENU         \
"\n\rWelcome to WileyMUD II.\n\r\n\
0) Exit from WileyMud II.\n\r\
1) Enter the game.\n\r\
2) Enter description.\n\r\
3) Read the background story\n\r\
4) Change password.\n\r\
5) See who is playing now.\n\r\n\r\
   Make your choice: "


#define RACEHELP \
"\n\r\
Races:  (Dwarven, Elven, Human, Halfling, Gnome)\n\r\n\r\
Dwarves:   Short. Less Movement.\n\r\
           Best strength and const.\n\r\
	   infravision.\n\r\
Elves:     Taller.  Most Movement.  Numerous Racial Hatreds\n\r\
	   Quicker, more agile.\n\r\
Humans:    Average.\n\r\
Halflings  Short, fast, agile.  Few hatreds\n\r\
	   weak magical ability.\n\r\
Gnomes     Very Short, less movement, few hatreds, strong magical ability.\n\r\
	   infravision.\n\r"

#define GREETINGS \
"
		           Welcome to WileyII			\n\r\n\r\
                  This is a research project into TCP/IP        \n\r\
                  If you would like your site blocked from      \n\r\
                  connecting please email to the address        \n\r\
                  found below.					\n\r\n\r\
		  running with permission on port 3000		\n\r\
		    on wiley.cs.wmich.edu (sun3/60)		\n\r\
		    email: wiley@wiley.cs.wmich.edu		\n\r\n\r\
                     Please read the backgroud story.		\n\r
"
#define WELC_MESSG \
"\n\rWelcome to WileyII. May your visit here be... Enjoyable.\
\n\r\n\r"


#define STORY  \
"Credits must go to :\n\r\
                       cyric\n\r\
                       harlequin\n\r\
                       muth\n\r\
		       grimwell\n\r\
                       derkhil\n\r\
		       elcid\n\r\
and many many more.....\n\r\n\r\
  For all the hard work and redesign of the world and code. Thanks!\n\r\
  If you wish to be part of WileyMudII, please email to wiley@wiley.cs.wmich.edu\n\r"
