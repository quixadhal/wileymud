#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "global.h"
#include "bug.h"
#include "utils.h"

#include "act_comm.h"
#include "act_info.h"
#include "act_move.h"
#include "act_obj.h"
#include "act_off.h"
#include "act_social.h"
#include "act_wiz.h"
#include "board.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "hash.h"
#include "interpreter.h"
#include "mudlimits.h"
#include "modify.h"
#include "multiclass.h"
#include "opinion.h"
#include "reception.h"
#include "shop.h"
#include "spells.h"
#include "spell_parser.h"
#include "fight.h"

#include "breath_weapons.h"
#include "mob_actions.h"
#include "tracking.h"
#include "weather.h" // time_info_data needed
#define _SPEC_PROCS_C
#include "spec_procs.h"

static struct special_proc_entry specials_m[] = {
    {1, puff, "puff (Astral Traveller)"},
    {3, RepairGuy, "RepairGuy (Blacksmith)"},
    {223, ghoul, "Ghoul (monsum three mob)"},
    {234, cleric, "dwarven cleric (monsum four mob)"},
    {236, ghoul, "Ghast (monsum four mob)"},
    {240, snake, "Poisonous toad (monsum five mob)"},
    {248, snake, "Two-headed snake (monsum five mob)"},
    {249, snake, "Rattlesnake (monsum five mob)"},
    {257, magic_user, "Ogre magi (monsum six mob)"},
    {666, zombie_master, "zombie_master (Xenthia)"},
    {667, mosquito, "mosquito (Mosquito)"},
    {1201, magic_user, "magic_user (Goblin Shaman)"},
    {1204, eli_priest, "eli_priest (Eli)"},
    {1206, RangerGuildMaster, "RangerGuildMaster (Grassland Ranger)"},
    {1208, RangerGuildMaster, "RangerGuildMaster (Forest Ranger)"},
    {1601, RangerGuildMaster, "RangerGuildMaster (Mountain Ranger)"},
    {2112, snake, "Rattlesnake (Forest zone?)"},
    {2113, snake, "Black snake (Forest zone?)"},
    {3005, receptionist, "receptionist (Butler)"},
    {3006, NudgeNudge, "NudgeNudge (Villager)"},
    {3008, fido, "fido (Small Dog)"},
    {3012, cleric, "cleric (Yentile)"},
    {3013, magic_user, "magic_user (Undrea)"},
    {3017, cleric, "cleric (Acolyte)"},
    {3020, MageGuildMaster, "MageGuildMaster (Jessica)"},
    {3021, ClericGuildMaster, "ClericGuildMaster (Pastue)"},
    {3022, ThiefGuildMaster, "ThiefGuildMaster (Tate)"},
    {3023, FighterGuildMaster, "FighterGuildMaster (Kark)"},
    {3024, replicant, "replicant (Glowing Squirrel)"},
    {3043, Ned_Nutsmith, "Ned_Nutsmith (Nedrick)"},
    {3044, janitor_eats, "janitor_eats (Barney)"},
    {3045, NudgeNudge, "NudgeNudge (Village Girl)"},
    {3047, shylar_guard, "shylar_guard (Village Guard)"},
    {4006, replicant, "replicant (Glowing Cave Worm)"},
    {4622, magic_user, "magic_user (Xytheus)"},
    {4623, magic_user, "magic_user (Fallath)"},
    {4706, wraith, "wraith (Grimdale)"},
    {5016, cityguard, "cityguard (Highstaff Gate Guard)"},
    {5017, cityguard, "cityguard (Highstaff Guard)"},
    {5027, MageGuildMaster, "MageGuildMaster (Incubad)"},
    {5030, BreathWeapon, "BreathWeapon (Immature Dragon)"},
    {5031, ThiefGuildMaster, "ThiefGuildMaster (Ahkrain)"},
    {5032, FighterGuildMaster, "FighterGuildMaster (Ulrick)"},
    {5033, ClericGuildMaster, "ClericGuildMaster (Delaron)"},
    {5034, receptionist, "receptionist (Maid)"},
    {5052, cityguard, "cityguard (Depository Guard)"},
    {5053, thief, "dark complected man (Highstaff)"},
    {5054, thief, "young thief (Highstaff)"},
    {5068, cityguard, "cityguard (Halfling Militia)"},
    {5069, cityguard, "cityguard (Halfling Captain)"},
    {5078, cleric, "cleric (High Priest)"},
    {5079, magic_user, "magic_user (Arcane Man)"},
    {5080, cityguard, "cityguard (Sector Guard)"},
    {5090, cityguard, "cityguard (Highstaff Captain)"},
    {5091, cityguard, "cityguard (High Sect Guard)"},
    {5436, MageGuildMaster, "MageGuildMaster (Small Man)"},
    {5437, ClericGuildMaster, "ClericGuildMaster (Noble Man)"},
    {5438, FighterGuildMaster, "FighterGuildMaster (Strong Man)"},
    {5439, ThiefGuildMaster, "ThiefGuildMaster (Small Man)"},
    {5440, RangerGuildMaster, "RangerGuildMaster (Bearded Man)"},
    {6107, FighterGuildMaster, "FighterGuildMaster (Large Man)"},
    {6108, ThiefGuildMaster, "ThiefGuildMaster (Small Man)"},
    {6109, MageGuildMaster, "MageGuildMaster (Thin Man)"},
    {6110, ClericGuildMaster, "ClericGuildMaster (Old Priest)"},
    {6111, magic_user, "magic_user (Old Witch)"},
    {6132, janitor_eats, "janitor_eats (Hermit)"},
    {6201, cleric, "cleric (Black-skinned Lizard)"},
    {6209, magic_user, "magic_user (Reddish-skinned Lizard)"},
    {6408, cleric, "cleric (Priest of Aklan)"},
    {6525, FighterGuildMaster, "FighterGuildMaster (Man)"},
    {6526, ClericGuildMaster, "ClericGuildMaster (Man)"},
    {6527, ThiefGuildMaster, "ThiefGuildMaster (Man)"},
    {6528, MageGuildMaster, "MageGuildMaster (Man)"},
    {6529, RangerGuildMaster, "RangerGuildMaster (Man)"},
    {6910, magic_user, "magic_user (Isha)"},
    {7001, Fighter, "Fighter (Drow Warrior)"},
    {7002, cleric, "cleric (Drow Priestess)"},
    {7003, magic_user, "magic_user (Drow Mage)"},
    {7020, snake, "snake (Drider)"},
    {9000, shadow, "shadow (Asgard)"},
    {9002, ghoul, "ghoul (Ghoul)"},
    {9004, wraith, "wraith (Wraith)"},
    {9005, wraith, "wraith (Wight)"},
    {10003, cleric, "cleric (High Priestess of Lovitar)"},
    {10018, receptionist, "receptionist (Alek)"},
    {15004, FighterGuildMaster, "FighterGuildMaster (Big Horn)"},
    {15013, RepairGuy, "RepairGuy (Dunkal)"},
    {15018, ClericGuildMaster, "ClericGuildMaster (Vail)"},
    {15019, ClericGuildMaster, "ClericGuildMaster (Elias)"},
    {15020, RangerGuildMaster, "RangerGuildMaster (Cael)"},
    {15021, ClericGuildMaster, "ClericGuildMaster (Reever)"},
    {15029, MageGuildMaster, "MageGuildMaster (Aahz)"},
    {15036, ThiefGuildMaster, "ThiefGuildMaster (Finnly)"},
    {15037, receptionist, "receptionist (Madam Lorna)"},
    {15052, FighterGuildMaster, "FighterGuildMaster (Swordsmaster)"},
    {15053, ClericGuildMaster, "ClericGuildMaster (Priest)"},
    {15054, GenericGuildMaster, "GenericGuildMaster (Ilrand)"},
    {20016, cleric, "cleric (Priest Giant)"},
    {20904, Karrn, "Karrn (Black Dragon)"},
    {32300, receptionist, "receptionist (Dorn)"},
    {-1, NULL, "NULL (none)"},
};

static struct special_proc_entry specials_o[] = {
    {3, fountain, "fountain (Highstaff Fountain)"},
    {3005, fountain, "fountain (Shylar Barrel)"},
    {3098, board_special, "board (Wizard's Board)"},
    {3099, board_special, "board (Shylar Board)"},
    {5099, board_special, "board (Highstaff Board)"},
    {4734, BerserkerAxe, "BerserkerAxe (Druegar Axe)"},
    {-1, NULL, "NULL (none)"},
};

static struct special_proc_entry specials_r[] = {
    {1014, House, "House (Muidnar's Home)"},
    {1020, House, "House (Quixadhal's Home)"},
    {1021, House, "House (Dirk's Home)"},
    {1025, House, "House (Highlander's Home)"},
    {3035, pet_shops, "pet_shops (Griffith's Abode, Shylar)"},
    {5029, bank, "bank (Highstaff Depository)"},
    {5182, pet_shops, "pet_shops (Companion's Place, Highstaff)"},
    {-1, NULL, "NULL (none)"},
};

/*
 * Special procedures for rooms
 */

char *how_good(int percent_known)
{
    static char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %d", __PRETTY_FUNCTION__, percent_known);

    if (percent_known == 0)
        strcpy(buf, " (not learned)");
    else if (percent_known <= 6)
        strcpy(buf, " (pitiful)");
    else if (percent_known <= 12)
        strcpy(buf, " (awful)");
    else if (percent_known <= 18)
        strcpy(buf, " (incredibly bad)");
    else if (percent_known <= 24)
        strcpy(buf, " (very bad)");
    else if (percent_known <= 30)
        strcpy(buf, " (bad)");
    else if (percent_known <= 36)
        strcpy(buf, " (very poor)");
    else if (percent_known <= 42)
        strcpy(buf, " (poor)");
    else if (percent_known <= 48)
        strcpy(buf, " (below average)");
    else if (percent_known <= 54)
        strcpy(buf, " (average)");
    else if (percent_known <= 60)
        strcpy(buf, " (better than average)");
    else if (percent_known <= 66)
        strcpy(buf, " (fair)");
    else if (percent_known <= 72)
        strcpy(buf, " (very fair)");
    else if (percent_known <= 78)
        strcpy(buf, " (good)");
    else if (percent_known <= 84)
        strcpy(buf, " (very good)");
    else if (percent_known <= 90)
        strcpy(buf, " (Superb)");
    else
        strcpy(buf, " (Master)");

    return (buf);
}

int GainLevel(struct char_data *master, struct char_data *ch, int chclass)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(master), SAFE_NAME(ch), chclass);

    if (GET_LEVEL(ch, chclass) < GetMaxLevel(master) - 20)
    {
        cprintf(ch, "%s snorts, 'I will not teach such a novice!'\r\n", NAME(master));
    }
    else if (GET_LEVEL(ch, chclass) < GetMaxLevel(master) - 10)
    {
        int cost;

        if (GET_EXP(ch) >= titles[chclass][GET_LEVEL(ch, chclass) + 1].exp)
        {
            cost = GOLD_NEEDED(ch, chclass);
            if (GET_LEVEL(ch, chclass) < 8)
            {
                if (GET_LEVEL(ch, chclass) < 4)
                {
                    cprintf(ch, "%s grumbles, 'Newbies... I'm going to STARVE at this rate of discount...'\r\n",
                            NAME(master));
                }
                else
                    cprintf(ch, "%s moans, 'I wish Quixadhal would let me TAX these youngsters...'\r\n", NAME(master));
            }

            if (GET_GOLD(ch) < cost)
            {
                cprintf(ch, "%s gasps, 'WHAT?  You haven't got the mere %d gold I ask?'\r\n", NAME(master), cost);
                cprintf(ch, "%s sniffs, 'Come back when you can pay me.  I won't starve myself for YOUR benefit.'\r\n",
                        NAME(master));
            }
            else
            {
                GET_GOLD(ch) -= cost;
                cprintf(ch, "You raise a level\r\n");
                advance_level(ch, chclass);
                set_title(ch);
                sprintf(buf, "Congratulations %s!  You have earned your %d%s level!", NAME(ch), GET_LEVEL(ch, chclass),
                        ordinal(GET_LEVEL(ch, chclass)));
                do_shout(master, buf, 0);
                return TRUE;
            }
        }
        else
        {
            cprintf(ch, "You haven't got enough experience yet!\r\n");
        }
    }
    else
    {
        cprintf(ch, "I can teach you nothing more %s.\r\n", NAME(ch));
    }
    return FALSE;
}

struct char_data *FindMobInRoomWithFunction(int room, ifuncp func)
{
    struct char_data *temp_char = NULL;
    struct char_data *targ = NULL;

    if (DEBUG > 2)
        log_info("called %s with %d, %08zx", __PRETTY_FUNCTION__, room, (size_t)func);

    if (room > NOWHERE)
    {
        for (temp_char = real_roomp(room)->people; (!targ) && (temp_char); temp_char = temp_char->next_in_room)
            if (IS_MOB(temp_char))
                if (mob_index[temp_char->nr].func == func)
                    targ = temp_char;
    }
    else
    {
        return (0);
    }

    return (targ);
}

int MageGuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    int anumber = 0;
    int i = 0;
    int target = MAGE_LEVEL_IND;
    struct char_data *master = NULL;
    char pagebuf[16384] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if ((cmd != CMD_practice) && (cmd != CMD_gain))
        return FALSE;
    if (!(master = FindMobInRoomWithFunction(ch->in_room, MageGuildMaster)))
        return FALSE;
    if (HasClass(ch, CLASS_MAGIC_USER))
    {
        if (cmd == CMD_gain)
        {
            GainLevel(master, ch, target);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if (!*arg)
        {
            sprintf(pagebuf, "You have got %d practice sessions left.\r\n", ch->specials.pracs);
            sprintf(pagebuf + strlen(pagebuf), "I can teach you these spells:\r\n");
            for (i = 0; i < MAX_SKILLS; i++)
                if (CanCastClass(ch, i, target) && (spell_info[i].min_level[target] <= GetMaxLevel(master) - 10) &&
                    (spell_info[i].min_level[target] >= GetMaxLevel(master) - 20))
                {
                    sprintf(pagebuf + strlen(pagebuf), "[%d] %s %s \r\n", spell_info[i].min_level[target],
                            spell_info[i].name, how_good(ch->skills[i].learned));
                }
            page_string(ch->desc, pagebuf, 1);
            return TRUE;
        }
        if ((anumber = GetSpellByName(arg)) == -1)
        {
            cprintf(ch, "I have never heard of this spell...\r\n");
            return TRUE;
        }
        if (spell_info[anumber].min_level[target] < GetMaxLevel(master) - 20)
        {
            cprintf(ch, "It would be beneath me to teach you such drivel!\r\n");
            return TRUE;
        }
        if (GetMaxLevel(master) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "I have not yet mastered that spell.\r\n");
            return TRUE;
        }
        if (GET_LEVEL(ch, target) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "You are yet too frail to attempt this spell.\r\n");
            return TRUE;
        }
        if (!CanCastClass(ch, anumber, target))
        {
            cprintf(ch, "You do not know of this spell...\r\n");
            return TRUE;
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "You do not seem to be able to practice now.\r\n");
            return TRUE;
        }
        if (ch->skills[anumber].learned >= 95)
        {
            cprintf(ch, "You know %s as well as I!\r\n", spell_info[anumber].name);
            return TRUE;
        }
        cprintf(ch, "You Practice for a while...\r\n");
        ch->specials.pracs--;

        if ((ch->skills[anumber].learned += (int_app[(int)GET_INT(ch)].learn + fuzz(1))) >= 95)
        {
            ch->skills[anumber].learned = 95;
            cprintf(ch, "You are now a master of this art!\r\n");
        }
    }
    else
    {
        cprintf(ch, "Oh.. i bet you think you're a magic user?\r\n");
        return FALSE;
    }
    return TRUE;
}

int ClericGuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    int anumber = 0;
    int i = 0;
    int target = CLERIC_LEVEL_IND;
    struct char_data *master = NULL;
    char pagebuf[16384] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if ((cmd != CMD_practice) && (cmd != CMD_gain))
        return FALSE;
    if (!(master = FindMobInRoomWithFunction(ch->in_room, ClericGuildMaster)))
        return FALSE;
    if (HasClass(ch, CLASS_CLERIC))
    {
        if (cmd == CMD_gain)
        {
            GainLevel(master, ch, target);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if (!*arg)
        {
            sprintf(pagebuf, "You have got %d practice sessions left.\r\n", ch->specials.pracs);
            sprintf(pagebuf + strlen(pagebuf), "I can show you these prayers:\r\n");
            for (i = 0; i < MAX_SKILLS; i++)
                if (CanCastClass(ch, i, target) && (spell_info[i].min_level[target] <= GetMaxLevel(master) - 10) &&
                    (spell_info[i].min_level[target] >= GetMaxLevel(master) - 20))
                {
                    sprintf(pagebuf + strlen(pagebuf), "[%d] %s %s \r\n", spell_info[i].min_level[target],
                            spell_info[i].name, how_good(ch->skills[i].learned));
                }
            page_string(ch->desc, pagebuf, 1);
            return TRUE;
        }
        if ((anumber = GetSpellByName(arg)) == -1)
        {
            cprintf(ch, "I know not of this miracle...\r\n");
            return TRUE;
        }
        if (spell_info[anumber].min_level[target] < GetMaxLevel(master) - 20)
        {
            cprintf(ch, "It would be beneath me to teach you such drivel!\r\n");
            return TRUE;
        }
        if (GetMaxLevel(master) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "I have not yet been granted that ability.\r\n");
            return TRUE;
        }
        if (GET_LEVEL(ch, target) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "You are not yet worthy to ask for this blessing.\r\n");
            return TRUE;
        }
        if (!CanCastClass(ch, anumber, target))
        {
            cprintf(ch, "You do not know of this prayer...\r\n");
            return TRUE;
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "You do not seem to be able to practice now.\r\n");
            return TRUE;
        }
        if (ch->skills[anumber].learned >= 95)
        {
            cprintf(ch, "You know %s as well as I!\r\n", spell_info[anumber].name);
            return TRUE;
        }
        cprintf(ch, "You Practice for a while...\r\n");
        ch->specials.pracs--;

        if ((ch->skills[anumber].learned += (int_app[(int)GET_INT(ch)].learn + fuzz(1))) >= 95)
        {
            ch->skills[anumber].learned = 95;
            cprintf(ch, "Your faith is strong in this art!\r\n");
        }
    }
    else
    {
        cprintf(ch, "You have no faith.\r\n");
        return FALSE;
    }
    return TRUE;
}

int ThiefGuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    int anumber = 0;
    int i = 0;
    int target = THIEF_LEVEL_IND;
    struct char_data *master = NULL;
    char pagebuf[16384] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if ((cmd != CMD_practice) && (cmd != CMD_gain))
        return FALSE;
    if (!(master = FindMobInRoomWithFunction(ch->in_room, ThiefGuildMaster)))
        return FALSE;
    if (HasClass(ch, CLASS_THIEF))
    {
        if (cmd == CMD_gain)
        {
            GainLevel(master, ch, target);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if (!*arg)
        {
            sprintf(pagebuf, "You have got %d practice sessions left.\r\n", ch->specials.pracs);
            sprintf(pagebuf + strlen(pagebuf), "You can practice any of these skills:\r\n");
            for (i = 0; i < MAX_SKILLS; i++)
                if (CanUseClass(ch, i, target) && (spell_info[i].min_level[target] <= GetMaxLevel(master) - 10) &&
                    (spell_info[i].min_level[target] >= GetMaxLevel(master) - 20))
                {
                    sprintf(pagebuf + strlen(pagebuf), "[%d] %s %s \r\n", spell_info[i].min_level[target],
                            spell_info[i].name, how_good(ch->skills[i].learned));
                }
            page_string(ch->desc, pagebuf, 1);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if ((anumber = GetSkillByName(arg)) == -1)
        {
            cprintf(ch, "I have no clue what skill that might be...\r\n");
            return FALSE;
        }
        if (spell_info[anumber].min_level[target] < GetMaxLevel(master) - 20)
        {
            cprintf(ch, "It would be beneath me to teach you such drivel!\r\n");
            return TRUE;
        }
        if (GetMaxLevel(master) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "I have not yet mastered that skill.\r\n");
            return TRUE;
        }
        if (GET_LEVEL(ch, target) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "You are yet too frail to attempt that!\r\n");
            return TRUE;
        }
        if (!CanUseClass(ch, anumber, target))
        {
            cprintf(ch, "You do not know of this skill...\r\n");
            return TRUE;
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "You do not seem to be able to practice now.\r\n");
            return TRUE;
        }
        if (ch->skills[anumber].learned >= 95)
        {
            cprintf(ch, "You know %s as well as I!\r\n", spell_info[anumber].name);
            return TRUE;
        }
        cprintf(ch, "You Practice for a while...\r\n");
        ch->specials.pracs--;
        if ((ch->skills[anumber].learned += (int_app[(int)GET_INT(ch)].learn + fuzz(1))) >= 95)
        {
            ch->skills[anumber].learned = 95;
            cprintf(ch, "You are now a master of this art!\r\n");
        }
    }
    else
    {
        cprintf(ch, "I could have you killed for unsanctioned thieving.\r\n");
        return FALSE;
    }
    return TRUE;
}

int FighterGuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    int anumber = 0;
    int i = 0;
    int target = WARRIOR_LEVEL_IND;
    struct char_data *master = NULL;
    char pagebuf[16384] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if ((cmd != CMD_practice) && (cmd != CMD_gain))
        return FALSE;
    if (!(master = FindMobInRoomWithFunction(ch->in_room, FighterGuildMaster)))
        return FALSE;
    if (HasClass(ch, CLASS_WARRIOR))
    {
        if (cmd == CMD_gain)
        {
            GainLevel(master, ch, target);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if (!*arg)
        {
            sprintf(pagebuf, "You have got %d practice sessions left.\r\n", ch->specials.pracs);
            sprintf(pagebuf + strlen(pagebuf), "You can practice any of these skills:\r\n");
            for (i = 0; i < MAX_SKILLS; i++)
                if (CanUseClass(ch, i, target) && (spell_info[i].min_level[target] <= GetMaxLevel(master) - 10) &&
                    (spell_info[i].min_level[target] >= GetMaxLevel(master) - 20))
                {
                    sprintf(pagebuf + strlen(pagebuf), "[%d] %s %s \r\n", spell_info[i].min_level[target],
                            spell_info[i].name, how_good(ch->skills[i].learned));
                }
            page_string(ch->desc, pagebuf, 1);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if ((anumber = GetSkillByName(arg)) == -1)
        {
            cprintf(ch, "I have no clue what skill that might be...\r\n");
            return FALSE;
        }
        if (spell_info[anumber].min_level[target] < GetMaxLevel(master) - 20)
        {
            cprintf(ch, "It would be beneath me to teach you such drivel!\r\n");
            return TRUE;
        }
        if (GetMaxLevel(master) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "I have not yet mastered that skill.\r\n");
            return TRUE;
        }
        if (GET_LEVEL(ch, target) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "You are yet too frail to attempt that!\r\n");
            return TRUE;
        }
        if (!CanUseClass(ch, anumber, target))
        {
            cprintf(ch, "You do not know of this skill...\r\n");
            return TRUE;
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "You do not seem to be able to practice now.\r\n");
            return TRUE;
        }
        if (ch->skills[anumber].learned >= 95)
        {
            cprintf(ch, "You know %s as well as I!\r\n", spell_info[anumber].name);
            return TRUE;
        }
        cprintf(ch, "You Practice for a while...\r\n");
        ch->specials.pracs--;
        if ((ch->skills[anumber].learned += (int_app[(int)GET_INT(ch)].learn + fuzz(1))) >= 95)
        {
            ch->skills[anumber].learned = 95;
            cprintf(ch, "You are now a master of this art!\r\n");
        }
    }
    else
    {
        cprintf(ch, "A wimp like you?  A Fighter?  Hahahahahaha!\r\n");
        return FALSE;
    }
    return TRUE;
}

int RangerGuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    int anumber = 0;
    int i = 0;
    int target = RANGER_LEVEL_IND;
    struct char_data *master = NULL;
    char pagebuf[16384] = "\0\0\0\0\0\0\0";
    int skill_num = 0;
    int spell_num = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if ((cmd != CMD_practice) && (cmd != CMD_gain))
        return FALSE;
    if (!(master = FindMobInRoomWithFunction(ch->in_room, RangerGuildMaster)))
        return FALSE;
    if (HasClass(ch, CLASS_RANGER))
    {
        if (cmd == CMD_gain)
        {
            GainLevel(master, ch, target);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        if (!*arg)
        {
            sprintf(pagebuf, "You have got %d practice sessions left.\r\n", ch->specials.pracs);
            sprintf(pagebuf + strlen(pagebuf), "You can practice any of these woodsy skills:\r\n");
            for (i = 0; i < MAX_SKILLS; i++)
                if (CanUseClass(ch, i, target) && (spell_info[i].min_level[target] <= GetMaxLevel(master) - 10) &&
                    (spell_info[i].min_level[target] >= GetMaxLevel(master) - 20))
                {
                    sprintf(pagebuf + strlen(pagebuf), "[%d] %s %s \r\n", spell_info[i].min_level[target],
                            spell_info[i].name, how_good(ch->skills[i].learned));
                }
            sprintf(pagebuf + strlen(pagebuf), "Or any of these leaf and twig charms:\r\n");
            for (i = 0; i < MAX_SKILLS; i++)
                if (CanCastClass(ch, i, target) && (spell_info[i].min_level[target] <= GetMaxLevel(master) - 10) &&
                    (spell_info[i].min_level[target] >= GetMaxLevel(master) - 20))
                {
                    sprintf(pagebuf + strlen(pagebuf), "[%d] %s %s \r\n", spell_info[i].min_level[target],
                            spell_info[i].name, how_good(ch->skills[i].learned));
                }
            page_string(ch->desc, pagebuf, 1);
            return TRUE;
        }
        for (; isspace(*arg); arg++)
            ;
        skill_num = GetSkillByName(arg);
        spell_num = GetSpellByName(arg);
        if (skill_num != -1)
            anumber = skill_num;
        else if (spell_num != -1)
            anumber = spell_num;
        else
        {
            cprintf(ch, "I have no clue what you are dreaming of...\r\n");
            return FALSE;
        }
        if (spell_info[anumber].min_level[target] < GetMaxLevel(master) - 20)
        {
            cprintf(ch, "It would be beneath me to teach you such drivel!\r\n");
            return TRUE;
        }
        if (GetMaxLevel(master) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "I'm not really very good at that.\r\n");
            return TRUE;
        }
        if (GET_LEVEL(ch, target) < spell_info[anumber].min_level[target])
        {
            cprintf(ch, "You are yet too frail to attempt that!\r\n");
            return TRUE;
        }
        if (!CanUseClass(ch, anumber, target) && !CanCastClass(ch, anumber, target))
        {
            cprintf(ch, "You know nothing about that...\r\n");
            return TRUE;
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "You do not seem to be able to practice now.\r\n");
            return TRUE;
        }
        if (ch->skills[anumber].learned >= 95)
        {
            cprintf(ch, "You know %s as well as I!\r\n", spell_info[anumber].name);
            return TRUE;
        }
        cprintf(ch, "You Practice for a while...\r\n");
        ch->specials.pracs--;
        if ((ch->skills[anumber].learned += (int_app[(int)GET_INT(ch)].learn + fuzz(1))) >= 95)
        {
            ch->skills[anumber].learned = 95;
            cprintf(ch, "You are now a master of this art!\r\n");
        }
    }
    else
    {
        cprintf(ch, "You have no respect for the forest!\r\n");
        return FALSE;
    }
    return TRUE;
}

int GenericGuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    int anumber = 0;
    int i = 0;
    int percent_chance = 0;
    struct char_data *master = NULL;

    struct skill_struct
    {
        const char skill_name[40];
        int skill_numb;
        int skill_class;
        int skill_lvl;
    };

    struct skill_struct r_skills[] = {{"swimming", SKILL_SWIMMING, CLASS_ALL, 1},
                                      {"bandage", SKILL_BANDAGE, CLASS_ALL, 1},
                                      {"track", SKILL_TRACK, CLASS_WARRIOR | CLASS_MAGIC_USER | CLASS_CLERIC, 3},
                                      {"riding", SKILL_RIDE, CLASS_ALL, 1},
                                      {"read magic", SKILL_READ_MAGIC, CLASS_ALL, 3},
                                      {"endurance", SKILL_ENDURANCE, CLASS_ALL, 3},
                                      {"two handed", SKILL_TWO_HANDED, CLASS_ALL, 5},
                                      {"brew", SKILL_BREW, CLASS_MAGIC_USER | CLASS_CLERIC, 65},
                                      {"scribe", SKILL_SCRIBE, CLASS_MAGIC_USER | CLASS_CLERIC, 65},
                                      {"punch", SKILL_PUNCH, CLASS_CLERIC, 2},
                                      {"barehand", SKILL_BARE_HAND, CLASS_ALL, 1},
                                      {"apraise", SKILL_APRAISE, CLASS_ALL, 2},
                                      {"bartering", SKILL_BARTER, CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_THIEF, 65},
                                      {"spellcraft", SKILL_SPELLCRAFT, CLASS_MAGIC_USER | CLASS_CLERIC, 7},
                                      {"meditation", SKILL_MEDITATION, CLASS_MAGIC_USER | CLASS_CLERIC, 7},
                                      {"\n", -1}};

    const char *rl_skills[] = {"swimming",   "bandage",    "track",      "riding", "read magic", "endurance",
                               "two handed", "brew",       "scribe",     "punch",  "barehand",   "apraise",
                               "bartering",  "spellcraft", "meditation", "\n"};

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if ((cmd != 164) && (cmd != 243))
        return (FALSE);

    master = FindMobInRoomWithFunction(ch->in_room, GenericGuildMaster);

    if (!master)
        return (FALSE);

    if (cmd == 243)
    {
        cprintf(ch, "I cannot train you.. You must find another.\r\n");
        return (TRUE);
    }
    if (!*arg)
    {
        cprintf(ch, "You have got %d practice sessions left.\r\n", ch->specials.pracs);
        cprintf(ch, "You can practice any of these skills:\r\n");
        for (i = 0; r_skills[i].skill_name[0] != '\n'; i++)
        {
            if (r_skills[i].skill_lvl <= GetMaxLevel(ch) || IS_IMMORTAL(ch))
            {
                if ((IS_SET(ch->player.chclass, r_skills[i].skill_class) && GetMaxLevel(ch) >= r_skills[i].skill_lvl) ||
                    IS_IMMORTAL(ch))
                {
                    cprintf(ch, "%s%s\r\n", r_skills[i].skill_name,
                            how_good(ch->skills[r_skills[i].skill_numb].learned));
                }
            }
        }
        return (TRUE);
    }
    else
    {
        for (; isspace(*arg); arg++)
            ;
        anumber = search_block(arg, rl_skills, FALSE);

        if (anumber == -1)
        {
            cprintf(ch, "You do not have ability to practice this skill!\r\n");
            return (TRUE);
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "You do not seem to be able to practice now.\r\n");
            return (TRUE);
        }
        if (anumber != -1)
        {
            if (ch->skills[r_skills[anumber].skill_numb].learned >= 95)
            {
                cprintf(ch, "You are already learned in this area.\r\n");
                return (TRUE);
            }
        }
        if (r_skills[anumber].skill_lvl > GetMaxLevel(ch) && !IS_IMMORTAL(ch))
        {
            cprintf(ch, "You do not know of this skill...\r\n");
            return (TRUE);
        }
        cprintf(ch, "You Practice for a while...\r\n");
        ch->specials.pracs--;

        if (anumber != -1)
        {
            percent_chance = ch->skills[r_skills[anumber].skill_numb].learned + int_app[(int)GET_INT(ch)].learn;
            ch->skills[r_skills[anumber].skill_numb].learned = MIN(95, percent_chance);
        }
        if (anumber != -1)
        {
            if (ch->skills[r_skills[anumber].skill_numb].learned >= 95)
            {
                cprintf(ch, "You are now a master in this area.\r\n");
                return (TRUE);
            }
        }
    }
    return TRUE;
}

int dump(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *k = NULL;
    struct char_data *tmp_char = NULL;
    int value = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    /*
     * char *fname(char *namelist);
     */

    for (k = real_roomp(ch->in_room)->contents; k; k = real_roomp(ch->in_room)->contents)
    {
        for (tmp_char = real_roomp(ch->in_room)->people; tmp_char; tmp_char = tmp_char->next_in_room)
            if (CAN_SEE_OBJ(tmp_char, k))
                cprintf(tmp_char, "The %s vanishes in a puff of smoke.\r\n", fname(k->name));
        extract_obj(k);
    }

    if (cmd != 60)
        return (FALSE);

    do_drop(ch, arg, cmd);

    value = 0;

    for (k = real_roomp(ch->in_room)->contents; k; k = real_roomp(ch->in_room)->contents)
    {
        for (tmp_char = real_roomp(ch->in_room)->people; tmp_char; tmp_char = tmp_char->next_in_room)
            if (CAN_SEE_OBJ(tmp_char, k))
                cprintf(tmp_char, "The %s vanish in a puff of smoke.\r\n", fname(k->name));
        value += (MIN(1000, MAX(k->obj_flags.cost / 4, 1)));
        /*
         * value += MAX(1, MIN(50, k->obj_flags.cost/10));
         */
        extract_obj(k);
    }

    if (value)
    {
        act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

        if (GetMaxLevel(ch) < 3)
            gain_exp(ch, MIN(100, value));
        else
            GET_GOLD(ch) += value;
    }
    return TRUE;
}

int mayor(struct char_data *ch, int cmd, const char *arg)
{
    static const char open_path[] = "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] = "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path = NULL;
    static int path_index = 0;
    static char move = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!move)
    {
        if (time_info.hours == 6)
        {
            move = TRUE;
            path = open_path;
            path_index = 0;
        }
        else if (time_info.hours == 20)
        {
            move = TRUE;
            path = close_path;
            path_index = 0;
        }
    }
    if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) || (GET_POS(ch) == POSITION_FIGHTING))
        return FALSE;

    switch (path[path_index])
    {
    case '0':
    case '1':
    case '2':
    case '3':
        do_move(ch, "", path[path_index] - '0' + 1);
        break;

    case 'W':
        GET_POS(ch) = POSITION_STANDING;
        act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'S':
        GET_POS(ch) = POSITION_SLEEPING;
        act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'a':
        act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'b':
        act("$n says 'What a view! I must get something done about that dump!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'c':
        act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'd':
        act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'e':
        act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'E':
        act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'O':
        do_unlock(ch, "gate", 0);
        do_open(ch, "gate", 0);
        break;

    case 'C':
        do_close(ch, "gate", 0);
        do_lock(ch, "gate", 0);
        break;

    case '.':
        move = FALSE;
        break;
    }

    path_index++;
    return FALSE;
}

struct char_data *find_mobile_here_with_spec_proc(ifuncp fcn, int rnumber)
{
    struct char_data *temp_char = NULL;

    if (DEBUG > 3)
        log_info("called %s with %08zx, %d", __PRETTY_FUNCTION__, (size_t)fcn, rnumber);

    for (temp_char = real_roomp(rnumber)->people; temp_char; temp_char = temp_char->next_in_room)
        if (IS_MOB(temp_char) && mob_index[temp_char->nr].func == fcn)
            return temp_char;
    return NULL;
}

/*
 * General special procedures for mobiles
 */

/* SOCIAL GENERAL PROCEDURES
 *
 * If first letter of the command is '!' this will mean that the following
 * command will be executed immediately.
 *
 * "G",n      : Sets next line to n
 * "g",n      : Sets next line relative to n, fx. line+=n
 * "m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
 * "w",n      : Wake up and set standing (if possible)
 * "c<txt>",n : Look for a person named <txt> in the room
 * "o<txt>",n : Look for an object named <txt> in the room
 * "r<int>",n : Test if the npc in room number <int>?
 * "s",n      : Go to sleep, return false if can't go sleep
 * "e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
 * contents of the **thing
 * "E<txt>",n : Send <txt> to person pointed to by thing
 * "B<txt>",n : Send <txt> to room, except to thing
 * "?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
 * Will as usual advance one line upon sucess, and change
 * relative n lines upon failure.
 * "O<txt>",n : Open <txt> if in sight.
 * "C<txt>",n : Close <txt> if in sight.
 * "L<txt>",n : Lock <txt> if in sight.
 * "U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */

void exec_social(struct char_data *npc, const char *cmd, int next_line, int *cur_line, void **thing)
{
    int ok = FALSE;

    if (DEBUG > 3)
        log_info("called %s with %s, %s, %d, %08zx, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(npc), VNULL(cmd), next_line,
                 (size_t)cur_line, (size_t)thing);

    if (GET_POS(npc) == POSITION_FIGHTING)
        return;

    ok = TRUE;

    switch (*cmd)
    {

    case 'G':
        *cur_line = next_line;
        return;

    case 'g':
        *cur_line += next_line;
        return;

    case 'e':
        act("%s", FALSE, npc, *thing, *thing, TO_ROOM, cmd + 1);
        break;

    case 'E':
        act("%s", FALSE, npc, 0, *thing, TO_VICT, cmd + 1);
        break;

    case 'B':
        act("%s", FALSE, npc, 0, *thing, TO_NOTVICT, cmd + 1);
        break;

    case 'm':
        do_move(npc, "", *(cmd + 1) - '0' + 1);
        break;

    case 'w':
        if (GET_POS(npc) != POSITION_SLEEPING)
            ok = FALSE;
        else
            GET_POS(npc) = POSITION_STANDING;
        break;

    case 's':
        if (GET_POS(npc) <= POSITION_SLEEPING)
            ok = FALSE;
        else
            GET_POS(npc) = POSITION_SLEEPING;
        break;

    case 'c': /* Find char in room */
        *thing = get_char_room_vis(npc, cmd + 1);
        ok = (*thing != 0);
        break;

    case 'o': /* Find object in room */
        *thing = get_obj_in_list_vis(npc, cmd + 1, real_roomp(npc->in_room)->contents);
        ok = (*thing != 0);
        break;

    case 'r': /* Test if in a certain room */
        ok = (npc->in_room == atoi(cmd + 1));
        break;

    case 'O': /* Open something */
        do_open(npc, cmd + 1, 0);
        break;

    case 'C': /* Close something */
        do_close(npc, cmd + 1, 0);
        break;

    case 'L': /* Lock something */
        do_lock(npc, cmd + 1, 0);
        break;

    case 'U': /* UnLock something */
        do_unlock(npc, cmd + 1, 0);
        break;

    case '?': /* Test a random number */
        if (atoi(cmd + 1) <= number(1, 100))
            ok = FALSE;
        break;

    default:
        break;
    } /* End Switch */

    if (ok)
        (*cur_line)++;
    else
        (*cur_line) += next_line;
}

void npc_steal(struct char_data *ch, struct char_data *victim)
{
    int gold = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(victim));

    if (IS_NPC(victim))
        return;
    if (GetMaxLevel(victim) > MAX_MORT)
        return;

    if (AWAKE(victim) && (number(0, GetMaxLevel(ch)) == 0))
    {
        act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
        act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    }
    else
    {
        /*
         * Steal some gold coins
         */
        gold = (int)((GET_GOLD(victim) * number(1, 10)) / 100);
        if (gold > 0)
        {
            GET_GOLD(ch) += gold;
            GET_GOLD(victim) -= gold;
        }
    }
}

int snake(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (GET_POS(ch) != POSITION_FIGHTING)
        return FALSE;

    if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room))
    {
        act("$n poisons $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n poisons you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        return TRUE;
    }
    return FALSE;
}

