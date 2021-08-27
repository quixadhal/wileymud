/*
 * file: Interpreter.h , Command interpreter module.      Part of DIKUMUD
 * Usage: Procedures interpreting user command
 */

#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#define COMMANDO(number, min_pos, pointer, min_level)                                                                  \
    {                                                                                                                  \
        cmd_info[(number)].command_pointer = (pointer);                                                                \
        cmd_info[(number)].minimum_position = (min_pos);                                                               \
        cmd_info[(number)].minimum_level = (min_level);                                                                \
    }

#define NOT !
#define AND &&
#define OR ||

#define WELC_MESSG VERSION_STR

#define STATE(d) ((d)->connected)
#define MAX_CMD_LIST 300

/* Here we Go!  The monster command list.... ewwwwwww! */
/* 292 is the current maximum */
#define CMD_EMOTE 214
#define CMD_GTELL 177
#define CMD_INTERNAL 0 /* NOT initiated from the command line */
#define CMD_SAY 169
#define CMD_TELL 229
#define CMD_WIZNET 228
#define CMD_accuse 96 /* social */
#define CMD_advance 95
#define CMD_allcommands 251
#define CMD_ansimap 286
#define CMD_appear 254
#define CMD_applaud 104 /* social */
#define CMD_apraise 263
#define CMD_ask 86
#define CMD_assist 235
#define CMD_at 85
#define CMD_autoexit 288
#define CMD_backstab 154
#define CMD_balance 221
#define CMD_ban 264
#define CMD_bandage 283
#define CMD_bash 157
#define CMD_beg 178   /* social */
#define CMD_bleed 179 /* social */
#define CMD_blush 105 /* social */
#define CMD_bonk 246  /* social */
#define CMD_bounce 22 /* social */
#define CMD_bow 98    /* social */
#define CMD_breath 239
#define CMD_brief 199
#define CMD_bug 82
#define CMD_burp 106 /* social */
#define CMD_bury 279
#define CMD_buy 56
#define CMD_cackle 26 /* social */
#define CMD_cast 84
#define CMD_chuckle 107 /* social */
#define CMD_clap 108    /* social */
#define CMD_close 100
#define CMD_comb 161   /* social */
#define CMD_comfort 34 /* social */
#define CMD_compact 213
#define CMD_consider 201
#define CMD_cough 109 /* social */
#define CMD_credits 212
#define CMD_cringe 180   /* social */
#define CMD_cry 53       /* social */
#define CMD_cuddle 51    /* social */
#define CMD_curse 171    /* social */
#define CMD_curtsey 110  /* social */
#define CMD_dance 24     /* social */
#define CMD_daydream 181 /* social */
#define CMD_debug 241
#define CMD_deposit 219
#define CMD_desecrate 280
#define CMD_disarm 245
#define CMD_dismount 270
#define CMD_doh 226
#define CMD_doorbash 267
#define CMD_down 6
#define CMD_drink 11
#define CMD_drop 60
#define CMD_east 2
#define CMD_eat 12
#define CMD_echo 41
#define CMD_emote 40
#define CMD_enter 7
#define CMD_equipment 55
#define CMD_event 276
#define CMD_examine 166
#define CMD_exits 8
#define CMD_fart 111 /* social */
#define CMD_flee 151
#define CMD_flip 112 /* social */
#define CMD_follow 91
#define CMD_fondle 113 /* social */
#define CMD_force 47
#define CMD_french 160 /* social */
#define CMD_frown 114  /* social */
#define CMD_fume 182   /* social */
#define CMD_gain 243
#define CMD_gasp 115 /* social */
#define CMD_get 10
#define CMD_giggle 28 /* social */
#define CMD_give 72
#define CMD_glare 116 /* social */
#define CMD_goto 61
#define CMD_grab 65
#define CMD_grep 252
#define CMD_grin 97   /* social */
#define CMD_groan 117 /* social */
#define CMD_grope 118 /* social */
#define CMD_group 202
#define CMD_grovel 183 /* social */
#define CMD_growl 31   /* social */
#define CMD_gtell 249
#define CMD_guard 75
#define CMD_help 38
#define CMD_hermit 215
#define CMD_hiccup 119 /* social */
#define CMD_hide 153
#define CMD_highfive 232
#define CMD_hint 291
#define CMD_hit 70
#define CMD_hold 150
#define CMD_home 282
#define CMD_hop 184 /* social */
#define CMD_hug 49  /* social */
#define CMD_idea 80
#define CMD_immtrack 285
#define CMD_info 168
#define CMD_insult 33 /* social */
#define CMD_inventory 20
#define CMD_invisible 242
#define CMD_json 292
#define CMD_junk 218
#define CMD_kick 159
#define CMD_kill 9
#define CMD_kiss 25 /* social */
#define CMD_land 271
#define CMD_laugh 27 /* social */
#define CMD_leave 103
#define CMD_levels 174
#define CMD_lick 120 /* social */
#define CMD_list 59
#define CMD_load 77
#define CMD_lock 101
#define CMD_logs 255
#define CMD_look 15
#define CMD_love 121 /* social */
#define CMD_map 170
#define CMD_massage 162 /* social */
#define CMD_mkzone 244
#define CMD_moan 122 /* social */
#define CMD_mount 269
#define CMD_news 54
#define CMD_nibble 123 /* social */
#define CMD_nod 35     /* social */
#define CMD_nohassle 222
#define CMD_north 1
#define CMD_noshout 210
#define CMD_nosummon 272
#define CMD_noteleport 273
#define CMD_notell 261
#define CMD_nudge 185 /* social */
#define CMD_nuzzle 52 /* social */
#define CMD_offer 93
#define CMD_open 99
#define CMD_order 87
#define CMD_pager 253
#define CMD_pat 165 /* social */
#define CMD_peer 186
#define CMD_pick 155
#define CMD_players 274
#define CMD_point 187  /* social */
#define CMD_poke 94    /* social */
#define CMD_ponder 188 /* social */
#define CMD_pose 209
#define CMD_pour 64
#define CMD_pout 124 /* social */
#define CMD_practice 164
#define CMD_pray 176
#define CMD_pretitle 250
#define CMD_pset 227
#define CMD_puke 30 /* social */
#define CMD_pull 224
#define CMD_punch 189
#define CMD_purge 78
#define CMD_purr 125 /* social */
#define CMD_put 67
#define CMD_quaff 206
#define CMD_qui 21
#define CMD_quit 73
#define CMD_read 63
#define CMD_reboot 289
#define CMD_recite 207
#define CMD_register 257
#define CMD_remove 66
#define CMD_rent 92
#define CMD_rentmode 248
#define CMD_reroll 175
#define CMD_rescue 158
#define CMD_reset 275
#define CMD_rest 44
#define CMD_restore 203
#define CMD_restoreall 268
#define CMD_return 204
#define CMD_ruffle 126 /* social */
#define CMD_save 69
#define CMD_say 17
#define CMD_score 16
#define CMD_scream 32 /* social */
#define CMD_scribe 262
#define CMD_search 265
#define CMD_sell 57
#define CMD_send 258
#define CMD_sethome 256
#define CMD_setreboot 281
#define CMD_shake 29   /* social */
#define CMD_shiver 127 /* social */
#define CMD_shout 18
#define CMD_show 240
#define CMD_shrug 128 /* social */
#define CMD_shutdow 68
#define CMD_shutdown 79
#define CMD_sigh 36  /* social */
#define CMD_sing 129 /* social */
#define CMD_sip 88
#define CMD_sit 43
#define CMD_skills 266
#define CMD_slap 130 /* social */
#define CMD_slay 216
#define CMD_sleep 45
#define CMD_smile 23  /* social */
#define CMD_smirk 131 /* social */
#define CMD_snap 132  /* social */
#define CMD_snarl 190 /* social */
#define CMD_sneak 152
#define CMD_sneeze 133  /* social */
#define CMD_snicker 134 /* social */
#define CMD_sniff 135   /* social */
#define CMD_snoop 90
#define CMD_snore 136 /* social */
#define CMD_snowball 148
#define CMD_snuggle 50 /* social */
#define CMD_south 3
#define CMD_spank 191 /* social */
#define CMD_spells 238
#define CMD_spit 137 /* social */
#define CMD_split 260
#define CMD_squeeze 138 /* social */
#define CMD_stand 42
#define CMD_stare 139 /* social */
#define CMD_stat 74
#define CMD_steal 156
#define CMD_stealth 225
#define CMD_steam 192 /* social */
#define CMD_string 71
#define CMD_strut 140 /* social */
#define CMD_sulk 37   /* social */
#define CMD_swat 236
#define CMD_switch 205
#define CMD_tackle 193 /* social */
#define CMD_take 167
#define CMD_taste 89
#define CMD_taunt 194 /* social */
#define CMD_tell 19
#define CMD_thank 141  /* social */
#define CMD_think 195  /* social */
#define CMD_tickle 163 /* social */
#define CMD_ticks 278
#define CMD_time 76
#define CMD_title 233
#define CMD_track 230
#define CMD_transfer 48
#define CMD_twiddle 142 /* social */
#define CMD_typo 81
#define CMD_unban 284
#define CMD_unlock 102
#define CMD_up 5
#define CMD_checkurl 290
#define CMD_use 172
#define CMD_users 208
#define CMD_value 58
#define CMD_version 287
#define CMD_wake 46
#define CMD_wall 223
#define CMD_wave 143 /* social */
#define CMD_wear 13
#define CMD_weather 62
#define CMD_west 4
#define CMD_where 173
#define CMD_whine 196 /* social */
#define CMD_whisper 83
#define CMD_whistle 144 /* social */
#define CMD_who 39
#define CMD_whod 259
#define CMD_whozone 234
#define CMD_wield 14
#define CMD_wiggle 145 /* social */
#define CMD_wimp 217
#define CMD_wink 146 /* social */
#define CMD_withdraw 220
#define CMD_wizhelp 211
#define CMD_wizlist 200
#define CMD_wizlock 231
#define CMD_wiznet 247
#define CMD_world 237
#define CMD_worship 197 /* social */
#define CMD_write 149
#define CMD_yawn 147  /* social */
#define CMD_yodel 198 /* social */
#define CMD_zpurge 277

