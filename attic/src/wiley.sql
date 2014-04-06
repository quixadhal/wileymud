--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'SQL_ASCII';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: banned; Type: TABLE; Schema: public; Owner: wiley; Tablespace: 
--

CREATE TABLE banned (
    banned_name text,
    banned_ip inet,
    banned_by text DEFAULT 'SYSTEM'::text,
    banned_date timestamp without time zone DEFAULT now()
);


ALTER TABLE public.banned OWNER TO wiley;

--
-- Name: board_messages; Type: TABLE; Schema: public; Owner: wiley; Tablespace: 
--

CREATE TABLE board_messages (
    board_id integer NOT NULL,
    message_id integer NOT NULL,
    message_date timestamp without time zone DEFAULT now() NOT NULL,
    message_sender text NOT NULL,
    message_header text NOT NULL,
    message_text text NOT NULL
);


ALTER TABLE public.board_messages OWNER TO wiley;

--
-- Name: log_types; Type: TABLE; Schema: public; Owner: wiley; Tablespace: 
--

CREATE TABLE log_types (
    log_type_id integer NOT NULL,
    name text NOT NULL,
    description text
);


ALTER TABLE public.log_types OWNER TO wiley;

--
-- Name: TABLE log_types; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON TABLE log_types IS 'Log message types';


--
-- Name: COLUMN log_types.log_type_id; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN log_types.log_type_id IS 'What kind of log message is this?';


--
-- Name: COLUMN log_types.name; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN log_types.name IS 'What do we call it?';


--
-- Name: COLUMN log_types.description; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN log_types.description IS 'What does it get used for?';


--
-- Name: logfile; Type: TABLE; Schema: public; Owner: wiley; Tablespace: 
--

CREATE TABLE logfile (
    log_type_id integer,
    log_date timestamp without time zone DEFAULT now(),
    log_entry text,
    log_file text,
    log_function text,
    log_line integer,
    log_areafile text,
    log_arealine integer,
    log_pc_actor text,
    log_pc_victim text,
    log_npc_actor integer,
    log_npc_victim integer,
    log_obj integer,
    log_area integer,
    log_room integer
);


ALTER TABLE public.logfile OWNER TO wiley;

--
-- Name: TABLE logfile; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON TABLE logfile IS 'Log messages';


--
-- Name: COLUMN logfile.log_type_id; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_type_id IS 'What kind of log message is this?';


--
-- Name: COLUMN logfile.log_date; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_date IS 'Time this log entry was created';


--
-- Name: COLUMN logfile.log_entry; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_entry IS 'Actual message';


--
-- Name: COLUMN logfile.log_file; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_file IS 'C source file of error call';


--
-- Name: COLUMN logfile.log_function; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_function IS 'C function of error caller';


--
-- Name: COLUMN logfile.log_line; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_line IS 'Line number of error call in C source';


--
-- Name: COLUMN logfile.log_areafile; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_areafile IS 'Area file being loaded at error point';


--
-- Name: COLUMN logfile.log_arealine; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_arealine IS 'Error point line number in area file';


--
-- Name: COLUMN logfile.log_pc_actor; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_pc_actor IS 'Player which caused the event';


--
-- Name: COLUMN logfile.log_pc_victim; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_pc_victim IS 'Player that the event happened to';


--
-- Name: COLUMN logfile.log_npc_actor; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_npc_actor IS 'Mobile which caused the event';


--
-- Name: COLUMN logfile.log_npc_victim; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_npc_victim IS 'Mobile that the event happened to';


--
-- Name: COLUMN logfile.log_obj; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_obj IS 'Object which caused the error';


--
-- Name: COLUMN logfile.log_area; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_area IS 'Area the actor was in when the error happened';


--
-- Name: COLUMN logfile.log_room; Type: COMMENT; Schema: public; Owner: wiley
--

COMMENT ON COLUMN logfile.log_room IS 'Room the actor was in when the error happened';


--
-- Name: log_today; Type: VIEW; Schema: public; Owner: wiley
--

CREATE VIEW log_today AS
    SELECT to_char(date_trunc('second'::text, logfile.log_date), 'HH24:MI:SS'::text) AS log_date, log_types.name AS log_type, logfile.log_entry FROM (logfile JOIN log_types USING (log_type_id)) WHERE (logfile.log_date > (now() - '1 day'::interval)) ORDER BY logfile.log_date DESC;


ALTER TABLE public.log_today OWNER TO wiley;

--
-- Data for Name: banned; Type: TABLE DATA; Schema: public; Owner: wiley
--

COPY banned (banned_name, banned_ip, banned_by, banned_date) FROM stdin;
fuck	\N	SYSTEM	2008-10-02 05:07:09.606346
shit	\N	SYSTEM	2008-10-02 05:07:09.62904
asshole	\N	SYSTEM	2008-10-02 05:07:09.631872
fucker	\N	SYSTEM	2008-10-02 18:54:25.235557
\.


--
-- Data for Name: board_messages; Type: TABLE DATA; Schema: public; Owner: wiley
--