int ninja_master(struct char_data *ch, int cmd, const char *arg)
{
    static const char *n_skills[] = {
        "track",  /* No. 180 */
        "disarm", /* No. 245 */
        "\n",
    };
    int percent_chance = 0;
    int anumber = 0;
    int charge = 0;
    int sk_num = 0;
    int mult = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!AWAKE(ch))
        return (FALSE);

    for (; *arg == ' '; arg++)
        ; /* ditch spaces */

    if (cmd == 164)
    {
        /*
         * So far, just track
         */
        if (!arg || (strlen(arg) == 0))
        {
            cprintf(ch, " track:   %s\r\n", how_good(ch->skills[SKILL_TRACK].learned));
            cprintf(ch, " disarm:  %s\r\n", how_good(ch->skills[SKILL_DISARM].learned));
            return (TRUE);
        }
        else
        {
            anumber = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
            cprintf(ch, "The ninja master says ");
            if (anumber == -1)
            {
                cprintf(ch, "'I do not know of this skill.'\r\n");
                return (TRUE);
            }
            charge = GetMaxLevel(ch) * 100;
            switch (anumber)
            {
            case 0:
            case 1:
                sk_num = SKILL_TRACK;
                break;
            case 2:
                sk_num = SKILL_DISARM;
                mult = 1;
                if (HasClass(ch, CLASS_MAGIC_USER))
                    mult = 4;
                if (HasClass(ch, CLASS_CLERIC))
                    mult = 3;
                if (HasClass(ch, CLASS_THIEF))
                    mult = 2;
                if (HasClass(ch, CLASS_RANGER))
                    mult = 0;
                if (HasClass(ch, CLASS_DRUID))
                    mult = 1;
                if (HasClass(ch, CLASS_WARRIOR))
                    mult = 1;
                charge *= mult;

                break;
            default:
                log_info("Strangeness in ninjamaster (%d)", anumber);
                return FALSE;
            }
        }
        if (GET_GOLD(ch) < charge)
        {
            cprintf(ch, "'Ah, but you do not have enough money to pay.'\r\n");
            return (FALSE);
        }
        if (ch->skills[sk_num].learned >= 95)
        {
            cprintf(ch, "'You are a master of this art, I can teach you no more.'\r\n");
            return (FALSE);
        }
        if (ch->specials.pracs <= 0)
        {
            cprintf(ch, "'You must first use the knowledge you already have.\r\n");
            return (FALSE);
        }
        GET_GOLD(ch) -= charge;
        cprintf(ch, "'We will now begin.'\r\n");
        ch->specials.pracs--;

        percent_chance = ch->skills[sk_num].learned + int_app[(int)GET_INT(ch)].learn;
        ch->skills[sk_num].learned = MIN(95, percent_chance);

        if (ch->skills[sk_num].learned >= 95)
        {
            cprintf(ch, "'You are now a master of this art.'\r\n");
            return (TRUE);
        }
    }
    else
    {
        return (FALSE);
    }
    return TRUE;
}

int PaladinGuildGuard(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (!cmd)
    {
        if (ch->specials.fighting)
        {
            if (GET_POS(ch) == POSITION_FIGHTING)
            {
                FighterMove(ch);
            }
            else
            {
                StandUp(ch);
            }
        }
    }
    else if (cmd >= 1 && cmd <= 6)
    {
        if (cmd == 4)
            return (FALSE); /* can always go west */
        if (!HasObject(ch, PGShield))
        {
            cprintf(ch, "The guard shakes his head, and blocks your way.\r\n");
            act("The guard shakes his head, and blocks $n's way.", TRUE, ch, 0, 0, TO_ROOM);
            return (TRUE);
        }
    }
    return (FALSE);
}

int AbyssGateKeeper(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);
    if (!cmd)
    {
        if (ch->specials.fighting)
        {
            if (GET_POS(ch) == POSITION_FIGHTING)
            {
                FighterMove(ch);
            }
            else
            {
                StandUp(ch);
            }
        }
    }
    else if ((cmd >= 1 && cmd <= 6) && (!IS_IMMORTAL(ch)))
    {
        if ((cmd == 6) || (cmd == 1))
        {
            cprintf(ch, "The gatekeeper shakes his head, and blocks your way.\r\n");
            act("The guard shakes his head, and blocks $n's way.", TRUE, ch, 0, 0, TO_ROOM);
            return (TRUE);
        }
    }
    return (FALSE);
}

int blink(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (GET_HIT(ch) < (int)GET_MAX_HIT(ch) / 3)
    {
        act("$n blinks.", TRUE, ch, 0, 0, TO_ROOM);
        cast_teleport(12, ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

int Ned_Nutsmith(struct char_data *ch, int cmd, const char *arg)
{
    int cost = 2500;
    struct char_data *vict = NULL;
    struct obj_data *obj = NULL;
    struct obj_data *new_obj = NULL;
    ifuncp neddy = Ned_Nutsmith; /* special procedure for this mob/obj */
    int Obj = 0;
    char obj_name[80] = "\0\0\0\0\0\0\0";
    char vict_name[80] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!AWAKE(ch))
        return (FALSE);

    if (cmd == 72)
    { /* give */
        arg = one_argument(arg, obj_name);
        if (!*obj_name)
        {
            cprintf(ch, "Give what?\r\n");
            return (FALSE);
        }
        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
        {
            cprintf(ch, "Give what?\r\n");
            return (FALSE);
        }
        arg = one_argument(arg, vict_name);
        if (!*vict_name)
        {
            cprintf(ch, "To who?\r\n");
            return (FALSE);
        }
        if (!(vict = get_char_room_vis(ch, vict_name)))
        {
            cprintf(ch, "To who?\r\n");
            return (FALSE);
        }
        /*
         * the target is the repairman, or an NPC
         */

        if (IS_PC(vict))
            return (FALSE);

        if (mob_index[vict->nr].func == neddy)
        {
            act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
            act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
        }
        else
        {
            return (FALSE);
        }

        act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

        if (ITEM_TYPE(obj) == ITEM_FOOD)
        {
            act("$N says 'Hmm, let me see here.'", TRUE, ch, 0, vict, TO_ROOM);
            act("$N says 'Hmm, let me see here.'", TRUE, ch, 0, vict, TO_CHAR);
            if (GET_GOLD(ch) < cost / 10)
            {
                act("$N says 'Ahem! I don't work for free!", TRUE, ch, obj, vict, TO_ROOM);
                act("$N says 'Ahem! I don't work for free!", TRUE, ch, obj, vict, TO_CHAR);
                act("$N says 'It will cost YOU 250 gold for me to even look!", TRUE, ch, obj, vict, TO_ROOM);
                act("$N says 'It will cost YOU 250 gold for me to even look!", TRUE, ch, obj, vict, TO_CHAR);
                return TRUE;
            }
            GET_GOLD(ch) -= cost / 10;
            act("You give $N %d coins.", TRUE, ch, 0, vict, TO_CHAR, cost / 10);
            act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);
            Obj = ObjVnum(obj);
            if (Obj != NUT_NUMBER)
            {
                act("$N says 'Sorry, can't help you!", TRUE, ch, obj, vict, TO_ROOM);
                act("$N says 'Sorry, can't help you!", TRUE, ch, obj, vict, TO_CHAR);
                act("$N gives the $p back.", TRUE, ch, obj, vict, TO_ROOM);
                act("$N gives the $p back.", TRUE, ch, obj, vict, TO_CHAR);
                act("$N says 'Thanks for the donation though!'", TRUE, ch, obj, vict, TO_ROOM);
                act("$N says 'Thanks for the donation though!'", TRUE, ch, obj, vict, TO_CHAR);
                return (TRUE);
            }
            if (GET_GOLD(ch) < cost)
            {
                act("$N says 'HEY, you trying to stiff me!", TRUE, ch, obj, vict, TO_ROOM);
                act("$N says 'HEY, you trying to stiff me!", TRUE, ch, obj, vict, TO_CHAR);
                act("$N says 'It will cost YOU 2500 gold to have me do that!", TRUE, ch, obj, vict, TO_ROOM);
                act("$N says 'It will cost YOU 2500 gold to have me do that!", TRUE, ch, obj, vict, TO_CHAR);
                return TRUE;
            }
            GET_GOLD(ch) -= cost;
            act("You give $N %d coins.", TRUE, ch, 0, vict, TO_CHAR, cost);
            act("$n gives some more money to $N.", TRUE, ch, obj, vict, TO_ROOM);
            extract_obj(obj);
            act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
            act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
            act("$N says 'Well, well, well, a mighty nice nut you have here.'", TRUE, ch, 0, vict, TO_ROOM);
            act("$N says 'Well, well, well, a mighty nice nut you have here.'", TRUE, ch, 0, vict, TO_CHAR);
            act(" KRACK ", TRUE, ch, obj, vict, TO_CHAR);
            act(" KRACK ", TRUE, ch, obj, vict, TO_ROOM);

            /*
             * load new nut here
             */
            new_obj = read_object(NUT_CRACKED_NUMBER, VIRTUAL);
            obj_to_char(new_obj, ch);

            act("$N gives you $p.", TRUE, ch, new_obj, vict, TO_CHAR);
            act("$N gives $p to $n.", TRUE, ch, new_obj, vict, TO_ROOM);
            return (TRUE);
        }
    }
    else
    {
        if (cmd)
            return FALSE;
        return (citizen(ch, cmd, arg));
    }
    return TRUE;
}

int RepairGuy(struct char_data *ch, int cmd, const char *arg)
{
    int cost = 0;
    int ave = 0;
    struct char_data *vict = NULL;
    struct obj_data *obj = NULL;
    ifuncp rep_guy = RepairGuy; /* special procedure for this mob/obj */
    char obj_name[80] = "\0\0\0\0\0\0\0";
    char vict_name[80] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!AWAKE(ch))
        return (FALSE);

    if (cmd == 72)
    { /* give */
        /*
         * determine the correct obj
         */
        arg = one_argument(arg, obj_name);
        if (!*obj_name)
        {
            cprintf(ch, "Give what?\r\n");
            return (FALSE);
        }
        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
        {
            cprintf(ch, "Give what?\r\n");
            return (FALSE);
        }
        arg = one_argument(arg, vict_name);
        if (!*vict_name)
        {
            cprintf(ch, "To who?\r\n");
            return (FALSE);
        }
        if (!(vict = get_char_room_vis(ch, vict_name)))
        {
            cprintf(ch, "To who?\r\n");
            return (FALSE);
        }
        /*
         * the target is the repairman, or an NPC
         */
        if (!IS_NPC(vict))
            return (FALSE);
        if (mob_index[vict->nr].func == rep_guy)
        {
            /*
             * we have the repair guy, and we can give him the stuff
             */
            act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
            act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
        }
        else
        {
            return (FALSE);
        }

        act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

        /*
         * make all the correct tests to make sure that everything is kosher
         */

        if (ITEM_TYPE(obj) == ITEM_ARMOR)
        {
            if ((obj->obj_flags.value[1] == 0) && (obj->obj_flags.value[0] != 0))
            {
                obj->obj_flags.value[1] = obj->obj_flags.value[0];
                act("$N says 'Well, I can't fix it, but I can make it last a little longer.'", TRUE, ch, 0, vict,
                    TO_ROOM);
                act("$N says 'Well, I can't fix it, but I can make it last a little longer.'", TRUE, ch, 0, vict,
                    TO_CHAR);
                act("$N says 'That armor is old, you might want to get some new stuff I can fix later on.'", TRUE, ch,
                    0, vict, TO_ROOM);
                act("$N says 'That armor is old, you might want to get some new stuff I can fix later on.'", TRUE, ch,
                    0, vict, TO_CHAR);
            }
            if (obj->obj_flags.value[1] > obj->obj_flags.value[0])
            {
                /*
                 * get the value of the object
                 */
                cost = obj->obj_flags.cost;
                /*
                 * divide by value[1]
                 */
                cost /= obj->obj_flags.value[1];
                /*
                 * then cost = difference between value[0] and [1]
                 */
                cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]);
                if (GetMaxLevel(vict) > 25) /* super repair guy */
                    cost *= 2;
                if (cost > GET_GOLD(ch))
                {
                    act("$N says 'I'm sorry, you don't have enough money.'", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N says 'I'm sorry, you don't have enough money.'", TRUE, ch, 0, vict, TO_CHAR);
                }
                else
                {
                    GET_GOLD(ch) -= cost;

                    act("You give $N %d coins.", TRUE, ch, 0, vict, TO_CHAR, cost);
                    act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);

                    /*
                     * fix the armor
                     */
                    act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
                    act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
                    if (GetMaxLevel(vict) > 25)
                    {
                        obj->obj_flags.value[0] = obj->obj_flags.value[1];
                    }
                    else
                    {
                        ave = MAX(obj->obj_flags.value[0], (obj->obj_flags.value[0] + obj->obj_flags.value[1]) / 2);
                        obj->obj_flags.value[0] = ave;
                        obj->obj_flags.value[1] = ave;
                    }
                    act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
                }
            }
            else
            {
                act("$N says 'Your armor looks fine to me.'", TRUE, ch, 0, vict, TO_ROOM);
                act("$N says 'Your armor looks fine to me.'", TRUE, ch, 0, vict, TO_CHAR);
            }
        }
        else
        {
            act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_ROOM);
            act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_CHAR);
        }

        act("$N gives you $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
        return (TRUE);
    }
    else
    {
        if (cmd)
            return FALSE;
        return (citizen(ch, cmd, arg));
    }
}

int citizen(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting)
    {
        if (GET_POS(ch) == POSITION_FIGHTING)
        {
            FighterMove(ch);
        }
        else
        {
            StandUp(ch);
        }

        if (!number(0, 18))
        {
            do_shout(ch, "Guards! Help me! Please!", 0);
        }
        else
        {
            act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
        }

        if (ch->specials.fighting)
            CallForGuard(ch, ch->specials.fighting, 3);
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

int shylar_guard(struct char_data *ch, int cmd, const char *arg)
{
    static struct obj_data *i = NULL;
    static struct char_data *tch = NULL;
    static struct char_data *evil = NULL;
    static int max_evil = 0;
    static int dir = 0;
    static int home_room = 3001;

    static int the_path[] = {3001,             /* start in the bar */
                             -1,   -2,   3000, /* taunt eli */
                             -3,   3003,       /* knock on tate's door */
                             -4,   3011,       /* knock on tate */
                             -5,   3049,       /* greet ned */
                             -6,   3041,       /* lick farmer's wife */
                             -7,   3017,       /* french jessica */
                             -8,   3421,       /* check the mansion */
                             3033,             /* steal from pastue's pot */
                             -9,   3147,       /* go past iron gate */
                             3001,             /* go back home to the bar */
                             -10,  -11};
    static int path_index = 0;
    static int move = FALSE;
    static int haslight = FALSE;
    static int saying_count = 14;

    const char *sayings[] = {"$n says 'Hey baby, wanna see my knightstick?'",
                             "$n shouts 'You there!  Stop loitering!'",
                             "$n says 'What's all this then?'",
                             "$n says 'Move along scum!  Bloody peasants!'",
                             "$n asks 'You don't have a doughnut do you?'",
                             "$n yells 'Hey!  Hey!  We'll have none of that in THIS town!'",
                             "$n snickers 'There's plenty of room in the jail for you.'",
                             "$n drools 'Mmmmm.. Doughnut...'",
                             "$n drools 'A nice frosty mug of Duff's Beer!  Mmmmm... Duff's...'",
                             "$n exclaims 'D'oh!'",
                             "$n chants 'Must KILL for Dread Quixadhal...'",
                             "$n screams 'Slow that wagon down!'",
                             "$n yawns and looks really bored.",
                             "$n squishes a bug with his heel and grins.",
                             NULL};
    static int spook_count = 7;

    const char *spooky[] = {"$n shudders and says 'Damn I hate those creepy things!'",
                            "$n shivers and says 'Those dead things give me the creeps!'",
                            "$n backs away and says 'Keep your dead away from me!'",
                            "$n glares and says 'Will you LIE DOWN and be DEAD?  Geez!'",
                            "$n whimpers and cries 'Get away!  I'm allergic to walking dead!'",
                            "$n cries out 'Ahhh!  Save me Eli!  Dead Uncle Joe is coming for me!'",
                            "$n exclaims 'Go back!  Back to the land of Mordor!  errrr... or something.'",
                            NULL};

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || (GET_POS(ch) < POSITION_SLEEPING))
        return FALSE;

    if (AWAKE(ch))
    {
        if (ch->specials.fighting)
        {
            if (GET_POS(ch) == POSITION_FIGHTING)
            {
                FighterMove(ch);
            }
            else
            {
                StandUp(ch);
            }
            if (number(0, 20) > 15)
            {
                do_shout(ch, "Help me Eli!  I don't get paid enough for THIS!!!", 0);
            }
            else
            {
                act("$n shouts 'Help me Eli!  I don't get paid enough for THIS!!!'", TRUE, ch, 0, 0, TO_ROOM);
            }
            if (ch->specials.fighting)
                CallForGuard(ch, ch->specials.fighting, 4);
            return (TRUE);
        }
        else
        {
            if (!haslight)
            {
                if (IS_DARKOUT(ch->in_room))
                    do_grab(ch, "torch", 0);
            }
            else
            {
                if (IS_LIGHTOUT(ch->in_room))
                    do_remove(ch, "torch", 0);
            }
            max_evil = 1000;
            evil = 0;
            for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
            {
                if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch))
                {
                    act("%s", FALSE, ch, 0, 0, TO_ROOM, spooky[number(0, spook_count - 1)]);
                    break;
                }
                if (tch->specials.fighting)
                {
                    if ((GET_ALIGNMENT(tch) < max_evil) && (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
                    {
                        max_evil = GET_ALIGNMENT(tch);
                        evil = tch;
                    }
                }
            }
            if (!check_peaceful(ch, "") && evil && ((GetMaxLevel(evil) >= 4) || IS_NPC(evil)) &&
                (GET_ALIGNMENT(evil->specials.fighting) >= 0))
            {
                act("$n screams 'Get out of my town you bastard!!!'", FALSE, ch, 0, 0, TO_ROOM);
                hit(ch, evil, TYPE_UNDEFINED);
                return (TRUE);
            }
            for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
            {
                if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE))
                {
                    act("$n picks up something and puts it in his pocket.", FALSE, ch, 0, 0, TO_ROOM);
                    obj_from_room(i);
                    obj_to_char(i, ch);
                    return (TRUE);
                }
            }
        }
    }
    if (!move)
    {
        switch (time_info.hours)
        {
        case 6:
        case 12:
        case 18:
        case 21:
            move = TRUE;
            path_index = 0;
            break;
        }
    }
    if (!move)
        return FALSE;
    if (!path_index)
    {
        if (ch->in_room != home_room)
        {
            if (GET_POS(ch) < POSITION_STANDING)
                GET_POS(ch) = POSITION_STANDING;
            // if (0 <= (dir = find_path(ch->in_room, is_target_room_p, (void *)home_room, -200))) {
            if (0 <= (dir = find_path(ch->in_room, is_target_room_p, (void *)(size_t)home_room, -200)))
            {
                go_direction(ch, dir);
                return TRUE;
            }
            else
            {
                act("$n whimpers 'Help me!  I'm lost!.'", FALSE, ch, 0, 0, TO_ROOM);
                mobile_wander(ch);
                return FALSE;
            }
        }
        else
        {
            GET_POS(ch) = POSITION_STANDING;
            act("$n awakens and looks around blearily.", FALSE, ch, 0, 0, TO_ROOM);
            path_index++;
            return TRUE;
        }
    }
    else
    {
        if (the_path[path_index] < 0)
        { /* do special-emote */
            switch (-the_path[path_index])
            {
            case 1:
                act("$n mumbles to Buck 'Strong coffee and a shot of anything.'", FALSE, ch, 0, 0, TO_ROOM);
                act("$n groans and blinks at the room.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 2:
                act("$n drinks his \"coffee\" and perks up.", FALSE, ch, 0, 0, TO_ROOM);
                act("$n says 'Thanks Buck!  Put it on my tab.'", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 3:
                act("$n says 'Eli, Why don't you get a REAL job?'", FALSE, ch, 0, 0, TO_ROOM);
                act("$n shakes his head.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 4:
                act("$n bellows 'Open up in there Tate!  I know you're up to something!'", FALSE, ch, 0, 0, TO_ROOM);
                act("$n bangs on Tate's door until it nearly breaks down.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 5:
                act("$n grabs Tate by the collar and shakes him.", FALSE, ch, 0, 0, TO_ROOM);
                act("$n says 'Tate!  One day I'll catch you and your thieves...'", FALSE, ch, 0, 0, TO_ROOM);
                act("WHAM!  $n slams Tate up against the wall.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 6:
                act("$n snickers 'Hey there Ned!  You still playing with your nuts?'", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 7:
                act("$n says 'I'm glad your husband is out in the fields Maam.'", FALSE, ch, 0, 0, TO_ROOM);
                act("$n licks the Farmer's Wife.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 8:
                act("$n murmers 'Let me show you some REAL magic Jessica.'", FALSE, ch, 0, 0, TO_ROOM);
                act("$n frenches Jessica and makes a charmed longstaff.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 9:
                act("$n mumbles a prayer to Dread Quixadhal.", FALSE, ch, 0, 0, TO_ROOM);
                act("$n slips a few coins from the Donation Pot.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 10:
                act("$n exclaims 'Another hard day's work done!'", FALSE, ch, 0, 0, TO_ROOM);
                act("$n orders several firebreathers and begins to down them.", FALSE, ch, 0, 0, TO_ROOM);
                break;
            case 11:
                act("$n yawns mightily and passes out at the bar.", FALSE, ch, 0, 0, TO_ROOM);
                GET_POS(ch) = POSITION_SLEEPING;
                move = FALSE;
                path_index = 0;
                return TRUE;
            }
            path_index++;
            return TRUE;
        }
        else
        { /* go to room number */
            if (number(0, 99) > 55)
            {
                if (ch->in_room != the_path[path_index])
                {
                    // if (0 <= (dir = find_path(ch->in_room, is_target_room_p, (void *)(the_path[path_index]), -200)))
                    //
                    // {
                    if (0 <= (dir = find_path(ch->in_room, is_target_room_p,
                                              (void *)(size_t)(the_path[path_index]), -200)))
                    {
                        go_direction(ch, dir);
                        return TRUE;
                    }
                    else
                    {
                        act("$n whimpers 'Help me!  I'm lost!.'", FALSE, ch, 0, 0, TO_ROOM);
                        mobile_wander(ch);
                        return FALSE;
                    }
                }
                else
                {
                    path_index++;
                }
            }
            else if (number(0, 99) > 50)
            { /* random emote */
                act("%s", FALSE, ch, 0, 0, TO_ROOM, sayings[number(0, saying_count - 1)]);
            }
        }
    }
    return FALSE;
}

int ghoul(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tar = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    tar = ch->specials.fighting;
    if (tar && (tar->in_room == ch->in_room))
    {
        if ((!IS_AFFECTED(tar, AFF_PROTECT_EVIL)) && (!IS_AFFECTED(tar, AFF_SANCTUARY)))
        {
            act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
            act("$n touches you!", 1, ch, 0, tar, TO_VICT);
            if (!IS_AFFECTED(tar, AFF_PARALYSIS))
            {
                cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tar, 0);
                return TRUE;
            }
        }
    }
    return FALSE;
}

int WizardGuard(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tch = NULL;
    struct char_data *evil = NULL;
    int max_evil = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting)
    {
        if (GET_POS(ch) == POSITION_FIGHTING)
        {
            FighterMove(ch);
        }
        else
        {
            StandUp(ch);
        }
        CallForGuard(ch, ch->specials.fighting, 9);
    }
    max_evil = 1000;
    evil = 0;

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (tch->specials.fighting)
        {
            if ((GET_ALIGNMENT(tch) < max_evil) && (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
            {
                max_evil = GET_ALIGNMENT(tch);
                evil = tch;
            }
        }
    }

    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) && !check_peaceful(ch, ""))
    {
        act("$n screams 'DEATH!!!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }
    return (FALSE);
}

int vampire(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room))
    {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room))
        {
            cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        }
        return TRUE;
    }
    return FALSE;
}

int wraith(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room))
    {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        return TRUE;
    }
    return FALSE;
}

int shadow(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room))
    {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        if (ch->specials.fighting)
        {
            cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
            if (number(1, 5) == 3)
                cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        }
        return TRUE;
    }
    return FALSE;
}