struct command_info
{
    // void (*command_pointer)(struct char_data *ch, const char *argument, int cmd);
    cfuncp command_pointer;
    char minimum_position;
    char minimum_level;
};

#ifndef _INTERPRETER_C
extern struct command_info cmd_info[MAX_CMD_LIST];
extern const char echo_on[];
extern const char echo_off[];
extern const char *command[];
extern const char *fill[];

#endif

int search_block(const char *arg, const char **list, char exact);
int old_search_block(const char *argument, int begin, int arglen, const char **list, int mode);
void old_command_interpreter(struct char_data *ch, char *argument);
void command_interpreter(struct char_data *ch, char *argument);
void argument_interpreter(const char *argument, char *first_arg, char *second_arg);
int is_number(const char *str);
const char *one_argument(const char *argument, char *first_arg);
void only_argument(const char *argument, char *dest);
int fill_word(char *argument);
int is_abbrev(const char *arg1, const char *arg2);
void half_chop(const char *str, char *arg1, char *arg2);
int special(struct char_data *ch, int cmd, const char *arg);
void assign_command_pointers(void);
int find_name(char *name);
int _parse_name(char *arg, char *name);
int valid_parse_name(const char *arg, char *name);
int already_mob_name(char *ack_name);
int check_reconnect(struct descriptor_data *d);
int check_playing(struct descriptor_data *d, char *tmp_name);
void nanny(struct descriptor_data *d, char *arg);
void update_player_list_entry(struct descriptor_data *ch);
/* void                                    PutPasswd(struct descriptor_data *d); */
int ValidPlayer(char *who, char *pwd, char *oldpwd);

#endif