COPY board_messages (board_id, message_id, message_date, message_sender, message_header, message_text) FROM stdin;
3098	0	1993-02-02 20:55:32	Cyric	Titles	Please don't give titles that are misleading to a characters\r\nlevel, avatar is rather high on my list :) even if it is for a\r\npiece of tail...grin\r\n\r\nCyric\r\n
3098	1	1993-02-04 12:06:43	Harlequin	titles	*cackle* I know, I was not aware of the Reserved status of Avatar\r\nuntil Tim mentioned it to me.  I was going to change it next time\r\nI was on, but ya beat me to it...:)\r\n
3098	2	1993-02-04 13:55:46	Cyric	Tail	well, did you get it :)\r\n
3098	3	1993-02-04 17:11:54	Harlequin	Re: Tail	Hell no.  You know me, I never get anythng anymore.  Hey, I\r\nnoticed an odd thing.  I was polymorphed in the shape of a giant\r\ntick and was killed by succubus.  I just returned to my original\r\nbody.  Is this because I'm an immort, or will that happen to\r\nplayers also?  Shouldn't they die if they die in polymorphed\r\nform?\r\n
3098	4	1993-02-08 23:04:32	Muth	Who/Pager	Suggestion:\r\nCyric, could you please put a page break in the command 'who'.\r\nSo many people now it scrolls off.  8)\r\n\r\nThanks!\r\n<M>\r\n
3098	5	1993-02-11 00:24:07	Muidnar	names	Well I was on and I had person come on the game as *sshole. the\r\nsite number is rs6007.esc.rpi.edu put when I tried to finger it I\r\ncouldn't?? What do you do in this case? purge <name> didn;t work\r\nand well... um slay killed him ;)\r\n\r\n                          See ya\r\n
3098	6	1993-02-12 00:15:08	Muidnar	Charm?	You know I've only been a Wiz for a little while and it seems I'm\r\ngoing drive you nuts. Well here we go.\r\n1. What do you do about mutli char by one person.\r\n2. Charm person works. Goodwind is running around using it. On a\r\n   lot of people.\r\n3. MUTH people LUV Drenth!  just though you should know :)\r\n\r\n                                 Later \r\n
3098	7	1993-02-12 09:31:49	Muth	Charm	-= People with multiple characters must quit with one or more\r\n   leaving only one on the game.  Players are given 3 warnings on\r\n   this subject, each time a warning is given out post it to the\r\n   board here so we all know what the situation is if we catch that\r\n   same person later.  Help rules explains what the warnings mean i\r\n   think.\r\n-= Charm person works?  Cyrics department sorry.  cackle\r\n-= People love drenth?  Great!  I was cackling to to myself\r\n   working on it hoping it would cause mass suffrage, and a bit of a\r\n   challenge.  8)\r\n\r\n<M>\r\n
3098	8	1993-02-13 16:49:45	Muidnar	board	well there I saw a character on today 11/11/11 his name is\r\naladdin and is character is only 17 years old 1 month so that\r\nwould mean the char has been around for a not even a week??? Just\r\ngeneral info\r\n
3098	9	1993-02-13 18:12:56	Muidnar	Grrrrr	Well I gave Xavier,Malrot,Beldore,Voytek there first warning for\r\nmulit char playing. I must admit it was a very nice killing group\r\n:)\r\n
3098	10	1993-02-13 22:21:05	Daisy	Re; board	:)  I set up a character on Friday for taking small groups of\r\nnewer payers out on Friday [Aladdin].   Or even large groups.  :)\r\nDidn't mean to make it  all 11th level but was too lazy to drain\r\nit back down.  It's not going to be used for anything but this\r\npurpose.  I seem to recall Cyric saying this was okay to do.\r\nCorrect me if I'm wrong...\r\n\r\nDaisy\r\n
3098	11	1993-02-14 13:18:26	Muidnar	board	I didn't know that Daisy :) I just came on and saw aladdin with\r\n11/11/11 and I had never seen him before.  So then you are the\r\none that plays aladdin :) Thats cool. I though it was kind of\r\nfunny that the char was only 17 and 11/11/11. (grin)\r\n
3098	12	1993-02-14 15:45:10	Cyric	Aladdin2	It is okay to play characters that Immorts jack up, I talked with\r\naladdin and asked that he not play this character anymore.  His\r\nexcuse what that he was using this character for helping newbies\r\nout. But when I found him he was adventuring by himself going to\r\nareas that his normal character could not have reached.\r\n
3098	13	1995-11-18 23:59:59	Quixadhal	ADMIN: GAP	There appears to be a gap in the board file records here.\r\nSome weeks were lost.\r\n
3098	14	1995-11-19 01:05:31	Quixadhal	group spells and resets	First, I figured out the reason that most DikuMUDs reboot at\r\nleast once a day.  LpMUDs reboot to free lost memory... dikus\r\nreboot because the reset code is broken!\r\n\r\nI need a volunteer to look at how the zone resets happen, and\r\npossibly rewrite part or all of it, so that it works as one\r\nexpects it to.  The game is less fun if chests filled with\r\nmagical items are empty because someone sold the item and thus it\r\nwon't create another one.\r\n
3098	16	1995-12-22 05:22:57	Quixadhal	New BUGS, er... Features!	I've been doing ALOT of hacking over break.  This is primarily to\r\ninstall the low-level support that will allow us to extend\r\nWileyMUD in major ways such as adding new classes.  I'm also\r\nchanging things to be more balanced, and adding new features as I\r\nsee opportunities in my editing.  As a result, it is more than\r\nlikely I've buggered it up in a few places.  If anyone sees\r\nthings they think are bugs, PLEASE report them to any immortal\r\nonline or using the bug command.  I'm trying to get this thing\r\nupgraded while I have the time to code all day, but I need help\r\nto spot the places where I got too tired at 6am. :)\r\n
3098	17	1996-02-05 18:52:13	Quixadhal	Dumbass wizards	Hey Buttknockers!\r\n\r\nLet's not leave the game whizz locked unless something really\r\nneat is being done eh?  Geez.... like we're being SWAMPED with\r\nplayers....\r\n
3098	18	1996-02-08 17:33:16	Muidnar	????	wizlocked??? how many high level wizards do you have that can do\r\nthat???  by the way 5 players are on now.\r\nMay you rot in hell!\r\n\r\nMuidnar\r\n
3099	0	1993-01-07 09:52:04	Machine	TO THE GODS!	Yesterday, I was on and the system hit some major lag or\r\nsomething because I    was sitting around for ten minutes and I\r\ngot no response, so I was forced to  terminate my link.  I had a\r\ncopper breastplate, a helm, a longblade, food, and  some padded\r\nleggings and sleeves.  Could I please get some of that stuff\r\nback?\r\n
3099	1	1993-01-07 11:14:35	Cyric	Consider	We posted along time ago about this Scruffy, the consider command\r\nis not a reliable means of judging your opponent.\r\n\r\nCyric\r\n
3099	2	1993-01-07 13:03:41	Alv	About AC again...	Well, my AC drops 2 steps....Ok that implies that my EQ isnt what\r\nit used to due to all the fights I've been in and so on....Then\r\nthere is the blacksmith..  He says that all of my EQ is OK....\r\n\r\nSo..here is an idea..make damaged EQ LOOK damaged and furthermore\r\nmake the Blacksmith a little more reliable.....i e make him\r\nwork...\r\n\r\nAnd a small favour for poor, death-pursued me....make the\r\nrecall-branch bit less expensive   :)\r\n\r\nThanks for your time!\r\nAlv the Elf\r\n
3099	3	1993-01-07 13:33:47	Muth	Info	It works again.\r\n
3099	4	1993-01-21 04:55:51	Ric	new weapons	How about some new weapons, this new wield two business has rendered most\r\ndecent backstabing weapons useless.\r\n
3099	5	1993-01-21 10:57:42	Muth	RE: weapons	Most of the weapons you were using for backstabbing shouldn't\r\nhave been used in the first place.  The longest sword you should\r\nbe able to use is a shortsword.  If there are items that are\r\nbeing used that seem to be longer than that please let me know as\r\nthey are bugs.\r\nShortsword\r\nDagger\r\nAny other short weapons.\r\nThe above should be the only weapons available.\r\n
3099	6	1993-01-19 13:16:13	Muth	Staff!!	If your message for wielding says 'You are forbidden to do that!'\r\nthen you cannot wield it do to your class.  Ie.  You are a cleric\r\ntrying to wield a weapon that has been enchanted only for mages\r\nto use.\r\n
3099	7	1993-01-19 14:36:59	Kharel	character one on again!	Well, i managed to get this char on, but i can't get Gnarl on\r\nwith macro or manual, sigh, one outta two ain't bad tho :)\r\nPaul.\r\n
3099	8	1993-01-19 17:44:32	Ender	sleeping Borg	I personally can think of\r\n
3099	9	1993-01-26 23:59:59	Quixadhal	ADMIN: GAP	There appears to be a gap in the board file records here.\r\nSome weeks were lost.\r\n
3099	10	1993-01-27 00:44:45	Cyric	New	new commands:\r\n\r\nnoteleport, nosummon.\r\n\r\nThis will allow you to choose if you wish to have either of the\r\nspells by the same name affect you or not, this is by the way\r\nonly for PC -> PC.  A mob can still cast either of these on you\r\nthough.\r\n\r\nYou should not be able to kill another player UNLESS you and\r\nhe/she are both registerd for player killing. If this is wrong,\r\nplease let me know. I am going to great lengths to make this\r\nplace fun for those that do not wish to be attacked by players.\r\n\r\nThis includes area affect spells :)\r\n\r\nCyric\r\n
3098	20	1996-02-14 03:45:51	Dirk	typos	Oh c'mon Muidnar, lighten up. There's only one or two that say\r\nanything like MTV ;^) The rest are pretty funny, and are much\r\nmore interesting than the standard error message\r\n
3098	21	1996-02-14 18:19:18	Quixadhal	RE: Grimwell	Sure.  A wizard who actually builds areas... what a unique\r\nconcept!  Is THAT what they were supposed to do?  I thought they\r\njust logged in and harassed the players every so often like me...\r\nok.\r\n\r\nFeel free  to whizzify him if he ever comes back :)\r\n
3098	22	1996-02-15 17:36:43	Muidnar	Grimwell	We can give him the Reach back being that was his original job,\r\nand no one else seems to be working on it. We need guild masters\r\nfor 15-20 for rangers.\r\n
3098	23	1996-08-08 12:26:16	Muidnar	wow	No one has put anything up here  in a while.\r\n
3098	24	1996-09-11 00:41:36	Quixadhal	RE:wow	You're right!  By the gods... oh, there aren't anymore...\r\n
3098	25	1996-09-21 14:28:45	Muidnar	sigh	still no access to the database.......????  oooo Quix!!!!!\r\n
3098	26	1996-09-27 18:24:38	Muidnar	QUIX	Still no access to the database....??? hmmmm do you not like me\r\nanymore????\r\n\r\n            Muhahahahahahahahahahahaha!!!\r\n
3098	27	1996-10-08 19:14:01	Muidnar	HEY!!!	Hey does anyone read this anymore???? You know the WIZARD BOARD!!\r\n\r\n                     may you all rot in HELL\r\n
3098	28	1996-10-08 21:06:07	Quixadhal	RE:HEY!!!	Yeah, you suck too.\r\n
3098	29	1996-10-28 21:25:41	Muidnar	biteme	Hey I think you should delete all wizards who don't reply to my\r\nmessage!!!  All you wizards can kiss my ass!!!!\r\n\r\n      Vile Muidnar the Dark One!\r\n\r\nNO comment?????\r\nfigures!!!!\r\n
3098	30	1996-10-30 19:35:01	Muidnar	Blowme	Why do still not have access to the data base???? I modified\r\nkarrn sorry chris!! hahahah\r\n
3098	31	1996-10-30 19:36:09	Quixadhal	RE:Blowme	Yeah, so did I... hope you like him.\r\nsmirk.\r\n
3098	32	1996-11-04 19:04:29	Muidnar	mud	Tim has posted a note to send dot mud players that dot is dead.\r\nThey may move here if they can hack it.... \r\n\r\nChris you need to see who is and is not a immort on the is game\r\nand explain to them unlike dot this mud is hard and immorts if\r\nnot building or testing should not be FUCKING with the players..\r\nor we will loose them. I also think you should make people who\r\nwant to be immorts at least make it to level 15 here so they\r\nunderstand how hard it really is!!!  Yes I know that you didn't\r\neven make it level 15 but then again you have the code and I have\r\nthe world file together they will suffer!!\r\n\r\n                                     Chris\r\n
3098	33	1999-03-30 16:33:42	Quixadhal	The Land is Dead.	Hello fellow immortals.\r\n\r\nI am surely speaking mostly to myself, but perhaps a few of you\r\nhave survived.  I am sorry this place has been neglected for so\r\nlong, and I fear it will never be what it once was.  But perhaps\r\nwe can carry someof it with us to a new place.\r\n\r\nFarewell.\r\n
3098	34	2004-07-25 00:33:27	Quixadhal	The Land is Empty	The land of WileyMUD is empty, but perhaps you can bring life\r\nback to it?  It waits only for a firm hand to force order back\r\ninto the chaos!\r\n
3099	11	1993-02-03 11:23:32	Kalamar	Comments	Why is it that when something as good as Wiley exists that it\r\nseems to receive more complaints than compliments?  Well, I for\r\none would like to use this space to compliment Cyric and crew for\r\nthe wonderful work they have done on Wiley.  Over the past year\r\nor two they have been extremely open to player comments and have\r\nimplemented many player friendly options.  These include:  free\r\nrent, loss of exp. base on level and number of classes when you\r\ndie (a sliding scale), user friendly commands too numerous to\r\nname, and a host of other nice features.  They have strived to\r\nkeep Wiley both challenging and playable, and to pump out new\r\nareas as quickly as their free time will permit.  I think we owe\r\nit to these dedicated individuals to give them a little praise\r\nnow and then to help temper the number of grumbles and gripes\r\nthey receive.\r\n\r\nGood work guys!\r\n\r\nI'd like to apologize for my personal comments on me\r\nreimbursement which Harlequin remarked upon.  It was not my\r\nintention to sound demanding or derogitory (although it may have\r\ncome out to do a frustration born of problems which I created for\r\nmyself).  I will en- deavor in the futu to phrase my problems or\r\ninquiries in a less derogitory manner.\r\n\r\nKalamar\r\n
3099	12	1993-02-03 16:09:59	Cyric	kalamar	Thanks man, I think the wiley crew DID need that... :)\r\n
3099	13	1993-02-06 10:14:08	Cyric	New Stuff	I am holding off from introducing any new code into the server\r\nright now. I am not going to work on anything new until I find\r\nthe reason for the games spontaneous hanging and crashing.\r\n\r\nPlease keep the suggestions coming though, I do plan on putting\r\nin alot of new spells suggested by player, corwin, goodwind,\r\nnomad and many others, so keep sending them.\r\n\r\nCyric\r\n
3099	14	1993-02-07 14:18:57	Muth	Registering	As Cyric has stated before, once you are registered, you can\r\nnever be unregistered. Eli tells you this as well when he makes\r\nsure you are fully understood of what this means.\r\n\r\nSorry Aliya, once a killer, always a killer.\r\n
3099	15	1993-02-08 10:29:25	Velventhon	First let me say that this is	First let me say that this is the best mud I have been on.  I\r\nhave had a greaat time.  The only problem I have had is losing\r\nall my equipment when the game crashes.  Right now I am a 4/3\r\nFighter/Mage.  Powerfl enough that noone has pi crashes.  Leaving\r\nme to powerful to evoke pity and to week to reequip my- self.  My\r\nonly other problem is finding things.  When a more expierenced\r\nplayeris leading me somewhere they go to fast for me to learn the\r\npath.  Thus I suggest selling maps to other locales:ie from\r\nvillage to goblins or village to Highstaff.  One final suggestion\r\nkeep villagers out of the bulletin room, and and make it noise\r\nproof.\r\n\r\nThanks for listening ever gratefull; Velventhon\r\n
3099	16	1993-02-08 23:08:41	Muth	Items/Magical	I am glad to hear that the stronger items are more difficult to\r\nget, that is the why we invisioned it to be.  On the issue of\r\nitems losing power and/or becoming junk\r\nHow many suits of dragon scale armor do you think one can make\r\nfrom the hide of a dragon.  Surely your answer is a hell of a\r\nlot!  Yes, indeed you could make a hell of a lot, and indeed\r\nthere are now a hell of a lot in the game.  My response to this\r\nwas to make more in different places, on different mobs.  \r\nHave fun finding the new stuff!  8)\r\n\r\n<M>\r\n
3099	17	1993-02-10 03:42:00	Scruffy	To the creators of Greth and Highstaff.	Hi all!\r\n   First ... for greth could you please check into the experience\r\ngiven for slavers.  It seems disproportionate compared to the\r\namount of damage they do.\r\n  Second... Can we tone down a few guards in highstaff?  One of\r\nthe reasons that you dont see more people staying there is that\r\nif you kill a youngster so she wont grow up into some goody goody\r\nand a guard comes by you get your backside smoked.  This chance\r\nis highly likely since the proportion of guards to people seems\r\nto be about 1 in 5.  I would make my home in Highstaff, but since\r\nI am usually running away from a Captain or Proud Guard I've\r\ntic'ed off it just isnt worth it.  Midguurd has the right idea\r\nsince the guards are fixed and the only other aggressives\r\n(starved men) are not large mobs.\r\n \r\n  Other questions... Is it possible for shops to be able to keep\r\ntheir inventory over reboots?  If I sell a set of steel banded\r\nbracers to the armourer and later get my current pair scrapped it\r\nwould be nice to be able to go back and by a pair that I had\r\nsold, but the inventory on every shop resets at reboot.\r\n  Also can you set a ceiling (other than available cash) for\r\nitems that shops will buy.  Someone comes in and sells a big item\r\nin the shops at shylar and poof he has no more cash for smaller\r\nitems.\r\n \r\n  Next... I've been through the jungles mapping and my first\r\nimpression is that they need a few more mobs.  Population density\r\nseems a bit low.  Also, is there any announced date for the\r\nopening of the city in the jungle? (the walled one).\r\n \r\n  My great Thx as always.\r\n    Scruffy\r\n
3099	18	1993-02-10 09:25:21	Muth	Scruffy	-= Will look into Gredth.\r\n-= Highstaff guards.  I will take a gander and see exactly what\r\n   the ratio is but you must remember, the reason the city is so\r\n   safe is because of the large number of guards.\r\n-= Elcid is in charge of the project of the City of Nesthar.  As\r\n   of yet he hasn't given me a finish date.  Sorry.\r\n\r\n<M>\r\n
3099	19	1993-02-10 11:09:48	Lightspeed	Hard but not fair?	Well this thing is bugging me a little first,I like this mud more\r\nthan others.\r\nBut better armor is only availible to higher levels like over 20\r\nsince people can charm Eli or stuff like that.I think that there\r\nshould be some way for level 8-13 to get some safe exp....because\r\nits just between those levels that peopl e stop playing their\r\ncharacters because they die too much.\r\nAgain i would like to say that this is a great mud but a little\r\ntoo hard.  So i would please get some better availible eq for\r\nlower levels?\r\nThe master of dying\r\nLightspeed\r\n
3099	20	1993-02-10 12:23:48	Jackal	Hermits . . . rehashed	  I am here to beg mercy from the gods.  Remember the day when\r\nthe goblins invaded shylar and you guys left a prize?  I was the\r\none who got the halberd.  I made it invisible so that If I got\r\ndisarmed I would be able to get it back, but today I lost it to a\r\nhermit who could apparently see invisible.  Can they all???  I\r\nknow the hermits are staying, and we've been through this subject\r\nbefore.  People say, 'Well, don't go to a screen with a hermit\r\nand start attacking.' It is kind of hard when an Aklan Guard just\r\nattacks you.\r\n  Please let me know if I can get my halberd back or if these\r\nwere your intensions for having hermits in the first place.\r\n       ----- Jackal\r\n
3099	21	1993-02-10 14:11:36	Drizzt	board	an Idea........being able to bring up the prac lists when\r\nsleeping.  such as spell lists to look at what your spells are at\r\nwhile asleep.\r\n
3099	22	1993-02-11 01:51:45	Emrys	quests part II	Well, Cyric, your cryptic message about quests really didn't help\r\nthat much.  So could you please tell us if there is going to be\r\none, or if there are not going to be any.  Thanks.\r\n\r\n-Emrys, that Genesis Listening, Mozart playing guy.\r\n
3099	23	1993-02-11 02:30:47	Scruffy	On Quests... From Uncle Scruffy	Hi\r\n   This is the information that I've gleaned on quests...  First\r\nof all there appear to be two types.  The first one seems to\r\nusually occur when a couple of the creators (gods, wizards) get\r\nonline and start giving out mysterious poems or clues to an\r\nobjective which you must find.  Usually there is a deterrent to\r\nyou getting to this objective in the form of a monster standing\r\nthere waiting to eat your face.  The rewards for success are\r\ngreat for the gods are generous.  Weapons and armor of great\r\npower and large (obscenely large for lower levels) amounts of\r\nexperience are the reward for success.\r\n  The second form of quests I must admit I have only heard in\r\nrumor and that takes the form of clues which are hidden in\r\nmysterious locations which lead you to further clues and so forth\r\nuntil a goal is reached.\r\n  I have no experience as to what the rewards to be for the\r\nsecond, but I have been lucky enough to be one (and there are\r\nmore) of the winners of the first.\r\n  My advice is as follows, keep your eyes and ears open!  The\r\ngods on this world are mysterious and devious and quests can turn\r\nup in voices from nowhere, mystic notes, and billboards on\r\ntravellers road for all I know.\r\n   Scruffy.\r\n
3099	24	1993-02-11 02:58:46	Scruffy	Spell Durations changed?	Seems to me that the past two days my spells have had a LOT\r\nshorter duration.  Shield, Fly, Armor, Bless, and especially\r\nStrength are going away after 2 tics or so.  It was my\r\nunderstanding that spells for multi-classes had returned to full\r\nstrength and duration. \r\n  Is the a wrong assumption, a bug, or something new?\r\n   Scruffy.\r\n
3099	25	1993-02-11 03:31:51	Scruffy	Uhmmm Muth? About Highstaff.	Uhmmm well gee, You see its like this.  If the city is safe with\r\nall the guards, it's also safe from us.  And while I really do\r\nenjoy walking around the city and seeing all the great mobiles it\r\nwould be nice to be able to kill a few.  :)\r\n   Scruffy.\r\n
3099	26	1993-02-11 16:50:34	Jack	A funeral announcement...	It is with sad misfortune that I write this. My brother, Lan,\r\nranger of the woods, has passed into his next life. I was near\r\nwhen he fought the slavers of Gredth. He had vowed to destroy\r\nevil, no matter what the cost would be. I have pledged to carry\r\non his mission, to make shylar and all of the surronding realms\r\nsafe for its people. I ask you to join me in this mission, to\r\nmake the lands safe for it citizens. \r\nPlease honor my brother by joining me to reclaim this fair land.\r\n                                            Jack Al'Mandragorian\r\n
3099	27	1993-02-11 20:17:08	Cyric	Quests	Don't know what you mean Emrys, when did I say what to whom.....\r\nI have run 2 quests on the game and they have worked out rather\r\nwell.\r\n\r\nC\r\n
3099	28	1993-02-11 20:35:00	Persius	Cyric :)	I just have a question for you which you will probably laugh at\r\nso...for some humour, summon me at your will.  Thanks, Persius\r\n
3099	29	1993-02-12 14:37:37	Kalamar	Comments	1. Charm monster still allows people to charm other registered\r\nAND non- registered characters.  Muidnar already knows about the\r\nproblem.\r\n2. Aliya,  I take offense at your statement that I'm trying to\r\nkiss up to the wiz's on Wiley.  After hearing several of the gods\r\ntalking about all the negative comments being thrown at them, I\r\nthought it might be nice to pay them a compliment or two.  I am\r\nusually one of the bigger complainers..er..critical observers\r\nmyself. :)\r\n3. I wholeheartedly agree with Drizzt about having the your spell\r\n(prac- tice) lists accessable when sleeping.  I see no reason\r\nthat a character who uses spells so frequently would not have a\r\nfew dreams about them.  :)\r\n\r\nKalamar\r\n
3099	30	1993-02-12 17:29:11	Bleys	I HATE Daisy now..	And you know why...\r\n\r\nBleys\r\n
3099	31	1993-02-12 17:36:38	Daisy	Re: I HATE Daisy now	Well... I'm clueless... Anyone wanna clue a blonde immort in ?\r\n\r\nDaisy\r\n
3099	32	1993-02-12 19:23:23	Lightspeed	about that thing with jack	If there is some way to help a man fighting against evil count me\r\nin...:) The man missing his silver hilted bastard sword\r\nLightspeed\r\n
3099	33	1993-02-13 01:15:22	Wulfgar	harlequin	the system crashed about 1 inthe morning 3 people lost all off\r\ntheir eq me(wulfgar),(palas); and a nother were victems of this\r\ntragerdy we beg that you retun what is our so that we may be off\r\nserving the gods\r\nsincerely Wulfy\r\n
3099	34	1993-02-13 04:22:16	Drizzt	50	IDEAS!!!! for new adventures. I would be interested in seeing a\r\nmaze of . and some more powerful undead such as wieghts and\r\nvampires and ghosts Also, what about a forest with Treents,\r\nHarpys, Carnivorous Plants, and more.......  \r\nA friend suggested that he would like to see werewolves and a\r\nweredargon with a human form and a human size dragon form.  He\r\nalso was wondering about being able to bet on the games in the\r\narena in highstaff.\r\nI was wondering if there was a place where you could dock boats\r\ntemporaryly if you buy one and If we would see horses and/or\r\nchairiots.  \r\nthats it fer now....the forest and horses/chairiots are my main\r\ninterests.\r\n\r\nThanxs fer listening\r\nDrizzt\r\n
3099	35	1993-02-13 10:30:23	Cyric	Reply in General	Spell durations, nothing new has been added nor changed in the\r\ngame for along time, I don't want to recompile until I find the\r\nbug that makes the server hang and crash.....\r\n\r\nPorts, harbors and stables are going to be added as soon as the\r\nabove is fixed....along with some more skills....\r\n\r\nDaisy, hating daisy....well, I am kinda partial to\r\ndaisy...grin...cute blonde at that...\r\n\r\nPersius, I will summon when I can :)\r\n\r\nEvil critters....look for asgard, he is about the best as evil\r\ngoes..  cackle, you like undead, you will like him....beware\r\nthough....\r\n\r\nCyric\r\n
3099	36	1993-02-13 13:57:21	Muth	Forest	A forest will all kinds of nasties...hmmm...I like it!\r\n\r\n(cackle)\r\n
3099	37	1993-02-13 15:39:04	Tom	Muth	This is the character that can be erased. Thanks!\r\nTom (Mot)\r\n
3099	38	1993-02-14 15:46:48	Drizzt	harlequin	Harlequin.......I lost my link a couple days ago and when i\r\nreconnected My Basterd Sword was gone....can you help me? I am on\r\nafternoons and very late at night..thanxs drizzt\r\n
3098	15	1995-12-20 05:03:09	Quixadhal	Meeting Results...	Hmmmm... results?  Were there any?  We DID kill off Malistrum and\r\ncripple Zar.  I think I outlined what we had done and what was in\r\nthe works fairly well.  Now, who is working on what?  Highlander\r\nhas his drow area in place.  A little more work to spruce it up\r\nwould be nice... and maybe a few more wandering critters.. but\r\ngood job so far!\r\n\r\nZar claims to be working on a paladin keep to be put in Serpent's\r\nReach... and Dirk claims to be working on Serpent's Reach when he\r\ngets back from Old People's Land.\r\n\r\nSedna and Zar will hopefully be getting together soon with me to\r\nhas out details about Nesthar.\r\n\r\nHighlander has also considered the idea of extending the\r\nUnderdark which would link into my own Faerie Village when I get\r\nthat done.\r\n\r\nMuidnar... Muidnar...  What is HE doing?\r\nPast glories are nice bub, but they don't buy the burger!  You\r\nneed to start hacking on something too.  Think about it and\r\nsuggest something.\r\n\r\nWhat am I doing?  Besides my twisted evil reincarnation of the\r\nFaerie Village from Smurfdom into a true Realm of the Fey... I've\r\nalso been and will continue to hack the code so we can add those\r\nnew classes (Paladins, Druids, Clerics of Good, Clerics of\r\nEvil...) and new spells (silence, entangle, insect swarm,\r\netc...).\r\n\r\nCheers!  Hope to hear from some of you!\r\n
3099	39	1993-02-14 20:59:58	Maiar	board	aaarrgghhh!!!!\r\nI was in highstaff got killed twice in a row trying to find my\r\nway back to shylar by a giant and orc... i severely\r\nfrustrated!!!!!  well after my second death i tryed to rent and\r\nit would not let me rent because i had no equipment so it said i\r\ndid not need to rent.  I have no idea why i lost my equipment.\r\nSo i had to quit ..... so i screamed to ask if anyone could help\r\nme , and a character said he could help me... he said he had some\r\nextra equipment in gredth so i preceded to goto gredth to pick up\r\nsome free extra equip and i got killed by a stupid starved\r\nman........\r\nAAAARRRRGGGGHH!!!!!  It has been a frustrating evening!!!!\r\nI just ask that i could get reimbursed sometime .... \r\nI am will am on usaully  around 11pm till 12pm....\r\nI think it is hard to get to and from highstaff if you are not a\r\nmage...  heck if you are not a mage you might as well forget\r\ngoing to highstaff because you will just get killed by a giant or\r\ntwo....\r\nMaiar the Giant HATER \r\n
3099	40	1993-02-15 00:21:09	Aliya	I'm REALLY PISSED!	steal \r\n
3099	41	1993-02-15 00:22:47	Aliya	IMPORTANT :GODS PLEASE READ!	Sorry about above...so anyway\r\nI was on killing ants with two other people. One got killed and\r\nended up back at the inn. We came to pick him up and an invisible\r\nsomeone was trying to get him to register. I said in general\r\nnever register. One of the guys I was with told the people to\r\nbugg off...because there were more than one trying to get him to\r\nregister.\r\nWell someone called him a jerk and I got upset and said "Bite me\r\nyou jerk!"\r\nWell Highlander thought that I was saying it to him when I was\r\nreally talking to the invis who was trying to get an innocent to\r\nregister!\r\nThat is how I got registered!\r\nSo anyway...he teleported me to somewhewre I never had even heard\r\nof before and I had to beg one guy to summon me to\r\nHighstaff...and another I had to pay a whole lot to summon me to\r\nthe inn...next to eli!\r\nI don't think registration should be so easey to do for one and\r\ntwo I think I am owed an apology from Highlander because he did\r\nthat and I didn't even do anything to him...I wasn't even talking\r\nto him to begin with!\r\nThat just REALLY PISSES ME OFF!\r\nAliya the really disgrunteled\r\nP.S. I also think you should be able to unregister once incase\r\nyou got tricked into it!\r\n
3099	42	1993-02-15 02:46:43	Tithian	immortality	   Hi Cyric!!!\r\n   I am writing to ask if the character Tithian could be made\r\nimmortal.  I like the mud and would like to do more to help it\r\nalong.  I have experience with the online editing commands and\r\nthe role of immortality.  It would be an honor to be an immort on\r\nthis mud.  I have seen a few muds go down the drain because new\r\nareas were not being added in.  I would also like to help design\r\nsome of the other classes in the AD&D players handbook.\r\nPaladins, Druids and Bards would be a great addition to the game.\r\nFeel free to summon me at anytime.  I will tell you more of my\r\nexperience and my internet address personally.  Please let me\r\nknow either way.   Thanks.......\r\n\r\n    Tithian High Templar of the King Kalak's Games\r\n
3099	43	1993-02-15 02:54:54	Palas	please help	This guy, named Ryno, then later LouSuckPalasDick keeps telling\r\nme messages or or shoutg at me, please get him removed from the\r\nsystem.  Ican't do anything if he follows me around saying\r\ncursewords all day.  and also i've lost all my equip twice in the\r\nlast two days, there never seems t to be a god on\r\n
3099	44	1993-02-15 04:47:33	Sneka	invis on ilrand	I am wondering if you could make it so you can't make ilrand\r\ninvis, I'm\r\n
3099	45	1993-02-15 05:32:37	Daisy	Bleys	Hello,\r\n\r\nContact me via email so I can explain something to you. \r\nThis week is going to be too busy to do it here online.\r\n\r\nDaisy\r\n
5099	0	1993-01-14 10:18:59	Muth	Welcome	Welcome to Highstaff, please feel free to post any bugs you find\r\nor other problems you find with the city.  \r\n\r\nMuth\r\n
3099	46	1993-02-15 07:44:50	Succubus	YOU BETTER READ THIS!	This MEANS YOU!  First I wanted to say what a fine Job is being\r\ndone here.  It encourages lots of people to spend lots of time\r\nplaying games.  GAME is the key word here.  I think that some\r\nthings here should be taken a little less seriously.  Some\r\nproblem could be ridden of if players would save.  I've played\r\nseveral diffent classes and characters on this mud, experiencing\r\nthe best of everything.  I've caused my share of trouble, and\r\nI've had more than my share of fun.  But what I was saying is\r\nthat I have NEVER EVER EVER lost anything in rent or because of a\r\ngame crash.  I've died SAVED and a crash occured.  I had all my\r\nstuff.  I've accidentally quit right before a crash.  Miraculusly\r\ngotten my stuff back, SAVED, then crash...I still had all my\r\nstuff.  As for Daisy, how could anyone and I do mean ANYONE, how\r\ncould anyone harbor any ill fealings towards such a wonderful\r\nwoman?  And I think it has been sufficiently brought up that\r\nregistering is a dangerous thing to do.  AND that it can't be\r\nreversed.  AND that even then Cyric and Co. have installed\r\nNOSUMMON and NOTELEPORT so People like Highlander can't teleport\r\nyou to a nasty place.  By the way, Nice try on me Highlander.\r\nWell, you all take care, and don't let the Demoness get you in\r\nthe night.\r\n*Succubus*\r\n
3099	47	1993-02-15 09:11:03	Baradakus	Agree with Succubus	I agree wholeheartedly with Succubus.  I've also played several\r\ncharacters and spent more than my share of time on this mud, and\r\nI've never had a problem with losing equipment.  Save is an\r\nimportant command, and it does work.  The gods have been more\r\nthan generous in installing safeguards to the game like no-\r\nsummon and registering; at some point characters have to start\r\ntaking responsibility for themselves (*grin*).  Thanks for a\r\ngreat mudding experience!\r\n                          BARADAKUS\r\n
3099	48	1993-02-15 10:31:26	Harlequin	My two cents....	And since I've got some weight around here, it's gonna be a BIG\r\ntwo cents....*snicker*...\r\nSAVE SAVE SAVE SAVE SAVE.  Succubus is right, the only way you\r\ncan lose your stuff around here is if you either A.) Don't save\r\nat all, or B.) you die and don't get a chance to save, i.e. the\r\ngame crashes as you die.  So save save save.  Get tintin and set\r\nup a macro to suto automatically save you at various points.  It\r\nreally helps.\r\n\r\nPalas: (and anyone else for that matter)  Just because you don't\r\nsee us on the who list, doesn't mean we are not on.  If someone\r\nis bugging you that way, just give a shout and we'll help you\r\nout.  Of course, just seeing a name like LouSucksPalasDick or\r\nsomething is grounds enough for us to kick the person off the\r\ngame....\r\nREGISTERING: Yes, we have warned time and time again that to\r\nregister leaves you open to much mischief, even Eli tells you\r\nthis before registering, and ultimately, the one responsible for\r\nyour actions in life is yourself.  If, after all the warnings,\r\nyou register, well...hey, we warned ya! \r\nFOREST: *cackle*\r\nHORSES etc.:  There are already mounts in the game, and I'm sure\r\nthat as soon as Cyric vanquishes the evil nasty bug that is\r\nmaking the game hang, stables and kennels will be forthcoming.\r\n
3099	49	1993-02-15 10:39:29	Harlequin	OH yeah...	About Highstaff.  I've seen many players of many classes make the\r\ntrip to highstaff with ease.  The trick to getting past the\r\ngiants and orcs?  Move REAL fast...*cackle*...Works for me!\r\n
3099	50	1993-02-15 12:18:48	Rolly	lewer	i miss you. please read your mail. love, me\r\n
3099	51	1995-09-23 23:59:59	Quixadhal	ADMIN GAP	Again, there appears to be a gap in the board file records.\r\nSome postings were lost.\r\n
3099	52	1995-09-24 17:11:09	Thrym	RE: Advancement	You have to find your guild.  If you're below 6th level, it will\r\nbe somewhere in or near Shylar (rangers get to track down their\r\nwandering guildmaster in the grasslands).  If you're ready to go\r\nto 6th level, you get to make the trek to Highstaff.  Follow the\r\nroad east out of town and take the first fork south.  Be careful!\r\nThere be GIANTS in the mountains!\r\n\r\n                                 -Thrym the eternally 12th level.\r\n
3099	53	1995-12-01 19:58:21	Highlander	Underdark Elves	Has anyone else heard of some nasty rumors of wield\r\nelves that live undergroud?\r\n
3099	54	1995-12-20 04:49:13	Quixadhal	RE:Drow	An unconfirmed report of Drow activities  along the wester river\r\nmight interest a few of you.\r\n
3099	55	1995-12-21 09:38:00	Chronos	T'is true!	I saw the benighted drow in the northwestern woods.  Their\r\nwarriors attacked my party.  We were fortunate to escape.  We\r\nkilled a few of their warriors.  We were lucky to escape with our\r\nlives.  Any brave warriors wish to accompany this mage to defeat\r\nthe evil drow?\r\n
3099	56	1995-12-28 22:24:01	Quixadhal	WANTED!	People to write stuff on bulletin boards.\r\n
3099	57	1996-01-27 21:29:03	Grimwell	Hermits!	Aw heck buddy, the point of the hermit is to make life tough :)\r\nHmmm, Hey Quix, you want any other generic messages? What's new\r\non the mud? Any new areas? This crusty old magician can't\r\nremember much so it's most all new again, but divine inspiration\r\nwould be lovely :)\r\n\r\nMake Muidnar suffer\r\n\r\nGrimwell, master of the arcane "I Forgot's"\r\n\r\np.s. I used to have a map too! :) **whooosh!**\r\n
3099	58	1996-01-31 00:40:25	Puck	new places	Well, I discovered the ogre's lair not too long ago... \r\nHigh-levelers only, I suggest, cuz I did die there.\r\n\r\nThen I just discovered a cemetary in the redspines... I might\r\nbe able to find it again if anybody is interested.\r\n\r\nThere's keeps everywhere... I think the key's should be strewn \r\nabout the map so we can go in and use them as our own.\r\n\r\nAnd there should definately be rules set for what a keep costs,\r\ncuz Dorn needs one and can probably afford it. \r\n\r\nOh, and until there's a reset, avoid Gredth.  Just trust me.\r\n\r\nOh, and quixadhal, Cynxena wants to experience the Axe.\r\n\r\n*smile*\r\n
3099	59	1996-02-14 18:30:35	Quixadhal	RE: Hermits et. al.	Wow... haven't read this board in a while!\r\nHiya Grimwell!  Long time no see!  New areas eh?  Well, nothing\r\nfinished but there's a couple half-finished.  You've probably\r\nnoticed lots of the changed commands/way-of-things (IE: darkness\r\noutdoors).\r\n\r\nI tried to update the "info" command a bit... that might be helpful.\r\n\r\nGood to see ya back!\r\n
3099	60	1996-02-14 18:44:20	Quixadhal	RE:Scourge	Yup, afraid Scourge really does have that good of stats.  He's\r\nalways had good stats since the days of Wiley I, which was before\r\n*I* had ever seen it (I came into Wiley II).  Since he's had to\r\nrestart his character (which had progressed to level 50) at least\r\ntwice now, I don't see granting him his original stats as\r\nanything unusual.  By right, he should also have a keep and be\r\n50th level too.\r\n\r\nI would hope that you might expect a similar respect when whoever\r\ntakes over this place erects Wiley IX and you log into it as a\r\nnewbie.  I'd also hope the other players respected and feared you\r\nas one of the Ancients who Knows.\r\n
3099	61	1996-02-14 23:02:47	Questor	barrel	What happened to the barrel in the bar?\r\n
3099	62	1996-02-15 14:26:27	Dorn	scourge	My bet's are puck has nothing to say now...\r\n\r\nMention that respect thing.  Heh.  I figured\r\nhe was a god's pet.  \r\n
3099	63	1996-02-25 23:59:59	Quixadhal	ADMIN GAP	Another gap in the board records.\r\n
5099	67	2004-07-25 00:38:12	Quixadhal	At last!	At last, you have come!  Long years have passed since anyone\r\nhas arrived from the hamlet of Shylar... we had believed the\r\nmountains were closed.  But you are here!  There may be hope\r\nfor us yet!\r\n
3099	64	1996-02-26 09:12:15	Quixadhal	NEW BUGS... er.. features!	Just to warn everyone... I did some recoding of things this\r\nmorning.  Experience is now handled differently for groups.... if\r\nyou are not grouped, or grouped only with yourself... you should\r\nget full exp for mobs (plus a 10% bonus for registered servents\r\nof Quixadhal!).  Groups of 2 or more will find the ratios changed\r\na bit... I tried to make it fair to both multi-classers and\r\nsingles and NOT to lose too much exp in the division.  Let me\r\nknow how that works out.\r\n\r\nALSO:  You can now use "me" or "myself" to refer to yourself in\r\nmany commands.  And a few may have noticed that kill now comes\r\nBEFORE kiss <grin>...  Hope you all have fun!\r\n\r\n-Dread Quixadhal, Dark Lord of VI.\r\n
3099	65	1996-02-28 07:25:20	Quixadhal	NEW NEW NEW!	It's done!\r\n\r\nAll of you who buy pets, or use magical means to obtain your\r\nfaithful servants, REJOICE!\r\n\r\nIf you can keep the little critters alive, they will grow more\r\npowerful as you do.  Gradually, they will follow you and become\r\neven better servants as they learn by watching your example.\r\n\r\nBURY COMMAND! (Quixadhal) Sun Mar  3 07:55:53 1996\r\nThis is important people!\r\n\r\nYou must now bury your dead!  If you do, you'll get a pleasant\r\nlittle exp bonus and become a little nicer.  If you don't...\r\nWell, you'll find out soon enough.  Let's say it won't be pretty.\r\n
3099	66	1996-03-14 20:32:13	owlbear	Exp	I've found exp is awfully hard to get for us mobs.\r\nOther mobs just aren't worth enough.\r\nI try to kill players, but they get pissed off and have their\r\nhigh-level friends hunt me down.\r\n
3099	67	1996-03-14 20:41:50	dragon	Mob disrespect	Yeah....if we mobs don't get respect, we're gonna start a UNION...\r\nThen we go on strike and all you get to kill is other players.\r\n\r\nRespect us or ELSE!!!\r\n
3099	68	1996-03-27 23:56:46	Suckling	YES!	Thank you Dread Lord!\r\n\r\nFor those of you who don't know... there is a new kick-ass\r\ncommand.\r\n\r\nDESECRATE!\r\n\r\nThis is the way to control the number of corpses that linger\r\nabout WITHOUT having to become a goody-two-shoes.\r\n\r\nMuahahahahahahahaha!!!!!!!!!!!!!!!!\r\n
3099	69	1996-06-09 20:43:02	Rincewind	guilds	where are the guilds and how do you raise to the next level?. I\r\nfound the places where i can practise... but i don't know how to\r\nadvance to the next level\r\n
3099	70	1996-06-09 22:39:21	Sedna	Guilds	It would be easy for me to give it away, but wouldn't it be\r\neasier to9 read the descriptions and find out for yourself?\r\n\r\nAll of the guildmasters are right here in Shylar.  Try entering\r\nthe prac and gain command to see what each one will do for you.\r\nI'll keep watching in case your online so I can help you out.\r\n\r\nSedna the grimy code devil\r\n
3099	71	1996-06-10 15:39:15	Rincewind	guilds..	oh... i found them alright... but i couldn't figure out how to\r\nraise levels.. as the gain command was not on the help...  you\r\nneed better help files on skills...\r\n\r\nand\r\n\r\ndoes your mastery of your skills increase each time you use it?\r\n
3099	72	1996-06-13 12:20:01	Sedna	gain	Apparently, it is not.  I'll ask Quixadhal to add the gain\r\ncommand to the help command list.  Also, the command ALLCOMMANDS\r\nwill give a list of all available commands available to players.  \r\n\r\nAnd, yes, your skill does increase to a certain level by use of\r\nthe skill.  Each time you cast a spell, use a fighting or\r\nstealing skill or use a weapon of a particular class, that skill\r\ngets better up to what is known as fair, I believe.  TO improve\r\nthe skill knowledge to master, you must practice it once or twice\r\nafter you get it to fair.\r\n
3099	73	1996-08-18 19:21:33	Fnord	my skills disappeared	I have lost all my skills.  I had the following skills\r\n\r\ntrack - superb\r\napraise - superb\r\npunch - very fair\r\nbarehand - very fair\r\n
3099	74	1996-08-18 19:29:49	Fnord	my skills disappeared part II	and I have line noise on my phone, but even immortals\r\ncannot stop that.....\r\n\r\nAnyway, I rented last night and when I logged in today\r\n\r\nmy character had no skills\r\n
3099	75	1996-08-29 08:22:45	Gilric	Spell Duration	Just a quick question on spell duration.  Do my spells last\r\nlonger as I advance my level? and if so do my cleric spells last\r\nas long as a 6th lvl cleric and my mage spells at only 5th level?\r\n\r\nGilric 'ooopps was I suppose to bury that'\r\n
3099	76	1996-11-04 19:19:42	Mia	WTF??>	What happened to underworld dreams???????\r\n\r\nI would really like to talk to bela or someone that made this\r\ndecison!\r\n\r\nplease email me at erbonjo\r\n
3099	77	1996-11-04 19:21:52	Mia	email messed	well sence my email was messed up because of the at sign here it\r\nis erbonjo at acadcomp.cmp.ilstu.edu\r\n\r\n                        Thanks\r\n\r\n                              Mia\r\n
3099	78	1996-11-20 00:33:00	Hugh	Kind of nice.	Well.. where to start.. first off, I would love to know why the\r\ngods keep killing my character.. especially when there are only\r\nabout 3 people who even come around here anymore in the first\r\nplace. \r\n\r\nThis game used to be fun, I thought about finding a new mud.. but\r\nsimply decided to stay on this one because the interaction was\r\nnormaly good.. but oh well, I guess my friends and I simply find\r\na new mud, no problem.\r\n
3099	79	1999-03-30 16:35:15	Quixadhal	The Land is Dead	Greetings wanderers.\r\n\r\nWhatever storm has driven you to this ancient place, I hope you\r\nfind it accomodating.  I fear years of neglect and mismanagement\r\nhave taken their toll.  If any of you actually care, perhaps we\r\ncan take you with us when we leave this place and journey ...\r\nBeyond the Rim.\r\n\r\nFarewell.\r\n
3099	80	2004-07-25 00:35:39	Quixadhal	Adventurers WANTED!	Over the years, many brave souls have set out from this bar.\r\nMany have died, some have wished they had died, and a very very\r\nfew became legends.  It has been a long time, do you think you\r\ncould become a legend?\r\n
3099	81	2006-03-02 16:18:38	Nomad	test	test\r\n
3099	82	2006-03-02 16:19:01	Nomad	Contact Info	I used to play this back in the day. Would be interested in what\r\nhappened over the times of ''95 till about now. I had a cleric\r\n(about level 14) whom I can''t remember his name. I was also\r\nsuccessful in getting the code and world maps about 1-2 years\r\nafter\r\nl board\r\n
3099	83	2006-03-02 16:25:06	Nomad	Contact Info 2	after it closed -- it barely complies and I never did work on it.\r\nWould be interested in getting Wiley up and running with some of\r\nthe old players (I can contact about 2-3). Anyhoo.. my email is\r\npythagoras\r\n
3099	84	2006-03-02 16:31:29	Nomad	Contact Info 3	at myrealbox.com  -- someone drop a mesg. [oh yeah, there is a\r\ntypo in the Farm...]. l8tr...\r\n
3099	85	2007-12-03 23:42:26	Grimwell	If you are reading this...	You are one of the six people who read my blog. The one with\r\ninitiative. Welcome to the wayback machine!\r\n
3099	86	2007-12-09 21:52:12	Osiris	...halb, halb halb	Hiya!\r\n
3099	87	2008-03-14 13:12:02	Quixadhal	Heh, life	Wow, a few signs of life... amazing!\r\n
3099	88	2009-01-16 14:04:40	Cire	Hello?	Is this similar to the WileyMUD i played back in 92 at Western\r\nMichigan University? I have been looking for this place for\r\nyears! PLease post a note to let me know if this is the case.\r\n/x\r\n?\r\n
3099	89	2009-01-31 16:10:58	Bellidore	Its the same wileymud from wmu	I'm not sure who''s running it but this is the same basic mud\r\nfrom western michigan.\r\n
3099	90	2010-05-09 09:48:43	Quixadhal	Testing the new SQL board code	This is just a test.  If it survies a reboot, we''re good to go.\r\n
5099	1	1993-01-21 21:24:43	Highlander	I don't think so...	 Okay, someone wanted me to tell about the I don't think so...\r\nmessage that is appearing for no particual reason. Well first\r\ntime I got it I was using a charmed guard to kill citizens <grin>\r\nsecond time I saw it I tried shouting in this room and than\r\nleaving east when I tried to reporduce the act nothing!, so beats\r\nme!>?\r\n\r\n   Highlander, the one that dies way to much\r\ncast 'magic missile' at \r\n
5099	2	1993-01-22 02:18:27	Muth	Explain	Explain a bit more on that please Highlander...\r\n
5099	3	1993-01-22 14:07:46	Highlander	ex, I don't think so...	  Well, the other day I was going along Highstaff killing things\r\nand I got the I don't think so... message (you know the one you\r\ngods give me when I've been naughty with my charm monster spell)\r\n<grin> But of course, I wasn't doing anything particually wrong\r\nat that time. Some god (no identification( told me to keep an eye\r\nout for it and let em know if it happens again it did only not\r\nthe same circumstances and I couldn't reproduce it  <shrug>\r\n  Oh, bye the way I love this board, helps make highstaff more of\r\na community type place like shylar, only for higher levels, but\r\nis there anyplace to BUY water??\r\n
5099	4	1993-01-22 14:42:14	Muth	water	Sorry bout that, water will be sold asap.\r\n
5099	5	1993-01-22 15:37:11	Emrys	bug/map inconsistency	right.  Okay, when you go 1 or 2 north from the outside of the\r\ngolden dragon then go east once you are put to the small area\r\nbehind the golden dragon.  the one you need to go east from\r\ninside the dragon to get to.\r\n
5099	6	1993-01-22 15:38:39	Emrys	bug	Dyou need to make this room like the back room.  No outside\r\nnoise.\r\n
5099	7	1993-01-23 01:35:38	Muth	RE: bug	That is no bug.  Do to our current exits available, there was no\r\nway to make that rooms exits logical.  You will have to read the\r\ndescriptions in that room to understand why those exits go where\r\nthey do.\r\n
5099	8	1993-01-25 16:27:16	Emrys	It's amazing	You know.  It's really amazing what goes on in the room next\r\ndoor.Hello. Balinda.\r\n
5099	9	1993-01-26 14:24:05	Highlander	Water	Thanks Muth for the availability of water at the bar.\r\nHighlander Lord of Evil and Despair!\r\n
5099	10	1993-01-26 23:07:41	Cyric	re:I don't	the I don't think so message is one letting you know a charmed\r\nfollower of yours is trying to shout... :)\r\n\r\ncyric\r\n
5099	11	1993-01-28 09:03:11	Ender	map	Is there a map of Highstaff available anywhere? If so, please\r\ntell me, and if not i'd like to suggest having a general map\r\navailable, much like the one in Shylar. (for a price, of\r\ncourse...)\r\n-\r\nender.\r\n
5099	12	1993-01-28 15:46:29	Muth	RE: map	I can work on a map.  Will be a day or two though\r\n
5099	13	1993-01-28 20:25:45	Ender	map	Thanks, Muth!!\r\n-ender\r\n
5099	14	1993-01-28 21:15:00	Kalamar	Lost Equipment	Cyric,\r\nI was playing last night (Wed. the 27th) and the game crashed.\r\nWhen I logged on today (28th) I found that I had lost all of my\r\nequipment.  I was wondering if a reimbursement would be possible.\r\nIt sounded like quite a few other people had lost their equipment\r\ntoo.\r\nKalamar\r\n
5099	15	1993-01-30 15:42:28	Ender	stats	My stats (str, dex, etc) really kinda suck. This is NOT just\r\nanother whiney note. I was thinking thaat perhaps one could use\r\npractices to increase one's stats. This is not entirely without\r\nbasis.  One could prctice "wieghtlifting" or some equivalent\r\nskill to increase strength instesd of practicing the latest\r\nfighting techniques.  One could read and study to increase\r\nknowledge(Wisdom), or practice juggling to increase dexterity.\r\nSkills like this would allow those of us born with low stats a\r\nchance to improve them without handing out stat increases\r\nindescriminately, or upsetting the "game balance" by having loads\r\nof characters running around with perfect stats, or if they did\r\nthey would hav no spells or other skills and so balance out the\r\ncharacter that way.\r\nJust my 2 cents.\r\n\r\n-ender(the good)\r\n
5099	16	1993-01-30 15:51:27	Melanie	The Customs House	On my way around the city trying to find the Mage's Guild, I\r\nwandered into the Custom's House.  I have a question about the\r\ncontents.....or two.\r\n#1 The map is a gype.\r\n#2 what does the paper and badges do???\r\nCan someone say???-Melanie the good.\r\n (unless you are a Wealthy or young lass snuggling me)\r\n
5099	17	1993-01-30 20:07:07	Highlander	Map	 Gee Melanie, My map of Highstaff is perfect, maybe you need glasses or\r\nsomething? <grin>\r\n\r\n        Highlander, the not so good!\r\n
5099	18	1993-01-31 13:54:05	Melanie	MAP	Not when it says that I was gyped the map is so decayed that I\r\ncouldn't read it!!!!!!!\r\n\r\nMelanie The Good\r\n
5099	19	1993-01-31 16:40:53	Orris	20	I agree with ender -- There are ways for a person to increase his\r\nstrength, dexterity but maybe not so much constitution and\r\nintelligence.  If widsom is common sense then there are certain\r\nthings that could be done to raise that also.\r\n  Morgan of Orris\r\n
5099	20	1993-02-02 13:10:13	Kalamar	Stats	There are already objects in the game that allow you to raise\r\nyour stats.  I think what you guys are looking for are new stats\r\naltogether.  I have a suggestion.  I've noticed that on several\r\nother muds that once a char- acter reaches a certain level\r\n(usually 5th) they have a place they can go to redistribute or\r\nreroll their statistics.  The Forgotten Realms mud eastablishes\r\nbase stats (8 for all) and allows you to add extra points from a\r\npool on to any stat you wish.  I think they limit the change to\r\nonly once or twice per character.  I realize that this would take\r\na while to implement, but I thought I'd throw the idea out for\r\ndiscussion.\r\n\r\nKalamar\r\n
5099	21	1993-02-04 17:54:38	Swift	An Important Suggestion	Swift, the amazing first level thief here!\r\n\r\n     I would like to make a suggestion...  I thinks that it would\r\nbe great if someone could install an autosave function, so that\r\nwhenever anyone dies, they are automatically saved...  I have\r\npersonally lost all my equipment six times, and I believe that\r\nthis detracts a bit from the gaming atmosphere.  Would an\r\nautosave help this?  I've played on other MUDs with such a\r\nfeature, and it worked quite well.\r\n\r\n                                     Just my thoughts,\r\n                                     Swift, the thief\r\n
5099	22	1993-02-05 21:53:35	Kim	quests	Hiya! not to try to sound pushy ar what not but i spend most of\r\nof my time in hi ghstaff now and ive noticed that the last 4\r\nquests have all been based around shylar, not to sound greedy or\r\nanything cause i did get the halberd from the goblins and the\r\ngold from grugnik but how about some highstaff based quests?\r\ntheres not a lot to attract people here and u think baseing some\r\nquests here might increase the population more\r\n\r\nsay just an idea - - Kim the princess of wisdom\r\n\r\nPS can i get my title changed to the abouve?\r\n
5099	23	1993-02-08 23:51:09	Highlander	Kim's title	Of WISDOM???\r\n<smirk> <laugh> <gag>\r\n
5099	24	1993-02-10 15:52:52	Thrym	RE:title	Well, I think that asking for such a title sort of implies that\r\nit would not really fit well.  <snicker>\r\nThink about it.\r\n                  -Thrym the Untitled One.\r\n
5099	25	1993-02-10 18:54:01	Ender	Stats II	Well,  yes i  kinda want  new  stats, and   i  like  the  idea\r\nof   being able   to reroll,  but I   am  willing to spend\r\npractice  sessinonsto  iimprove stats,  or   most  any  other\r\ncost/quest/effort  the  gods  see   fit.  I agree that   there\r\nare  objects to  raiise   stats,   but  when  stats  suck  as\r\nbadly as mine  (5 con,  8  str  9  wis  11 dex  and  14  wis)\r\nthe  objects (except  for  tiera)  don't   really help  worth\r\nmentionioning.  (sorry for   the extra  spacing,  but  ths\r\nkeyboard  is  really  funky toniite)\r\nEnder  the usually  good.\r\n
5099	26	1993-02-11 09:54:38	Krinn	stats III	well Ender if you will recall in D&D there is such a thing as Max\r\nlevel which is based on races and according to abilities, i dont\r\nthink the code here has been written for that but maybee it just\r\nfal into place that the higher you get with such low stats the\r\nless you actually increase power. my suggestion would be retire\r\nand start again, looking at your stats i would guarantee better\r\nrolls, and you will have equpment to 'hand down' to your new guy\r\nto make low levels easier to obtain and in no time youll be back\r\nwhere you are and better than before\r\n\r\nthoughts from Krinn - a traveler from Kolyema\r\n
5099	27	1993-02-11 10:16:20	Krinn	WOW	Bravo Muth! awesome new area tha temple drenth is really cool\r\nabout time we had an area that requiered as much thought as it\r\ndoes strength!(not that all the areas arent cool) this just\r\nhappens to be my favorite so far anyway excelent job! Krinn -\r\ntraveler from afar\r\nP.S. ever played Eye of the Beholder II, muth?\r\n
5099	28	1993-02-11 12:52:41	Jackal	re:WOW	I agree wholeheartedly with Krinn -- that is a kick-ass place.  I\r\nthink we missed part of it though.  That wierd coffin??  But\r\nanyway, for the longest time you guys have been trying to get us\r\nto go to Highstaff and now at least for my you have succeeded.\r\nThere is plenty to do here now, with the arena, temple, orc\r\ncaves.  I will never leave.\r\n   Lord Jackal of Orris\r\n
5099	29	1993-02-11 20:56:19	Ender	stats IV	Well, i belive i will take the proffered advice and restart.\r\n*sigh* I just felt like writing one final message on the\r\nHighstaff board, since I doubt i will be back here soon, with\r\nclasses and all...  Here's to my next Reincarnation!!!\r\n*glug* *glug* *glug*\r\nshout where are you?, i can pr cure blindness, but i'm in\r\nHighstaff\r\n- oops, ignore that last part\r\n-Ender the formerly extant.\r\n
5099	30	1993-02-11 22:28:45	Muth	RE: WOW	Thanks! I had fun building that area.  8)\r\nLook forward to more things opening up within the wall of\r\nhighstaff, and not far travel from the city itself.  (Don't\r\ndispair thieves, you will soon have a wonderful place to use your\r\nabilities.)\r\n\r\n<M>\r\n
5099	31	1993-02-12 21:15:57	Goodwind	Drinks.....	There is a problem with the Barkeep and Valture and his fine\r\nwines...  They always seem to be out or when you buy\r\nfirebreather, which Is on the list, they had never heard of it?\r\n\r\nJust an observation.....\r\n\r\nGoodwind The Party Animal....\r\n
5099	32	1993-02-12 21:19:20	Goodwind	Areas....	Muth,\r\nTwo things:\r\n1) I love what you are doing with the areas.....Highstaff is now\r\n   a better place to live, Midgaard is more exciting, and the Orc\r\n   caves Are tough..  Great diversity....\r\n\r\n2) The guards in midgaard seem to have a problem?  After i kill\r\n   the one at the gate, the square and the one at the temple, they\r\n   regenerate at the alter....????  Why is that?  5 guards at once\r\n   is even a challenge to Corwin, let alone me??\r\n\r\nThanks,\r\n    Goodwind of Loucks\r\n
5099	33	1993-02-13 13:58:52	Muth	Aklan guards	I will fix that...sorry.\r\n
5099	34	1993-02-13 14:15:28	Muth	Goodwind...	We are fully aware of the charm monster bug everyone.  Until\r\nCyric is able to fix the hanging/crashing server it won't be\r\nfixed.\r\n\r\n  Goodwind: Stop abusing this bug.  If your actions continue I\r\n            will personally deal with you in a appropriate\r\n            fashion.\r\n<M>\r\n
5099	35	1993-02-13 17:42:26	Jackal	Orc Guards	Muth,\r\n   The orc guards have a similar problem.\r\n\r\nLord Jackal of Orris -- A being who goes where he doesn't belong.\r\n
5099	36	1993-02-14 01:53:48	Muth	Orc guards	They should be more spread now...\r\n<M>\r\n
5099	37	1993-02-14 13:35:10	Jackal	Some thoughts	Scrolls --\r\n  They are much better now that they have titles on them.  Could\r\nthose of us with spell craft, or who already know the spell on\r\nthe scroll understand those cryptic titles?\r\n\r\nStrength spell --\r\n  Why can't fighter mages or even mages alone get 18 with bonuses\r\nfrom the strength spell?  I have an 18 strength without the spell\r\nand there are still some weapons, a heavy hammer for example,\r\nthat I am unable to wield.  Does the spell help fighters with an\r\n18 (50) strength?\r\n
5099	38	1993-02-14 15:22:19	Goodwind	Blacksmith...	There seems to be a problem with the blacksmith.  Whenever He\r\nfixes armor, he decreases the value by one.  On apraise it says:\r\n   effective : [4]\r\n   when repaired: [5]\r\n\r\nBut when i get it repaired, It goes down to [4]. Why is this?...\r\n\r\nAnd if it supposed to be like this, why did you take away our\r\nsupply to Isha's armor?  Now it will never be the same........\r\n\r\nGoodwind of Loucks\r\n
5099	39	1993-02-14 17:27:11	Krinn	some thoughts and answers	- i like the scroll idea jackal has\r\n- goodwind not even a blacksmith can make armor last forever\r\n- it was allready explained to us why isha has no more dragon\r\n  scale\r\n- there is more dragon scale in the game we havenet found it (so\r\n  i was told)\r\n\r\n- can we get our prac lists during sleep? woul dbe a major help\r\n- muth the orc guards are stil in the same place, and warriors\r\n  still all regen in same room\r\n- this is th most awesome mud ive ever played!\r\n- ill never play another!\r\n- nifty new area, Muidnar! pretty tough\r\n\r\n- Krinn the traveler from kolyema who has found his new home in\r\n  highstaff!\r\n
5099	40	1993-02-14 17:36:32	Wulfgar	harlequin	Harlequin....I have lost feathered armplates, small leather\r\npouch, hawk staff and 18 cassoroles. please help\r\n
5099	41	1993-02-14 17:38:02	Wulfgar	harlequin	I also lost black chain vest and black chain leggins thank-you!!!\r\n
5099	42	1993-02-14 20:07:38	Krinn	one more question	i have a heavy hammer i would love to use for aweapon\r\ni have an 18 str and cant wield it\r\ngoodwind was a 17 str and can wield it\r\nnomad has an even lower strength and can wield it\r\nwhy cant i wield it?\r\n\r\nKrinn the confused traveler from Kolyema\r\n
5099	43	1993-02-15 09:38:45	Muth	Orcs...	What orcs are you talking about?  The cave with four orc guards\r\nat the entrance has been fixed...pehaps you are speaking of\r\nanother cave?\r\n<M>\r\n
5099	44	1993-02-15 11:13:17	Jefe	lewer	hello. honey. are you surprised? You must have been playing with\r\nthis character cuz he has a bunch of stuff. How are you?\r\n
5099	45	1996-02-27 23:59:59	Quixadhal	ADMIN: GAP	There appears to be a gap in the board file records here.\r\nSome weeks were lost.\r\n
5099	46	1996-02-28 07:28:36	Quixadhal	NEW NEW NEW	It's done!\r\n\r\nAll of you who buy pets, or use magical means to obtain your\r\nfaithful servants, REJOICE!\r\n\r\nIf you can keep the little critters alive, they will grow more\r\npowerful as you do.  Gradually, they will follow you and become\r\neven better servants as they learn by watching your example.\r\n
5099	47	1996-03-03 07:57:55	Quixadhal	RE: levels	Yup.  Sorry about that, but you must admit the old exp tables\r\nwere horribly unfair.  Once you hit level 20+ it became trivial\r\nto go up in levels, which tells ME that either it shouldn't be so\r\neasy, or we should go back to only having 20 levels.  IE:  If the\r\nlevels aren't hard to get, they don't mean much... and if they\r\ndon't mean much, why bother?\r\n\r\nSo, I apologize to the few high level people who will feel hurt\r\nby this, but look at it this way... as long as you don't get\r\nlevel drained, you're WAY ahead of everyone else!\r\n
5099	48	1996-03-03 15:29:58	Dorn	dead	The new undea thing is cool.\r\nI love it\r\nif a newibe is killing chickens, ya can bury them...  even if\r\nyou're in HS or greth or something.... *giggle* and I'd just like\r\nto announce that... I'll probably not be burying the corpses.....\r\n:likes killing undead.\r\n
5099	49	1996-03-04 03:27:21	Puck	40	Hmmm... I agree with the new level tables... they are far more\r\nright than they were.  Except that between 14th-20th theres not\r\nmany good places to get experience on your own.  (God only knows\r\nhow mnay times I leveled the stands).\r\n\r\nBut really, these levels are pretty worthless for me.  I haven't\r\ngained a new spell or skill since 16th, and I'm 22nd now!  Its \r\ngetting dull.\r\n\r\nand the undead thing is okay, but it kinda sucks... burying every\r\ncorpse is time-consuming...  but thats a god's decision...\r\n\r\nanyhow... add new ranger spells.\r\n\r\nPuck\r\n
5099	50	1996-03-04 09:28:14	Dorn	exp	Gee, I have to agree with storm and puck.\r\nHmmm.... The new level tables are slick.\r\nI like working to advance.\r\nI don't like starting over when I die.\r\nSee, I died and suddenly needed like 4.2 million to advance.\r\nNow, I am  31st level, so it's coming back, but I'm also \r\nlogged in for like 30 hours a week.  and 30 hours is a lot \r\nto advance.\r\n\r\nThe Paliadin \r\nP.S. My mana Regain hasn't changed since 20th.  Is that like\r\nthe level tables?  Has no-one thought of that?\r\n
5099	51	1996-03-11 00:01:54	Sedna	Rules of the game	Hello, players.\r\n\r\nIt has always been the policy of WileyMUD in the past to allow\r\nonly one playing character per person.  The rule is somewhat\r\nrelaxed, but the Gods consider the simultaneous use of characters\r\nby one person to be EVIL.  Such players are warned against using\r\ntwo characters, lest the wrath of the gods might be visited upon\r\nthe offenders.\r\n\r\nIf you have any questions, please post them to this board and I\r\nwill attempt to clarify the policy.\r\n\r\nThanks\r\n\r\n-Sedna\r\n
5099	52	1996-03-11 16:21:42	Dorn	Appraise	So, I was feeling like there autta be SOME way to tell if a\r\nweapon is damaged.  I figured that perhaps apraise autta give a\r\nhint.  But I still feel like apraise might also mention something\r\nabout the damage roll.\r\n
5099	53	1996-03-11 17:36:54	Sedna	Appraising	What I see when I appraise an item is the statements:\r\nIt is very well balance.\r\nand\r\nIt has definite magical properties.\r\nGranted, it doesn't give hit/dam scores or such, but is a good\r\nindicator of the damage level of the weapon and it's current\r\ncondition.  I will look into this further to see if I can improve\r\nit.\r\n\r\n-Sedna\r\n
5099	54	1996-03-11 20:14:43	Dorn	RE: apr by sedna	No it isn't.  Those descriptions are about the Majikal proporties\r\nonly.\r\n
5099	55	1996-03-11 23:55:19	Sedna	Puck, stop complaining.	You have a new spell.  Check it out.\r\n
5099	56	1996-03-13 13:08:31	Muidnar	PwD	Wow.... My password is "false"\r\nBelive me?\r\nTry it out and see.\r\n
5099	57	1996-03-14 00:10:09	Sedna	Muidnar	It won't do any good, of course....Muidnar is level 1 now....\r\n\r\nThe Dark One has fallen!!!\r\n
5099	58	1996-03-18 03:25:00	Edge	ACandSuch	ok, so im lucky enough to have my ac at 0 with eq and no spells,\r\ndont call me greedy but when i equip my shield that is 15ac i\r\nwould think that my ac would drop to -1 at least. Am i wrong? or\r\nis there a limit to AC that can be gained by eq?\r\n\r\nAlso, in an effort to try different things, i am lowering my\r\nalign to try different eq and cant see to lower my alignment\r\nanymore.  I was told that bury makes you better so i stopped\r\nburying and tried to get and junk them or give them to hermit.\r\nThis isnt working.  I am killing good citizens of\r\nhighstaff,doves,young lasses, its not working.  Perhaps something\r\nis wrong with this Pfile, i dont know.\r\n\r\nQuix made some doves for me once, and that worked, but it hasnt changed since.\r\n
5099	59	1996-03-18 18:36:29	Puck	Pouting	Stop your pouting, edge.  Go to shylar, slay all the villagers\r\nand let them become demons to kill the newbies.\r\n\r\nGo after the pegasuses in the mountains.  Avoid killing thieves,\r\ngypsies, seedy boys, gamesman, drenth, slaadi, mountain giants,\r\netc.\r\n\r\nDon't bury anything, and don't kill the undead that come from\r\nthat, if possible.\r\n\r\nHopefully there will be a nice area for you to become evil in\r\nsoon.  Its much easier to make experience being good.\r\n\r\nPuck the All-Good.\r\n
5099	60	1996-04-18 19:26:50	Storm	poison	I got a question about the poison spell, cause i tried to cast it\r\non a weapon, one magical and another nonmagical.  Each one i\r\ntried to detect poison on it and also on myself and non of those\r\ntold me if the object was actually poisoned.  Shouldnt detect\r\npoison and the poison work on objects in the way that i used em?\r\ncould some one look at it? and is the poison permanent on\r\nobjects? or what?\r\n\r\nhey, thanks again\r\n\r\n>======------->Storm,  the soon to be employed\r\n
5099	61	1996-06-13 12:23:59	Sedna	poison	I don't believe poison works on weapons.  That is a neat idea,\r\nthough.  I cast poison on a weapon and ran stat on it and nothing\r\nchanged.\r\n
5099	62	1996-07-13 21:22:27	Questor	undead	I don't think it's fair that undead roam shylar and highstaff and\r\nthink I should get my weapons and xp back.\r\n\r\nquestor\r\n
5099	63	1996-07-13 21:26:19	Questor	unfair	this game has become very unfair.\r\n\r\nIt's no wonder noone wants to play anymore.\r\n
5099	64	1996-07-22 00:00:54	Quixadhal	RE: unfair	The only reason it's unfair is that nobody is playing.  The\r\nfeatures I've added to make the game more challenging assume a\r\nsmalish-normal playerbase of about 10-20 people online at any\r\ngiven moment.  As you see, with only 1 or 2, it becomes MUCH\r\nharder.\r\n\r\nWiley has *ALWAYS* been designed around people grouping together\r\ninstead of wandering around solo... yes, you could do it solo...\r\nbut it was DAMN hard!\r\n\r\nMy suggestion would be to try and convince a few friends to give\r\nthe place a chance and join up with you.  I haven't gone out of\r\nmy way to advertise the place because, frankly, I don't think\r\nit's out of beta-testing yet.  I think you'll find that if we get\r\nsomeone dedicated to runing the place full-time (IE:  not a coder\r\nlike me... a real IMM with no other side-projects to distract\r\nhim/her), and a few dozen players, the game will be a hell of\r\nalot of fun.\r\n\r\nFYI:  My last goal in coding this thing is to finish divorcing\r\nthe data files from their hard-coded state so future imms can\r\nexpand this thing easily.  If I ever get the chance to finish\r\nthat, I'll be more than happy to look around for anyone wanting\r\nto really run this game as a game instead of a test-bed.\r\n\r\nCheers!                             -Dread Quixadhal.\r\n
5099	65	1996-11-18 17:15:22	Suuphi	Players	How come nobody else EVER plays here?\r\n\r\nIt's not that bad.... *sigh*\r\n
5099	66	1999-03-30 16:29:42	Quixadhal	The Land is Dead.	Greetings, to any who might wander in this forsaken land.\r\nThrought years of neglect, mismanagement, and battling egos, the\r\nworld has faded into obsucurity and dust.\r\n\r\nIt shall never be as it once was, but perhaps I can take a little\r\nbit of it with me to another place.\r\n\r\nFarewell.\r\n
3098	19	1996-02-12 09:47:00	Muidnar	Grimwell	An old wizard Grimwell wants to build again. I was reading the\r\nideas and typoo logs and saw his message. Yes people do read\r\nthose! Well those of us who know where they are at. Anyway Quix\r\nwhat do you think. His areas are good and need little changing\r\nwhen finished. and he will build unlike the rest. Well with the\r\nexecption of the drow area that was placed in by Highlander.\r\nUnlike wizards like Zar??? I think he is a wizard? when was the\r\nlast time he logged in????\r\n\r\nAlso why am I still 59th?\r\n\r\nWhy do I not have access to the world file?\r\nIt has only been about 6 months. I provided 3500 rooms to restore\r\nthis mud to its current condition how much longer do I have to\r\nwait?\r\n\r\nI also hate your typo lines. This is NOT MTV it is a mud with a\r\nmid-evil setting, If I'm not mistaken I believe that is what we\r\nwhere going to work on??? or am I wrong.\r\n\r\nHave a Day?\r\n\r\n                 Muidnar Hopes you rot in Hell\r\n
3098	35	2010-07-18 01:04:35	Quixadhal	SQL boards!	So, I've now removed normal file-based boards and moved\r\neverything into SQL.  Hopefully, it works ok. :)\r\n
\.