int geyser(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (number(0, 3) == 0)
    {
        act("You erupt.", 1, ch, 0, 0, TO_CHAR);
        cast_geyser(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
        return (TRUE);
    }
    return FALSE;
}

int green_slime(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *cons = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
        if ((!IS_NPC(cons)) && (GetMaxLevel(cons) < LOW_IMMORTAL))
            cast_green_slime(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, cons, 0);
    return TRUE;
}

int DracoLich(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    return FALSE;
}

int Drow(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    return FALSE;
}

int Leader(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    return FALSE;
}

int thief(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *cons = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (GET_POS(ch) != POSITION_STANDING)
        return FALSE;

    for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
        if ((!IS_NPC(cons)) && (GetMaxLevel(cons) < LOW_IMMORTAL) && (number(1, 5) == 1))
            npc_steal(ch, cons);

    return TRUE;
}

int magic_user(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;
    int lspell = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (!ch->specials.fighting)
        return FALSE;

    if ((GET_POS(ch) > POSITION_STUNNED) && (GET_POS(ch) < POSITION_FIGHTING))
    {
        StandUp(ch);
        return (TRUE);
    }
    /*
     * Find a dude to to evil things upon !
     */

    vict = FindVictim(ch);

    if (!vict)
        vict = ch->specials.fighting;

    if (!vict)
        return (FALSE);

    lspell = number(0, GetMaxLevel(ch)); /* gen number from 0 to level */

    if (lspell < 1)
        lspell = 1;

    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 10))
    {
        say_spell(ch, SPELL_CURE_BLIND);
        cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return TRUE;
    }
    if (IS_AFFECTED(ch, AFF_BLIND))
        return (FALSE);

    if ((vict != ch->specials.fighting) && (lspell > 13) && (number(0, 7) == 0))
    {
        say_spell(ch, SPELL_SLEEP);
        cast_sleep(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return TRUE;
    }
    if ((lspell > 5) && (number(0, 6) == 0))
    {
        say_spell(ch, SPELL_WEAKNESS);
        cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return TRUE;
    }
    if ((lspell > 5) && (number(0, 7) == 0))
    {
        say_spell(ch, SPELL_ARMOR);
        cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return TRUE;
    }
    if ((lspell > 12) && (number(0, 7) == 0))
    {
        say_spell(ch, SPELL_CURSE);
        cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return TRUE;
    }
    if ((lspell > 7) && (number(0, 5) == 0))
    {
        say_spell(ch, SPELL_BLINDNESS);
        cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return TRUE;
    }
    switch (lspell)
    {
    case 1:
    case 2:
    case 3:
    case 4:
        say_spell(ch, SPELL_MAGIC_MISSILE);
        cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 5:
        say_spell(ch, SPELL_SHOCKING_GRASP);
        cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        say_spell(ch, SPELL_LIGHTNING_BOLT);
        cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 11:
        say_spell(ch, SPELL_DISPEL_MAGIC);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 12:
    case 13:
    case 14:
        say_spell(ch, SPELL_COLOR_SPRAY);
        cast_color_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 15:
    case 16:
        say_spell(ch, SPELL_FIREBALL);
        cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 17:
    case 18:
    case 19:
        if (IS_EVIL(ch))
        {
            say_spell(ch, SPELL_ENERGY_DRAIN);
            cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }
    default:
        say_spell(ch, SPELL_FIREBALL);
        cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    }
    return TRUE;
}

int cleric(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;
    int lspell = 0;
    int healperc = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (GET_POS(ch) != POSITION_FIGHTING)
    {
        if ((GET_POS(ch) < POSITION_STANDING) && (GET_POS(ch) > POSITION_STUNNED))
        {
            StandUp(ch);
        }
        return FALSE;
    }
    if (!ch->specials.fighting)
        return FALSE;

    /*
     * Find a dude to to evil things upon !
     */

    vict = FindVictim(ch);

    if (!vict)
        vict = ch->specials.fighting;

    if (!vict)
        return (FALSE);

    /*
     * gen number from 0 to level
     */

    lspell = number(1, GetMaxLevel(ch));

    if (ch->points.hit < (ch->points.max_hit / 8))
        healperc = 7;
    else if (ch->points.hit < (ch->points.max_hit / 4))
        healperc = 5;
    if (ch->points.hit < (ch->points.max_hit / 2))
        healperc = 3;
    else if (number(1, healperc + 1) < 3)
    {
        if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING) && (lspell >= 15) && (number(0, 3) == 0))
        {
            say_spell(ch, SPELL_CALL_LIGHTNING);
            cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return (TRUE);
        }
        switch (lspell)
        {
        case 1:
        case 2:
        case 3:
            say_spell(ch, SPELL_CAUSE_LIGHT);
            cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 4:
        case 5:
        case 6:
            say_spell(ch, SPELL_BLINDNESS);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 7:
            say_spell(ch, SPELL_DISPEL_MAGIC);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 8:
            say_spell(ch, SPELL_POISON);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 9:
        case 10:
            say_spell(ch, SPELL_CAUSE_CRITICAL);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 11:
            say_spell(ch, SPELL_FLAMESTRIKE);
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 12:
            say_spell(ch, SPELL_CURSE);
            cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 13:
        case 14:
        case 15:
        case 16: {
            if ((GET_ALIGNMENT(vict) <= 0) && (GET_ALIGNMENT(ch) > 0))
            {
                say_spell(ch, SPELL_DISPEL_EVIL);
                cast_dispel_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            }
            else if ((GET_ALIGNMENT(vict) >= 0) && (GET_ALIGNMENT(ch) < 0))
            {
                say_spell(ch, SPELL_DISPEL_GOOD);
                cast_dispel_good(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            }
            else
            {
                if (!IS_SET(vict->M_immune, IMM_FIRE))
                {
                    say_spell(ch, SPELL_FLAMESTRIKE);
                    cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
                }
                else if (IS_AFFECTED(vict, AFF_SANCTUARY))
                {
                    say_spell(ch, SPELL_DISPEL_MAGIC);
                    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
                }
                else
                {
                    say_spell(ch, SPELL_CAUSE_CRITICAL);
                    cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
                }
            }
            break;
        }
        case 17:
        case 18:
        case 19:
        default:
            say_spell(ch, SPELL_HARM);
            cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        }

        return (TRUE);
    }
    else
    {

        if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0, 3) == 0))
        {
            say_spell(ch, SPELL_CURE_BLIND);
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }
        if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0, 6) == 0))
        {
            say_spell(ch, SPELL_REMOVE_CURSE);
            cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }
        if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0, 6) == 0))
        {
            say_spell(ch, SPELL_REMOVE_POISON);
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }
        switch (lspell)
        {
        case 1:
        case 2:
        case 3:
        case 4:
            say_spell(ch, SPELL_ARMOR);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            say_spell(ch, SPELL_CURE_LIGHT);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            say_spell(ch, SPELL_CURE_CRITIC);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
        case 18: /* heal */
            say_spell(ch, SPELL_HEAL);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            say_spell(ch, SPELL_SANCTUARY);
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        }
        return (TRUE);
    }
    return TRUE;
}

/*
 * Special procedures for mobiles
 */

int guild_guard(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd > 6 || cmd < 1)
        return FALSE;

    if ((IS_NPC(ch) && (IS_POLICE(ch))) || (GetMaxLevel(ch) >= DEMIGOD) || (IS_AFFECTED(ch, AFF_SNEAK)))
        return (FALSE);

    /*
     **  Remove-For-Multi-Class
     */
    if ((ch->in_room == 3017) && (cmd == 3))
    {
        if (!HasClass(ch, CLASS_MAGIC_USER))
        {
            act("The guard humiliates $n, and blocks $s way.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "The guard humiliates you, and block your way.\r\n");
            return TRUE;
        }
    }
    else if ((ch->in_room == 3004) && (cmd == 1))
    {
        if (!HasClass(ch, CLASS_CLERIC))
        {
            act("The guard humiliates $n, and blocks $s way.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "The guard humiliates you, and block your way.\r\n");
            return TRUE;
        }
    }
    else if ((ch->in_room == 3027) && (cmd == 2))
    {
        if (!HasClass(ch, CLASS_THIEF))
        {
            act("The guard humiliates $n, and blocks $s way.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "The guard humiliates you, and block your way.\r\n");
            return TRUE;
        }
    }
    else if ((ch->in_room == 3021) && (cmd == 2))
    {
        if (!HasClass(ch, CLASS_WARRIOR))
        {
            act("The guard humiliates $n, and blocks $s way.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "The guard humiliates you, and block your way.\r\n");
            return TRUE;
        }
    }
    return FALSE;
}

static const char *random_puff_message(void)
{
    static const char *oops[] = {"Suffer! I will make you all suffer!!!!!",
                                 "Suffer!!!!!! ALL will Suffer Quixadhal's WRATH!!!",
                                 "Any good weapons for sale?",
                                 "I wish there were more victims, err... I mean players!",
                                 "Anyone want a red ring??",
                                 "The Vax is DEAD!!!!",
                                 "Windows 95 SUCKS!!!",
                                 "Bill Gates has been forced to WALK the PLANK!",
                                 "Linux RULES!!!!!",
                                 "The Amiga LIVES!",
                                 "SAVE, and Ye Shall Be SAVED!",
                                 "I hear the Dread Lord is nice when Satan is chilly...",
                                 "Zar!  Where are you Zar?",
                                 "Muidnar!  Stop teasing the mortals!",
                                 "Dirk is idle again!",
                                 "Sedna, stop trying to code me to steal from players!",
                                 "Damnit Quixadhal!  Stop snooping me!",
                                 "He's dead Jim.",
                                 "What?  Nobody's DIED recently???",
                                 "Is it me, or does everyone hear an echo.. echo...  echo....",
                                 "EEK!  Someone's been animating the chicken fajitas again!",
                                 "I shall HEAL you.... No, on second thought I won't.",
                                 NULL};
    static int howmany = 22;

    if (DEBUG > 3)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    return oops[number(1, howmany) - 1];
}

int puff(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *i = NULL;
    char buf[80] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return (0);

    /*
     * PULSE_MOBILE is currently set to 25, which means that this routine
     * (and all specials) get called approximately every 6 seconds, at 4
     * pulses per second.
     *
     * Since we don't want puff spamming everyone, a flat .5% chance exists
     * that he'll do ANYTHING at all... this works out to him doing something
     * on a average of every two minutes.
     */

    if (number(1, 1000) > 5)
        return 1;

    /*
     * The case statement uses a GNU extension that breaks indent, you were warned!
     */
    switch (number(1, 100))
    {
    case 1 ... 25:
        do_shout(ch, random_puff_message(), 0);
        break;
    case 26 ... 35:
        for (i = character_list; i; i = i->next)
            if (IS_PC(i) && IS_IMMORTAL(i) && !number(0, 2))
            {
                sprintf(buf, "%s!  I need reimbursement!", GET_NAME(i));
                do_shout(ch, buf, 0);
                return 1;
            }
        break;
    case 36 ... 40:
        for (i = character_list; i; i = i->next)
            if (IS_PC(i) && !IS_IMMORTAL(i) && !number(0, 5))
            {
                sprintf(buf, "Hiya %s!", GET_NAME(i));
                do_shout(ch, buf, 0);
                return 1;
            }
        break;
    case 76 ... 85:
        for (i = character_list; i; i = i->next)
            if (IS_PC(i) && (i->in_room != NOWHERE))
            {
                sprintf(buf, "%s save", GET_NAME(i));
                do_force(ch, buf, 0);
            }
        break;
    case 86 ... 93:
        for (i = character_list; i; i = i->next)
            if (!IS_IMMORTAL(i) && !number(0, (IS_PC(i) ? 5 : 50)))
            {
                switch (GET_SEX(i))
                {
                case SEX_MALE:
                    sprintf(buf, "Be at ease brother %s!", fname(GET_NAME(i)));
                    break;
                case SEX_FEMALE:
                    sprintf(buf, "Be at ease sister %s!", fname(GET_NAME(i)));
                    break;
                default:
                    sprintf(buf, "Be at ease %s, whatever you are!", fname(GET_NAME(i)));
                    break;
                }
                do_shout(ch, buf, 0);
                cast_refresh(20, ch, "", SPELL_TYPE_SPELL, i, 0);
                return 1;
            }
        break;
    case 94 ... 97:
        for (i = character_list; i; i = i->next)
            if (!IS_IMMORTAL(i) && !number(0, (IS_PC(i) ? 5 : 50)))
            {
                switch (GET_SEX(i))
                {
                case SEX_MALE:
                    sprintf(buf, "I shall HEAL you brother %s!", fname(GET_NAME(i)));
                    break;
                case SEX_FEMALE:
                    sprintf(buf, "I shall HEAL you sister %s!", fname(GET_NAME(i)));
                    break;
                default:
                    sprintf(buf, "I shall HEAL you %s, whatever you are!", fname(GET_NAME(i)));
                    break;
                }
                do_shout(ch, buf, 0);
                cast_cure_light(20, ch, "", SPELL_TYPE_SPELL, i, 0);
                return 1;
            }
        break;
    case 98 ... 99:
        for (i = character_list; i; i = i->next)
            if (IS_PC(i) && !IS_IMMORTAL(i) && !number(0, 10))
            {
                sprintf(buf, "%s shout Where is Puff?", GET_NAME(i));
                do_force(ch, buf, 0);
                do_restore(ch, GET_NAME(i), 0);
                return 1;
            }
        break;
    case 100:
        for (i = character_list; i; i = i->next)
            if (!IS_IMMORTAL(i) && OUTSIDE(i))
            {
                if (((weather_info.sky == SKY_CLOUDY) && !number(0, 20)) ||
                    ((weather_info.sky == SKY_RAINING) && !number(0, 10)) ||
                    ((weather_info.sky == SKY_LIGHTNING) && !number(0, 5)))
                {
                    if (GET_HIT(i) > 10)
                    {
                        if (saves_spell(i, SAVING_SPELL))
                        {
                            GET_HIT(i) -= dice(1, 4);
                        }
                        else
                        {
                            GET_HIT(i) -= dice(2, 4);
                        }
                        act("The looming dark clouds above you suddenly rumble and you feel a searing\r\npain in your "
                            "chest!",
                            FALSE, i, 0, 0, TO_CHAR);
                        act("A brilliant flash of light blinds you for a moment. As your vision clears\r\nyou see "
                            "smoke rising from $N.",
                            FALSE, i, 0, i, TO_ROOM);
                        update_pos(i);
                    }
                    else
                    {
                        act("The looming dark clouds above you suddenly rumble and you feel your hair\r\nstanding on "
                            "end!",
                            FALSE, i, 0, 0, TO_CHAR);
                        act("A brilliant flash of light blinds you for a moment. As your vision clears\r\nyou see $N's "
                            "hair standing on end.",
                            FALSE, i, 0, i, TO_ROOM);
                    }
                    if (!number(0, 10))
                        return 1;
                }
            }
        break;
    }
    return 1;
}

int regenerator(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return (FALSE);

    if (GET_HIT(ch) < GET_MAX_HIT(ch))
    {
        GET_HIT(ch) += 9;
        GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

        act("$n regenerates.", TRUE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    }
    return TRUE;
}

int replicant(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *mob = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return FALSE;

    if (GET_HIT(ch) < GET_MAX_HIT(ch))
    {
        act("Drops of $n's blood hits the ground, and springs up into another one!", TRUE, ch, 0, 0, TO_ROOM);
        mob = read_mobile(ch->nr, REAL);
        char_to_room(mob, ch->in_room);
        act("Two undamaged opponents face you now.", TRUE, ch, 0, 0, TO_ROOM);
        GET_HIT(ch) = GET_MAX_HIT(ch);
    }
    return FALSE;
}

int Tytan(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting)
    {
        return (magic_user(ch, cmd, arg));
    }
    else
    {
        switch (ch->act_ptr)
        {
        case TYT_NONE:
            if ((vict = FindVictim(ch)))
            {
                ch->act_ptr = TYT_CIT;
                SetHunting(ch, vict);
            }
            break;
        case TYT_CIT:
            if (ch->specials.hunting)
            {
                if (ch->in_room == ch->specials.hunting->in_room)
                {
                    act("Where is the Citadel?", TRUE, ch, 0, 0, TO_ROOM);
                    ch->act_ptr = TYT_WHAT;
                }
            }
            else
            {
                ch->act_ptr = TYT_NONE;
            }
            break;
        case TYT_WHAT:
            if (ch->specials.hunting)
            {
                if (ch->in_room == ch->specials.hunting->in_room)
                {
                    act("What must we do?", TRUE, ch, 0, 0, TO_ROOM);
                    ch->act_ptr = TYT_TELL;
                }
            }
            else
            {
                ch->act_ptr = TYT_NONE;
            }
            break;
        case TYT_TELL:
            if (ch->specials.hunting)
            {
                if (ch->in_room == ch->specials.hunting->in_room)
                {
                    act("Tell Us!  Command Us!", TRUE, ch, 0, 0, TO_ROOM);
                    ch->act_ptr = TYT_HIT;
                }
            }
            else
            {
                ch->act_ptr = TYT_NONE;
            }
            break;
        case TYT_HIT:
            if (ch->specials.hunting)
            {
                if (ch->in_room == ch->specials.hunting->in_room)
                {
                    if (!check_peaceful(ch, ""))
                    {
                        hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
                        ch->act_ptr = TYT_NONE;
                    }
                    else
                    {
                        ch->act_ptr = TYT_CIT;
                    }
                }
            }
            else
            {
                ch->act_ptr = TYT_NONE;
            }
            break;
        default:
            ch->act_ptr = TYT_NONE;
        }
    }
    return TRUE;
}

int AbbarachDragon(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *targ = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (!ch->specials.fighting)
    {
        targ = (struct char_data *)FindAnyVictim(ch);
        if (targ && !check_peaceful(ch, ""))
        {
            hit(ch, targ, TYPE_UNDEFINED);
            act("You have now payed the price of crossing.", TRUE, ch, 0, 0, TO_ROOM);
            return (TRUE);
        }
    }
    else
    {
        return (BreathWeapon(ch, cmd, arg));
    }
    return TRUE;
}

int fido(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *i = NULL;
    struct obj_data *temp = NULL;
    struct obj_data *next_obj = NULL;
    struct char_data *v = NULL;
    struct char_data *next = NULL;
    int found = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    for (v = character_list; (v && (!found)); v = next)
    {
        next = v->next;
        if ((IS_NPC(v)) && (mob_index[v->nr].vnum == 100) && (v->in_room == ch->in_room) && CAN_SEE(ch, v))
        {
            if (v->specials.fighting)
                stop_fighting(v);
            make_corpse(v);
            extract_char(v);
            found = TRUE;
        }
    }

    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    {
        if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3])
        {
            act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
            for (temp = i->contains; temp; temp = next_obj)
            {
                next_obj = temp->next_content;
                obj_from_obj(temp);
                obj_to_room(temp, ch->in_room);
            }
            extract_obj(i);
            return (TRUE);
        }
    }
    return (FALSE);
}

int janitor(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    {
        if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE))
        {
            act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
            obj_from_room(i);
            obj_to_char(i, ch);
            return (TRUE);
        }
    }
    return (FALSE);
}

