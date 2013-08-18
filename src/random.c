#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#ifdef I3
#include "i3.h"
#endif
#define _RANDOM_C
#include "random.h"

/* extern struct descriptor_data *descriptor_list; */

void random_death_message(struct char_data *ch, struct char_data *victim)
{
    char                                    tease[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    perp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     razz = 0;
    int                                     bugcount = 25;
    struct descriptor_data                 *xx = NULL;

    const char                             *bugger[] = {
	"%s shouts '%s is no longer a problem.'\r\n",
	"%s beams to everyone '%s no longer causes lag!'\r\n",
	"%s taunts '%s was too easy!  I need a greater challenge!'\r\n",
	"%s screams 'DIE %s!  So shall ALL perish who dare attack ME!'\r\n",
	"%s asks '%s, is that the BEST you can DO?'\r\n",
	"%s mocks '%s died like a squealing pig!  What a wuss!'\r\n",
	"%s yells 'I spit on the rotting flesh of %s!'\r\n",
	"%s smirks 'So, %s, you come back to lie at my feet again?'\r\n",
	"%s says 'Hope you have a pleasant stay in Hell, %s.'\r\n",
	"%s grins and says 'Good!  I will use %s's intestines for my raft!'\r\n",
	"%s cackles 'Hey %s!  You forgot all your stuff when you DIED!'\r\n",
	"%s grunts 'Ptuey!  %s, your brain is too salty!'\r\n",
	"%s snickers 'Ha!  I bet %s thought a GOD would intervene, eh?'\r\n",
	"%s prays 'Rust in pieces %s.  May your soul be eaten quickly.'\r\n",
	"%s jumps up and down on %s's corpse and shouts 'YES!!!'\r\n",
	"%s licks up the blood of %s and cackles 'You make a wonderful snack!'\r\n",
	"%s sneers 'Oh!  Did you think this was WussyMUD, %s?'\r\n",
	"%s cries 'Sorry %s!  I thought you tried to sell me POISONED salami!'\r\n",
	"%s advises 'Don't quit your day job %s.'\r\n",
	"%s smiles 'Have a nice day %s...'\r\n",
	"%s snickers 'Next time, try HELP CONSIDER %s!'\r\n",
	"%s says 'Damn %s, I thought you were a mosquito!'\r\n",
	"%s smirks 'Get this!  %s tried to KILL me!  Can you believe it?'\r\n",
	"%s laughs 'God!  I didn't even break a sweat %s!\r\n",
	"%s inquires 'Was it a good day to die %s?\r\n",
	NULL
    };
    const char                             *mybugger[] = {
	"You shout '%s is no longer a problem.'\r\n",
	"You beam to everyone '%s no longer causes lag!'\r\n",
	"You taunt '%s was too easy!  I need a greater challenge!'\r\n",
	"You scream 'DIE %s!  So shall ALL perish who dare attack ME!'\r\n",
	"You ask '%s, is that the BEST you can DO?'\r\n",
	"You mock '%s died like a squealing pig!  What a wuss!'\r\n",
	"You yell 'I spit on the rotting flesh of %s!'\r\n",
	"You smirk 'So, %s, you come back to lie at my feet again?'\r\n",
	"You say 'Hope you have a pleasant stay in Hell, %s.'\r\n",
	"You grin and say 'Good!  I will use %s's intestines for my raft!'\r\n",
	"You cackle 'Hey %s!  You forgot all your stuff when you DIED!'\r\n",
	"You grunt 'Ptuey!  %s, your brain is too salty!'\r\n",
	"You snicker 'Ha!  I bet %s thought a GOD would intervene, eh?'\r\n",
	"You pray 'Rust in pieces %s.  May your soul be eaten quickly.\r\n",
	"You jump up and down on %s's corpse and shout 'YES!!!'\r\n",
	"You lick up the blood of %s and cackle 'You make a wonderful snack!'\r\n",
	"You sneer 'Oh!  Did you think this was WussyMUD, %s?\r\n",
	"You cry 'Sorry %s!  I thought you tried to sell me POISONED salami!\r\n",
	"You advise 'Don't quit your day job %s.\r\n",
	"You smile 'Have a nice day %s...\r\n",
	"You snicker 'Next time, try HELP CONSIDER %s!'\r\n",
	"You say 'Damn %s, I thought you were a mosquito!'\r\n",
	"You smirk 'Get this!  %s tried to KILL me!  Can you believe it?'\r\n",
	"You laugh 'God!  I didn't even break a sweat %s!\r\n",
	"You inquire 'Was it a good day to die %s?\r\n",
	NULL
    };

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 SAFE_NAME(victim));

    if (ch == victim) {
        strcpy(perp, "Death");
    } else {
        strcpy(perp, NAME(ch));
    }

    sprintf(tease, bugger[razz = number(0, bugcount - 1)], perp, GET_NAME(victim));
    for (xx = descriptor_list; xx; xx = xx->next)
        if (xx->character != ch && !xx->connected &&
            !IS_SET(xx->character->specials.act, PLR_NOSHOUT) &&
            !IS_SET(xx->character->specials.act, PLR_DEAF))
            act("%s", 0, ch, 0, xx->character, TO_VICT, tease);
    cprintf(victim, "%s", tease);

#ifdef I3
    sprintf(tease, bugger[razz], "", GET_NAME(victim));
    i3_npc_chat("wiley", perp, tease);
#endif

    if (ch != victim) {
        sprintf(tease, mybugger[razz], NAME(victim));
        cprintf(ch, "%s", tease);
    }
}

void random_error_message(struct char_data *ch)
{
    static const char                      *oops[] = {
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
	NULL
    };
    static int                              howmany = 34;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    cprintf(ch, "%s\r\n", oops[number(1, howmany) - 1]);
}

void random_miscast(struct char_data *ch, const char *name)
{
    static const char                      *oops[] = {
	"You just can't seem to concentrate on %s...\r\n",
	"Your %s refuses to work without back-pay.\r\n",
	"You cast your %s aside and decide to go home.\r\n",
	"You proudly watch your %s spell fly away into the sky.\r\n",
	"Quixadhal walks up and grabs your %s, muttering 'Fools!'\r\n",
	"Your %s falls to the ground and shatters!\r\n",
	"I don't remember any %s?  D'oh!\r\n",
	"The warrenty on your %s spell has expired.\r\n",
	"That last spell didn't LOOK like %s...\r\n",
	NULL
    };
    static int                              howmany = 9;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name));

    cprintf(ch, oops[number(1, howmany) - 1], name);
}

void random_magic_failure(struct char_data *ch)
{
    static const char                      *oops[] = {
	"Bylle Grylle Grop Gryf???",
	"Olle Bolle Snop Snyf?",
	"Olle Grylle Bolle Bylle?!?",
	"Gryffe Olle Gnyffe Snop???",
	"Bolle Snylle Gryf Bylle?!!?",
	"A dragon appears and WHAPS you for trying to cast that!",
	"You try to cast, but it makes no sense...",
	"Did you mean cast it in bronze?",
	"Cast away matee's!  The tide's a turnin'!",
	NULL
    };
    static int                              howmany = 9;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    cprintf(ch, "%s\r\n", oops[number(1, howmany) - 1]);
}