--
-- Data for Name: log_types; Type: TABLE DATA; Schema: public; Owner: wiley
--

COPY log_types (log_type_id, name, description) FROM stdin;
0	INFO	Generic messages, typically things to note, not errors
1	ERROR	Nasy errors that the driver can continue to live with
2	FATAL	Catestrophic errors that will force the drive to shutdown
3	BOOT	Messages generated while booting the database
4	AUTH	Login events
5	KILL	Player or NPC killed something
6	DEATH	Player or NPC died to something
7	RESET	Zone reset event
8	IMC	Message from the IMC2 network
\.


--
-- Data for Name: logfile; Type: TABLE DATA; Schema: public; Owner: wiley
--

COPY logfile (log_type_id, log_date, log_entry, log_file, log_function, log_line, log_areafile, log_arealine, log_pc_actor, log_pc_victim, log_npc_actor, log_npc_victim, log_obj, log_area, log_room) FROM stdin;
3	2010-07-10 12:56:52.553786	Connected to database!	comm.c	main	203	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.628809	PostgreSQL 8.4.4 on i486-pc-linux-gnu, compiled by GCC gcc-4.4.real (Debian 4.4.4-5) 4.4.4, 32-bit\n	comm.c	main	204	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.632207	Signal trapping.	comm.c	run_the_game	227	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.634137	Opening mother connection.	comm.c	run_the_game	230	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.63627	Boot db -- BEGIN.	db.c	load_db	122	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.63817	- Resetting game time and weather:	db.c	load_db	124	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.64023	- Reading news	db.c	load_db	127	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.671142	- Reading credits	db.c	load_db	129	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.682646	- Reading motd	db.c	load_db	131	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.714884	- Reading help	db.c	load_db	133	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.731024	- Reading info	db.c	load_db	135	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.733489	- Reading wizlist	db.c	load_db	137	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.735743	- Reading wiz motd	db.c	load_db	139	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.7475	- Reading greetings	db.c	load_db	141	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.749751	- Reading login menu	db.c	load_db	143	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.751982	- Reading sex menu	db.c	load_db	145	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.754163	- Reading race menu	db.c	load_db	147	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.756356	- Reading class menu	db.c	load_db	149	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.758546	- Reading race help	db.c	load_db	151	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.760787	- Reading class help	db.c	load_db	153	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.762984	- Reading story	db.c	load_db	155	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.76525	- Reading suicide warning	db.c	load_db	157	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.767452	- Reading suicide result	db.c	load_db	159	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.772454	- Loading banned name list from SQL	ban.pgc	load_bans	195	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.859064	- Loading banned ip list from SQL	ban.pgc	load_bans	210	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.86607	- Loading banned name@ip list from SQL	ban.pgc	load_bans	225	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.869234	- Loading rent mode	db.c	load_db	164	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.930407	- Loading player list	db.c	load_db	188	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.94329	- Loading reboot times	db.c	load_db	210	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.945531	- Loading help files	db.c	load_db	241	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:52.995516	- Loading fight messages	db.c	load_db	251	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.005123	- Loading social messages	db.c	load_db	253	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.008616	- Loading pose messages	db.c	load_db	255	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.010555	- Booting mobiles	db.c	load_db	258	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.012517	- Booting objects	db.c	load_db	263	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.014447	- Booting zones	db.c	load_db	268	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.088094	- Booting rooms	db.c	load_db	270	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.385104	- All Rooms loaded!	db.c	boot_world	518	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.387171	- Generating mobile index	db.c	load_db	273	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.430734	- Generating object index	db.c	load_db	275	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.438815	- Renumbering zones	db.c	load_db	277	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.441341	- Assigining mobile functions	db.c	load_db	280	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.489112	- Assigining object functions	db.c	load_db	282	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.491103	- Assigining room functions	db.c	load_db	284	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.493076	- Assigning command functions	db.c	load_db	287	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.495021	- Assigning spell functions	db.c	load_db	289	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.50182	- Loading board 3098 from SQL	board.pgc	new_load_board	505	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.509061	- Loaded!	board.pgc	new_load_board	543	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.528495	- Loading board 3099 from SQL	board.pgc	new_load_board	505	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.549214	- Loaded!	board.pgc	new_load_board	543	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.568213	- Loading board 5099 from SQL	board.pgc	new_load_board	505	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.574338	- Loaded!	board.pgc	new_load_board	543	\N	\N	\N	\N	\N	\N	\N	\N	\N
3	2010-07-10 12:56:53.731002	Boot db -- DONE.	db.c	load_db	295	\N	\N	\N	\N	\N	\N	\N	\N	\N
8	2010-07-10 13:22:06.507999	[07/10 13:22] [dchat] Ninja@Dead_Souls_Dev: telnet to the router or web-surf to it?	\N	\N	\N	\N	\N	\N	\N	\N	\N	\N	\N	\N
8	2010-07-10 14:09:06.999494	[07/10 14:09] [dchat] Cratylus@Dead_Souls_Dev: so why was it worse to be boiled in oil than water?	\N	\N	\N	\N	\N	\N	\N	\N	\N	\N	\N	\N
\.


