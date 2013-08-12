#ifndef _STRUCTS_H
#define _STRUCTS_H

typedef struct vnum {
    int                                     Number;
    long                                    Line;
    long                                    Pos;
} vnum;

typedef struct vnum_index {
    int                                     Count;
    vnum                                   *VNum;
} vnum_index;

typedef struct pair {
    int                                     x;
    int                                     y;
} pair;

typedef struct keyword {
    int                                     Count;
    char                                  **Word;
} keyword;

typedef struct zone_cmds {
    char                                    Command;	       /* command to execute, MOGEDRPLH: M - load mobile to
							        * room O - load object to room G - give object to
							        * previous mob E - equip object to previous mob D - set 
							        * * door state in room R - remove object in room P -
							        * put object in object L - load mob who is led by last
							        * M H - hatred flags */
    int                                     IfFlag;	       /* only execute if the last command worked */
    int                                     Arg[3];	       /* args read from zone command line */
} zone_cmds;

typedef struct zone {
    int                                     Number;	       /* zone virtual number */
    char                                   *Name;	       /* zone name */
    int                                     Top;	       /* highest room vnum allowed in this zone */
    int                                     Time;	       /* time between zone resets (minutes) */
    int                                     Mode;	       /* mode of zone reset: 0 - never reset 1 - only reset if 
							        * no PC's are in the zone 2 - always reset on time */
    int                                     Count;	       /* number of zone commands in this zone */
    zone_cmds                              *Cmds;	       /* list of commands to be executed in zone */
} zone;

typedef struct zones {
    int                                     Count;	       /* number of zones in universe */
    zone                                   *Zone;	       /* list of zone data */
} zones;

typedef struct EXIT {
    int                                     Direction;
    char                                   *Description;
    keyword                                *Keyword;
    int                                     Type;
    int                                     KeyNumber;
    int Room;
    int                                     RoomIndex;
    int                                     Error;
} EXIT;

typedef struct extra {
    char                                   *Description;
    keyword                                *Keyword;
} extra;

typedef struct coordinate {
    int                                     x;
    int                                     y;
    int                                     z;
} coordinate;

typedef struct room {
    int                                     Number;	       /* virtual room number */
    int                                     Colour;	       /* used for graph colouration */
    coordinate                              Location;	       /* used for graphing calculations */
    char                                   *Name;	       /* name/short description */
    char                                   *Description;       /* long description */
    int                                     Zone;	       /* zone room is supposed to belong to */
    int                                     Flags;	       /* room flags */
    int                                     Sector;	       /* sector type */
    int                                     SoundCount;	       /* how many sounds are actually used? */
    char                                   *SoundText[2];      /* exactly two sounds allowed */
    int                                     TeleportTime;      /* how long until teleport, if used */
    int                                     TeleportTo;	       /* target room for teleport */
    int                                     TeleportLook;      /* do we automatically look after teleport? */
    int                                     TeleportSector;    /* what sector type for movement purposes? */
    int                                     RiverSpeed;	       /* how fast is the (possible) river moving? */
    int                                     RiverDirection;    /* what direction will it send us in? */
    int                                     ExitCount;	       /* how many exits are really used? */
    EXIT                                   *Exit;	       /* standard 6 diku exits, NSEWUD */
    int                                     ExtraCount;	       /* how many extra descriptions */
    extra                                  *Extra;	       /* unknown number of extra descriptions */
    int                                     MoveCost;	       /* movement cost of sector */
} room;

typedef struct rooms {
    int                                     Count;	       /* number of rooms in universe */
    room                                   *Room;	       /* list of room data */
} rooms;

typedef struct shop {
    int                                     Number;	       /* zone virtual number */
    int                                     Sell[5];	       /* object VNUMs sold (up to 5) infinite supply */
    int                                     SellCount;	       /* redundant number of objects for sale */
    double                                  Selling;	       /* selling profit (must be >= 1.0) */
    double                                  Buying;	       /* buying profit (must be <= 1.0) */
    int                                     Trade[5];	       /* object TYPES for trade (up to 5) */
    int                                     TradeCount;	       /* redundant number of types for trade */
    char                                   *Message[7];	       /* messages from keepers (7 of them) %s is shopkeeper,
							        * %d is coin value 0 - trying to buy something not
							        * there 1 - trying to sell something not there 2 -
							        * trying to sell wrong type of item 3 - shop cannot
							        * afford to buy item 4 - player cannot afford to buy
							        * item 5 - player bought item for %d 6 - player sold
							        * item for %d */
    int                                     MessageCount;      /* redundant number of messages */
    /*
     * int Temper[2]; * messages when keepers lose their temper (2) * 0 - player cannot afford an item: * 0 - The
     * shopkeeper spits player in the face. * 1 - The shopkeeper smokes his joint. * 1 - player attempts to kill
     * shopkeeper: * 0 - Shopkeeper tells player "Don't ever try * that again!", and gets the first hit. * 1 -
     * Shopkeeper tells player "I'm too * powerful for you - midget!", and * killing is impossible. 
     */
    int                                     Attitude;	       /* This is just an expansion of Temper[0] */
    int                                     Immortal;	       /* This is an expansion of Temper[1] */
    int                                     Keeper;	       /* keeper Mobile vnum */
    int                                     Unused;	       /* odd, a flag that is always zero??? */
    int Room;						       /* room vnum to place the shop in */
    /*
     * int Hours[2][2];* Hours the shop is open... * 0 - actual hours. * 0 - open at hour X, use a zero in Wiley. * 1 - 
     * close at hour X, Wiley 28 means never. * 1 - unused. 
     */
    int                                     Open;	       /* this is only an expansion of Hours[0][0] */
    int                                     Close;	       /* and this is for Hours[0][1] */
    int                                     Unused2[2];	       /* the other hours fields */
} shop;