int janitor_eats(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    {
        if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE))
        {
            act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
            act("$n nibbles on the new found garbage.", FALSE, ch, 0, 0, TO_ROOM);
            extract_obj(i);
            return (TRUE);
        }
    }
    return (FALSE);
}

int tormentor(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!cmd)
        return (FALSE);
    if (IS_NPC(ch))
        return (FALSE);
    if (IS_IMMORTAL(ch))
        return (FALSE);
    return (TRUE);
}

int Fighter(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (ch->specials.fighting)
    {
        if (GET_POS(ch) == POSITION_FIGHTING)
        {
            FighterMove(ch);
            return (TRUE);
        }
        else
            StandUp(ch);
    }
    return (FALSE);
}

int RustMonster(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;
    struct obj_data *t_item = NULL;
    int t_pos = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    /*
     * **   find a victim
     */
    if (ch->specials.fighting)
    {
        vict = ch->specials.fighting;
    }
    else
    {
        vict = FindVictim(ch);
        if (!vict)
        {
            return (FALSE);
        }
    }

    /*
     * **   choose an item of armor or a weapon that is metal
     * **  since metal isn't defined, we'll just use armor and weapons
     */

    /*
     * **  choose a weapon first, then if no weapon, choose a shield,
     * **  if no shield, choose breast plate, then leg plate, sleeves,
     * **  helm
     */

    if (vict->equipment[WIELD])
    {
        t_item = vict->equipment[WIELD];
        t_pos = WIELD;
    }
    else if (vict->equipment[WEAR_SHIELD])
    {
        t_item = vict->equipment[WEAR_SHIELD];
        t_pos = WEAR_SHIELD;
    }
    else if (vict->equipment[WEAR_BODY])
    {
        t_item = vict->equipment[WEAR_BODY];
        t_pos = WEAR_BODY;
    }
    else if (vict->equipment[WEAR_LEGS])
    {
        t_item = vict->equipment[WEAR_LEGS];
        t_pos = WEAR_LEGS;
    }
    else if (vict->equipment[WEAR_ARMS])
    {
        t_item = vict->equipment[WEAR_ARMS];
        t_pos = WEAR_ARMS;
    }
    else if (vict->equipment[WEAR_HEAD])
    {
        t_item = vict->equipment[WEAR_HEAD];
        t_pos = WEAR_HEAD;
    }
    else
    {
        return (FALSE);
    }

    /*
     * **  item makes save (or not)
     */
    if (DamageOneItem(vict, ACID_DAMAGE, t_item))
    {
        t_item = unequip_char(vict, t_pos);
        if (t_item)
        {
            /*
             * **  if it doesn't make save, falls into a pile of scraps
             */
            MakeScrap(vict, t_item);
        }
    }
    return (FALSE);
}

int temple_labrynth_liar(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (0);

    switch (number(0, 15))
    {
    case 0:
        do_say(ch, "I'd go west if I were you.", 0);
        return (1);
    case 1:
        do_say(ch, "I heard that Addiction is a cute babe.", 0);
        return (1);
    case 2:
        do_say(ch, "Going east will avoid the beast!", 0);
        return (1);
    case 4:
        do_say(ch, "North is the way to go.", 0);
        return (1);
    case 6:
        do_say(ch, "Dont dilly dally go south.", 0);
        return (1);
    case 8:
        do_say(ch, "Great treasure lies ahead", 0);
        return (1);
    case 10:
        do_say(ch, "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
        return (1);
    case 12:
        do_say(ch, "I am a very clever liar.", 0);
        return (1);
    case 14:
        do_say(ch, "Loki is a really great guy!", 0);
        return (1);
    default:
        do_say(ch, "Then again I could be wrong!", 0);
        return (1);
    }
}

int temple_labrynth_sentry(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tch = NULL;
    int counter = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return FALSE;

    if (GET_POS(ch) != POSITION_FIGHTING)
        return FALSE;

    if (!ch->specials.fighting)
        return FALSE;

    /*
     * Find a dude to do very evil things upon !
     */

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (GetMaxLevel(tch) > 10 && CAN_SEE(ch, tch))
        {
            act("The sentry snaps out of his trance and ...", 1, ch, 0, 0, TO_ROOM);
            do_say(ch, "You will die for your insolence, pig-dog!", 0);
            for (counter = 0; counter < 4; counter++)
                if (GET_POS(tch) > POSITION_SITTING)
                    cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0);
                else
                    return TRUE;
            return TRUE;
        }
        else
        {
            act("The sentry looks concerned and continues to push you away", 1, ch, 0, 0, TO_ROOM);
            do_say(ch, "Leave me alone. My vows do not permit me to kill you!", 0);
        }
    }
    return TRUE;
}

int Whirlwind(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tmp = NULL;
    const char *names[] = {"Quixadhal", "", 0};
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (ch->in_room == -1)
        return (FALSE);

    if (cmd == 0 && ch->act_ptr == WW_LOOSE)
    {
        for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room)
        {
            while (names[i])
            {
                if (!strcmp(GET_NAME(tmp), names[i]) && ch->act_ptr == WW_LOOSE)
                {
                    /*
                     * start following
                     */
                    if (circle_follow(ch, tmp))
                        return (FALSE);
                    if (ch->master)
                        stop_follower(ch);
                    add_follower(ch, tmp);
                    ch->act_ptr = WW_FOLLOW;
                }
                i++;
            }
        }
        if (ch->act_ptr == WW_LOOSE && !cmd)
        {
            act("The $n suddenly dissispates into nothingness.", 0, ch, 0, 0, TO_ROOM);
            extract_char(ch);
        }
    }
    return TRUE;
}