--
-- Name: banned_banned_ip_key; Type: CONSTRAINT; Schema: public; Owner: wiley; Tablespace: 
--

ALTER TABLE ONLY banned
    ADD CONSTRAINT banned_banned_ip_key UNIQUE (banned_ip);


--
-- Name: log_types_name_key; Type: CONSTRAINT; Schema: public; Owner: wiley; Tablespace: 
--

ALTER TABLE ONLY log_types
    ADD CONSTRAINT log_types_name_key UNIQUE (name);


--
-- Name: log_types_pkey; Type: CONSTRAINT; Schema: public; Owner: wiley; Tablespace: 
--

ALTER TABLE ONLY log_types
    ADD CONSTRAINT log_types_pkey PRIMARY KEY (log_type_id);


--
-- Name: ix_banned_name; Type: INDEX; Schema: public; Owner: wiley; Tablespace: 
--

CREATE INDEX ix_banned_name ON banned USING btree (banned_name);


--
-- Name: ix_banned_name_lc; Type: INDEX; Schema: public; Owner: wiley; Tablespace: 
--

CREATE UNIQUE INDEX ix_banned_name_lc ON banned USING btree (lower(banned_name));


--
-- Name: ix_board_messages; Type: INDEX; Schema: public; Owner: wiley; Tablespace: 
--

CREATE UNIQUE INDEX ix_board_messages ON board_messages USING btree (board_id, message_id);


--
-- Name: ix_logfile_date; Type: INDEX; Schema: public; Owner: wiley; Tablespace: 
--

CREATE INDEX ix_logfile_date ON logfile USING btree (log_date);


--
-- Name: logfile_log_type_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: wiley
--

ALTER TABLE ONLY logfile
    ADD CONSTRAINT logfile_log_type_id_fkey FOREIGN KEY (log_type_id) REFERENCES log_types(log_type_id);


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