typedef struct shops {
    int                                     Count;	       /* number of zones in universe */
    shop                                   *Shop;	       /* list of zone data */
} shops;

typedef struct obj_flags {
    int                                     Value[4];	       /* Values of the item (see list) */
    int                                     Wear;	       /* Where you can wear it */
    int                                     Extra;	       /* If it hums,glows etc */
    int                                     Bits;	       /* To set chars bits */
} obj_flags;

typedef struct obj_affect {
    int                                     location;	       /* Which ability to change (APPLY_XXX) */
    int                                     modifier;	       /* How much it changes by */
} obj_affect;

typedef struct object {
    int                                     Number;	       /* virtual object number */
    char                                   *Name;	       /* name/id-list */
    char                                   *ShortDesc;	       /* used for inventory, etc... */
    char                                   *ActionDesc;	       /* Shown when used... */
    char                                   *Description;       /* In-Room description... */
    int                                     Zone;	       /* zone object is supposed to belong to */
    int                                     ExtraCount;	       /* how many extra descriptions */
    extra                                  *Extra;	       /* unknown number of extra descriptions */
    int                                     AffectCount;       /* how many affects? */
    obj_affect                             *Affect;	       /* affects the object may have */
    int                                     Type;	       /* type of object */
    int                                     Weight;	       /* weight */
    int                                     Value;	       /* value of object when sold */
    int                                     Rent;	       /* rent cost per day */
    int                                     Timer;	       /* time to disintigration, etc. */
    obj_flags                               Flags;	       /* various flags */
} object;

typedef struct objects {
    int                                     Count;	       /* number of objects in universe */
    object                                 *Object;	       /* list of object data */
} objects;

typedef struct dice {
    int                                     Rolls;
    int                                     Die;
    int                                     Modifier;
} dice;

typedef struct attack {
    int                                     Rolls;
    int                                     Die;
    int                                     Modifier;
    int                                     Type;
} attack;

typedef struct skill {
    int                                     Number;
    int                                     Learned;
    int                                     Recognise;
} skill;

typedef struct mob {
    int                                     Number;	       /* virtual mob number */
    char                                   *Name;	       /* name/id-list */
    char                                   *ShortDesc;	       /* used for fighting, etc... */
    char                                   *LongDesc;	       /* standing in the room... */
    char                                   *Description;       /* Look-at description... */
    int                                     Zone;	       /* zone mob is supposed to belong to */
    int                                     ActFlags;	       /* ACT_Flags... ACT_ISNPC is implied */
    int                                     AffectedBy;	       /* AFF_Flags... */
    int                                     Alignment;	       /* good, evil, whatever */
    char                                    Type;	       /* mob letter to denote type, M, S, D, C */
/* Type C fields first */
    int                                     Race;	       /* race, class, sex, height, weight */
    int                                     Class;
    int                                     Sex;
    int                                     Height;
    int                                     Weight;
    dice                                    Gold;	       /* gold */
    dice                                    Experience;	       /* experience */
    int                                     Level;	       /* level, all classes in Class bitmask or warrior if
							        * none */
    dice                                    HitPoints;	       /* hp */
    int                                     ManaPoints;	       /* not read in, defaults to 100 */
    int                                     MovePoints;	       /* not read in, defaults to 100 */
    int                                     ArmourClass;       /* AC, 10 * value */
    int                                     ToHit;	       /* To-Hit roll, 20 - value */
    int                                     Immunity;	       /* M_immune */
    int                                     Resistance;	       /* immune */
    int                                     Susceptible;       /* susc */
    dice                                    Strength;
    dice                                    ExtraStrength;
    dice                                    Dexterity;
    dice                                    Constitution;
    dice                                    Intelligence;
    dice                                    Wisdom;
    int                                     SavingThrow[5];    /* five saving throws, max(20 - value, 2) */
    int                                     Position;	       /* We load in, in this position */
    int                                     DefaultPosition;   /* Our "normal" position */
    char                                   *Sound;	       /* sounds for the room we are in */
    char                                   *DistantSound;      /* adjacent room sounds */
    int                                     AttackCount;       /* Number of attacks, if < 0, set to 1 */
    attack                                 *Attack;
    int                                     SkillCount;
    skill                                  *Skill;
} mob;

typedef struct mobs {
    int                                     Count;	       /* number of mobs in universe */
    mob                                    *Mob;	       /* list of mob data */
} mobs;

#endif
