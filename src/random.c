#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#include "include/global.h"
#include "include/utils.h"
#include "include/comm.h"
#define _RANDOM_C
#include "include/random.h"

extern struct descriptor_data *descriptor_list;

void random_death_message(struct char_data *ch, struct char_data *victim) {
  char tease[256];
  int  razz, bugcount= 25;
  struct descriptor_data *xx;
  char *bugger[] = {
        "%s shouts '%s is no longer a problem.'\n\r",
        "%s beams to everyone '%s no longer causes lag!'\n\r",
        "%s taunts '%s was too easy!  I need a greater challenge!'\n\r",
        "%s screams 'DIE %s!  So shall ALL perish who dare attack ME!'\n\r",
        "%s asks '%s, is that the BEST you can DO?'\n\r",
        "%s mocks '%s died like a squealing pig!  What a wuss!'\n\r",
        "%s yells 'I spit on the rotting flesh of %s!'\n\r",
        "%s smirks 'So, %s, you come back to lie at my feet again?'\n\r",
        "%s says 'Hope you have a pleasant stay in Hell, %s.'\n\r",
        "%s grins and says 'Good!  I will use %s's intestines for my raft!'\n\r",
	"%s cackles 'Hey %s!  You forgot all your stuff when you DIED!'\n\r",
	"%s grunts 'Ptuey!  %s, your brain is too salty!'\n\r",
	"%s snickers 'Ha!  I bet %s thought a GOD would intervene, eh?'\n\r",
	"%s prays 'Rust in pieces %s.  May your soul be eaten quickly.'\n\r",
	"%s jumps up and down on %s's corpse and shouts 'YES!!!'\n\r",
	"%s licks up the blood of %s and cackles 'You make a wonderful snack!'\n\r",
	"%s sneers 'Oh!  Did you think this was WussyMUD, %s?'\n\r",
	"%s cries 'Sorry %s!  I thought you tried to sell me POISONED salami!'\n\r",
	"%s advises 'Don't quit your day job %s.'\n\r",
	"%s smiles 'Have a nice day %s...'\n\r",
	"%s snickers 'Next time, try HELP CONSIDER %s!'\n\r",
	"%s says 'Damn %s, I thought you were a mosquito!'\n\r",
	"%s smirks 'Get this!  %s tried to KILL me!  Can you believe it?'\n\r",
	"%s laughs 'God!  I didn't even break a sweat %s!\n\r",
	"%s inquires 'Was it a good day to die %s?\n\r",
        NULL
  };
  char *mybugger[] = {
        "You shout '%s is no longer a problem.'\n\r",
        "You beam to everyone '%s no longer causes lag!'\n\r",
        "You taunt '%s was too easy!  I need a greater challenge!'\n\r",
        "You scream 'DIE %s!  So shall ALL perish who dare attack ME!'\n\r",
        "You ask '%s, is that the BEST you can DO?'\n\r",
        "You mock '%s died like a squealing pig!  What a wuss!'\n\r",
        "You yell 'I spit on the rotting flesh of %s!'\n\r",
        "You smirk 'So, %s, you come back to lie at my feet again?'\n\r",
        "You say 'Hope you have a pleasant stay in Hell, %s.'\n\r",
        "You grin and say 'Good!  I will use %s's intestines for my raft!'\n\r",
	"You cackle 'Hey %s!  You forgot all your stuff when you DIED!'\n\r",
	"You grunt 'Ptuey!  %s, your brain is too salty!'\n\r",
	"You snicker 'Ha!  I bet %s thought a GOD would intervene, eh?'\n\r",
	"You pray 'Rust in pieces %s.  May your soul be eaten quickly.\n\r",
	"You jump up and down on %s's corpse and shout 'YES!!!'\n\r",
	"You lick up the blood of %s and cackle 'You make a wonderful snack!'\n\r",
	"You sneer 'Oh!  Did you think this was WussyMUD, %s?\n\r",
	"You cry 'Sorry %s!  I thought you tried to sell me POISONED salami!\n\r",
	"You advise 'Don't quit your day job %s.\n\r",
	"You smile 'Have a nice day %s...\n\r",
	"You snicker 'Next time, try HELP CONSIDER %s!'\n\r",
	"You say 'Damn %s, I thought you were a mosquito!'\n\r",
	"You smirk 'Get this!  %s tried to KILL me!  Can you believe it?'\n\r",
	"You laugh 'God!  I didn't even break a sweat %s!\n\r",
	"You inquire 'Was it a good day to die %s?\n\r",
        NULL
  };
  sprintf(tease, bugger[razz= number(0,bugcount-1)],
          (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
          GET_NAME(victim));
  for (xx = descriptor_list; xx; xx = xx->next)
    if (xx->character != ch && !xx->connected &&
        !IS_SET(xx->character->specials.act, PLR_NOSHOUT) &&
        !IS_SET(xx->character->specials.act, PLR_DEAF))
      act(tease, 0, ch, 0, xx->character, TO_VICT);
  cprintf(victim, tease);
  sprintf(tease, mybugger[razz], GET_NAME(victim));
  cprintf(ch, tease);
}

void random_error_message(struct char_data *ch) {
  static char *oops[] = {
"Pardon?",
"I didn't quite catch that one...",
"I'm sorry Dave, I'm afraid I can't let you do that.",
"Quixadhal will spank you for typing that!",
"Connection closed by foreign host",
"NO CARRIER",
"Ummmm.. go away, we already got one.",
"Huh huh huh, what a dork!",
"You WISH you could type...",
"Just type it and get it over with.",
"No.",
"Maybe later...",
"Arglebargle, glop-glyf!?!",
"Quixadhal snickers 'You think THAT will stop me?'",
"The keyboard refuses to let go.",
"Ouch!  Not that key!  Damnit, that one hurts!",
"Stop calling me buttknocker!!!",
"Muidnar cackles 'Nice typo!  Muahahahahaha!!!'",
"I hope YOU understood that, I certainly didn't.",
"Now where did I put that punchcard with the Highstaff macro on it?",
"Muth sighs 'Wiley was alot harder back in MY days.'",
"Isn't the X-Files on or something?",
"I'll pretend you didn't type that.",
"Blah blah blah blah, blah blah, blah...",
"Uhhhh..... Shutup!",
"Yeah, right.",
"I will have to kill you for that!",
"You have insulted the honour of the keyboard!",
"If I talked to you that way, you'd be upset wouldn't you?",
"You hear a faint click behind you.",
"Eli steps up and whispers, 'Use the HOMEROW next time...'",
"I bet you heard that in an AOL commercial.",
"Klackety-Klack-klack-klack-klack-ClickClickClick-klack--klack",
"NOBODY expects the Spammish Inquisition!",
NULL };
  static int howmany= 34;

  cprintf(ch, "%s\n\r", oops[number(1,howmany)-1]);
}

void random_miscast(struct char_data *ch, char *name) {
  static char *oops[] = {
"You just can't seem to concentrate on %s...\n\r",
"Your %s refuses to work without back-pay.\n\r",
"You cast your %s aside and decide to go home.\n\r",
"You proudly watch your %s spell fly away into the sky.\n\r",
"Quixadhal walks up and grabs your %s, muttering 'Fools!'\n\r",
"Your %s falls to the ground and shatters!\n\r",
"I don't remember any %s?  D'oh!\n\r",
"The warrenty on your %s spell has expired.\n\r",
"That last spell didn't LOOK like %s...\n\r",
NULL };
  static int howmany= 9;

  cprintf(ch, oops[number(1,howmany)-1], name);
}

void random_magic_failure(struct char_data *ch) {
  static char *oops[] = {
"Bylle Grylle Grop Gryf???",
"Olle Bolle Snop Snyf?",
"Olle Grylle Bolle Bylle?!?",
"Gryffe Olle Gnyffe Snop???",
"Bolle Snylle Gryf Bylle?!!?",
"A dragon appears and WHAPS you for trying to cast that!",
"You try to cast, but it makes no sense...",
"Did you mean cast it in bronze?",
"Cast away matee's!  The tide's a turnin'!",
NULL };
  static int howmany= 9;

  cprintf(ch, "%s\n\r", oops[number(1,howmany)-1]);
}