int NudgeNudge(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting)
    {
        return (FALSE);
    }
    switch (ch->act_ptr)
    {
    case NN_LOOSE:
        vict = FindVictim(ch);
        if (!vict)
            return (FALSE);
        if (IS_IMMORTAL(vict) && !IS_SET(vict->specials.act, PLR_STEALTH) && (number(0, 99) < 50))
        {
            act("$n falls and grovels in abject terror before you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n falls and grovels in abject terror before $N.", FALSE, ch, 0, vict, TO_ROOM);
            return FALSE;
        }
        /*
         * start following
         */
        if (circle_follow(ch, vict))
        {
            return (FALSE);
        }
        if (ch->master)
            stop_follower(ch);
        add_follower(ch, vict);
        ch->act_ptr = NN_FOLLOW;
        if (!AWAKE(vict))
            do_wake(ch, GET_NAME(vict), 0);
        if (ch->player.sex == SEX_MALE)
        {
            do_say(ch, "Good Evenin' Squire!", 0);
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
        }
        else if (ch->player.sex == SEX_FEMALE)
        {
            do_say(ch, "Hello, handsome!", 0);
            act("$n snuggles up to you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n snuggles up to $N.", FALSE, ch, 0, ch->master, TO_ROOM);
        }
        else
        {
            do_say(ch, "What the fuck?  I'm neuter!", 0);
            act("$n looks between it's legs and screams!!!", FALSE, ch, 0, 0, TO_ROOM);
        }
        break;
    case NN_FOLLOW:
        if (!ch->master)
        {
            ch->act_ptr = NN_STOP;
            break;
        }
        if (number(0, 20) < 5)
            if (!AWAKE(ch->master))
                do_wake(ch, GET_NAME(ch->master), 0);
        switch (number(0, 20))
        {
        case 0:
            if (ch->player.sex == SEX_MALE)
            {
                do_say(ch, "Is your wife a goer?  Know what I mean, eh?", 0);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                do_say(ch, "So, do you have a girlfriend?", 0);
                act("$n hugs you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n hugs $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else
            {
                do_say(ch, "My nads are gone!  Help!!", 0);
                act("$n looks around for something it thinks is missing.", FALSE, ch, 0, 0, TO_ROOM);
            }
            break;
        case 1:
            if (ch->player.sex == SEX_MALE)
            {
                act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                do_say(ch, "Say no more!  Say no MORE!", 0);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                act("$n kisses you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n hugs you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n kisses $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n hugs $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                do_say(ch, "You're so handsome.  Will you marry me?", 0);
            }
            else
            {
                act("$n starts sobbing ridiculously and curses the gods for his fate", FALSE, ch, 0, 0, TO_ROOM);
            }
            break;
        case 2:
            if (ch->player.sex == SEX_MALE)
            {
                do_say(ch, "You been around, eh?", 0);
                do_say(ch, "...I mean you've ..... done it, eh?", 0);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                act("$n giggles, winks at you, and sashays about the room.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n giggles, winks at $N, and sashays about the room.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else
            {
                act("$n looks glassy-eyed.", FALSE, ch, 0, 0, TO_ROOM);
            }
            break;
        case 3:
            if (ch->player.sex == SEX_MALE)
            {
                do_say(ch, "A nod's as good as a wink to a blind bat, eh?", 0);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                do_say(ch, "You're so strong and brave.", 0);
                act("$n asks you, 'If a dragon held me captive, would you save me?'", FALSE, ch, 0, 0, TO_CHAR);
                act("$n asks $N, 'If a dragon held me captive, would you save me?'", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else
            {
                act("$n grumbles something about incompetent mud builders...", FALSE, ch, 0, 0, TO_ROOM);
            }
            break;
        case 4:
            if (ch->player.sex == SEX_MALE)
            {
                do_say(ch, "You're WICKED, eh!  WICKED!", 0);
                act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                act("$n blows you a kiss.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n blows $N a kiss.", FALSE, ch, 0, ch->master, TO_ROOM);
                act("$n giggles and winks at you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n giggles and winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else
            {
                act("$n looks bewildered.", FALSE, ch, 0, 0, TO_ROOM);
            }
            break;
        case 5:
            if (ch->player.sex == SEX_MALE)
            {
                do_say(ch, "Wink. Wink.", 0);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                act("$n says, '$N, I love you!'", FALSE, ch, 0, 0, TO_CHAR);
                act("$n says to $N, '$N, I love you!'", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else
            {
            }
            break;
        case 6:
            if (ch->player.sex == SEX_MALE)
            {
                do_say(ch, "Nudge. Nudge.", 0);
            }
            else if (ch->player.sex == SEX_FEMALE)
            {
                act("$n kisses you passionately.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n kisses $N passionately.", FALSE, ch, 0, ch->master, TO_ROOM);
            }
            else
            {
            }
            break;
        case 7:
        case 8:
        case 9:
        case 10:
            ch->act_ptr = NN_STOP;
            break;
        default:
            break;
        }
        break;
    case NN_STOP:
        if (ch->player.sex == SEX_MALE)
        {
            do_say(ch, "Evening, Squire", 0);
            stop_follower(ch);
        }
        else if (ch->player.sex == SEX_FEMALE)
        {
            do_say(ch, "Whenever you're in town again, look me up!", 0);
            act("$n blows you a kiss.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n blows $N a kiss.", FALSE, ch, 0, ch->master, TO_CHAR);
            stop_follower(ch);
        }
        else
        {
            stop_follower(ch);
        }
        ch->act_ptr = NN_LOOSE;
        break;
    default:
        ch->act_ptr = NN_LOOSE;
        break;
    }
    return TRUE;
}

int AGGRESSIVE(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *i = NULL;
    struct char_data *next = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);
    if (ch->in_room > -1)
    {
        for (i = real_roomp(ch->in_room)->people; i; i = next)
        {
            next = i->next_in_room;
            if (i->nr != ch->nr)
            {
                if (!IS_IMMORTAL(i))
                {
                    hit(ch, i, TYPE_UNDEFINED);
                }
            }
        }
    }
    return TRUE;
}

int cityguard(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tch = NULL;
    struct char_data *evil = NULL;
    int max_evil = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting)
    {
        if (GET_POS(ch) == POSITION_FIGHTING)
        {
            FighterMove(ch);
        }
        else
        {
            StandUp(ch);
        }

        if (number(0, 20) > 15)
        {
            do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
        }
        else
        {
            act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0, TO_ROOM);
        }

        if (ch->specials.fighting)
            CallForGuard(ch, ch->specials.fighting, 4);
        return (TRUE);
    }
    max_evil = 1000;
    evil = 0;

    if (check_peaceful(ch, ""))
        return FALSE;

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch))
        {
            max_evil = -1000;
            evil = tch;
            act("$n screams 'Suffer!!!  I will make you suffer!'", FALSE, ch, 0, 0, TO_ROOM);
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        }
        if (tch->specials.fighting)
        {
            if ((GET_ALIGNMENT(tch) < max_evil) && (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
            {
                max_evil = GET_ALIGNMENT(tch);
                evil = tch;
            }
        }
    }

    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0))
    {
        act("$n screams 'Suffer!!!!! You will suffer'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }
    return (FALSE);
}

int WarrenGuard(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tch = NULL;
    struct char_data *good = NULL;
    int max_good = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);

    if (ch->specials.fighting)
    {
        if (GET_POS(ch) == POSITION_FIGHTING)
        {
            FighterMove(ch);
        }
        else
        {
            StandUp(ch);
        }

        return (TRUE);
    }
    max_good = -1000;
    good = 0;

    if (check_peaceful(ch, ""))
        return FALSE;

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (tch->specials.fighting)
        {
            if ((GET_ALIGNMENT(tch) > max_good) && (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
            {
                max_good = GET_ALIGNMENT(tch);
                good = tch;
            }
        }
    }

    if (good && (GET_ALIGNMENT(good->specials.fighting) <= 0))
    {
        act("$n screams 'DEATH TO GOODY-GOODIES!!!!'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, good, TYPE_UNDEFINED);
        return (TRUE);
    }
    return (FALSE);
}

int zm_tired(struct char_data *zmaster)
{
    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(zmaster));

    return GET_HIT(zmaster) < GET_MAX_HIT(zmaster) / 2 || GET_MANA(zmaster) < 40;
}

int zm_stunned_followers(struct char_data *zmaster)
{
    struct follow_type *fwr = NULL;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(zmaster));

    for (fwr = zmaster->followers; fwr; fwr = fwr->next)
        if (GET_POS(fwr->follower) == POSITION_STUNNED)
            return TRUE;
    return FALSE;
}

void zm_zap_spell_at(struct char_data *ch, struct char_data *vict, int maxlevel)
{
    if (DEBUG > 3)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(vict), maxlevel);

    if (GET_POS(vict) <= POSITION_DEAD)
        return;

    switch (number(1, maxlevel))
    {
    case 1:
    case 2:
        say_spell(ch, SPELL_SHOCKING_GRASP);
        cast_shocking_grasp(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 3:
        say_spell(ch, SPELL_MAGIC_MISSILE);
        cast_magic_missile(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 4:
    case 5:
        say_spell(ch, SPELL_CHILL_TOUCH);
        cast_chill_touch(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 6:
        say_spell(ch, SPELL_WEAKNESS);
        cast_weakness(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 7:
        say_spell(ch, SPELL_DISPEL_MAGIC);
        cast_dispel_magic(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 8:
    case 9:
        say_spell(ch, SPELL_ACID_BLAST);
        cast_acid_blast(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 10:
    case 11:
        say_spell(ch, SPELL_BLINDNESS);
        cast_blindness(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 12:
        say_spell(ch, SPELL_FEAR);
        cast_fear(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 13:
    case 14:
    case 15:
    case 16:
        say_spell(ch, SPELL_LIGHTNING_BOLT);
        cast_lightning_bolt(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 17:
        say_spell(ch, SPELL_POISON);
        cast_poison(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 18:
        say_spell(ch, SPELL_PARALYSIS);
        cast_paralyze(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    default:
        say_spell(ch, SPELL_LIGHTNING_BOLT);
        cast_lightning_bolt(maxlevel, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    }
}

void zm_zap_area_at(struct char_data *ch, int maxlevel)
{
    if (DEBUG > 3)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), maxlevel);

    if (!ch->specials.fighting)
        return;

    if (GET_POS(ch->specials.fighting) <= POSITION_DEAD)
        return;

    switch (number(0, 3))
    {
    case 0:
        say_spell(ch, SPELL_ICE_STORM);
        cast_ice_storm(maxlevel, ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        break;
    case 1:
        say_spell(ch, SPELL_CONE_OF_COLD);
        cast_cone_of_cold(maxlevel, ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        break;
    case 2:
        say_spell(ch, SPELL_COLOR_SPRAY);
        cast_color_spray(maxlevel, ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
        break;
    case 3:
        act("$N flails about blindly and curses the name of $n.", TRUE, ch, 0, ch->specials.fighting, TO_ROOM);
        break;
    }
}

void zm_init_combat(struct char_data *zmaster, struct char_data *target)
{
    struct follow_type *fwr = NULL;

    if (DEBUG > 3)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(zmaster), SAFE_NAME(target));

    if (GET_POS(target) <= POSITION_DEAD)
        return;
    for (fwr = zmaster->followers; fwr; fwr = fwr->next)
        if (IS_AFFECTED(fwr->follower, AFF_CHARM) && fwr->follower->specials.fighting == NULL &&
            fwr->follower->in_room == target->in_room)
        {
            if (GET_POS(fwr->follower) == POSITION_STANDING)
            {
                hit(fwr->follower, target, TYPE_UNDEFINED);
            }
            else if (GET_POS(fwr->follower) > POSITION_SLEEPING && GET_POS(fwr->follower) < POSITION_FIGHTING)
            {
                do_stand(fwr->follower, "", -1);
            }
        }
}

int zm_kill_fidos(struct char_data *zmaster)
{
    struct char_data *fido_b = NULL;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(zmaster));

    if (check_peaceful(zmaster, ""))
        return FALSE;
    fido_b = find_mobile_here_with_spec_proc(fido, zmaster->in_room);
    if (fido_b)
    {
        act("$n shrilly screams 'Kill that carrion beast!'", FALSE, zmaster, 0, 0, TO_ROOM);
        zm_init_combat(zmaster, fido_b);
        if (GET_POS(fido_b) > POSITION_DEAD)
        {
            say_spell(zmaster, SPELL_MAGIC_MISSILE);
            cast_magic_missile(GetMaxLevel(zmaster), zmaster, "", SPELL_TYPE_SPELL, fido_b, 0);
        }
        return TRUE;
    }
    return FALSE;
}

int zm_kill_aggressor(struct char_data *zmaster)
{
    struct follow_type *fwr = NULL;
    int maxlevel = 0;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(zmaster));

    if (check_peaceful(zmaster, ""))
        return FALSE;
    maxlevel = GetMaxLevel(zmaster);
    if (zmaster->specials.fighting && (zmaster->specials.fighting->in_room == zmaster->in_room))
    {
        act("$n bellows 'Kill the mortal that dares lay hands on me!'", FALSE, zmaster, 0, 0, TO_ROOM);
        zm_init_combat(zmaster, zmaster->specials.fighting);
        if (IS_AFFECTED(zmaster, AFF_PARALYSIS))
        {
            if (number(1, maxlevel) > 5)
            {
                say_spell(zmaster, SPELL_REMOVE_PARALYSIS);
                cast_remove_paralysis(maxlevel, zmaster, "", SPELL_TYPE_SPELL, zmaster, 0);
            }
            else
                return TRUE;
        }
        StandUp(zmaster);
        if (IS_AFFECTED(zmaster, AFF_BLIND))
        {
            if (number(1, maxlevel) > 8)
            {
                say_spell(zmaster, SPELL_CURE_BLIND);
                cast_cure_blind(maxlevel, zmaster, "", SPELL_TYPE_SPELL, zmaster, 0);
            }
            else
            {
                zm_zap_area_at(zmaster, maxlevel);
                return TRUE;
            }
        }
        zm_zap_spell_at(zmaster, zmaster->specials.fighting, maxlevel);
        return TRUE;
    }
    for (fwr = zmaster->followers; fwr; fwr = fwr->next)
        if (fwr->follower->specials.fighting && (fwr->follower->specials.fighting->in_room == zmaster->in_room) &&
            IS_AFFECTED(fwr->follower, AFF_CHARM))
        {
            act("$n bellows 'Assist your brethren, my loyal servants!'", FALSE, zmaster, 0, 0, TO_ROOM);
            zm_init_combat(zmaster, fwr->follower->specials.fighting);
            if ((GetMaxLevel(fwr->follower->specials.fighting) <= ((maxlevel = GetMaxLevel(zmaster)) / 4)) &&
                (fwr->follower->specials.fighting->in_room == zmaster->in_room))
            {
                zm_zap_spell_at(zmaster, fwr->follower->specials.fighting, maxlevel);
                return TRUE;
            }
        }
    return FALSE;
}

int zm_kill_wimps(struct char_data *zmaster)
{
    struct char_data *vict = NULL;
    int mlev = 0;
    int maxlevel = 0;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(zmaster));

    if (check_peaceful(zmaster, ""))
        return FALSE;
    maxlevel = GetMaxLevel(zmaster);
    for (vict = real_roomp(zmaster->in_room)->people; vict; vict = vict->next_in_room)
    {
        if (vict->master != zmaster && vict != zmaster && ((mlev = GetMaxLevel(vict)) < maxlevel) &&
            (GET_POS(vict) > POSITION_DEAD) && (number(0, 99) < 65))
        {
            if ((real_roomp(vict->in_room)->zone == 10) && IS_PC(vict) && (mlev < 4))
                continue;
            if (IS_REALLY_HOLY(vict))
            {
                if (mlev <= (maxlevel - 2))
                {
                    act("$n howls 'BEGONE foul creature of Light!'", FALSE, zmaster, 0, 0, TO_ROOM);
                    zm_init_combat(zmaster, vict);
                    say_spell(zmaster, SPELL_LIGHTNING_BOLT);
                    cast_lightning_bolt(maxlevel, zmaster, "", SPELL_TYPE_SPELL, vict, 0);
                    return TRUE;
                }
                else
                    act("$n glares at $N with unimaginable horror!", TRUE, zmaster, 0, vict, TO_ROOM);
            }
            else if (IS_HOLY(vict))
            {
                if (mlev <= ((maxlevel / 2) + (maxlevel / 4)))
                {
                    act("$n screams 'DIE Slave of Goodness!'", FALSE, zmaster, 0, 0, TO_ROOM);
                    zm_init_combat(zmaster, vict);
                    zm_zap_spell_at(zmaster, vict, maxlevel);
                    return TRUE;
                }
                else
                    act("$n glares at $N with undisguised hatred.", TRUE, zmaster, 0, vict, TO_ROOM);
            }
            else if (IS_GOOD(vict))
            {
                if (mlev <= (maxlevel / 2))
                {
                    act("$n taunts 'You shall perish for your good intentions!'", FALSE, zmaster, 0, 0, TO_ROOM);
                    zm_init_combat(zmaster, vict);
                    zm_zap_spell_at(zmaster, vict, maxlevel);
                    return TRUE;
                }
                else
                    act("$n glares at $N with contempt.", TRUE, zmaster, 0, vict, TO_ROOM);
            }
            else if (IS_REALLY_VILE(vict))
            {
                act("$n nods slightly to $N and continues her work.", TRUE, zmaster, 0, vict, TO_ROOM);
            }
            else if (IS_VILE(vict))
            {
                if (mlev <= (maxlevel / 8))
                {
                    act("$n exclaims 'Your sacrifice will help me destroy Shylar!'", FALSE, zmaster, 0, 0, TO_ROOM);
                    zm_init_combat(zmaster, vict);
                    return TRUE;
                }
            }
            else if (IS_EVIL(vict))
            {
                if (mlev <= (maxlevel / 6))
                {
                    act("$n mocks 'You have not gone far enough into the darkness.'", FALSE, zmaster, 0, 0, TO_ROOM);
                    zm_init_combat(zmaster, vict);
                    return TRUE;
                }
            }
            else
            {
                if (mlev <= (maxlevel / 4))
                {
                    act("$n snickers 'You will provide me with flesh for my army.'", FALSE, zmaster, 0, 0, TO_ROOM);
                    zm_init_combat(zmaster, vict);
                    return TRUE;
                }
                else
                    act("$n looks $N over with disgust.", TRUE, zmaster, 0, vict, TO_ROOM);
            }
        }
    }
    return FALSE;
}

int zombie_master(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *temp1 = NULL;
    struct char_data *zmaster = NULL;
    int dir = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    zmaster = find_mobile_here_with_spec_proc(zombie_master, ch->in_room);

    if (cmd != 0 || ch != zmaster || !AWAKE(ch))
        return FALSE;

    if (zm_kill_aggressor(zmaster) || zm_kill_fidos(zmaster) || zm_kill_wimps(zmaster))
        return TRUE;

    switch (GET_POS(zmaster))
    {
    case POSITION_RESTING:
        if (!zm_tired(zmaster))
            do_stand(zmaster, "", -1);
        break;
    case POSITION_SITTING:
        if (!zm_stunned_followers(zmaster))
        {
            act("$n says 'It took you long enough...'", FALSE, zmaster, 0, 0, TO_ROOM);
            do_stand(zmaster, "", -1);
        }
        break;
    case POSITION_STANDING:
        if (zm_tired(zmaster))
        {
            do_rest(zmaster, "", -1);
            return TRUE;
        }
        temp1 = get_obj_in_list_vis(zmaster, "corpse", real_roomp(zmaster->in_room)->contents);

        if (temp1)
        {
            if (GET_MANA(zmaster) < ZM_MANA)
            {
                if (1 == dice(1, 20))
                    act("$n says 'So many bodies, so little time' and sighs.", FALSE, zmaster, 0, 0, TO_ROOM);
            }
            else if (number(0, 99) < 75)
            {
                act("$n says 'Come!  Join me in eternal Life!' and gestures sharply.", FALSE, zmaster, 0, 0, TO_ROOM);
                GET_MANA(zmaster) -= ZM_MANA;
                spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1);
                /*
                 * assume the new follower is top of the list?
                 */
                AddHatred(zmaster->followers->follower, OP_VNUM, ZM_NEMESIS);
                SET_BIT(zmaster->followers->follower->specials.act, ACT_GUARDIAN);
                SET_BIT(zmaster->followers->follower->specials.act, ACT_USE_ITEM);
                SET_BIT(zmaster->followers->follower->specials.affected_by, AFF_FLYING);
            }
            else
            {
                act("$n sighs 'Decisions, decisions...'", FALSE, zmaster, 0, 0, TO_ROOM);
            }
            return TRUE;
        }
        else if (zm_stunned_followers(zmaster))
        {
            do_sit(zmaster, "", -1);
            return TRUE;
        }
        else if (1 == dice(1, 20))
        {
            act("$n searches for bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
            return TRUE;
        }
        else if (0 <= (dir = find_path(zmaster->in_room, named_object_on_ground, "corpse", -200)))
        {
            go_direction(zmaster, dir);
            return TRUE;
        }
        else if (1 == dice(1, 5))
        {
            struct follow_type *pfft;
            int x, y;
            int top, bottom;
            struct follow_type *ack;
            struct room_data *rp;
            int room;

            x = 0;
            for (pfft = zmaster->followers; pfft; pfft = pfft->next)
                x++;
            if (x > 50)
            {
                allprintf("\r\nA loud feminine voice, dripping with malice, suddenly booms out!\r\n'I, Xenthia, have "
                          "returned to take my revenge upon Eli!  He shall regret the\r\nday he spurned me and "
                          "renounced our love!!!'\r\n\r\nThe wind dies and you hear the shuffling of many feet in the "
                          "land around you.\r\n\r\n");
                for (y = x; y > 5; y--)
                {
                    do
                    {
                        /*
                         * zone 10 is Shylar, 11 is Grasslands
                         */
                        bottom = zone_table[9].top + 1;
                        top = zone_table[10].top;
                        room = number(bottom, top);
                        rp = real_roomp(room);
                        log_info("Selecting zombie %d room... [#%d]\r", y, room);
                    } while (!rp || IS_SET(rp->room_flags, (NO_MOB | PEACEFUL | PRIVATE)));
                    log_info("Got zombie %d room... [#%d]\n", y, room);
                    ack = zmaster->followers;
                    if (!ack)
                        break;
                    zmaster->followers = ack->next;
                    ack->follower->master = NULL;
                    REMOVE_BIT(ack->follower->specials.affected_by, AFF_CHARM | AFF_GROUP | AFF_FLYING);
#if 0
			stop_follower(ack);
#endif
                    REMOVE_BIT(ack->follower->specials.act, ACT_GUARDIAN);
                    SET_BIT(ack->follower->specials.act, ACT_AGGRESSIVE | ACT_ANNOYING);
                    char_to_room(ack->follower, room);
                    act("A gash of reddish light opens and $N stumbles through!", FALSE, ack->follower, 0,
                        ack->follower, TO_ROOM);
                    DESTROY(ack); /* in stop_follower code... sure? */
                }
                return TRUE;
            }
            else if (x >= 40)
            {
                allprintf("\r\nXenthia shouts, 'Soon Eli!  I shall make you PAY for your crimes!'\r\n\r\n");
                return TRUE;
            }
            else
            {
                act("$n can't find any bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
                return TRUE;
            }
        }
        else
        {
            mobile_wander(zmaster);
        }
        break;
    }
    return FALSE;
}

int pet_shops(struct char_data *ch, int cmd, const char *arg)
{
    int pet_room = 0;
    struct char_data *pet = NULL;
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char pet_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    pet_room = ch->in_room + 1;

    if (cmd == 59)
    { /* List */
        cprintf(ch, "Available pets are:\r\n");
        for (pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room)
        {
            cprintf(ch, "%8d - %s\r\n", 10 * GET_EXP(pet), pet->player.short_descr);
        }
        return (TRUE);
    }
    else if (cmd == 56)
    { /* Buy */

        arg = one_argument(arg, buf);
        only_argument(arg, pet_name);

        /*
         * Pet_Name is for later use when I feel like it
         */

        if (!(pet = get_char_room(buf, pet_room)))
        {
            cprintf(ch, "There is no such pet!\r\n");
            return (TRUE);
        }
        if (GET_GOLD(ch) < (GET_EXP(pet) * 10))
        {
            cprintf(ch, "You don't have enough gold!\r\n");
            return (TRUE);
        }
        GET_GOLD(ch) -= GET_EXP(pet) * 10;

        pet = read_mobile(pet->nr, REAL);
        GET_EXP(pet) = 0;
        SET_BIT(pet->specials.affected_by, AFF_CHARM);

        if (*pet_name)
        {
            sprintf(buf, "%s %s", pet->player.name, pet_name);
            DESTROY(pet->player.name);
            pet->player.name = strdup(buf);

            sprintf(buf, "%sA small sign on a chain around the neck says 'My Name is %s'\r\n", pet->player.description,
                    pet_name);
            DESTROY(pet->player.description);
            pet->player.description = strdup(buf);
        }
        char_to_room(pet, ch->in_room);
        add_follower(pet, ch);

        IS_CARRYING_W(pet) = 0;
        IS_CARRYING_N(pet) = 0;

        cprintf(ch, "May you enjoy your pet.\r\n");
        act("$n bought $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

        return (TRUE);
    }
    /*
     * All commands except list and buy
     */
    return (FALSE);
}

int bank(struct char_data *ch, int cmd, const char *arg)
{
    int money = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    money = atoi(arg);

    if (!IS_NPC(ch))
        save_char(ch, ch->in_room);

    /*
     * deposit
     */
    if (cmd == 219)
    {
        if (money > GET_GOLD(ch))
        {
            cprintf(ch, "You don't have enough for that!\r\n");
            return (TRUE);
        }
        else if (money <= 0)
        {
            cprintf(ch, "Go away, you bother me.\r\n");
            return (TRUE);
        }
        else
        {
            cprintf(ch, "Thank you.\r\n");
            GET_GOLD(ch) = GET_GOLD(ch) - money;
            GET_BANK(ch) = GET_BANK(ch) + money;
            cprintf(ch, "Your balance is %d.\r\n", GET_BANK(ch));
            if (GET_BANK(ch) > 200000)
            {
                log_info("%s has %d coins in the bank.", GET_NAME(ch), GET_BANK(ch));
            }
            return (TRUE);
        }
        /*
         * withdraw
         */
    }
    else if (cmd == 220)
    {
        if (money > GET_BANK(ch))
        {
            cprintf(ch, "You don't have enough in the bank for that!\r\n");
            return (TRUE);
        }
        else if (money <= 0)
        {
            cprintf(ch, "Go away, you bother me.\r\n");
            return (TRUE);
        }
        else
        {
            cprintf(ch, "Thank you.\r\n");
            GET_GOLD(ch) = GET_GOLD(ch) + money;
            GET_BANK(ch) = GET_BANK(ch) - money;
            cprintf(ch, "Your balance is %d.\r\n", GET_BANK(ch));
            return (TRUE);
        }
    }
    else if (cmd == 221)
    {
        cprintf(ch, "Your balance is %d.\r\n", GET_BANK(ch));
        return (TRUE);
    }
    return (FALSE);
}

/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

int pray_for_items(struct char_data *ch, int cmd, const char *arg)
{
    int key_room = 0;
    int gold = 0;
    char found = FALSE;
    struct obj_data *tmp_obj = NULL;
    struct obj_data *obj = NULL;
    struct extra_descr_data *ext = NULL;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd != CMD_pray) /* You must pray to get the stuff */
        return FALSE;

    key_room = 1 + ch->in_room;

    strcpy(buf, "item_for_");
    strcat(buf, GET_NAME(ch));

    gold = 0;
    found = FALSE;

    for (tmp_obj = real_roomp(key_room)->contents; tmp_obj; tmp_obj = tmp_obj->next_content)
        for (ext = tmp_obj->ex_description; ext; ext = ext->next)
            if (str_cmp(buf, ext->keyword) == 0)
            {
                if (gold == 0)
                {
                    gold = 1;
                    act("$n kneels at the altar and chants a prayer to Quixadhal.", FALSE, ch, 0, 0, TO_ROOM);
                    act("You notice a faint spark in Quixadhal's eye.", FALSE, ch, 0, 0, TO_CHAR);
                }
                obj = read_object(tmp_obj->item_number, REAL);
                obj_to_room(obj, ch->in_room);
                act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_ROOM);
                act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_CHAR);
                gold += obj->obj_flags.cost;
                found = TRUE;
            }
    if (found)
    {
        GET_GOLD(ch) -= gold;
        GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
        return TRUE;
    }
    return FALSE;
}

/*
 * Special procedures for objects
 */

int chalice(struct char_data *ch, int cmd, const char *arg)
{
    /*
     * 222 is the normal chalice, 223 is chalice-on-altar
     */

    struct obj_data *chalice_obj = NULL;
    static int chl = -1;
    static int achl = -1;
    char buf1[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char buf2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (chl < 1)
    {
        chl = real_object(222);
        achl = real_object(223);
    }
    switch (cmd)
    {
    case 10: /* get */
        if (!(chalice_obj = get_obj_in_list_num(chl, real_roomp(ch->in_room)->contents)) &&
            CAN_SEE_OBJ(ch, chalice_obj))
            if (!(chalice_obj = get_obj_in_list_num(achl, real_roomp(ch->in_room)->contents)) &&
                CAN_SEE_OBJ(ch, chalice_obj))
                return (0);

        /*
         * we found a chalice.. now try to get us
         */
        do_get(ch, arg, cmd);
        /*
         * if got the altar one, switch her
         */
        if (chalice_obj == get_obj_in_list_num(achl, ch->carrying))
        {
            extract_obj(chalice_obj);
            chalice_obj = read_object(chl, VIRTUAL);
            obj_to_char(chalice_obj, ch);
        }
        return (1);
        break;
    case 67: /* put */
        if (!(chalice_obj = get_obj_in_list_num(chl, ch->carrying)))
            return (0);

        argument_interpreter(arg, buf1, buf2);
        if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar"))
        {
            extract_obj(chalice_obj);
            chalice_obj = read_object(achl, VIRTUAL);
            obj_to_room(chalice_obj, ch->in_room);
            cprintf(ch, "Ok.\r\n");
        }
        return (1);
        break;
    case 176: /* pray */
        if (!(chalice_obj = get_obj_in_list_num(achl, real_roomp(ch->in_room)->contents)))
            return (0);

        do_action(ch, arg, cmd); /* pray */
        cprintf(ch, CHAL_ACT);
        extract_obj(chalice_obj);
        act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, 2500); /* before the fiery gates */
        do_look(ch, "", 15);
        return (1);
        break;
    default:
        return (0);
        break;
    }
}

int kings_hall(struct char_data *ch, int cmd, const char *arg)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd != 176)
        return (0);

    do_action(ch, arg, 176);
    cprintf(ch, "You feel as if some mighty force has been offended.\r\n");
    cprintf(ch, CHAL_ACT);
    act("$n is struck by an intense beam of light and vanishes.", TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, 1420); /* behind the altar */
    do_look(ch, "", 15);
    return (1);
}

/*
 *  house routine for saved items.
 */

int House(struct char_data *ch, int cmd, const char *arg)
{
    int i = 0;
    int save_room = 0;
    struct extra_descr_data *ext = NULL;
    int found = 0;
    struct obj_cost cost;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (IS_NPC(ch))
        return (FALSE);

    if (cmd != 92)
    {
        return (FALSE);
    }
    else
    {
        /*
         * Verify that a person can rent here, the name of the character wil
         * be found in the extra description of the room itself, thus leaving
         * the name of the room to be what ever the owner wishes.
         */

        found = 0;
        for (ext = real_roomp(ch->in_room)->ex_description; ext && !found; ext = ext->next)
            if (str_cmp(GET_NAME(ch), ext->keyword) == 0)
            {
                found = 1;
                cprintf(ch, "Okay, found your name in the annals.\r\n");
            }
        if (!found)
        {
            if (strncmp(GET_NAME(ch), real_roomp(ch->in_room)->name, strlen(GET_NAME(ch))))
            {
                cprintf(ch, "Sorry, you'll have to find your own house.\r\n");
                return (FALSE);
            }
            else
            {
                cprintf(ch, "Ah, you own this room.\r\n");
            }
        }
        cost.total_cost = 0; /* Minimum cost */
        cost.no_carried = 0;
        cost.ok = TRUE; /* Use if any "-1" objects */

        add_obj_cost(ch, 0, ch->carrying, &cost);
        for (i = 0; i < MAX_WEAR; i++)
            add_obj_cost(ch, 0, ch->equipment[i], &cost);

        if (!cost.ok)
        {
            return (FALSE);
        }
        cost.total_cost = 0;

        GET_HOME(ch) = ch->in_room;
        new_save_equipment(ch, &cost, FALSE);
        save_obj(ch, &cost, 1);
        save_room = ch->in_room;
        extract_char(ch);
        ch->in_room = save_room;
        save_char(ch, ch->in_room);
        return (TRUE);
    }
}

int paramedics(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;
    struct char_data *most_hurt = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!cmd)
    {
        if (ch->specials.fighting)
        {
            return (cleric(ch, 0, ""));
        }
        else
        {
            if (GET_POS(ch) == POSITION_STANDING)
            {

                /*
                 * Find a dude to do good things upon !
                 */

                most_hurt = real_roomp(ch->in_room)->people;
                for (vict = real_roomp(ch->in_room)->people; vict; vict = vict->next_in_room)
                {
                    if (((float)GET_HIT(vict) / (float)hit_limit(vict) <
                         (float)GET_HIT(most_hurt) / (float)hit_limit(most_hurt)) &&
                        (CAN_SEE(ch, vict)))
                        most_hurt = vict;
                }
                if (!most_hurt)
                    return (FALSE); /* nobody here */

                if ((float)GET_HIT(most_hurt) / (float)hit_limit(most_hurt) > 0.66)
                {
                    if (number(0, 5) == 0)
                    {
                        act("$n shrugs helplessly in unison.", 1, ch, 0, 0, TO_ROOM);
                    }
                    return TRUE; /* not hurt enough */
                }
                if (number(0, 4) == 0)
                {
                    if (most_hurt != ch)
                    {
                        act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
                        act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
                    }
                    act("$n utters the words 'judicandus dies' in unison.", 1, ch, 0, 0, TO_ROOM);
                    cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, most_hurt, 0);
                    return (TRUE);
                }
            }
            else
            { /* I'm asleep or sitting */
                return (FALSE);
            }
        }
    }
    return (FALSE);
}

int jugglernaut(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *tmp_obj = NULL;
    int i = 0;
    int j = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return (FALSE);

    if (GET_POS(ch) == POSITION_STANDING)
    {

        if (random() % 3)
            return FALSE;

        /*
         * juggle something
         */

        if (IS_CARRYING_N(ch) < 1)
            return FALSE;

        i = random() % IS_CARRYING_N(ch);
        j = 0;
        for (tmp_obj = ch->carrying; (tmp_obj) && (j < i); j++)
        {
            tmp_obj = tmp_obj->next_content;
        }

        if (random() % 6)
        {
            if (random() % 2)
            {
                act("$n tosses $p high into the air and catches it.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
            }
            else
            {
                act("$n sends $p whirling.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
            }
        }
        else
        {
            act("$n tosses $p but fumbles it!", TRUE, ch, tmp_obj, NULL, TO_ROOM);
            obj_from_char(tmp_obj);
            obj_to_room(tmp_obj, ch->in_room);
        }
        return (TRUE); /* don't move, I dropped something */
    }
    else
    { /* I'm asleep or sitting */
        return FALSE;
    }
    return (FALSE);
}

int delivery_beast(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *o = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return (FALSE);

    if (time_info.hours == 6)
    {
        do_drop(ch, "all.loaf", 0);
        do_drop(ch, "all.biscuit", 0);
    }
    else if (time_info.hours < 2)
    {
        if (number(0, 1))
        {
            o = read_object(3012, VIRTUAL);
            obj_to_char(o, ch);
        }
        else
        {
            o = read_object(3013, VIRTUAL);
            obj_to_char(o, ch);
        }
    }
    else
    {
        if (GET_POS(ch) > POSITION_SLEEPING)
        {
            do_sleep(ch, "", 0);
        }
    }
    return TRUE;
}

int StormGiant(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return (FALSE);

    if (ch->specials.fighting)
    {
        if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED))
        {
            StandUp(ch);
        }
        else
        {
            if (number(0, 5))
            {
                FighterMove(ch);
            }
            else
            {
                act("$n creates a lightning bolt", TRUE, ch, 0, 0, TO_ROOM);
                vict = FindVictim(ch);
                if (!vict)
                    return (FALSE);
                cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

                /*
                 * do nothing
                 */
            }
        }
        return (FALSE);
    }
    return TRUE;
}

int firenewt(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd)
        return FALSE;

    if (GET_POS(ch) != POSITION_FIGHTING)
        return FALSE;

    for (vict = real_roomp(ch->in_room)->people; vict; vict = vict->next_in_room)
        if ((ch != vict) && (vict->specials.fighting == ch))
        {
            break;
        }
    if (!vict)
        return FALSE;

    return TRUE;
}

int eli_priest(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *vict = NULL;
    int is_evil = FALSE;
    int which_spell = SPELL_BLESS;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd == CMD_register)
    { /* register */
        while (*arg && isspace(*arg))
            arg++;
        if (!strncmp("me", arg, 2))
        {
            SET_BIT(ch->specials.new_act, NEW_PLR_KILLOK);
            if (IS_SET(ch->specials.new_act, NEW_PLR_TELEPORT))
                REMOVE_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
            if (IS_SET(ch->specials.new_act, NEW_PLR_SUMMON))
                REMOVE_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
            GET_HIT(ch) = 1;
            cprintf(ch, "Eli gets a maniacal grin on his face as he draws forth his holy symbol.\r\n"
                        "You notice it is a jet black image of a gigantic axe cleaving a tree.\r\n"
                        "Sparkling emeralds from the leaves, and it looks like ruby red blood seeps\r\n"
                        "from around the axe.  The black metal begins to glow brilliant red as Eli\r\n"
                        "incants some powerful spell over it.\r\n"
                        "Suddenly, Eli lunges forward and pushes the holy relic into your forehead!\r\n"
                        "You hear a searing sound, and feel IMMENSE PAIN... and it feels GOOD!\r\n");
            allprintf("Eli shouts, 'Congratulations %s!  You have registered to kill other players in the name of "
                      "Dread Lord Quixadhal!'\r\n",
                      GET_NAME(ch));
        }
        else
        {
            cprintf(ch, "Registering your character as a player killer is a permenant thing, and\r\n"
                        "once done, it can never be undone.  By entering the command: \"register me\"\r\n"
                        "you are agreeing to not only kill, but to be killed by other registered\r\n"
                        "players.  No mercy or pity will be shown to you, any losses incurred\r\n"
                        "by your character will NOT be compensated by ANY of the Gods.\r\n");
        }
        return (TRUE);
    }
    if (cmd)
        return FALSE;

    if (GET_POS(ch) == POSITION_FIGHTING)
        return FALSE;

    if ((GET_MANA(ch) < 60) && (GET_POS(ch) != POSITION_SLEEPING))
    {
        act("$n falls to the ground into a deep slumber.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SLEEPING;
    }
    if ((GET_MANA(ch) > 90) && (GET_POS(ch) != POSITION_STANDING))
    {
        GET_POS(ch) = POSITION_STANDING;
        act("$n rises to his feet from a sound sleep.", TRUE, ch, 0, 0, TO_ROOM);
        return TRUE;
    }
    if (GET_POS(ch) == POSITION_SLEEPING)
        return FALSE;

    for (vict = real_roomp(ch->in_room)->people; vict; vict = vict->next_in_room)
    {
        if (!IS_IMMORTAL(vict))
        {
            if (IsUndead(vict))
            {
                act("$n points at $N and bellows, 'BEGONE Abomination! Flee back to Dread Quixadhal, your dark "
                    "master!'",
                    TRUE, ch, 0, vict, TO_ROOM);
                say_spell(ch, SPELL_FLAMESTRIKE);
                cast_flamestrike(40, ch, "", SPELL_TYPE_SPELL, vict, 0);
                return TRUE;
            }
        }
        if ((GET_POS(vict) != POSITION_FIGHTING) && (GET_POS(vict) != POSITION_SLEEPING) && !IS_IMMORTAL(vict))
        {
            if ((GET_HIT(vict) < 16) && (GET_MAX_HIT(vict) != GET_HIT(vict)))
            {
                if ((GET_ALIGNMENT(vict) > ALIGN_VERY_EVIL) ||
                    ((GET_ALIGNMENT(vict) > ALIGN_VILE) && (GetMaxLevel(vict) < 4)))
                {
                    act("$n says, You are hurt $N, let me heal you.", TRUE, ch, 0, vict, TO_ROOM);
                    is_evil = FALSE;
                    which_spell = SPELL_CURE_LIGHT;
                    break;
                }
                else
                {
                    is_evil = TRUE;
                    if (number(0, 10) > 5)
                    {
                        act("$n says, I pity you $N, let me show you the light.", TRUE, ch, 0, vict, TO_ROOM);
                        which_spell = SPELL_BLESS;
                        break;
                    }
                    else
                    {
                        act("$n says, You are wicked $N, I shall not help you.", TRUE, ch, 0, vict, TO_ROOM);
                        continue;
                    }
                }
            }
            else if ((GET_MOVE(vict) < 25) && (GET_MAX_MOVE(vict) != GET_MOVE(vict)))
            {
                if ((GET_ALIGNMENT(vict) > ALIGN_VERY_EVIL) ||
                    ((GET_ALIGNMENT(vict) > ALIGN_VILE) && (GetMaxLevel(vict) < 4)))
                {
                    act("$n says, You seem tired $N, let me help you.", TRUE, ch, 0, vict, TO_ROOM);
                    is_evil = FALSE;
                    which_spell = SPELL_REFRESH;
                    break;
                }
                else
                {
                    is_evil = TRUE;
                    if (number(0, 10) > 5)
                    {
                        act("$n says, I pity you $N, let me show you the light.", TRUE, ch, 0, vict, TO_ROOM);
                        which_spell = SPELL_BLESS;
                        break;
                    }
                    else
                    {
                        act("$n says, You are wicked $N, I shall let you suffer.", TRUE, ch, 0, vict, TO_ROOM);
                        continue;
                    }
                }
            }
            else if (IS_AFFECTED(vict, AFF_BLIND))
            {
                if ((GET_ALIGNMENT(vict) > ALIGN_VERY_EVIL) ||
                    ((GET_ALIGNMENT(vict) > ALIGN_VILE) && (GetMaxLevel(vict) < 4)))
                {
                    act("$n says, You seem to be blind $N, let me cure you.", TRUE, ch, 0, vict, TO_ROOM);
                    is_evil = FALSE;
                    which_spell = SPELL_CURE_BLIND;
                    break;
                }
                else
                {
                    act("$n says, You are wicked $N, I shall let you suffer.", TRUE, ch, 0, vict, TO_ROOM);
                    continue;
                }
            }
            else if (IS_AFFECTED(vict, AFF_POISON))
            {
                if ((GET_ALIGNMENT(vict) > ALIGN_VERY_EVIL) ||
                    ((GET_ALIGNMENT(vict) > ALIGN_VILE) && (GetMaxLevel(vict) < 4)))
                {
                    act("$n says, You seem to be sick $N, let me cure you.", TRUE, ch, 0, vict, TO_ROOM);
                    is_evil = FALSE;
                    which_spell = SPELL_REMOVE_POISON;
                    break;
                }
                else
                {
                    act("$n says, You are wicked $N, I shall let you suffer.", TRUE, ch, 0, vict, TO_ROOM);
                    continue;
                }
            }
            else if (IS_REALLY_VILE(vict) && (GetMaxLevel(vict) > 4))
            {
                act("$n screams, BEGONE foul $N, I shall not suffer your presence!", TRUE, ch, 0, vict, TO_ROOM);
                say_spell(ch, SPELL_FEAR);
                cast_fear(40, ch, "", SPELL_TYPE_SPELL, vict, 0);
                return TRUE;
            }
        }
    }
    if (!vict)
    {
        if (number(0, 10) > 6)
            return TRUE;
        switch (number(0, 20))
        {
        case 0:
        case 1:
            act("$n says, It is so nice to be back in Shylar!", TRUE, ch, 0, 0, TO_ROOM);
            break;
        case 2:
        case 3:
            act("$n says, I heard the lands were getting tough, so I decide to move a church here.", TRUE, ch, 0, 0,
                TO_ROOM);
            break;
        case 4:
        case 5:
            act("$n says, Have you been to my parish just north of Pastue's?", TRUE, ch, 0, 0, TO_ROOM);
            break;
        }
        return TRUE;
    }
    if (is_evil)
    {
#if 0
	GET_ALIGNMENT(vict) += number(20, 100);
#endif
    }
    switch (which_spell)
    {
    case SPELL_BLESS:
        say_spell(ch, SPELL_BLESS);
        cast_bless(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case SPELL_CURE_LIGHT:
        say_spell(ch, SPELL_CURE_LIGHT);
        cast_cure_light(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case SPELL_REFRESH:
        say_spell(ch, SPELL_REFRESH);
        cast_refresh(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case SPELL_CURE_BLIND:
        say_spell(ch, SPELL_CURE_BLIND);
        cast_cure_blind(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case SPELL_REMOVE_POISON:
        say_spell(ch, SPELL_REMOVE_POISON);
        cast_remove_poison(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    }
    GET_MANA(ch) -= 5;
    return TRUE;
}

int fountain(struct char_data *ch, int cmd, const char *arg)
{
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd != 11)
        return (0);

    one_argument(arg, tmp);

    if (!*tmp || !isname(tmp, "water barrel fountain pool"))
        return (0);

    if (GET_COND(ch, THIRST) != -1)
    {
        if (GetMaxLevel(ch) > 50)
        {
            GET_COND(ch, THIRST) = -1;
        }
        else
        {
            GET_COND(ch, THIRST) += 8;
            GET_COND(ch, THIRST) = MIN(24, GET_COND(ch, THIRST));
            act("$n drinks from the %s.", TRUE, ch, 0, 0, TO_ROOM, tmp);
            if (GET_COND(ch, THIRST) == 24)
            {
                cprintf(ch, "You are full!\n");
            }
            else
                cprintf(ch, "You drink the cool water.\n");
        }
    }
    if (GET_COND(ch, THIRST) < 24 && number(0, 99) < 5)
    {
        switch (number(0, 40))
        {
        case 0:
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 1:
            cast_bless(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 2:
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 3:
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 4:
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 5:
            cast_detect_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
            cast_detect_invisibility(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 7:
            cast_detect_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 8:
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 9:
            cast_invisibility(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 10:
            cast_protection_from_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 11:
            cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 12:
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 13:
            cast_strength(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 14:
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 15:
            cast_sense_life(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 16:
            cast_infravision(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
            cast_flying(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 18:
            cast_shield(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 19:
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 20:
            cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 21:
            cast_stone_skin(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 22:
            cast_aid(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 23:
            cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 24:
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 25:
            cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 26:
            cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 27:
            cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 28:
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 29:
            cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 30:
            cast_sleep(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 31:
            cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 32:
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 33:
            cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 34:
            cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 35:
            cast_faerie_fire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 36:
            cast_poly_self(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            break;
        }
    }
    return 1;
}

int mosquito(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *tch = NULL;
    struct char_data *bloody = NULL;
    int blood = 0;
    int most_blood = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd || !AWAKE(ch))
        return (FALSE);
    if (check_peaceful(ch, ""))
        return FALSE;

    if (ch->specials.fighting)
    {
        if (GET_POS(ch) != POSITION_FIGHTING)
        {
            StandUp(ch);
        }
        return TRUE;
    }
    most_blood = 0;
    bloody = 0;

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (tch == ch)
            continue;
        if (IS_MOB(tch))
            continue;
        blood = ((GET_HIT(tch) + 1) * 100) / (GET_MAX_HIT(tch) + 1);
        if (blood < most_blood)
        {
            most_blood = blood;
            bloody = tch;
        }
    }

    if (bloody && number(0, 99) < 75)
    {
        if (bloody->in_room == ch->in_room)
        {
            if (IS_MORTAL(bloody) && GET_POS(bloody) > POSITION_DEAD)
            {
                act("$n dives at $N!", TRUE, ch, 0, bloody, TO_ROOM);
                hit(ch, bloody, TYPE_UNDEFINED);
                return TRUE;
            }
            else
            {
                if (number(0, 99) < 10)
                {
                    act("$n howls in a suicide mision at $N!", TRUE, ch, 0, bloody, TO_ROOM);
                    cprintf(bloody, "\nYou have DIED to a tiny mosquito!\r\n");
                    rprintf(bloody->in_room,
                            "The mighty %s was vanquished by a mosquito!!!\r\nA dead body hits the ground and begins "
                            "to rot.\r\n",
                            GET_NAME(bloody));
                    make_corpse(bloody);
                    zero_rent(bloody);
                    char_from_room(bloody);
                    char_to_room(bloody, GET_HOME(bloody));
                    GET_HIT(bloody) = 1;
                    GET_POS(bloody) = POSITION_SLEEPING;
                    save_char(bloody, NOWHERE);
                    rprintf(bloody->in_room, "A humbled %s appears in a flash of light!\r\n", GET_NAME(bloody));
                    return FALSE;
                }
                else
                {
                    act("$n anticipates the taste of $N's immortal blood!", TRUE, ch, 0, bloody, TO_ROOM);
                    return FALSE;
                }
            }
        }
    }
    return FALSE;
}

int BerserkerAxe(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *victim = NULL;
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (cmd == CMD_flee)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        act("$n looks abashed and then seems taken by a sudden fury!", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "You are suddenly furious at your cowardice... You fight on!\r\n");
        return 1;
    }
    if (cmd == CMD_remove)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        one_argument(arg, tmp);
        if (!*tmp || !isname(tmp, "druegar berserker axe"))
            return 0;
        act("$n clutches a gigantic axe tightly and glares at you.", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "You cannot bear to relinquish your weapon while you yet live!\r\n");
        return 1;
    }
    if (cmd == CMD_sell)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        one_argument(arg, tmp);
        if (!*tmp || !isname(tmp, "druegar berserker axe"))
            return 0;
        act("$n clutches a gigantic axe tightly and glares at you.", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "Not for all the gold in the world!\r\n");
        return 1;
    }
    if (cmd == CMD_junk)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        one_argument(arg, tmp);
        if (!*tmp || !isname(tmp, "druegar berserker axe"))
            return 0;
        act("$n seems nervous and quickly looks over a giant axe.", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "It hurts to even joke about destroying your weapon!\r\n");
        return 1;
    }
    if (cmd == CMD_sleep)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        act("$n suddenly snaps to attention and begins drooling.", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "No time to sleep!  There are too many things to kill!\r\n");
        return 1;
    }
    if (cmd == CMD_rest)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        act("$n sits down and fidgets.", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "Just for a moment. There are things that must die!\r\n");
        return 0;
    }
    if (cmd == CMD_cast)
    {
        if (IS_IMMORTAL(ch))
            return 0;
        act("$n sneers, 'A pox on all filthy magi and their weakling spells!'", TRUE, ch, 0, ch, TO_ROOM);
        cprintf(ch, "You have no need for weakling tricks!  FIGHT!\r\n");
        return 1;
    }
    if (IS_IMMORTAL(ch))
        return 0;
    if (!AWAKE(ch))
        return 0;
    if (check_peaceful(ch, ""))
        return 0;
    if (GET_POS(ch) == POSITION_FIGHTING)
        return 0;
    if (GET_POS(ch) == POSITION_RESTING)
    {
        if (number(0, 99) < 50)
        {
            act("$n jumps up and stares all around with a savage grin!", TRUE, ch, 0, ch, TO_ROOM);
            cprintf(ch, "You can't stand sitting still any longer!\r\n");
            StandUp(ch);
        }
        else
        {
            act("$n fidgets and grumbles about death.", TRUE, ch, 0, ch, TO_ROOM);
        }
    }

    for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
    {
        if (ch == victim)
            continue;
        if (IS_IMMORTAL(victim))
            continue;
        if (!CheckKill(ch, victim))
            continue;
        if (GetMaxLevel(victim) > MAX(5, (2 * GetMaxLevel(ch))))
            continue;
        if (IS_NPC(ch))
            if (GetMaxLevel(victim) < (GetMaxLevel(ch) / 2))
                continue;
        if (GET_RACE(victim) == RACE_DWARF)
            continue;
        if (GET_RACE(victim) == RACE_ELVEN)
        {
            act("$n charges $N, screaming 'DIE Filthy ELF!!!'", TRUE, ch, 0, victim, TO_ROOM);
            cprintf(ch, "Puny %s is elven!  You must KILL!!!!\r\n", NAME(victim));
            hit(ch, victim, TYPE_UNDEFINED);
            return 1;
        }
        if (number(0, 99) < 50)
            break;
    }
    if (victim)
    {
        if (number(0, 99) < 40)
        {
            act("$n charges $N, screaming 'You MUST DIE!!!'", TRUE, ch, 0, victim, TO_ROOM);
            cprintf(ch, "You cannot control your hatred of %s, Arrrghh!\r\n", NAME(victim));
            hit(ch, victim, TYPE_UNDEFINED);
            return 1;
        }
        else
        {
            act("$n stares at $N with undisguised contempt.", TRUE, ch, 0, victim, TO_ROOM);
            return 0;
        }
    }
    return 0;
}

/*
 * Assignments
 */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
    int i = 0;
    int rnum = 0;

    if (DEBUG > 3)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (i = 0; specials_m[i].vnum >= 0; i++)
    {
        rnum = real_mobile(specials_m[i].vnum);
        if (rnum < 0)
        {
            log_info("mobile_assign: Mobile %d not found in database.", specials_m[i].vnum);
        }
        else
        {
            mob_index[rnum].func = specials_m[i].proc;
            /*      log_info("mobile_assign: Mobile %d assigned function %d.",
                          specials_m[i].vnum,specials_m[i].proc); */
        }
    }

    boot_the_shops();
    assign_the_shopkeepers();
}

/* assign special procedures to objects */
void assign_objects(void)
{
    int i = 0;
    int onum = 0;

    if (DEBUG > 3)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (i = 0; specials_o[i].vnum >= 0; i++)
    {
        onum = real_object(specials_o[i].vnum);
        if (onum < 0)
        {
            log_info("negative object index?");
        }
        else
        {
            obj_index[onum].func = specials_o[i].proc;
        }
    }
    // InitBoards();
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
    int i = 0;
    struct room_data *rp = NULL;

    if (DEBUG > 3)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (i = 0; specials_r[i].vnum >= 0; i++)
    {
        rp = real_roomp(specials_r[i].vnum);
        if (rp == NULL)
        {
            log_info("assign_rooms: unknown room");
        }
        else
            rp->funct = specials_r[i].proc;
    }
}

const char *name_special_proc(int type, int vnum)
{
    int i = 0;
    struct special_proc_entry *spec = NULL;

    if (DEBUG > 3)
        log_info("called %s with %d, %d", __PRETTY_FUNCTION__, type, vnum);

    if (vnum < 0)
        return "Invalid VNUM";
    switch (type)
    {
    case SPECIAL_MOB:
        spec = specials_m;
        break;
    case SPECIAL_OBJ:
        spec = specials_o;
        break;
    case SPECIAL_ROOM:
        spec = specials_r;
        break;
    default:
        return "Invalid TYPE";
    }
    for (i = 0; spec[i].vnum >= 0; i++)
        if (spec[i].vnum == vnum)
            return spec[i].name;
    return "Unregistered";
}

/*
 * This routine cannot be used until we have mobs that can be assigned
 * classes at boot time.  When this is installed, a guildmaster can train
 * the class he is set to.  If he is not given a class, he will be able
 * to train generic skills to anyone... I may do away with this for simplicity
 * of coding in the future, since the benefits are small versus the added
 * conditional checking.
 * NOTE:  If a guildmaster is given multiple classes, the player MUST
 * then type 'prac class skill' or 'gain class'.  This would be trivial
 * except for generic skills...
 */
void gm_wrong_class(struct char_data *master, struct char_data *vict)
{
    if (DEBUG > 3)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(master), SAFE_NAME(vict));

    if (IS_EVIL(master))
    {
        if (IS_GOOD(vict))
        {
            cprintf(vict, "%s laughs, 'My knowledge would burn one of your ilk, BEGONE!'\r\n", NAME(master));
        }
        else if (IS_EVIL(vict))
        {
            cprintf(vict, "%s says, 'I cannot help you.  Leave me and seek another's aid.'\r\n", NAME(master));
        }
        else
        {
            cprintf(vict, "%s sneers, 'You are a fool.  Go find another fool to teach you!'\r\n", NAME(master));
        }
    }
    else if (IS_GOOD(master))
    {
        if (IS_EVIL(vict))
        {
            cprintf(vict, "%s cries, 'Repent!  I shall never teach one of your kind.'\r\n", NAME(master));
        }
        else if (IS_GOOD(vict))
        {
            cprintf(vict, "%s says, 'I cannot help you.  You follow a different path.'\r\n", NAME(master));
        }
        else
        {
            cprintf(vict, "%s says, 'You can learn nothing here.'\r\n", NAME(master));
        }
    }
    else
    {
        cprintf(vict, "%s says, 'I have nothing to teach you.'\r\n", NAME(master));
    }
}

void gm_wrong_alignment(struct char_data *master, struct char_data *vict)
{
    if (DEBUG > 3)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(master), SAFE_NAME(vict));

    if (IS_REALLY_VILE(master))
    {
        if (IS_VERY_GOOD(vict))
        {
            act("$n screams 'How dare you insult my presence, DIE!'", FALSE, master, 0, 0, TO_ROOM);
            hit(master, vict, TYPE_UNDEFINED);
        }
        else if (IS_GOOD(vict))
        {
            cprintf(vict, "%s cackles, 'Leave now, while you still cling to life!'\r\n", NAME(master));
        }
        else
        {
            cprintf(vict, "%s smirks, 'Simpering fool!  Begone, ere I feast upon your heart!'\r\n", NAME(master));
        }
    }
    else if (IS_EVIL(master))
    {
        if (IS_GOOD(vict))
        {
            cprintf(vict, "%s screams, 'BEGONE wretched minion of Light!'\r\n", NAME(master));
        }
        else
        {
            cprintf(vict, "%s sniffs, 'I shall have no traffic with indecisive fools!'\r\n", NAME(master));
        }
    }
    else if (IS_REALLY_HOLY(master))
    {
        if (IS_VERY_EVIL(vict))
        {
            act("$n cries 'How dare you defile this place, REPENT!'", FALSE, master, 0, 0, TO_ROOM);
            hit(master, vict, TYPE_UNDEFINED);
        }
        else if (IS_EVIL(vict))
        {
            cprintf(vict, "%s says, 'Go away foul wretch!  There is nothing for you here.'\r\n", NAME(master));
        }
        else
        {
            cprintf(vict, "%s says, 'I cannot help you until you repent and reject darkness.'\r\n", NAME(master));
        }
    }
    else if (IS_GOOD(master))
    {
        if (IS_EVIL(vict))
        {
            cprintf(vict, "%s exclaims, 'You cannot learn until you can see the light!'\r\n", NAME(master));
        }
        else
        {
            cprintf(vict, "%s says, 'Only in the light can you find knowledge.'\r\n", NAME(master));
        }
    }
    else
    {
        if (!IS_NEUTRAL(vict))
            cprintf(vict, "%s says, 'Only in balance can we truely learn anything.'\r\n", NAME(master));
    }
}

void gm_gain(struct char_data *master, struct char_data *vict, int target)
{
    if (DEBUG > 3)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(master), SAFE_NAME(vict), target);

    if (GET_LEVEL(vict, target) < GetMaxLevel(master) - 20)
    {
        cprintf(vict, "%s snorts, 'I will not teach such a novice!'\r\n", NAME(master));
    }
    else if (GET_LEVEL(vict, target) < GetMaxLevel(master) - 10)
    {
        GainLevel(master, vict, target);
    }
    else
    {
        cprintf(vict, "I can teach you nothing more %s.\r\n", NAME(vict));
    }
}

void gm_prac(struct char_data *master, struct char_data *vict, int target, const char *arg)
{
    int i = 0;
    int anumber = 0;
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 3)
        log_info("called %s with %s, %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(master), SAFE_NAME(vict), target,
                 VNULL(arg));

    if (!*arg)
    {
        sprintf(buf, "You have got %d practice sessions left.\r\n", vict->specials.pracs);
        sprintf(buf + strlen(buf), "I can teach you these spells:\r\n");
        for (i = 0; i < MAX_SKILLS; i++)
            if (CanCastClass(vict, i, target) && (spell_info[i].min_level[target] <= GET_LEVEL(master, target) - 10) &&
                (spell_info[i].min_level[target] >= GET_LEVEL(master, target) - 20))
            {
                sprintf(buf + strlen(buf), "[%d] %s %s \r\n", spell_info[i].min_level[target], spell_info[i].name,
                        how_good(vict->skills[i].learned));
            }
        page_string(vict->desc, buf, 1);
        return;
    }
    if ((anumber = GetSpellByName(arg)) == -1)
    {
        cprintf(vict, "I have never heard of this spell...\r\n");
        return;
    }
    if (spell_info[anumber].min_level[target] < GET_LEVEL(master, target) - 20)
    {
        cprintf(vict, "It would be beneath me to teach you such drivel!\r\n");
        return;
    }
    if (GET_LEVEL(master, target) - 10 < spell_info[anumber].min_level[target])
    {
        cprintf(vict, "I have not yet mastered that spell.\r\n");
        return;
    }
    if (GET_LEVEL(vict, target) < spell_info[anumber].min_level[target])
    {
        cprintf(vict, "You are too frail to cast this spell.\r\n");
        return;
    }
    if (vict->specials.pracs <= 0)
    {
        cprintf(vict, "You do not seem to be able to practice now.\r\n");
        return;
    }
    if (vict->skills[anumber].learned >= 95)
    {
        cprintf(vict, "You know it as well as I!\r\n");
        return;
    }
    cprintf(vict, "You Practice for a while...\r\n");
    vict->specials.pracs--;

    if ((vict->skills[anumber].learned += (int_app[(int)GET_INT(vict)].learn + fuzz(1))) >= 95)
    {
        vict->skills[anumber].learned = 95;
        cprintf(vict, "You are now a master of this art.\r\n");
    }
}

int GuildMaster(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *master = NULL;
    int i = 0;
    int myclasses = 0;
    int yourclasses = 0;
    int targetclass = 0;
    const char *argument = NULL;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!cmd)
    { /* actions of the guildmaster himself */
        return FALSE;
    }
    if ((cmd != CMD_practice) && (cmd != CMD_gain))
        return FALSE;
    if (!(master = FindMobInRoomWithFunction(ch->in_room, GuildMaster)))
        return FALSE;

    myclasses = HowManyClasses(master);
    yourclasses = HowManyClasses(ch);
    if (!myclasses)
    { /* I teach only generic skills */
    }
    else if (myclasses > 1)
    { /* you might learn more than one class from me */
        if (yourclasses > 1)
        { /* you must specify which class */
            if (!arg)
            {
                cprintf(ch, "%s asks, 'But for which class?'\r\n", NAME(master));
                return TRUE;
            }
            argument = one_argument(arg, buf);
            targetclass = -1;
            for (i = 0; i < ABS_MAX_CLASS; i++)
                if (is_abbrev(buf, class_name[i]))
                {
                    if (targetclass == -1)
                        targetclass = i;
                    else
                        break;
                }
            if (targetclass < 0)
            {
                cprintf(ch, "%s exclaims, 'That's not any class I've heard of!'\r\n", NAME(master));
                return TRUE;
            }
            if (!(HasClass(master, 1 << targetclass)))
            {
                gm_wrong_class(master, ch);
                return TRUE;
            }
            switch (cmd)
            {
            case CMD_gain:
                gm_gain(master, ch, targetclass);
                break;
            case CMD_practice:
                while (isspace(*argument))
                    argument++;
                gm_prac(master, ch, targetclass, argument);
                break;
            default:
                return FALSE;
            }
            return TRUE;
        }
        else if (HasClass(master, ch->player.chclass))
        {
            targetclass = ch->player.chclass;
            argument = arg;
            switch (cmd)
            {
            case CMD_gain:
                gm_gain(master, ch, targetclass);
                break;
            case CMD_practice:
                while (isspace(*argument))
                    argument++;
                gm_prac(master, ch, targetclass, argument);
                break;
            default:
                return FALSE;
            }
            return TRUE;
        }
        else
        {
            gm_wrong_class(master, ch);
            return TRUE;
        }
    }
    else
    { /* you must possess my class */
        if (HasClass(ch, master->player.chclass))
        {
            targetclass = ch->player.chclass;
            argument = arg;
            switch (cmd)
            {
            case CMD_gain:
                gm_gain(master, ch, targetclass);
                break;
            case CMD_practice:
                while (isspace(*argument))
                    argument++;
                gm_prac(master, ch, targetclass, argument);
                break;
            default:
                return FALSE;
            }
            return TRUE;
        }
        else
        { /* I can teach you nothing */
            gm_wrong_class(master, ch);
            return TRUE;
        }
    }
    return TRUE;
}

int k_tired(struct char_data *karrn)
{
    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(karrn));

    return GET_HIT(karrn) < GET_MAX_HIT(karrn) / 3 || GET_MANA(karrn) < 20;
}

int k_kill_aggressor(struct char_data *karrn)
{
    int maxlevel = 0;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(karrn));

    if (check_peaceful(karrn, ""))
        return FALSE;
    maxlevel = GetMaxLevel(karrn);
    if (karrn->specials.fighting && (karrn->specials.fighting->in_room == karrn->in_room))
    {
        act("$n hisses 'You shall regret that mortal!'", FALSE, karrn, 0, 0, TO_ROOM);
        if (IS_AFFECTED(karrn, AFF_PARALYSIS))
        {
            if (number(1, maxlevel) > 5)
            {
                say_spell(karrn, SPELL_REMOVE_PARALYSIS);
                cast_remove_paralysis(maxlevel, karrn, "", SPELL_TYPE_SPELL, karrn, 0);
            }
            else
                return TRUE;
        }
        StandUp(karrn);
        if (IS_AFFECTED(karrn, AFF_BLIND))
        {
            if (number(1, maxlevel) > 8)
            {
                say_spell(karrn, SPELL_CURE_BLIND);
                cast_cure_blind(maxlevel, karrn, "", SPELL_TYPE_SPELL, karrn, 0);
            }
            else
            {
                zm_zap_area_at(karrn, maxlevel);
                return TRUE;
            }
        }
        zm_zap_spell_at(karrn, karrn->specials.fighting, maxlevel);
        return TRUE;
    }
    return FALSE;
}

int k_kill_wimps(struct char_data *karrn)
{
    struct char_data *vict = NULL;
    int maxlevel = 0;
    int count = 0;
    int choice = 0;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(karrn));

    if (check_peaceful(karrn, ""))
        return FALSE;
    maxlevel = GetMaxLevel(karrn);
    for (count = 0, vict = real_roomp(karrn->in_room)->people; vict; vict = vict->next_in_room)
        count++;
    if (count > 1)
    { /* Karrn himself is always present */
        choice = number(0, count);

        for (vict = real_roomp(karrn->in_room)->people; vict; vict = vict->next_in_room)
        {
            if (vict->master != karrn && vict != karrn && (GET_POS(vict) > POSITION_DEAD))
            {
                if (!str_cmp(GET_NAME(vict), "Quixadhal"))
                {
                    if ((choice = number(0, 99)) < 10)
                    {
                        switch (choice)
                        {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            act("$n dances around $N and yips with delight!", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        case 5:
                        case 6:
                        case 7:
                            act("$N laughs as $n buffets him with his wings.", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        case 8:
                            act("$N waves to $n, who hops out and brings back a dark beer.", TRUE, karrn, 0, vict,
                                TO_ROOM);
                            break;
                        case 9:
                            act("$n grovels before $N!", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        }
                    }
                }
                else if (!str_cmp(GET_NAME(vict), "Muidnar"))
                {
                    if ((choice = number(0, 99)) < 10)
                    {
                        switch (choice)
                        {
                        case 0:
                            act("$n retches and gags as $N forgot to bathe.", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        case 1:
                            act("$n farts...the smell is hideous!", TRUE, karrn, 0, vict, TO_ROOM);
                            act("$N gags and doubles over.", TRUE, karrn, 0, vict, TO_ROOM);
                            cast_poison(maxlevel, karrn, "", SPELL_TYPE_SPELL, vict, 0);
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            act("$n laughs, \"Mock ME will you Muidnar?\"", TRUE, karrn, 0, vict, TO_ROOM);
                            say_spell(karrn, SPELL_LIGHTNING_BOLT);
                            cast_lightning_bolt(200, karrn, "", SPELL_TYPE_SPELL, vict, 0);
                            act("$n growls, \"Stupid...\"", TRUE, karrn, 0, vict, TO_ROOM);
                            cast_lightning_bolt(200, karrn, "", SPELL_TYPE_SPELL, vict, 0);
                            act("$n growls, \"Idiotic...\"", TRUE, karrn, 0, vict, TO_ROOM);
                            cast_lightning_bolt(200, karrn, "", SPELL_TYPE_SPELL, vict, 0);
                            act("$n growls, \"DemiGOD!\"", TRUE, karrn, 0, vict, TO_ROOM);
                            say_spell(karrn, SPELL_ENERGY_DRAIN);
                            cast_energy_drain(200, karrn, "", SPELL_TYPE_SPELL, vict, 0);
                            break;
                        case 6:
                        case 7:
                            act("$n lifts his leg and pisses on $N.", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        case 8:
                            act("$n coughs up a huge hairball on $N.", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        case 9:
                            act("$n grovels before $N!", TRUE, karrn, 0, vict, TO_ROOM);
                            break;
                        }
                    }
                }
                else
                {
                    act("$n hisses 'Disturb me NOT!  I shall eat now foolish mortal.'", FALSE, karrn, 0, 0, TO_ROOM);
                    say_spell(karrn, SPELL_ACID_BLAST);
                    cast_acid_blast(maxlevel * 2, karrn, "", SPELL_TYPE_SPELL, vict, 0);
                    if (number(0, 99) < 10)
                        zm_zap_area_at(karrn, maxlevel);
                    else
                        zm_zap_spell_at(karrn, vict, maxlevel);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

int Karrn(struct char_data *ch, int cmd, const char *arg)
{
    struct char_data *karrn = NULL;
    struct char_data *i = NULL;
    short int target = 0;
    struct char_data *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    karrn = find_mobile_here_with_spec_proc(Karrn, ch->in_room);

    if (cmd != 0 || ch != karrn || !AWAKE(ch))
        return FALSE;

    if (k_kill_aggressor(karrn) || k_kill_wimps(karrn))
        return TRUE;

    switch (GET_POS(karrn))
    {
    case POSITION_RESTING:
        if (!k_tired(karrn))
            do_stand(karrn, "", -1);
        break;
    case POSITION_SITTING:
        do_stand(karrn, "", -1);
        break;
    case POSITION_STANDING:
        if (k_tired(karrn))
        {
            do_rest(karrn, "", -1);
            return TRUE;
        }
        else
        {
#if 1
            if (number(0, 99) < 10)
            {
                for (i = character_list; i; i = i->next)
                {
                    if (IS_PC(i) && IS_IMMORTAL(i) && i != karrn && i->in_room != karrn->in_room)
                    {
                        if (!str_cmp(GET_NAME(i), "Muidnar"))
                        {
                            tmp = i;
                            target = karrn->in_room;
                            do_shout(karrn, "Muidnar!  Get over here you BASTARD!", 0);
                            act("A red slice of fire opens before $n, who is enveloped screaming, then silence.", TRUE,
                                tmp, 0, 0, TO_ROOM);
                            if (MOUNTED(tmp))
                            {
                                char_from_room(tmp);
                                char_from_room(MOUNTED(tmp));
                                char_to_room(tmp, target);
                                char_to_room(MOUNTED(tmp), target);
                                act("$n emerges from a column of fire that Karrn dismisses with a flick of his tail.",
                                    FALSE, tmp, 0, 0, TO_ROOM);
                                if (IS_PC(tmp))
                                    do_look(tmp, "", 15);
                                if (IS_PC(tmp))
                                    do_look(MOUNTED(tmp), "", 15);
                            }
                            else if (RIDDEN(tmp))
                            {
                                char_from_room(RIDDEN(tmp));
                                char_from_room(tmp);
                                char_to_room(RIDDEN(tmp), target);
                                char_to_room(tmp, target);
                                act("$n emerges from a column of fire that Karrn dismisses with a flick of his tail.",
                                    FALSE, tmp, 0, 0, TO_ROOM);
                                if (IS_PC(tmp))
                                    do_look(tmp, "", 15);
                                if (IS_PC(tmp))
                                    do_look(RIDDEN(tmp), "", 15);
                            }
                            else
                            {
                                char_from_room(tmp);
                                char_to_room(tmp, target);
                                act("$n emerges from a column of fire that Karrn dismisses with a flick of his tail.",
                                    FALSE, tmp, 0, 0, TO_ROOM);
                                if (IS_PC(tmp))
                                    do_look(tmp, "", 15);
                            }
                        }
                    }
                }
            }
#endif
            /*
             * mobile_wander(karrn);
             */
        }
    }
    return FALSE;
}

char *MobFunctionNameByFunc(ifuncp func)
{
    int i = 0;
    static char mobname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 3)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)func);

    mobname[0] = '\0';
    for (i = 0; specials_m[i].vnum > 0; i++)
        if (specials_m[i].proc == func)
            strncpy(mobname, specials_m[i].name, MAX_INPUT_LENGTH - 1);
    return mobname;
}
