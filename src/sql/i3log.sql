--
-- PostgreSQL database dump
--

-- Dumped from database version 15.4 (Debian 15.4-1)
-- Dumped by pg_dump version 15.4 (Debian 15.4-1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

ALTER TABLE IF EXISTS ONLY public.speakers DROP CONSTRAINT IF EXISTS speakers_pinkfish_fkey;
ALTER TABLE IF EXISTS ONLY public.hours DROP CONSTRAINT IF EXISTS hours_pinkfish_fkey;
ALTER TABLE IF EXISTS ONLY public.channels DROP CONSTRAINT IF EXISTS channels_pinkfish_fkey;
DROP INDEX IF EXISTS public.ix_urls_checksum;
DROP INDEX IF EXISTS public.ix_i3log_username;
DROP INDEX IF EXISTS public.ix_i3log_url;
DROP INDEX IF EXISTS public.ix_i3log_speaker;
DROP INDEX IF EXISTS public.ix_i3log_local;
DROP INDEX IF EXISTS public.ix_i3log_channel;
DROP INDEX IF EXISTS public.ix_i3log_bot;
DROP INDEX IF EXISTS public.ix_i3_packets_type;
DROP INDEX IF EXISTS public.ix_i3_packets_local;
DROP INDEX IF EXISTS public.ix_i3_packets_length;
DROP INDEX IF EXISTS public.ix_i3_packets_created;
ALTER TABLE IF EXISTS ONLY public.speakers DROP CONSTRAINT IF EXISTS speakers_pkey;
ALTER TABLE IF EXISTS ONLY public.pinkfish_map DROP CONSTRAINT IF EXISTS pinkfish_map_pkey;
ALTER TABLE IF EXISTS ONLY public.i3log DROP CONSTRAINT IF EXISTS ix_i3log_row;
ALTER TABLE IF EXISTS ONLY public.hours DROP CONSTRAINT IF EXISTS hours_pkey;
ALTER TABLE IF EXISTS ONLY public.channels DROP CONSTRAINT IF EXISTS channels_pkey;
DROP TABLE IF EXISTS public.urls;
DROP VIEW IF EXISTS public.page_view;
DROP VIEW IF EXISTS public.packet_view;
DROP VIEW IF EXISTS public.new_page_view;
DROP TABLE IF EXISTS public.speakers;
DROP TABLE IF EXISTS public.pinkfish_map;
DROP TABLE IF EXISTS public.logfile;
DROP TABLE IF EXISTS public.i3_packets;
DROP TABLE IF EXISTS public.hours;
DROP VIEW IF EXISTS public.death_watch;
DROP TABLE IF EXISTS public.i3log;
DROP TABLE IF EXISTS public.channels;
DROP EXTENSION IF EXISTS plperlu;
-- *not* dropping schema, since initdb creates it
--
-- Name: public; Type: SCHEMA; Schema: -; Owner: -
--

-- *not* creating schema, since initdb creates it


--
-- Name: plperlu; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS plperlu WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plperlu; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON EXTENSION plperlu IS 'PL/PerlU untrusted procedural language';


SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: channels; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.channels (
    channel text NOT NULL,
    pinkfish text NOT NULL
);


--
-- Name: i3log; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.i3log (
    created timestamp without time zone DEFAULT timezone('UTC'::text, now()) NOT NULL,
    local timestamp with time zone DEFAULT now() NOT NULL,
    is_emote boolean,
    is_url boolean,
    is_bot boolean,
    channel text NOT NULL,
    speaker text NOT NULL,
    mud text NOT NULL,
    message text,
    username text
);


--
-- Name: death_watch; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW public.death_watch AS
 SELECT s.speaker,
    max(i3log.local) AS last_spoke
   FROM (public.i3log
     JOIN ( SELECT DISTINCT i3log_1.speaker
           FROM public.i3log i3log_1) s USING (speaker))
  GROUP BY s.speaker
 HAVING (max(i3log.local) < (now() - '7 days'::interval))
  ORDER BY (max(i3log.local)) DESC
 LIMIT 20;


--
-- Name: hours; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.hours (
    hour integer NOT NULL,
    pinkfish text NOT NULL
);


--
-- Name: i3_packets; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.i3_packets (
    created timestamp without time zone DEFAULT timezone('UTC'::text, now()) NOT NULL,
    local timestamp with time zone DEFAULT now() NOT NULL,
    packet_type text NOT NULL,
    packet_length integer NOT NULL,
    packet_content text
);


--
-- Name: logfile; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.logfile (
    created timestamp without time zone DEFAULT timezone('UTC'::text, now()) NOT NULL,
    logtype text DEFAULT 'INFO'::text,
    filename text,
    function text,
    line integer,
    area_file text,
    area_line integer,
    "character" text,
    character_room integer,
    victim text,
    victim_room integer,
    message text
);


--
-- Name: pinkfish_map; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.pinkfish_map (
    pinkfish text NOT NULL,
    html text NOT NULL
);


--
-- Name: speakers; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.speakers (
    speaker text NOT NULL,
    pinkfish text NOT NULL
);


--
-- Name: new_page_view; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW public.new_page_view AS
 SELECT i3log.local,
    i3log.is_emote,
    i3log.is_url,
    i3log.is_bot,
    i3log.channel,
    i3log.speaker,
    i3log.mud,
    i3log.message,
    to_char(i3log.local, 'YYYY-MM-DD'::text) AS the_date,
    to_char(i3log.local, 'HH24:MI:SS'::text) AS the_time,
    to_char(i3log.local, 'YYYY'::text) AS the_year,
    to_char(i3log.local, 'MM'::text) AS the_month,
    to_char(i3log.local, 'DD'::text) AS the_day,
    to_char(i3log.local, 'HH24'::text) AS the_hour,
    to_char(i3log.local, 'MI'::text) AS the_minute,
    to_char(i3log.local, 'SS'::text) AS the_second,
    date_part('hour'::text, i3log.local) AS int_hour,
    hours.pinkfish AS hour_color,
    channels.pinkfish AS channel_color,
    speakers.pinkfish AS speaker_color,
    pinkfish_map_hour.html AS hour_html,
    pinkfish_map_channel.html AS channel_html,
    pinkfish_map_speaker.html AS speaker_html
   FROM ((((((public.i3log
     LEFT JOIN public.hours ON ((date_part('hour'::text, i3log.local) = (hours.hour)::double precision)))
     LEFT JOIN public.channels ON ((lower(i3log.channel) = channels.channel)))
     LEFT JOIN public.speakers ON ((lower(i3log.username) = speakers.speaker)))
     LEFT JOIN public.pinkfish_map pinkfish_map_hour ON ((hours.pinkfish = pinkfish_map_hour.pinkfish)))
     LEFT JOIN public.pinkfish_map pinkfish_map_channel ON ((channels.pinkfish = pinkfish_map_channel.pinkfish)))
     LEFT JOIN public.pinkfish_map pinkfish_map_speaker ON ((speakers.pinkfish = pinkfish_map_speaker.pinkfish)));


--
-- Name: packet_view; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW public.packet_view AS
 SELECT to_char(max(i3_packets.local), 'FMDY HH24:MI:SS.MS'::text) AS recent,
    i3_packets.packet_type,
    count(i3_packets.packet_type) AS count,
    min(i3_packets.packet_length) AS min,
    to_char(avg(i3_packets.packet_length), '999999D99'::text) AS avg,
    max(i3_packets.packet_length) AS max
   FROM public.i3_packets
  GROUP BY i3_packets.packet_type;


--
-- Name: page_view; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW public.page_view AS
 SELECT i3log.local,
    i3log.is_emote,
    i3log.is_url,
    i3log.is_bot,
    i3log.channel,
    i3log.speaker,
    i3log.mud,
    i3log.message,
    to_char(i3log.local, 'YYYY-MM-DD'::text) AS the_date,
    to_char(i3log.local, 'HH24:MI:SS'::text) AS the_time,
    to_char(i3log.local, 'YYYY'::text) AS the_year,
    to_char(i3log.local, 'MM'::text) AS the_month,
    to_char(i3log.local, 'DD'::text) AS the_day,
    to_char(i3log.local, 'HH24'::text) AS the_hour,
    to_char(i3log.local, 'MI'::text) AS the_minute,
    to_char(i3log.local, 'SS'::text) AS the_second,
    date_part('hour'::text, i3log.local) AS int_hour,
    hours.pinkfish AS hour_color,
    channels.pinkfish AS channel_color,
    speakers.pinkfish AS speaker_color,
    pinkfish_map_hour.html AS hour_html,
    pinkfish_map_channel.html AS channel_html,
    pinkfish_map_speaker.html AS speaker_html
   FROM ((((((public.i3log
     LEFT JOIN public.hours ON ((date_part('hour'::text, i3log.local) = (hours.hour)::double precision)))
     LEFT JOIN public.channels ON ((lower(i3log.channel) = channels.channel)))
     LEFT JOIN public.speakers ON ((lower(i3log.username) = speakers.speaker)))
     LEFT JOIN public.pinkfish_map pinkfish_map_hour ON ((hours.pinkfish = pinkfish_map_hour.pinkfish)))
     LEFT JOIN public.pinkfish_map pinkfish_map_channel ON ((channels.pinkfish = pinkfish_map_channel.pinkfish)))
     LEFT JOIN public.pinkfish_map pinkfish_map_speaker ON ((speakers.pinkfish = pinkfish_map_speaker.pinkfish)));


--
-- Name: urls; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.urls (
    created timestamp without time zone DEFAULT timezone('UTC'::text, now()) NOT NULL,
    processed boolean,
    channel text NOT NULL,
    speaker text NOT NULL,
    mud text NOT NULL,
    url text,
    message text,
    checksum text,
    hits integer DEFAULT 1,
    tiny text
);


--
-- Data for Name: channels; Type: TABLE DATA; Schema: public; Owner: -
--

COPY public.channels (channel, pinkfish) FROM stdin;
%^reset%^	%^CYAN%^
1	%^CYAN%^
4l	%^WHITE%^%^BOLD%^
adr	%^BLUE%^%^BOLD%^
animechat	%^MAGENTA%^
ascii_art	%^YELLOW%^
avatar	%^YELLOW%^
bd_devtalk	%^BLUE%^
bestiality	%^RED%^%^BOLD%^
blah	%^YELLOW%^
bofh	%^CYAN%^
bot	%^MAGENTA%^
bsg	%^MAGENTA%^
church	%^ORANGE%^
cleric	%^BLUE%^
coffeemud_universe	%^MAGENTA%^
crat	%^MAGENTA%^%^BOLD%^
crossmud	%^CYAN%^
dchat	%^CYAN%^
dead_souls	%^ORANGE%^
dead_test4	%^MAGENTA%^
dead_test8	%^GREEN%^%^BOLD%^
default	%^BLUE%^%^BOLD%^
default-IMC2	%^B_BLUE%^%^WHITE%^
dgd	%^MAGENTA%^
discord	%^ORANGE%^
discworld-chat	%^WHITE%^%^BOLD%^
discworld-cre	%^GREEN%^%^BOLD%^
discworld_chat	%^WHITE%^%^BOLD%^
dragon	%^WHITE%^
dreamverse_gossip	%^RED%^
ds	%^ORANGE%^
ds_test	%^BLUE%^%^BOLD%^
dutch	%^GREEN%^
dwchat	%^WHITE%^%^BOLD%^
dwcre	%^BLUE%^
ea	%^CYAN%^%^BOLD%^
fluffos	%^CYAN%^%^BOLD%^
free_speach	%^YELLOW%^
free_speech	%^RED%^
french	%^RED%^
furry	%^RED%^%^BOLD%^
german	%^RED%^
godwars	%^GREEN%^
gossip	%^WHITE%^
gw_imm	%^ORANGE%^
havenlib	%^BLUE%^
ichat	%^ORANGE%^
icode	%^GREEN%^
ifree	%^WHITE%^%^BOLD%^
igossip	%^RED%^%^BOLD%^
imud_code	%^ORANGE%^
imud_gossip	%^GREEN%^
imud_sgz	%^GREEN%^
inews	%^BLUE%^
insomnia-chat	%^RED%^%^BOLD%^
inter-router	%^CYAN%^%^BOLD%^
intercre	%^ORANGE%^
intergossip	%^GREEN%^
jammstest	%^BLUE%^%^BOLD%^
lima	%^MAGENTA%^%^BOLD%^
linux	%^BLUE%^%^BOLD%^
lpuni	%^WHITE%^
minecraft	%^YELLOW%^
mlp	%^GREEN%^%^BOLD%^
mudnews	%^WHITE%^%^BOLD%^
mychannel	%^BLUE%^
newbie	%^GREEN%^%^BOLD%^
nschat	%^CYAN%^%^BOLD%^
nscre	%^WHITE%^
ooc	%^WHITE%^%^BOLD%^
otg	%^MAGENTA%^%^BOLD%^
public	%^BLUE%^%^BOLD%^
pyom	%^FLASH%^%^GREEN%^
rome_chat	%^RED%^
serenity	%^WHITE%^
smblah	%^GREEN%^%^BOLD%^
sport	%^GREEN%^
test	%^RED%^%^BOLD%^
trivia_games	%^ORANGE%^
troll	%^RED%^
url	%^YELLOW%^%^B_BLUE%^
vest	%^CYAN%^%^BOLD%^
wiley	%^YELLOW%^
wileymud	%^YELLOW%^
wolchat	%^CYAN%^
wolimm	%^MAGENTA%^
wotf-devel	%^MAGENTA%^%^BOLD%^
zblah	%^MAGENTA%^%^BOLD%^
bsg2	%^CYAN%^
wizard	%^RED%^%^BOLD%^
i3testers	%^GREEN%^%^BOLD%^
japanese	%^WHITE%^%^BOLD%^
diku	%^YELLOW%^
pkiller	%^MAGENTA%^%^BOLD%^
coffeemudesp	%^BLUE%^%^BOLD%^
tinyurl	%^CYAN%^%^BOLD%^
mage	%^WHITE%^%^BOLD%^
foobar	%^RED%^
eotwtest	%^GREEN%^
intermud	%^ORANGE%^
gwimm	%^BLUE%^
\.


--
-- Data for Name: hours; Type: TABLE DATA; Schema: public; Owner: -
--

COPY public.hours (hour, pinkfish) FROM stdin;
0	%^DARKGREY%^
1	%^DARKGREY%^
10	%^GREEN%^
11	%^GREEN%^
12	%^LIGHTGREEN%^
13	%^LIGHTGREEN%^
14	%^WHITE%^
15	%^WHITE%^
16	%^LIGHTCYAN%^
17	%^LIGHTCYAN%^
18	%^CYAN%^
19	%^CYAN%^
2	%^DARKGREY%^
20	%^LIGHTBLUE%^
21	%^LIGHTBLUE%^
22	%^BLUE%^
23	%^BLUE%^
3	%^DARKGREY%^
4	%^RED%^
5	%^RED%^
6	%^ORANGE%^
7	%^ORANGE%^
8	%^YELLOW%^
9	%^YELLOW%^
\.


--
-- Data for Name: logfile; Type: TABLE DATA; Schema: public; Owner: -
--

COPY public.logfile (created, logtype, filename, function, line, area_file, area_line, "character", character_room, victim, victim_room, message) FROM stdin;
\.


--
-- Data for Name: pinkfish_map; Type: TABLE DATA; Schema: public; Owner: -
--

COPY public.pinkfish_map (pinkfish, html) FROM stdin;
%^FLASH%^%^LIGHTGREEN%^	<SPAN style="color: #55ff55">
%^YELLOW%^%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb; color: #ffff55">
%^B_MAGENTA%^%^YELLOW%^	<SPAN style="background-color: #bb00bb; color: #ffff55">
%^B_RED%^%^LIGHTGREEN%^	<SPAN style="background-color: #bb0000; color: #00ff00">
%^B_MAGENTA%^%^WHITE%^	<SPAN style="background-color: #bb00bb; color: #ffffff">
%^B_MAGENTA%^%^BLACK%^	<SPAN style="background-color: #bb00bb; color: #000000">
%^BLACK%^%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb; color: #000000">
%^WHITE%^%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb; color: #ffffff">
%^B_YELLOW%^%^GREEN%^	<SPAN style="background-color: #ffff55; color: #00bb00">
%^YELLOW%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb; color: #ffff55">
%^B_GREEN%^%^YELLOW%^	<SPAN style="background-color: #00bb00; color: #ffff55">
%^FLASH%^LIGHTGREEN%^	<SPAN style="color: #55ff55">
%^YELLOW%^%^B_GREEN%^	<SPAN style="background-color: #00bb00; color: #ffff55">
%^B_MAGENTA%^YELLOW%^	<SPAN style="background-color: #bb00bb; color: #ffff55">
%^B_YELLOW%^%^BLACK%^	<SPAN style="background-color: #ffff55; color: #000000">
%^GREEN%^%^B_YELLOW%^	<SPAN style="background-color: #ffff55; color: #00bb00">
%^BLACK%^%^B_YELLOW%^	<SPAN style="background-color: #ffff55; color: #000000">
%^B_RED%^LIGHTGREEN%^	<SPAN style="background-color: #bb0000; color: #00ff00">
%^B_BLUE%^%^YELLOW%^	<SPAN style="background-color: #0000bb; color: #ffff55">
%^B_GREEN%^%^WHITE%^	<SPAN style="background-color: #00bb00; color: #ffffff">
%^WHITE%^%^B_BLACK%^	<SPAN style="color: #ffffff">
%^B_MAGENTA%^WHITE%^	<SPAN style="background-color: #bb00bb; color: #ffffff">
%^B_WHITE%^%^GREEN%^	<SPAN style="background-color: #ffffff; color: #00bb00">
%^BLACK%^%^B_GREEN%^	<SPAN style="background-color: #00bb00; color: #000000">
%^B_WHITE%^%^BLACK%^	<SPAN style="background-color: #ffffff; color: #000000">
%^WHITE%^%^B_GREEN%^	<SPAN style="background-color: #00bb00; color: #ffffff">
%^BLUE%^%^B_YELLOW%^	<SPAN style="background-color: #ffff55; color: #0000bb">
%^B_GREEN%^%^BLACK%^	<SPAN style="background-color: #00bb00; color: #000000">
%^B_YELLOW%^%^BLUE%^	<SPAN style="background-color: #ffff55; color: #0000bb">
%^WHITE%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb; color: #ffffff">
%^BLACK%^%^B_WHITE%^	<SPAN style="background-color: #ffffff; color: #000000">
%^B_MAGENTA%^BLACK%^	<SPAN style="background-color: #bb00bb; color: #000000">
%^BLACK%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb; color: #000000">
%^YELLOW%^%^B_BLUE%^	<SPAN style="background-color: #0000bb; color: #ffff55">
%^B_BLACK%^%^WHITE%^	<SPAN style="color: #ffffff">
%^GREEN%^%^B_WHITE%^	<SPAN style="background-color: #ffffff; color: #00bb00">
%^B_YELLOW%^BLACK%^	<SPAN style="background-color: #ffff55; color: #000000">
%^YELLOW%^B_GREEN%^	<SPAN style="background-color: #00bb00; color: #ffff55">
%^MAGENTA%^%^BOLD%^	<SPAN style="color: #ff55ff">
%^BLUE%^%^B_WHITE%^	<SPAN style="background-color: #ffffff; color: #0000bb">
%^BLACK%^%^B_CYAN%^	<SPAN style="background-color: #00bbbb; color: #000000">
%^B_CYAN%^%^BLACK%^	<SPAN style="background-color: #00bbbb; color: #000000">
%^B_YELLOW%^GREEN%^	<SPAN style="background-color: #ffff55; color: #00bb00">
%^BLACK%^B_YELLOW%^	<SPAN style="background-color: #ffff55; color: #000000">
%^YELLOW%^%^B_RED%^	<SPAN style="background-color: #bb0000; color: #ffff55">
%^B_GREEN%^YELLOW%^	<SPAN style="background-color: #00bb00; color: #ffff55">
%^WHITE%^%^B_BLUE%^	<SPAN style="background-color: #0000bb; color: #ffffff">
%^B_BLACK%^%^GREY%^	<SPAN style="color: #bbbbbb">
%^GREY%^%^B_BLACK%^	<SPAN style="color: #bbbbbb">
%^GREEN%^B_YELLOW%^	<SPAN style="background-color: #ffff55; color: #00bb00">
%^B_BLUE%^%^WHITE%^	<SPAN style="background-color: #0000bb; color: #ffffff">
%^BOLD%^%^MAGENTA%^	<SPAN style="color: #ff55ff">
%^B_RED%^%^YELLOW%^	<SPAN style="background-color: #bb0000; color: #ffff55">
%^B_WHITE%^%^BLUE%^	<SPAN style="background-color: #ffffff; color: #0000bb">
%^GREEN%^%^B_RED%^	<SPAN style="background-color: #bb0000; color: #00ff00">
%^B_RED%^%^BLACK%^	<SPAN style="background-color: #bb0000; color: #000000">
%^B_RED%^%^GREEN%^	<SPAN style="background-color: #bb0000; color: #00ff00">
%^BLUE%^%^B_CYAN%^	<SPAN style="background-color: #00bbbb; color: #0000bb">
%^B_CYAN%^%^BLUE%^	<SPAN style="background-color: #00bbbb; color: #0000bb">
%^BLACK%^%^B_RED%^	<SPAN style="background-color: #bb0000; color: #000000">
%^WHITE%^B_GREEN%^	<SPAN style="background-color: #00bb00; color: #ffffff">
%^B_GREEN%^BLACK%^	<SPAN style="background-color: #00bb00; color: #000000">
%^WHITE%^B_BLACK%^	<SPAN style="color: #ffffff">
%^WHITE%^%^B_RED%^	<SPAN style="background-color: #bb0000; color: #ffffff">
%^BLACK%^B_GREEN%^	<SPAN style="background-color: #00bb00; color: #000000">
%^YELLOW%^B_BLUE%^	<SPAN style="background-color: #0000bb; color: #ffff55">
%^B_BLACK%^WHITE%^	<SPAN style="color: #ffffff">
%^BLUE%^B_YELLOW%^	<SPAN style="background-color: #ffff55; color: #0000bb">
%^B_BLUE%^YELLOW%^	<SPAN style="background-color: #0000bb; color: #ffff55">
%^BLACK%^B_WHITE%^	<SPAN style="background-color: #ffffff; color: #000000">
%^B_WHITE%^GREEN%^	<SPAN style="background-color: #ffffff; color: #00bb00">
%^B_GREEN%^WHITE%^	<SPAN style="background-color: #00bb00; color: #ffffff">
%^FLASH%^%^GREEN%^	<SPAN style="color: #55ff55">
%^B_WHITE%^BLACK%^	<SPAN style="background-color: #ffffff; color: #000000">
%^B_YELLOW%^BLUE%^	<SPAN style="background-color: #ffff55; color: #0000bb">
%^GREEN%^B_WHITE%^	<SPAN style="background-color: #ffffff; color: #00bb00">
%^BOLD%^%^YELLOW%^	<SPAN style="color: #ffff55">
%^B_RED%^%^WHITE%^	<SPAN style="background-color: #bb0000; color: #ffffff">
%^GREY%^B_BLACK%^	<SPAN style="color: #bbbbbb">
%^BOLD%^%^BLACK%^	<SPAN style="color: #555555">
%^B_BLACK%^GREY%^	<SPAN style="color: #bbbbbb">
%^BOLD%^%^WHITE%^	<SPAN style="color: #ffffff">
%^BOLD%^%^GREEN%^	<SPAN style="color: #55ff55">
%^B_WHITE%^BLUE%^	<SPAN style="background-color: #ffffff; color: #0000bb">
%^YELLOW%^B_RED%^	<SPAN style="background-color: #bb0000; color: #ffff55">
%^BLACK%^B_CYAN%^	<SPAN style="background-color: #00bbbb; color: #000000">
%^WHITE%^B_BLUE%^	<SPAN style="background-color: #0000bb; color: #ffffff">
%^WHITE%^%^BOLD%^	<SPAN style="color: #ffffff">
%^B_CYAN%^BLACK%^	<SPAN style="background-color: #00bbbb; color: #000000">
%^BLACK%^%^BOLD%^	<SPAN style="color: #555555">
%^BOLD%^MAGENTA%^	<SPAN style="color: #ff55ff">
%^FLASH%^%^CYAN%^	<SPAN style="color: #00bbbb">
%^MAGENTA%^BOLD%^	<SPAN style="color: #ff55ff">
%^BLUE%^B_WHITE%^	<SPAN style="background-color: #ffffff; color: #0000bb">
%^B_BLUE%^WHITE%^	<SPAN style="background-color: #0000bb; color: #ffffff">
%^GREEN%^%^BOLD%^	<SPAN style="color: #55ff55">
%^B_RED%^YELLOW%^	<SPAN style="background-color: #bb0000; color: #ffff55">
%^FLASH%^GREEN%^	<SPAN style="color: #55ff55">
%^B_CYAN%^BLUE%^	<SPAN style="background-color: #00bbbb; color: #0000bb">
%^BOLD%^%^CYAN%^	<SPAN style="color: #55ffff">
%^CYAN%^%^BOLD%^	<SPAN style="color: #55ffff">
%^GREEN%^B_RED%^	<SPAN style="background-color: #bb0000; color: #00ff00">
%^WHITE%^B_RED%^	<SPAN style="background-color: #bb0000; color: #ffffff">
%^BLUE%^%^BOLD%^	<SPAN style="color: #5555ff">
%^BLACK%^B_RED%^	<SPAN style="background-color: #bb0000; color: #000000">
%^BOLD%^YELLOW%^	<SPAN style="color: #ffff55">
%^LIGHTMAGENTA%^	<SPAN style="color: #ff55ff">
%^B_RED%^WHITE%^	<SPAN style="background-color: #bb0000; color: #ffffff">
%^B_RED%^BLACK%^	<SPAN style="background-color: #bb0000; color: #000000">
%^BOLD%^%^BLUE%^	<SPAN style="color: #5555ff">
%^B_RED%^GREEN%^	<SPAN style="background-color: #bb0000; color: #00ff00">
%^BLUE%^B_CYAN%^	<SPAN style="background-color: #00bbbb; color: #0000bb">
%^GREEN%^BOLD%^	<SPAN style="color: #55ff55">
%^WHITE%^BOLD%^	<SPAN style="color: #ffffff">
%^BOLD%^%^RED%^	<SPAN style="color: #ff5555">
%^BLACK%^BOLD%^	<SPAN style="color: #555555">
%^BOLD%^WHITE%^	<SPAN style="color: #ffffff">
%^RED%^%^BOLD%^	<SPAN style="color: #ff5555">
%^BOLD%^BLACK%^	<SPAN style="color: #555555">
%^FLASH%^CYAN%^	<SPAN style="color: #00bbbb">
%^BOLD%^GREEN%^	<SPAN style="color: #55ff55">
%^LIGHTGREEN%^	<SPAN style="color: #55ff55">
%^BOLD%^CYAN%^	<SPAN style="color: #55ffff">
%^BOLD%^BLUE%^	<SPAN style="color: #5555ff">
%^BLUE%^BOLD%^	<SPAN style="color: #5555ff">
%^CYAN%^BOLD%^	<SPAN style="color: #55ffff">
%^LIGHTBLUE%^	<SPAN style="color: #5555ff">
%^LIGHTCYAN%^	<SPAN style="color: #55ffff">
%^RED%^BOLD%^	<SPAN style="color: #ff5555">
%^BOLD%^RED%^	<SPAN style="color: #ff5555">
%^LIGHTRED%^	<SPAN style="color: #ff5555">
%^DARKGREY%^	<SPAN style="color: #555555">
%^MAGENTA%^	<SPAN style="color: #bb00bb">
%^ORANGE%^	<SPAN style="color: #bbbb00">
%^YELLOW%^	<SPAN style="color: #ffff55">
%^GREEN%^	<SPAN style="color: #00bb00">
%^BLACK%^	<SPAN style="color: #000000">
%^WHITE%^	<SPAN style="color: #bbbbbb">
%^GREY%^	<SPAN style="color: #bbbbbb">
%^PINK%^	<SPAN style="color: #ff55ff">
%^BLUE%^	<SPAN style="color: #0000bb">
%^CYAN%^	<SPAN style="color: #00bbbb">
%^RED%^	<SPAN style="color: #bb0000">
%^RESET%^	</SPAN>
%^B_RED%^	<SPAN style="background-color: #bb0000;">
%^B_CYAN%^	<SPAN style="background-color: #00bbbb;">
%^B_BLUE%^	<SPAN style="background-color: #0000bb;">
%^B_WHITE%^	<SPAN style="background-color: #bbbbbb;">
%^B_GREEN%^	<SPAN style="background-color: #00bb00;">
%^B_YELLOW%^	<SPAN style="background-color: #ffff55;">
%^B_ORANGE%^	<SPAN style="background-color: #bbbb00;">
%^B_MAGENTA%^	<SPAN style="background-color: #bb00bb;">
%^FLASH%^	<SPAN class="flash_tag">
%^BLACK%^%^B_ORANGE%^	<SPAN style="background-color: #bbbb00; color: #000000">
%^BLACK%^B_ORANGE%^	<SPAN style="background-color: #bbbb00; color: #000000">
%^B_ORANGE%^%^BLACK%^	<SPAN style="background-color: #bbbb00; color: #000000">
%^B_ORANGE%^BLACK%^	<SPAN style="background-color: #bbbb00; color: #000000">
%^WHITE%^%^B_ORANGE%^	<SPAN style="background-color: #bbbb00; color: #ffffff">
%^WHITE%^B_ORANGE%^	<SPAN style="background-color: #bbbb00; color: #ffffff">
%^B_ORANGE%^%^WHITE%^	<SPAN style="background-color: #bbbb00; color: #ffffff">
%^B_ORANGE%^WHITE%^	<SPAN style="background-color: #bbbb00; color: #ffffff">
%^BOLD%^	
\.


--
-- Data for Name: speakers; Type: TABLE DATA; Schema: public; Owner: -
--

COPY public.speakers (speaker, pinkfish) FROM stdin;
a brutal orc	%^BLACK%^%^B_WHITE%^
a bull	%^MAGENTA%^%^BOLD%^
a chicken	%^WHITE%^%^B_MAGENTA%^
a cow	%^BLACK%^%^B_GREEN%^
a gargantuan bird	%^CYAN%^%^BOLD%^
a grassland wolf	%^WHITE%^%^B_GREEN%^
a hat	%^BLACK%^%^B_YELLOW%^
a lamb	%^BLACK%^%^B_YELLOW%^
a mountain giant	%^BLACK%^%^B_WHITE%^
a nesting chicken	%^BLACK%^%^BOLD%^
a perched chicken	%^WHITE%^%^B_BLUE%^
a presence	%^BLACK%^%^B_CYAN%^
a rg us	%^BLACK%^%^B_MAGENTA%^
a shadow	%^BLACK%^%^B_YELLOW%^
a shadow(invis)	%^WHITE%^%^B_GREEN%^
a small rabbit	%^BLACK%^%^B_RED%^
a villager	%^CYAN%^
a wild fighter	%^WHITE%^%^B_BLUE%^
a zombie	%^BLACK%^%^B_GREEN%^
aaa	%^GREEN%^
aadin	%^BLACK%^%^B_GREEN%^
aagbb	%^WHITE%^%^B_GREEN%^
aammfdjagil	%^BLACK%^%^B_RED%^
aarewin	%^WHITE%^
aarinfel	%^WHITE%^%^B_MAGENTA%^
aaronr	%^RED%^
abhishek	%^RED%^
abigale	%^MAGENTA%^
abilfmekoj	%^GREEN%^
abinidi	%^BLACK%^%^B_WHITE%^
abjgmomijna	%^BLACK%^%^B_RED%^
abmhofjngh	%^YELLOW%^
abner	%^BLACK%^%^B_RED%^
abraxas	%^BLACK%^%^B_WHITE%^
abreu	%^YELLOW%^
abulia	%^BLACK%^%^B_YELLOW%^
acantha	%^GREEN%^%^BOLD%^
acerr	%^WHITE%^%^B_BLUE%^
acgmhgogi	%^BLACK%^%^B_MAGENTA%^
achille	%^BLACK%^%^B_GREEN%^
acoma tibet	%^BLACK%^%^B_GREEN%^
acomatibet	%^WHITE%^%^B_MAGENTA%^
actar airis	%^BLACK%^%^B_RED%^
adaephon	%^CYAN%^
adam	%^GREEN%^%^BOLD%^
adamish	%^CYAN%^%^BOLD%^
adgfngm	%^BLACK%^%^B_RED%^
admin	%^BLACK%^%^B_MAGENTA%^
administrator	%^BLACK%^%^B_WHITE%^
aduwen	%^CYAN%^
advant	%^WHITE%^%^B_MAGENTA%^
advtech	%^BLACK%^%^B_CYAN%^
ae	%^WHITE%^%^B_RED%^
aegir	%^WHITE%^%^B_RED%^
aegora	%^YELLOW%^
aell	%^BLUE%^
aelyah	%^WHITE%^
aeneas	%^WHITE%^%^B_BLUE%^
aerelus	%^BLACK%^%^B_RED%^
aeris	%^BLACK%^%^B_CYAN%^
aerlindir	%^GREEN%^%^BOLD%^
aero	%^WHITE%^%^B_BLUE%^
aeroc	%^BLUE%^%^BOLD%^
aesc	%^WHITE%^%^B_MAGENTA%^
aethion	%^CYAN%^
agathezol	%^BLACK%^%^BOLD%^
aghbenmdke	%^YELLOW%^
agrarian	%^YELLOW%^
agrippa	%^BLACK%^%^B_GREEN%^
ahimoth	%^BLACK%^%^B_YELLOW%^
ahti	%^RED%^
ahumado	%^WHITE%^
aidil	%^BLACK%^%^B_RED%^
aikanaro	%^RED%^
aim	%^WHITE%^%^B_GREEN%^
ainvar	%^BLACK%^%^B_MAGENTA%^
air	%^CYAN%^%^BOLD%^
airin	%^WHITE%^%^B_RED%^
airk	%^BLACK%^%^B_YELLOW%^
aisling	%^BLACK%^%^B_CYAN%^
aithne	%^BLUE%^%^BOLD%^
ajd	%^CYAN%^
ajhk	%^BLUE%^%^BOLD%^
akane	%^BLACK%^%^B_GREEN%^
akiinobu	%^ORANGE%^
akira	%^WHITE%^%^B_MAGENTA%^
akkjbk	%^RED%^%^BOLD%^
akva	%^YELLOW%^
al	%^WHITE%^%^B_RED%^
alaedra	%^WHITE%^%^B_GREEN%^
alan	%^BLACK%^%^B_GREEN%^
albiorix	%^BLACK%^%^BOLD%^
albus	%^WHITE%^%^B_GREEN%^
alecksy	%^WHITE%^%^BOLD%^
aleph	%^BLACK%^%^B_CYAN%^
aleric	%^WHITE%^%^B_BLUE%^
alerion	%^ORANGE%^
alessandra	%^BLUE%^
alex	%^WHITE%^
alexander	%^WHITE%^%^B_RED%^
alexi	%^CYAN%^%^BOLD%^
alexia	%^BLACK%^%^B_GREEN%^
alfe	%^GREEN%^%^BOLD%^
alien	%^BLACK%^%^B_CYAN%^
alixandra	%^GREEN%^%^BOLD%^
aljhhjhkim	%^WHITE%^%^B_MAGENTA%^
aljldjge	%^BLUE%^
alper	%^RED%^%^BOLD%^
alphoen	%^WHITE%^%^B_MAGENTA%^
alphonse	%^BLUE%^%^BOLD%^
alric	%^BLACK%^%^B_RED%^
alton	%^RED%^%^BOLD%^
alzghamael	%^BLUE%^%^BOLD%^
am	%^WHITE%^%^B_RED%^
amaaboaa	%^BLACK%^%^B_GREEN%^
amanda	%^WHITE%^%^BOLD%^
amaterasu	%^MAGENTA%^
ambros	%^BLUE%^%^BOLD%^
ambrosius	%^CYAN%^%^BOLD%^
amethyst	%^WHITE%^%^B_GREEN%^
amok	%^RED%^
amun-ra	%^CYAN%^
amylase	%^BLACK%^%^B_RED%^
amythist	%^RED%^%^BOLD%^
an acolyte	%^BLACK%^%^BOLD%^
an invisible immortal	%^CYAN%^%^BOLD%^
an ugly half-orc	%^BLUE%^
anaiah	%^GREEN%^%^BOLD%^
analjesus	%^WHITE%^%^B_MAGENTA%^
anamika	%^CYAN%^
anarto	%^BLUE%^
anastrad	%^CYAN%^
anaythea	%^BLACK%^%^B_YELLOW%^
andeddu	%^BLUE%^
andras	%^ORANGE%^
andress	%^BLACK%^%^B_MAGENTA%^
andrew	%^GREEN%^%^BOLD%^
andrewh	%^WHITE%^%^B_MAGENTA%^
andril	%^CYAN%^%^BOLD%^
andro	%^MAGENTA%^
andschana	%^ORANGE%^
anfauglir	%^GREEN%^
angel	%^WHITE%^
angela	%^GREEN%^%^BOLD%^
angelin	%^ORANGE%^
angette	%^GREEN%^
angus	%^BLACK%^%^B_MAGENTA%^
aniline	%^BLUE%^%^BOLD%^
animalia	%^BLACK%^%^BOLD%^
ankit	%^YELLOW%^
anna	%^MAGENTA%^
annabeth	%^BLACK%^%^B_GREEN%^
annihilator	%^BLACK%^%^B_WHITE%^
anomander	%^BLUE%^%^BOLD%^
anonymous	%^YELLOW%^
antisocial	%^CYAN%^%^BOLD%^
antoha	%^WHITE%^%^B_RED%^
anubis	%^BLACK%^%^B_MAGENTA%^
apache	%^RED%^%^BOLD%^
aphid	%^BLACK%^%^B_MAGENTA%^
aphrodite	%^RED%^
apollo	%^MAGENTA%^
apostle	%^MAGENTA%^
appelhof	%^BLUE%^%^BOLD%^
apresence	%^GREEN%^%^BOLD%^
aquilo	%^GREEN%^
araex	%^BLACK%^%^B_CYAN%^
aragon	%^BLACK%^%^B_YELLOW%^
aragorn	%^CYAN%^%^BOLD%^
araltaln	%^RED%^
aram	%^BLACK%^%^B_MAGENTA%^
aransa	%^MAGENTA%^
aransus	%^CYAN%^
arasiel	%^YELLOW%^
arbu	%^ORANGE%^
arcand	%^BLACK%^%^B_YELLOW%^
archaegeo	%^RED%^
archbaron	%^BLUE%^%^BOLD%^
archimedes	%^BLACK%^%^B_YELLOW%^
archon	%^GREEN%^
arde	%^CYAN%^
ardneh	%^GREEN%^%^BOLD%^
are	%^BLACK%^%^B_GREEN%^
ares	%^WHITE%^%^B_BLUE%^
aresian	%^RED%^
argh	%^YELLOW%^
argus	%^BLACK%^%^B_YELLOW%^
argyll	%^MAGENTA%^
arianna	%^ORANGE%^
arias	%^WHITE%^%^B_RED%^
arielle	%^BLACK%^%^B_GREEN%^
aries	%^GREEN%^%^BOLD%^
ariex	%^CYAN%^
arioch	%^RED%^%^BOLD%^
arisoth	%^GREEN%^%^BOLD%^
aristophanes	%^BLUE%^
arithorn	%^WHITE%^%^B_RED%^
arkon	%^BLACK%^%^B_CYAN%^
armadillo	%^YELLOW%^
armok	%^RED%^
armrha	%^MAGENTA%^%^BOLD%^
arnold	%^BLACK%^%^B_WHITE%^
arnvorax	%^CYAN%^
aron	%^WHITE%^
arren	%^BLUE%^%^BOLD%^
arruns	%^BLACK%^%^B_CYAN%^
artanis	%^BLACK%^%^B_MAGENTA%^
artemis	%^MAGENTA%^
arthmoor	%^ORANGE%^
arthur	%^MAGENTA%^
artymis	%^RED%^
arukor	%^CYAN%^%^BOLD%^
arwyn	%^BLACK%^%^B_YELLOW%^
aryabutta	%^BLACK%^%^B_GREEN%^
arynth	%^GREEN%^%^BOLD%^
as time winds itself to a standstill, shadyman	%^MAGENTA%^%^BOLD%^
asaph	%^BLACK%^%^B_CYAN%^
asgeroth	%^MAGENTA%^
asha	%^RED%^
asha*	%^CYAN%^
ashadow	%^GREEN%^%^BOLD%^
ashelyn	%^RED%^%^BOLD%^
asher	%^BLUE%^%^BOLD%^
ashi	%^WHITE%^%^B_RED%^
ashleep	%^MAGENTA%^%^BOLD%^
ashon	%^MAGENTA%^%^BOLD%^
ashron	%^BLACK%^%^B_GREEN%^
asilor	%^GREEN%^
asmodean	%^RED%^
asmodeo	%^WHITE%^%^B_MAGENTA%^
asmodeus	%^BLACK%^%^B_WHITE%^
asmoth	%^BLACK%^%^B_YELLOW%^
asp	%^WHITE%^%^B_BLUE%^
asparagus	%^ORANGE%^
assyri	%^CYAN%^
astep	%^RED%^%^BOLD%^
aster	%^GREEN%^
asteris	%^BLUE%^%^BOLD%^
astofingas	%^MAGENTA%^%^BOLD%^
astohamas	%^WHITE%^%^B_BLUE%^
astrum	%^BLACK%^%^B_YELLOW%^
asura	%^WHITE%^%^B_BLUE%^
aten	%^BLACK%^%^B_CYAN%^
athalaus	%^WHITE%^%^B_MAGENTA%^
athame	%^MAGENTA%^
athen	%^RED%^%^BOLD%^
athena	%^BLUE%^
athi	%^BLACK%^%^B_RED%^
atlordrith	%^WHITE%^%^BOLD%^
atomic	%^BLACK%^%^B_WHITE%^
atothek	%^BLACK%^%^B_CYAN%^
atricus	%^BLACK%^%^B_GREEN%^
atrus	%^BLACK%^%^B_WHITE%^
atticus	%^BLACK%^%^B_GREEN%^
augustus	%^BLACK%^%^B_CYAN%^
auran	%^WHITE%^%^B_GREEN%^
aureus	%^RED%^
avant	%^WHITE%^%^B_MAGENTA%^
avarice	%^MAGENTA%^
avarra	%^WHITE%^%^B_MAGENTA%^
avatar	%^WHITE%^%^B_BLUE%^
ave	%^BLACK%^%^B_GREEN%^
avenged	%^CYAN%^
averros	%^GREEN%^%^BOLD%^
avila	%^CYAN%^%^BOLD%^
axent	%^BLACK%^%^B_WHITE%^
ayala	%^BLACK%^%^B_WHITE%^
ayane	%^BLACK%^%^B_GREEN%^
ayesha	%^GREEN%^%^BOLD%^
aynin	%^BLACK%^%^B_GREEN%^
aysel swan	%^BLUE%^%^BOLD%^
azagur	%^WHITE%^%^B_MAGENTA%^
azash	%^BLACK%^%^B_GREEN%^
azathoth	%^GREEN%^%^BOLD%^
azazel	%^BLACK%^%^BOLD%^
azile	%^MAGENTA%^%^BOLD%^
aziz	%^BLACK%^%^B_GREEN%^
azoshin	%^WHITE%^
azru	%^WHITE%^
baal	%^MAGENTA%^%^BOLD%^
babycat	%^WHITE%^%^BOLD%^
bac	%^WHITE%^%^B_MAGENTA%^
bacchus	%^GREEN%^%^BOLD%^
back	%^BLACK%^%^B_RED%^
bah	%^MAGENTA%^%^BOLD%^
bailey	%^BLACK%^%^B_YELLOW%^
baiwuchang	%^MAGENTA%^%^BOLD%^
balev	%^MAGENTA%^
balmung	%^BLACK%^%^B_GREEN%^
balor	%^WHITE%^%^B_RED%^
balrog	%^CYAN%^%^BOLD%^
balthazar	%^WHITE%^%^B_RED%^
ban	%^YELLOW%^
bane	%^GREEN%^
banokles	%^WHITE%^%^BOLD%^
bardioc	%^BLACK%^%^B_MAGENTA%^
barek	%^CYAN%^%^BOLD%^
barnabus	%^WHITE%^%^B_BLUE%^
barrick	%^WHITE%^
barrien	%^WHITE%^%^BOLD%^
bartleby	%^BLACK%^%^B_RED%^
basso	%^YELLOW%^
bastetx	%^WHITE%^%^B_MAGENTA%^
bastian	%^BLACK%^%^B_CYAN%^
batastrophe	%^MAGENTA%^%^BOLD%^
batxi	%^WHITE%^%^B_MAGENTA%^
bayard	%^BLACK%^%^B_RED%^
bazain klzaid	%^MAGENTA%^
bazainklzaid	%^BLUE%^
bbc	%^GREEN%^%^BOLD%^
bbcscience	%^CYAN%^
bbcsport	%^BLACK%^%^B_YELLOW%^
bbctech	%^RED%^%^BOLD%^
bbigfglf	%^WHITE%^
bct	%^BLACK%^%^B_GREEN%^
beale	%^BLACK%^%^B_CYAN%^
bear	%^BLACK%^%^B_MAGENTA%^
becky	%^GREEN%^
bedcjkakdgg	%^CYAN%^%^BOLD%^
beefstick	%^WHITE%^%^B_BLUE%^
befjadan	%^BLACK%^%^B_CYAN%^
beldin	%^BLACK%^%^B_YELLOW%^
beleqwaya	%^BLACK%^%^BOLD%^
belias	%^BLACK%^%^B_RED%^
belron	%^BLACK%^%^B_WHITE%^
ben	%^BLUE%^
benabik	%^BLACK%^%^B_GREEN%^
benden	%^WHITE%^%^B_GREEN%^
benedick	%^BLACK%^%^B_CYAN%^
beohn	%^BLACK%^%^B_MAGENTA%^
beren	%^BLUE%^
berg	%^CYAN%^
berke	%^WHITE%^%^BOLD%^
berun	%^MAGENTA%^%^BOLD%^
bessus	%^BLACK%^%^B_MAGENTA%^
betta	%^GREEN%^
beyonder	%^MAGENTA%^
bgamlk	%^RED%^
bgkmbf	%^RED%^%^BOLD%^
bgknji	%^WHITE%^%^B_BLUE%^
bigb	%^MAGENTA%^
bigd	%^BLACK%^%^B_MAGENTA%^
biggus dickus	%^MAGENTA%^%^BOLD%^
biggusdickus	%^MAGENTA%^%^BOLD%^
bilar	%^WHITE%^%^BOLD%^
billy	%^BLACK%^%^B_MAGENTA%^
bimbletrot	%^GREEN%^
bitch-hitter	%^BLACK%^%^B_MAGENTA%^
bixby	%^CYAN%^%^BOLD%^
bjcnh	%^BLACK%^%^B_GREEN%^
blackfoot	%^BLACK%^%^B_WHITE%^
blackjack	%^ORANGE%^
blackraven	%^BLACK%^%^B_WHITE%^
blain	%^WHITE%^%^BOLD%^
blake	%^MAGENTA%^%^BOLD%^
blargh	%^RED%^%^BOLD%^
blarghy	%^WHITE%^%^B_RED%^
blaze	%^BLACK%^%^B_MAGENTA%^
blink	%^BLACK%^%^BOLD%^
blitz	%^WHITE%^%^B_BLUE%^
blitzkrieg	%^GREEN%^%^BOLD%^
blood wolf	%^MAGENTA%^
blue	%^GREEN%^%^BOLD%^
bluewolf	%^ORANGE%^
bluius	%^BLACK%^%^B_MAGENTA%^
bmgl	%^WHITE%^%^B_RED%^
bncd	%^BLACK%^%^B_CYAN%^
bob	%^WHITE%^%^BOLD%^
bobby	%^MAGENTA%^
bodea	%^CYAN%^
boga	%^BLACK%^%^BOLD%^
bones	%^BLACK%^%^B_GREEN%^
bonobo	%^BLACK%^%^BOLD%^
bonta	%^BLACK%^%^B_WHITE%^
boogeyman	%^BLACK%^%^B_GREEN%^
boot	%^WHITE%^
boraleas	%^CYAN%^
borat	%^BLUE%^
borealis	%^GREEN%^%^BOLD%^
borsch	%^MAGENTA%^%^BOLD%^
borsch gorval	%^BLACK%^%^B_WHITE%^
bossman	%^BLACK%^%^B_RED%^
boyo	%^CYAN%^%^BOLD%^
brad	%^WHITE%^%^B_BLUE%^
brahmus	%^BLACK%^%^B_WHITE%^
brain	%^WHITE%^
brainstorm	%^WHITE%^%^B_GREEN%^
brak	%^YELLOW%^
branded	%^GREEN%^
brandon	%^YELLOW%^
branitar	%^BLACK%^%^B_MAGENTA%^
brashen	%^BLACK%^%^B_CYAN%^
brawndel	%^WHITE%^%^B_RED%^
braxus	%^WHITE%^%^B_MAGENTA%^
breezy	%^WHITE%^%^B_GREEN%^
brent	%^YELLOW%^
brian	%^WHITE%^
brisko	%^BLUE%^%^BOLD%^
brodbane	%^BLACK%^%^B_CYAN%^
brog	%^BLACK%^%^B_GREEN%^
brogon	%^CYAN%^
bruc	%^RED%^%^BOLD%^
brutus	%^WHITE%^%^BOLD%^
bryanbkm	%^CYAN%^%^BOLD%^
bryantos	%^GREEN%^%^BOLD%^
bubba	%^WHITE%^
bula	%^WHITE%^%^B_MAGENTA%^
bunabiros	%^RED%^%^BOLD%^
bunny	%^RED%^%^BOLD%^
bunnywolf	%^BLACK%^%^B_GREEN%^
cab	%^GREEN%^
caelreth	%^WHITE%^%^B_GREEN%^
cain	%^GREEN%^
caitlynn	%^BLUE%^%^BOLD%^
caldor	%^BLUE%^
calis	%^GREEN%^%^BOLD%^
calroth	%^WHITE%^%^B_RED%^
calvin	%^WHITE%^%^B_BLUE%^
cam	%^BLACK%^%^BOLD%^
caminus	%^CYAN%^%^BOLD%^
camlorn	%^ORANGE%^
capita	%^BLUE%^%^BOLD%^
capo	%^BLACK%^%^B_CYAN%^
capricorn	%^BLACK%^%^B_YELLOW%^
carazz	%^BLUE%^%^BOLD%^
carcer	%^BLUE%^
carino	%^YELLOW%^
carl	%^WHITE%^%^B_GREEN%^
carmine	%^WHITE%^%^B_MAGENTA%^
carnifex	%^YELLOW%^
carter	%^WHITE%^%^B_GREEN%^
casey	%^GREEN%^
cavrone	%^RED%^%^BOLD%^
caya	%^WHITE%^%^B_MAGENTA%^
cbaker	%^BLACK%^%^B_CYAN%^
cc	%^WHITE%^
cclbiehlim	%^MAGENTA%^
cecil	%^RED%^%^BOLD%^
ceetar	%^BLACK%^%^B_RED%^
celeo	%^WHITE%^
celestial	%^GREEN%^%^BOLD%^
celoril	%^WHITE%^%^B_BLUE%^
celt	%^RED%^
celthric	%^ORANGE%^
cenn	%^BLACK%^%^BOLD%^
centollo	%^RED%^
centuari	%^BLACK%^%^B_GREEN%^
ceres	%^BLACK%^%^B_GREEN%^
cerihan	%^MAGENTA%^%^BOLD%^
cerkres	%^RED%^
chagrin	%^WHITE%^
chainsol	%^CYAN%^
chanemiemm	%^BLUE%^
charchess	%^YELLOW%^
charity	%^RED%^%^BOLD%^
charles	%^WHITE%^%^B_MAGENTA%^
charon	%^BLUE%^
chat	%^ORANGE%^
chat_d	%^BLACK%^%^B_GREEN%^
cheeky	%^BLACK%^%^B_YELLOW%^
cheeky*	%^RED%^%^BOLD%^
cheeseball	%^WHITE%^
chef	%^BLACK%^%^B_WHITE%^
chemizt	%^RED%^
chet	%^WHITE%^
cheyne	%^YELLOW%^
chil	%^BLACK%^%^B_WHITE%^
chilalin	%^YELLOW%^
chimara	%^CYAN%^%^BOLD%^
chinaski	%^BLACK%^%^BOLD%^
chislev	%^CYAN%^%^BOLD%^
choddesh	%^BLACK%^%^BOLD%^
chosen	%^WHITE%^%^BOLD%^
chowmein	%^ORANGE%^
chris	%^GREEN%^
chris bailey	%^RED%^
chrisnan	%^MAGENTA%^
chromaldt	%^WHITE%^%^B_RED%^
chronos	%^CYAN%^
chuck	%^BLACK%^%^B_CYAN%^
cicy	%^BLUE%^%^BOLD%^
cin	%^WHITE%^%^BOLD%^
cinaeeddnol	%^BLUE%^%^BOLD%^
cindel	%^CYAN%^
cinnamon	%^WHITE%^%^BOLD%^
ciras	%^BLACK%^%^B_MAGENTA%^
cirdan	%^BLUE%^%^BOLD%^
cize	%^WHITE%^%^BOLD%^
cjcgafg	%^BLACK%^%^B_RED%^
clanger	%^WHITE%^%^BOLD%^
claudius	%^MAGENTA%^%^BOLD%^
clayton	%^CYAN%^%^BOLD%^
clerk	%^BLACK%^%^B_CYAN%^
clicken	%^GREEN%^
clint	%^GREEN%^
cobalt	%^MAGENTA%^%^BOLD%^
cochise	%^WHITE%^
code	%^WHITE%^%^BOLD%^
cody	%^CYAN%^%^BOLD%^
coffeebreak	%^BLACK%^%^B_RED%^
coffeepot	%^BLACK%^%^B_YELLOW%^
coffeymug	%^BLACK%^%^BOLD%^
colone	%^BLACK%^%^B_CYAN%^
communist	%^WHITE%^%^B_RED%^
conan	%^BLACK%^%^B_MAGENTA%^
confuto	%^BLACK%^%^B_RED%^
connick	%^WHITE%^%^B_BLUE%^
constructus	%^CYAN%^%^BOLD%^
coogan	%^BLUE%^
cordwangle	%^MAGENTA%^
corey	%^BLACK%^%^B_RED%^
corrik	%^BLACK%^%^B_GREEN%^
corsair	%^BLACK%^%^B_RED%^
cortest	%^GREEN%^%^BOLD%^
corvin	%^WHITE%^
cotoharla	%^BLACK%^%^B_RED%^
courie	%^BLACK%^%^BOLD%^
coy	%^CYAN%^%^BOLD%^
coyote	%^BLACK%^%^B_YELLOW%^
cozminsky	%^ORANGE%^
craig	%^YELLOW%^
craigifer	%^WHITE%^%^B_GREEN%^
crakadmin	%^GREEN%^%^BOLD%^
crash	%^BLACK%^%^B_CYAN%^
crashbot	%^CYAN%^%^BOLD%^
cratylus	%^BLACK%^%^B_RED%^
crayn	%^WHITE%^%^BOLD%^
crayron	%^WHITE%^
crazyweasl	%^GREEN%^%^BOLD%^
crickets	%^BLACK%^%^B_RED%^
crilly	%^BLACK%^%^B_MAGENTA%^
crisis	%^BLACK%^%^BOLD%^
crism	%^MAGENTA%^
cromulent quixadhal	%^RED%^
cron	%^BLACK%^%^BOLD%^
cronus	%^RED%^
crowbar	%^WHITE%^%^B_RED%^
cruentus	%^WHITE%^%^B_GREEN%^
crunch	%^CYAN%^
cruxis	%^WHITE%^%^BOLD%^
cthulhu	%^BLACK%^%^B_GREEN%^
culex	%^MAGENTA%^%^BOLD%^
cuzz	%^WHITE%^%^B_GREEN%^
cyberbob	%^BLUE%^%^BOLD%^
cybertiger	%^GREEN%^%^BOLD%^
cyix	%^BLACK%^%^B_MAGENTA%^
cynique	%^BLACK%^%^BOLD%^
cyric	%^ORANGE%^
cyric the bloody eyed	%^BLACK%^%^B_MAGENTA%^
cyteo	%^WHITE%^%^B_GREEN%^
cyto	%^BLUE%^%^BOLD%^
czyrxis	%^WHITE%^
daburgermeista	%^MAGENTA%^%^BOLD%^
dacrian	%^ORANGE%^
daddy	%^GREEN%^
daelas	%^BLUE%^
daem	%^CYAN%^
daffy	%^RED%^%^BOLD%^
daggerwind	%^BLACK%^%^B_YELLOW%^
daglo	%^WHITE%^%^B_MAGENTA%^
dagorsul	%^YELLOW%^
daihsan	%^RED%^%^BOLD%^
dain	%^WHITE%^
dainnil	%^WHITE%^%^B_RED%^
daius	%^WHITE%^%^B_BLUE%^
dakra	%^MAGENTA%^%^BOLD%^
dalmuros	%^BLACK%^%^B_MAGENTA%^
dalsor	%^BLUE%^%^BOLD%^
dameon	%^ORANGE%^
damia	%^BLUE%^%^BOLD%^
damian	%^BLACK%^%^B_MAGENTA%^
damien	%^BLACK%^%^B_WHITE%^
damouze	%^WHITE%^%^BOLD%^
dan	%^CYAN%^%^BOLD%^
danaer	%^WHITE%^
danatos	%^WHITE%^
danduin	%^BLUE%^%^BOLD%^
daneel	%^BLACK%^%^B_YELLOW%^
daniel	%^CYAN%^%^BOLD%^
daos	%^WHITE%^%^B_MAGENTA%^
dara	%^RED%^%^BOLD%^
darclas	%^WHITE%^%^BOLD%^
darian	%^BLACK%^%^B_RED%^
dark rider	%^RED%^
darkangel	%^BLACK%^%^B_MAGENTA%^
darkarn	%^RED%^
darkbane	%^BLACK%^%^B_MAGENTA%^
darkchaos	%^ORANGE%^
darken	%^WHITE%^%^BOLD%^
darkhelmet	%^RED%^
darkman	%^RED%^
darknaj	%^BLACK%^%^BOLD%^
darkstar	%^BLACK%^%^B_MAGENTA%^
darkwalker	%^BLACK%^%^B_GREEN%^
darkytoo	%^GREEN%^%^BOLD%^
darloth	%^RED%^%^BOLD%^
darneth	%^BLACK%^%^BOLD%^
darrant	%^ORANGE%^
darren	%^WHITE%^%^BOLD%^
dartal	%^WHITE%^
dartonion	%^CYAN%^
dartos	%^BLACK%^%^B_YELLOW%^
dasquian	%^WHITE%^%^B_RED%^
dastuun	%^BLUE%^
dave	%^RED%^
davian	%^BLACK%^%^BOLD%^
davidhaley	%^BLACK%^%^B_YELLOW%^
davidian	%^WHITE%^
davion	%^GREEN%^%^BOLD%^
davroar	%^GREEN%^%^BOLD%^
davy jones	%^GREEN%^
davyjones	%^RED%^
dax	%^BLUE%^%^BOLD%^
daxhall	%^BLACK%^%^B_WHITE%^
dayian	%^BLACK%^%^B_RED%^
dd	%^CYAN%^%^BOLD%^
ddaickfo	%^BLACK%^%^B_MAGENTA%^
ddread quixadhal	%^RED%^
deadman	%^BLACK%^%^B_RED%^
deakan	%^BLACK%^%^BOLD%^
death	%^WHITE%^%^B_BLUE%^
deathknight	%^BLACK%^%^BOLD%^
deb	%^BLACK%^%^B_WHITE%^
debug	%^YELLOW%^
deckard	%^GREEN%^
dedalus	%^BLUE%^
deepspawn	%^CYAN%^%^BOLD%^
default	%^WHITE%^
deforce	%^YELLOW%^
deklax	%^BLACK%^%^B_WHITE%^
delojo	%^BLACK%^%^BOLD%^
delphi	%^YELLOW%^
deltreey	%^RED%^
dementia	%^BLACK%^%^B_WHITE%^
demetri	%^RED%^%^BOLD%^
demitris	%^BLACK%^%^B_WHITE%^
demona	%^ORANGE%^
demonic	%^BLACK%^%^B_WHITE%^
densux	%^MAGENTA%^%^BOLD%^
denzil	%^WHITE%^%^B_MAGENTA%^
dergrinch	%^GREEN%^
derision	%^BLACK%^%^B_GREEN%^
derpy	%^GREEN%^%^BOLD%^
deruk	%^BLACK%^%^B_WHITE%^
des	%^BLACK%^%^B_MAGENTA%^
descartes	%^BLACK%^%^B_MAGENTA%^
deschain	%^WHITE%^%^B_BLUE%^
designa	%^CYAN%^
detah	%^ORANGE%^
deus	%^WHITE%^%^B_BLUE%^
devet	%^BLACK%^%^B_GREEN%^
devo	%^WHITE%^%^B_MAGENTA%^
devonin	%^WHITE%^%^B_MAGENTA%^
dextar	%^ORANGE%^
dexter	%^YELLOW%^
dextrose	%^CYAN%^
dgleks	%^CYAN%^
dhaos	%^WHITE%^%^B_GREEN%^
dhavid	%^BLACK%^%^BOLD%^
dheianarah	%^BLUE%^
dheroan	%^BLACK%^%^B_WHITE%^
dhruv	%^CYAN%^
diablo	%^WHITE%^%^B_RED%^
diaden	%^WHITE%^%^B_BLUE%^
dias	%^BLACK%^%^B_WHITE%^
dibcdhlbona	%^WHITE%^%^BOLD%^
diggle	%^GREEN%^%^BOLD%^
dillon	%^GREEN%^
din	%^GREEN%^
dinchak	%^GREEN%^
dinin	%^BLACK%^%^B_CYAN%^
dior	%^BLUE%^
directrix	%^BLACK%^%^BOLD%^
dirt	%^WHITE%^%^B_GREEN%^
diruces	%^RED%^
disaster	%^CYAN%^
discworks	%^GREEN%^%^BOLD%^
diskus	%^BLACK%^%^B_WHITE%^
dizz-e	%^CYAN%^
django	%^BLACK%^%^B_YELLOW%^
djin	%^BLACK%^%^B_WHITE%^
djoan	%^WHITE%^%^B_MAGENTA%^
dklcaoaa	%^WHITE%^%^B_GREEN%^
dlloyd	%^WHITE%^%^B_GREEN%^
dm	%^BLACK%^%^B_WHITE%^
dmiedm	%^ORANGE%^
dobbin	%^CYAN%^
doc	%^GREEN%^%^BOLD%^
doctore	%^RED%^%^BOLD%^
dodo	%^MAGENTA%^%^BOLD%^
dogbolter	%^BLACK%^%^B_GREEN%^
dogsbody	%^MAGENTA%^
dolk	%^WHITE%^%^B_RED%^
domino	%^WHITE%^%^BOLD%^
dommg	%^BLACK%^%^B_YELLOW%^
don	%^BLACK%^%^B_CYAN%^
donk	%^WHITE%^%^BOLD%^
donky	%^YELLOW%^
donnel	%^BLACK%^%^B_YELLOW%^
donpon	%^CYAN%^
doodoobumface	%^BLACK%^%^B_YELLOW%^
doris	%^GREEN%^%^BOLD%^
douhnin	%^MAGENTA%^%^BOLD%^
doulos	%^GREEN%^%^BOLD%^
dowaito	%^MAGENTA%^%^BOLD%^
dozy	%^WHITE%^%^B_RED%^
dracmas	%^BLACK%^%^B_RED%^
draconius	%^CYAN%^%^BOLD%^
draeand	%^YELLOW%^
drakkos	%^BLUE%^%^BOLD%^
drassk	%^GREEN%^
draun	%^YELLOW%^
dravon	%^BLUE%^%^BOLD%^
dread lord quixadhal	%^RED%^
dread quixadhal	%^RED%^
dreamweaver	%^WHITE%^%^B_MAGENTA%^
drekion	%^YELLOW%^
drenon	%^RED%^%^BOLD%^
drifter	%^YELLOW%^
drixaen	%^RED%^%^BOLD%^
droid	%^YELLOW%^
dryade	%^RED%^%^BOLD%^
duck	%^WHITE%^%^B_MAGENTA%^
dummius	%^YELLOW%^
dunkelheit	%^CYAN%^
durak	%^MAGENTA%^
durandal	%^BLACK%^%^B_RED%^
duriel	%^MAGENTA%^
dusk	%^WHITE%^%^B_MAGENTA%^
dusk's ghost	%^BLACK%^%^B_RED%^
duuk	%^ORANGE%^
dux syphiral	%^WHITE%^%^B_MAGENTA%^
dvinn	%^BLACK%^%^B_RED%^
dvykian	%^BLACK%^%^B_RED%^
dwarian	%^BLACK%^%^B_CYAN%^
dworkin	%^YELLOW%^
dyorion	%^GREEN%^%^BOLD%^
dzsoker	%^WHITE%^
ea	%^BLACK%^%^B_YELLOW%^
earin	%^WHITE%^%^B_MAGENTA%^
ebersar	%^WHITE%^%^B_GREEN%^
ecaeldlm	%^BLACK%^%^BOLD%^
ecb	%^BLACK%^%^BOLD%^
echlori	%^BLACK%^%^B_MAGENTA%^
echnos	%^ORANGE%^
echo	%^WHITE%^%^B_MAGENTA%^
eclipse	%^CYAN%^%^BOLD%^
ecru	%^BLACK%^%^B_YELLOW%^
ecthelion	%^MAGENTA%^%^BOLD%^
eddard	%^MAGENTA%^
edesar	%^BLACK%^%^B_RED%^
edge	%^WHITE%^%^B_BLUE%^
eeeg	%^CYAN%^
eek	%^BLACK%^%^B_WHITE%^
eerr	%^BLUE%^%^BOLD%^
egfmghdjn	%^GREEN%^
egnis	%^BLACK%^%^B_RED%^
eh	%^ORANGE%^
ehlana	%^WHITE%^%^B_RED%^
ejibke	%^CYAN%^%^BOLD%^
ekcahf	%^MAGENTA%^
ekoygugu	%^WHITE%^%^B_BLUE%^
el	%^BLACK%^%^B_YELLOW%^
eladriell	%^CYAN%^%^BOLD%^
elaine	%^ORANGE%^
elaine benes	%^MAGENTA%^
elanor	%^BLACK%^%^B_RED%^
elastica	%^MAGENTA%^
elbereth	%^RED%^
eldermoor	%^MAGENTA%^
elera	%^CYAN%^%^BOLD%^
elfindrol	%^BLACK%^%^BOLD%^
eli	%^BLUE%^
eli the priest	%^GREEN%^%^BOLD%^
eli*	%^MAGENTA%^
elise	%^BLACK%^%^B_YELLOW%^
elitter	%^ORANGE%^
elixir	%^BLACK%^%^B_CYAN%^
eliza	%^BLUE%^
elo'him	%^WHITE%^%^B_RED%^
elohim	%^WHITE%^%^BOLD%^
elomis	%^BLACK%^%^B_MAGENTA%^
elora	%^MAGENTA%^
elorf	%^BLACK%^%^B_RED%^
elroy	%^MAGENTA%^%^BOLD%^
elsha	%^WHITE%^%^B_BLUE%^
elspeth	%^BLUE%^%^BOLD%^
eltari	%^WHITE%^%^B_BLUE%^
elysium	%^BLUE%^
ember	%^RED%^
emes	%^BLACK%^%^B_CYAN%^
emiko	%^WHITE%^%^B_GREEN%^
emily	%^MAGENTA%^%^BOLD%^
emkjdfgeng	%^WHITE%^%^B_GREEN%^
emknaps	%^WHITE%^%^B_GREEN%^
emma	%^RED%^
emn	%^BLACK%^%^B_WHITE%^
emporer mamious	%^MAGENTA%^%^BOLD%^
emporer manious	%^BLUE%^%^BOLD%^
emraef	%^BLUE%^%^BOLD%^
emsenn	%^BLUE%^
enathep	%^WHITE%^%^B_BLUE%^
encryption	%^BLACK%^%^B_WHITE%^
engywuck	%^MAGENTA%^
enigma	%^BLACK%^%^B_GREEN%^
enkidu	%^ORANGE%^
enkil	%^BLUE%^%^BOLD%^
enkill	%^BLUE%^
ennis	%^MAGENTA%^%^BOLD%^
enraged, elastica	%^YELLOW%^
enter	%^GREEN%^
eoedmfl	%^BLUE%^
eonwe	%^CYAN%^%^BOLD%^
epic	%^RED%^
epitaph	%^CYAN%^
epoch	%^CYAN%^%^BOLD%^
eprius	%^WHITE%^
eral	%^YELLOW%^
erebos	%^WHITE%^%^B_RED%^
erebus	%^MAGENTA%^%^BOLD%^
erganil	%^CYAN%^
eric	%^BLUE%^
eriol	%^BLACK%^%^B_CYAN%^
erion	%^RED%^%^BOLD%^
eris	%^MAGENTA%^
ermintrude	%^WHITE%^%^B_GREEN%^
ero	%^WHITE%^%^B_GREEN%^
errdegahr	%^WHITE%^%^B_BLUE%^
eru	%^ORANGE%^
espn	%^WHITE%^%^B_RED%^
essense	%^BLUE%^
estinevial	%^WHITE%^%^B_MAGENTA%^
estoufee	%^CYAN%^
ethan	%^RED%^
ethin	%^WHITE%^%^BOLD%^
eugenides	%^MAGENTA%^
eugine	%^MAGENTA%^%^BOLD%^
evagrius	%^GREEN%^
evan	%^WHITE%^%^B_BLUE%^
evensong	%^BLUE%^%^BOLD%^
evhor	%^BLUE%^%^BOLD%^
evil mog	%^RED%^%^BOLD%^
evilmog	%^CYAN%^
evis	%^MAGENTA%^
eviscerator	%^RED%^%^BOLD%^
ewt	%^BLUE%^
exodia	%^BLACK%^%^B_MAGENTA%^
exor	%^BLACK%^%^B_GREEN%^
exorn	%^WHITE%^%^B_GREEN%^
exote	%^CYAN%^%^BOLD%^
expresso	%^BLACK%^%^BOLD%^
eycibiem	%^WHITE%^%^B_BLUE%^
eyekiller	%^WHITE%^%^B_MAGENTA%^
ezaco	%^WHITE%^%^B_RED%^
ezekiel	%^BLACK%^%^B_YELLOW%^
fabulous	%^RED%^
fado	%^ORANGE%^
fafdihancog	%^WHITE%^
faggot	%^BLACK%^%^B_WHITE%^
falerin	%^WHITE%^
famp	%^BLUE%^%^BOLD%^
fantom	%^BLACK%^%^B_YELLOW%^
fastfinge	%^MAGENTA%^%^BOLD%^
faye	%^BLUE%^%^BOLD%^
faylen	%^WHITE%^
fayte	%^BLACK%^%^B_YELLOW%^
feantur	%^BLACK%^%^B_YELLOW%^
feef	%^WHITE%^
feldegast	%^BLACK%^%^BOLD%^
felice	%^ORANGE%^
felix	%^WHITE%^%^BOLD%^
fewms	%^YELLOW%^
fiasco	%^BLACK%^%^B_YELLOW%^
fille	%^RED%^%^BOLD%^
fin	%^MAGENTA%^%^BOLD%^
fiona	%^BLUE%^
fireblade	%^WHITE%^%^B_MAGENTA%^
firefly	%^BLACK%^%^BOLD%^
firestarman	%^MAGENTA%^%^BOLD%^
firican	%^YELLOW%^
fishie	%^BLACK%^%^BOLD%^
five	%^WHITE%^%^BOLD%^
fizban	%^BLACK%^%^B_YELLOW%^
fjga	%^WHITE%^%^B_BLUE%^
flak	%^BLACK%^%^BOLD%^
flammweiss	%^BLACK%^%^BOLD%^
flamore	%^YELLOW%^
flaterectomy	%^BLACK%^%^B_GREEN%^
fleacircus	%^BLACK%^%^B_CYAN%^
flembobs	%^ORANGE%^
flexure	%^BLACK%^%^B_YELLOW%^
flip	%^YELLOW%^
floor	%^BLACK%^%^B_MAGENTA%^
flumpy	%^BLUE%^%^BOLD%^
flurble	%^BLUE%^
flyboy	%^BLACK%^%^B_MAGENTA%^
fmgmdljkj	%^WHITE%^
foaly	%^RED%^
foh	%^WHITE%^%^BOLD%^
fokke	%^WHITE%^%^B_RED%^
fol	%^CYAN%^
foo	%^BLACK%^%^B_WHITE%^
fooboy	%^RED%^%^BOLD%^
forcestar	%^RED%^%^BOLD%^
forlan	%^WHITE%^
fortunado	%^RED%^
fortune	%^BLUE%^%^BOLD%^
four	%^RED%^%^BOLD%^
fox	%^RED%^%^BOLD%^
foxer	%^BLUE%^%^BOLD%^
foxnews	%^BLACK%^%^B_WHITE%^
foxworth	%^BLACK%^%^B_GREEN%^
fozzie	%^BLACK%^%^B_WHITE%^
frazyl	%^WHITE%^%^B_GREEN%^
freddy	%^WHITE%^%^B_BLUE%^
frey	%^WHITE%^
freyalise	%^BLUE%^%^BOLD%^
fritos	%^BLACK%^%^B_MAGENTA%^
frobozz	%^MAGENTA%^%^BOLD%^
frog	%^MAGENTA%^
frogleap	%^MAGENTA%^
frutsel	%^BLACK%^%^B_YELLOW%^
ftang	%^RED%^
fthmdg	%^WHITE%^%^B_BLUE%^
fubar	%^BLACK%^%^BOLD%^
fuchur	%^YELLOW%^
fumanchu	%^WHITE%^%^B_RED%^
function	%^BLACK%^%^B_WHITE%^
fury	%^BLUE%^
futility	%^YELLOW%^
fuzz	%^RED%^
fyda	%^RED%^%^BOLD%^
fyrwen	%^MAGENTA%^%^BOLD%^
gabe	%^BLACK%^%^BOLD%^
gaborn	%^WHITE%^%^B_RED%^
gabriel	%^BLACK%^%^B_YELLOW%^
gaebfok	%^BLACK%^%^B_GREEN%^
gaelen	%^BLACK%^%^B_GREEN%^
gafney	%^YELLOW%^
gaidin	%^BLACK%^%^B_GREEN%^
gakbkii	%^MAGENTA%^
galawin	%^YELLOW%^
galbraith	%^GREEN%^
galderas	%^BLACK%^%^B_RED%^
gambol	%^WHITE%^%^B_GREEN%^
gammler	%^WHITE%^%^B_RED%^
gammon	%^YELLOW%^
gan	%^CYAN%^
gand	%^BLACK%^%^B_CYAN%^
gandalf	%^YELLOW%^
gandalph	%^RED%^%^BOLD%^
gaokaafkd	%^RED%^%^BOLD%^
garett	%^BLACK%^%^B_GREEN%^
garland	%^BLACK%^%^B_RED%^
garrett	%^YELLOW%^
garrion	%^WHITE%^%^B_BLUE%^
gatz	%^BLUE%^
gavin	%^MAGENTA%^
gawain	%^GREEN%^%^BOLD%^
gaz	%^YELLOW%^
gbdela	%^RED%^
gbifboclido	%^GREEN%^%^BOLD%^
gdabmbkh	%^WHITE%^%^B_BLUE%^
gdh	%^WHITE%^%^B_MAGENTA%^
geekman	%^WHITE%^%^B_MAGENTA%^
gellan	%^CYAN%^%^BOLD%^
genericadmin	%^BLUE%^
gentry	%^WHITE%^%^BOLD%^
george	%^WHITE%^%^B_RED%^
gerkstrom	%^BLACK%^%^B_MAGENTA%^
geryon	%^MAGENTA%^
gesph	%^BLACK%^%^B_CYAN%^
ghidorah	%^MAGENTA%^
ghost	%^RED%^
ghost of quixadhal	%^RED%^
gicker	%^CYAN%^%^BOLD%^
gidget	%^BLACK%^%^B_RED%^
gidsey	%^BLACK%^%^B_RED%^
giggles	%^MAGENTA%^
gilaed	%^WHITE%^%^B_BLUE%^
gimlet	%^WHITE%^%^B_MAGENTA%^
gin	%^BLACK%^%^B_WHITE%^
giric	%^BLACK%^%^B_CYAN%^
girkan	%^WHITE%^%^B_RED%^
github	%^ORANGE%^
gix	%^BLACK%^%^B_YELLOW%^
gkdoifgffnl	%^CYAN%^
glalofkfeh	%^GREEN%^
glister	%^WHITE%^%^B_BLUE%^
glyvmort	%^WHITE%^%^B_RED%^
gnash	%^WHITE%^%^B_GREEN%^
gncfc	%^WHITE%^%^BOLD%^
gnews	%^CYAN%^%^BOLD%^
gnillot	%^WHITE%^%^B_BLUE%^
god	%^BLACK%^%^B_YELLOW%^
godhand	%^ORANGE%^
gongoliar	%^BLACK%^%^B_RED%^
goober	%^GREEN%^
goose	%^WHITE%^%^BOLD%^
gorm	%^RED%^
gornac	%^BLACK%^%^B_RED%^
gorto	%^WHITE%^%^B_GREEN%^
gothmasterflash	%^BLUE%^%^BOLD%^
gouzz	%^BLACK%^%^B_RED%^
grandfather	%^MAGENTA%^
grannymi	%^YELLOW%^
grannymii	%^BLACK%^%^B_CYAN%^
grave	%^BLUE%^%^BOLD%^
graveluth	%^YELLOW%^
gravior	%^BLACK%^%^BOLD%^
gravity	%^CYAN%^
greatland	%^BLACK%^%^B_GREEN%^
grek	%^BLACK%^%^BOLD%^
grendel	%^BLACK%^%^B_RED%^
grey	%^BLACK%^%^B_RED%^
gribbles	%^BLACK%^%^B_GREEN%^
griffen	%^WHITE%^%^BOLD%^
grimrak	%^BLACK%^%^B_GREEN%^
grisheen	%^ORANGE%^
grishna	%^RED%^%^BOLD%^
grissom	%^BLACK%^%^BOLD%^
grommet	%^WHITE%^%^B_RED%^
gronk	%^YELLOW%^
groo	%^WHITE%^%^BOLD%^
grr	%^ORANGE%^
gruad	%^MAGENTA%^%^BOLD%^
grukar	%^RED%^%^BOLD%^
guest	%^CYAN%^
guest1	%^RED%^
guff	%^GREEN%^%^BOLD%^
guido	%^WHITE%^%^B_RED%^
guiver	%^WHITE%^%^BOLD%^
gungho	%^WHITE%^%^BOLD%^
gushi	%^BLACK%^%^B_RED%^
gustav	%^BLUE%^
gwyndrith	%^WHITE%^%^BOLD%^
gypsie	%^BLACK%^%^BOLD%^
gyre	%^BLACK%^%^B_WHITE%^
habngffib	%^YELLOW%^
haderach	%^BLACK%^%^B_GREEN%^
hades	%^CYAN%^%^BOLD%^
hafbkfgkhb	%^WHITE%^%^BOLD%^
haggis	%^WHITE%^%^B_RED%^
hagi	%^GREEN%^
hair	%^ORANGE%^
hairam	%^CYAN%^%^BOLD%^
halayko	%^BLUE%^
halcyon	%^WHITE%^
haldir	%^BLUE%^
halodge	%^WHITE%^%^B_BLUE%^
hambone	%^RED%^%^BOLD%^
hambone*	%^GREEN%^%^BOLD%^
hamish	%^MAGENTA%^%^BOLD%^
hamlet	%^WHITE%^%^B_RED%^
hannazus	%^BLACK%^%^B_CYAN%^
hanse	%^GREEN%^%^BOLD%^
hansel	%^ORANGE%^
harik	%^BLUE%^%^BOLD%^
harrigen	%^MAGENTA%^
harry	%^WHITE%^
harun	%^RED%^%^BOLD%^
hassan	%^WHITE%^%^B_MAGENTA%^
hatchytt	%^BLUE%^
hatori	%^WHITE%^%^B_MAGENTA%^
haug	%^MAGENTA%^
havelock	%^BLACK%^%^B_WHITE%^
haven	%^RED%^
havok	%^WHITE%^%^B_BLUE%^
heakda	%^BLUE%^
heilig	%^BLUE%^
helaena	%^WHITE%^%^B_MAGENTA%^
helbragga	%^WHITE%^%^BOLD%^
hellmonger	%^WHITE%^%^B_GREEN%^
hells	%^WHITE%^%^B_RED%^
hellspawned	%^WHITE%^%^B_BLUE%^
hemjold	%^RED%^
hemperion	%^BLACK%^%^B_YELLOW%^
hennes	%^BLACK%^%^BOLD%^
henrey	%^BLUE%^
henry	%^BLACK%^%^B_RED%^
hensetsu	%^WHITE%^
herb	%^CYAN%^%^BOLD%^
hercules	%^RED%^
heresiarch	%^BLACK%^%^B_CYAN%^
heretics	%^CYAN%^%^BOLD%^
hesiod	%^ORANGE%^
hexous	%^BLACK%^%^B_WHITE%^
hfi	%^CYAN%^
hfox	%^BLACK%^%^B_MAGENTA%^
hg	%^WHITE%^%^B_BLUE%^
higgs	%^BLACK%^%^B_MAGENTA%^
hilapdatus	%^WHITE%^
hilden	%^CYAN%^
hinalllljb	%^GREEN%^
hirad	%^WHITE%^%^BOLD%^
hirandu	%^BLACK%^%^B_MAGENTA%^
hitchhiker	%^WHITE%^
hitsurume	%^RED%^
hiuchiishi	%^CYAN%^
hlij	%^CYAN%^%^BOLD%^
hoebmnig	%^BLACK%^%^B_YELLOW%^
hogan	%^WHITE%^%^B_BLUE%^
holando	%^BLACK%^%^BOLD%^
holborn	%^WHITE%^%^BOLD%^
holger	%^BLUE%^
holister	%^GREEN%^
holyph	%^BLACK%^%^B_WHITE%^
homo	%^BLACK%^%^B_YELLOW%^
hope	%^WHITE%^%^B_GREEN%^
hortin	%^WHITE%^%^B_RED%^
horus	%^YELLOW%^
horyd	%^RED%^%^BOLD%^
host	%^ORANGE%^
hotaru	%^BLACK%^%^B_MAGENTA%^
hugo	%^ORANGE%^
hugs	%^MAGENTA%^%^BOLD%^
hurukan	%^CYAN%^%^BOLD%^
huuntyr	%^CYAN%^
huwenchao	%^ORANGE%^
hymael	%^BLACK%^%^B_CYAN%^
hypereye	%^GREEN%^
hypo	%^WHITE%^%^B_MAGENTA%^
hytophian	%^BLUE%^%^BOLD%^
i3bot	%^CYAN%^
i3bot2_irc	%^MAGENTA%^
i3client	%^WHITE%^%^B_GREEN%^
i3ircbot_irc	%^CYAN%^
iambe	%^BLACK%^%^BOLD%^
icaros	%^BLACK%^%^B_MAGENTA%^
icebear	%^CYAN%^%^BOLD%^
icewolfz	%^BLACK%^%^B_CYAN%^
ichthus	%^WHITE%^
id	%^WHITE%^%^B_RED%^
idb	%^ORANGE%^
ideysus	%^WHITE%^%^B_BLUE%^
iel	%^WHITE%^%^B_GREEN%^
igabod	%^GREEN%^
igoehddkao	%^ORANGE%^
ihledo	%^MAGENTA%^%^BOLD%^
ihsahn	%^WHITE%^
iilak	%^BLACK%^%^B_MAGENTA%^
iinconfh	%^BLACK%^%^BOLD%^
iituem	%^GREEN%^%^BOLD%^
ilanax	%^WHITE%^%^B_GREEN%^
ilenore	%^CYAN%^
illaris	%^RED%^
illuminati	%^GREEN%^%^BOLD%^
illura	%^BLUE%^
illvei	%^BLACK%^%^BOLD%^
ilmarinen	%^ORANGE%^
ilyasviel	%^CYAN%^
imrik	%^MAGENTA%^
imrix	%^BLACK%^%^B_WHITE%^
inari	%^WHITE%^
incoherent	%^BLACK%^%^B_MAGENTA%^
indeed	%^ORANGE%^
inform_bureau.freenode	%^GREEN%^%^BOLD%^
inige	%^RED%^
inlbmea	%^WHITE%^%^B_GREEN%^
inpho	%^CYAN%^
insanctus	%^CYAN%^%^BOLD%^
insanity	%^MAGENTA%^%^BOLD%^
intchanter	%^WHITE%^%^BOLD%^
intermud	%^MAGENTA%^%^BOLD%^
io	%^BLACK%^%^BOLD%^
ioakecgbf	%^WHITE%^%^B_RED%^
irc_bridge	%^BLACK%^%^B_RED%^
irian	%^BLACK%^%^B_WHITE%^
irmo	%^BLACK%^%^B_RED%^
ironman	%^WHITE%^%^B_BLUE%^
ironmaster	%^BLACK%^%^BOLD%^
isaac	%^GREEN%^
isha	%^CYAN%^
ishako	%^MAGENTA%^
isham	%^BLACK%^%^B_WHITE%^
ishap	%^WHITE%^%^B_GREEN%^
isis	%^WHITE%^%^B_MAGENTA%^
istabo	%^BLUE%^
isth	%^BLACK%^%^B_WHITE%^
isus	%^MAGENTA%^
ith	%^YELLOW%^
ithicar	%^MAGENTA%^
iuz	%^BLACK%^%^B_CYAN%^
ivory	%^BLACK%^%^B_YELLOW%^
ixliam	%^MAGENTA%^
iyrs	%^BLACK%^%^BOLD%^
jaak	%^WHITE%^
jacintha	%^BLACK%^%^B_YELLOW%^
jack hooligan	%^BLACK%^%^BOLD%^
jackel	%^WHITE%^%^B_GREEN%^
jacob	%^CYAN%^
jadon	%^WHITE%^%^B_MAGENTA%^
jadsm	%^BLUE%^%^BOLD%^
jae	%^BLUE%^%^BOLD%^
jaeyd	%^ORANGE%^
jair	%^GREEN%^%^BOLD%^
jake	%^BLACK%^%^B_WHITE%^
jakubczajka	%^WHITE%^%^B_GREEN%^
james	%^WHITE%^%^B_BLUE%^
jandice	%^WHITE%^%^BOLD%^
jane	%^WHITE%^%^B_GREEN%^
janet	%^WHITE%^
janua	%^CYAN%^
japoo	%^WHITE%^%^B_GREEN%^
jared	%^BLUE%^%^BOLD%^
jasmin	%^WHITE%^%^B_GREEN%^
jasmyn	%^WHITE%^%^B_MAGENTA%^
jason	%^CYAN%^
jave	%^RED%^%^BOLD%^
javelin	%^WHITE%^%^B_RED%^
jazhi	%^MAGENTA%^%^BOLD%^
jcdenton	%^RED%^
jeanie	%^WHITE%^%^BOLD%^
jeb	%^GREEN%^%^BOLD%^
jebus	%^YELLOW%^
jeebies	%^BLUE%^%^BOLD%^
jeff	%^WHITE%^%^B_MAGENTA%^
jeffy	%^BLACK%^%^B_MAGENTA%^
jentr	%^BLACK%^%^B_RED%^
jericho	%^BLACK%^%^B_MAGENTA%^
jerin	%^BLUE%^
jerran	%^BLACK%^%^B_CYAN%^
jess	%^WHITE%^%^B_RED%^
jessica	%^BLUE%^
jester	%^GREEN%^
jewels	%^WHITE%^%^B_BLUE%^
jg	%^BLACK%^%^B_YELLOW%^
jian	%^MAGENTA%^%^BOLD%^
jianh	%^BLACK%^%^B_MAGENTA%^
jianhua	%^BLACK%^%^B_CYAN%^
jim	%^YELLOW%^
jimorie	%^BLUE%^
jindrak	%^BLUE%^%^BOLD%^
jinx	%^RED%^%^BOLD%^
jjelabmjcj	%^MAGENTA%^%^BOLD%^
jjldcjkcgjc	%^RED%^%^BOLD%^
jm	%^BLACK%^%^B_GREEN%^
joe	%^WHITE%^%^B_GREEN%^
joffie	%^WHITE%^%^B_RED%^
johan	%^WHITE%^%^B_MAGENTA%^
john	%^WHITE%^%^BOLD%^
johnnie walker	%^ORANGE%^
johnnywalker	%^GREEN%^%^BOLD%^
jonas	%^BLACK%^%^B_CYAN%^
jornalrecord	%^MAGENTA%^
jorril	%^BLUE%^
joseph	%^CYAN%^
joshua	%^WHITE%^%^BOLD%^
joth	%^BLACK%^%^B_MAGENTA%^
jozne	%^BLACK%^%^B_CYAN%^
jub	%^YELLOW%^
juju	%^RED%^
jules	%^GREEN%^
julnar	%^BLACK%^%^B_GREEN%^
jumpsteady	%^BLUE%^%^BOLD%^
juneaux	%^CYAN%^%^BOLD%^
jupiter	%^WHITE%^%^B_BLUE%^
juppie	%^BLUE%^
justinjm	%^BLUE%^
jym	%^MAGENTA%^
k'azdean	%^BLUE%^
kaatil	%^GREEN%^%^BOLD%^
kadaan	%^BLACK%^%^B_RED%^
kaden	%^WHITE%^
kaea	%^WHITE%^%^B_GREEN%^
kairehn	%^YELLOW%^
kaiser	%^BLACK%^%^B_MAGENTA%^
kaitan	%^BLACK%^%^B_YELLOW%^
kaldhal	%^WHITE%^%^BOLD%^
kalibarr	%^CYAN%^
kalinash	%^MAGENTA%^
kalinice	%^WHITE%^%^BOLD%^
kall	%^BLACK%^%^B_YELLOW%^
kalmar	%^BLACK%^%^BOLD%^
kalyth	%^BLACK%^%^B_MAGENTA%^
kamarihi	%^ORANGE%^
kamigo	%^BLACK%^%^B_CYAN%^
kamikaze	%^GREEN%^
kana	%^WHITE%^%^B_RED%^
kane	%^WHITE%^%^B_GREEN%^
kang	%^MAGENTA%^%^BOLD%^
kara	%^BLUE%^%^BOLD%^
kari	%^GREEN%^%^BOLD%^
karka	%^BLUE%^%^BOLD%^
karos	%^YELLOW%^
karri	%^BLACK%^%^B_RED%^
karthius	%^WHITE%^%^B_RED%^
kashis	%^BLUE%^
kasji	%^BLACK%^%^B_WHITE%^
katana	%^BLACK%^%^B_CYAN%^
kataro	%^BLACK%^%^B_GREEN%^
kathor	%^BLACK%^%^B_GREEN%^
katia	%^ORANGE%^
katiara	%^BLUE%^%^BOLD%^
katniss	%^ORANGE%^
katsuun	%^WHITE%^
katta	%^WHITE%^
katton	%^WHITE%^%^B_BLUE%^
kaven	%^RED%^
kawakisan	%^GREEN%^
kayjay	%^WHITE%^
kaylar	%^CYAN%^%^BOLD%^
kayle	%^RED%^
kaylus	%^BLACK%^%^B_WHITE%^
kaytone	%^BLACK%^%^B_CYAN%^
kayuq	%^GREEN%^%^BOLD%^
kazdean	%^ORANGE%^
kaziem	%^CYAN%^%^BOLD%^
kce	%^BLACK%^%^B_GREEN%^
kcocm	%^GREEN%^%^BOLD%^
keb	%^BLACK%^%^B_WHITE%^
keegan	%^MAGENTA%^
keeperofkeys	%^GREEN%^
kefka	%^BLACK%^%^B_GREEN%^
keg	%^BLACK%^%^B_GREEN%^
kegalstrength	%^YELLOW%^
keill	%^WHITE%^%^B_MAGENTA%^
keiran	%^CYAN%^%^BOLD%^
keita	%^ORANGE%^
kejab	%^BLUE%^%^BOLD%^
kejope	%^WHITE%^%^B_GREEN%^
kelli	%^WHITE%^%^B_RED%^
kelnale	%^YELLOW%^
kelton	%^YELLOW%^
kelvin	%^BLACK%^%^B_MAGENTA%^
kelvin:_canadians_and_northerners	%^BLACK%^%^B_RED%^
kenny	%^YELLOW%^
kenobi	%^BLACK%^%^B_RED%^
kenon	%^RED%^%^BOLD%^
kensei	%^BLACK%^%^B_YELLOW%^
kenton	%^WHITE%^%^B_BLUE%^
ker	%^WHITE%^
kerrie	%^WHITE%^%^B_GREEN%^
kesler	%^ORANGE%^
kesric	%^WHITE%^%^B_BLUE%^
kevin	%^MAGENTA%^
kevina	%^WHITE%^%^B_RED%^
kfednhmof	%^BLACK%^%^B_WHITE%^
kg	%^CYAN%^
kgagbjodb	%^RED%^
kgnbili	%^BLACK%^%^B_YELLOW%^
khadija	%^WHITE%^%^B_GREEN%^
khairn	%^MAGENTA%^
khajed	%^WHITE%^%^BOLD%^
khaos	%^BLACK%^%^B_WHITE%^
kheoinn	%^WHITE%^%^B_BLUE%^
khnemu	%^BLACK%^%^BOLD%^
khronos	%^BLACK%^%^B_WHITE%^
kibbits	%^BLACK%^%^B_RED%^
kieve	%^GREEN%^
kiki	%^WHITE%^%^B_GREEN%^
kikon	%^CYAN%^%^BOLD%^
kill	%^BLUE%^%^BOLD%^
kina	%^RED%^%^BOLD%^
kit	%^MAGENTA%^%^BOLD%^
kivo	%^ORANGE%^
kiyo	%^WHITE%^%^B_GREEN%^
kizayaen	%^CYAN%^%^BOLD%^
kizeren	%^BLUE%^
kj	%^WHITE%^%^B_RED%^
kju	%^BLACK%^%^BOLD%^
kknobgojeke	%^BLACK%^%^B_MAGENTA%^
klobnb	%^WHITE%^
klok	%^MAGENTA%^
knladob	%^BLACK%^%^B_MAGENTA%^
knucklehead	%^WHITE%^%^B_RED%^
knutselmaaster	%^BLACK%^%^B_MAGENTA%^
kobol	%^RED%^%^BOLD%^
kogana	%^BLACK%^%^BOLD%^
kohl	%^CYAN%^
komalictor	%^ORANGE%^
kong-fu-zi	%^BLACK%^%^B_MAGENTA%^
koofle	%^WHITE%^%^B_BLUE%^
koopa	%^BLACK%^%^BOLD%^
korentsu	%^BLACK%^%^B_YELLOW%^
koresh	%^WHITE%^%^BOLD%^
kotaku	%^MAGENTA%^
kraiven	%^BLUE%^
krazel	%^ORANGE%^
kreezxi	%^BLUE%^%^BOLD%^
kreezxil	%^MAGENTA%^%^BOLD%^
kreldin	%^CYAN%^%^BOLD%^
krem	%^BLACK%^%^BOLD%^
krhonos	%^GREEN%^%^BOLD%^
krilzhin	%^WHITE%^%^B_BLUE%^
krimpal	%^BLACK%^%^B_YELLOW%^
krinata	%^MAGENTA%^%^BOLD%^
krisela	%^BLACK%^%^B_CYAN%^
kristus	%^ORANGE%^
kriton	%^WHITE%^%^B_GREEN%^
krokken	%^ORANGE%^
krylin	%^BLACK%^%^B_MAGENTA%^
kshito	%^BLACK%^%^B_MAGENTA%^
kudde	%^WHITE%^%^B_RED%^
kujuta	%^MAGENTA%^
kulyax	%^WHITE%^
kurique	%^ORANGE%^
kuurus	%^CYAN%^%^BOLD%^
kvarim	%^MAGENTA%^%^BOLD%^
kyou	%^GREEN%^%^BOLD%^
kyr	%^WHITE%^%^B_MAGENTA%^
kyra	%^BLACK%^%^B_CYAN%^
kythorn	%^WHITE%^%^B_GREEN%^
kyurama	%^GREEN%^
lady	%^MAGENTA%^%^BOLD%^
lag	%^BLUE%^
lallander	%^WHITE%^%^BOLD%^
lambert	%^CYAN%^%^BOLD%^
lamir	%^WHITE%^%^B_BLUE%^
lancustran	%^BLACK%^%^B_GREEN%^
lanikai	%^WHITE%^
lanstri	%^RED%^
laoise	%^WHITE%^%^B_RED%^
laotzu	%^BLACK%^%^B_WHITE%^
largo	%^BLACK%^%^B_GREEN%^
lark	%^WHITE%^%^B_MAGENTA%^
lars	%^WHITE%^%^BOLD%^
lash	%^WHITE%^%^B_RED%^
lateralus	%^WHITE%^%^BOLD%^
laurelen	%^BLUE%^
lauryn	%^CYAN%^
lautrec	%^RED%^
layla	%^WHITE%^%^B_MAGENTA%^
laylle	%^WHITE%^%^B_GREEN%^
lazarus	%^BLACK%^%^B_GREEN%^
ldfnhcbdl	%^WHITE%^%^B_MAGENTA%^
ldkkhjjn	%^RED%^
le	%^ORANGE%^
leanna	%^BLACK%^%^B_CYAN%^
legumie	%^CYAN%^%^BOLD%^
lei	%^RED%^%^BOLD%^
lemonseural	%^WHITE%^%^BOLD%^
lenaric	%^WHITE%^%^B_GREEN%^
leonidas	%^BLUE%^
lesa	%^ORANGE%^
lesik	%^WHITE%^%^B_BLUE%^
letitia	%^WHITE%^%^B_MAGENTA%^
leto	%^WHITE%^%^BOLD%^
levinon	%^RED%^%^BOLD%^
leviticus	%^BLACK%^%^B_RED%^
lexus	%^WHITE%^%^BOLD%^
lexx	%^BLACK%^%^B_CYAN%^
lez	%^WHITE%^%^BOLD%^
lfdje	%^BLACK%^%^B_CYAN%^
lfdodagio	%^WHITE%^%^B_RED%^
lfgajmnokejcdo	%^YELLOW%^
lhamffhnk	%^GREEN%^%^BOLD%^
lhckhka	%^GREEN%^
liam	%^WHITE%^
liandra	%^BLACK%^%^B_YELLOW%^
liard	%^ORANGE%^
liasa	%^BLUE%^
lidia	%^WHITE%^%^B_MAGENTA%^
lilcutie	%^ORANGE%^
lilithsucci	%^BLUE%^%^BOLD%^
lily	%^BLACK%^%^B_MAGENTA%^
lingeng	%^BLACK%^%^B_WHITE%^
link	%^ORANGE%^
links	%^WHITE%^%^B_BLUE%^
linoge	%^YELLOW%^
liondriel	%^BLACK%^%^B_YELLOW%^
lipseng	%^GREEN%^%^BOLD%^
liquidpotatoes	%^BLACK%^%^BOLD%^
littlehoward	%^BLUE%^%^BOLD%^
lividity	%^WHITE%^%^B_MAGENTA%^
lix	%^WHITE%^%^B_BLUE%^
lixi	%^BLUE%^
lizard	%^GREEN%^
lkagboi	%^MAGENTA%^
lkcoe	%^RED%^%^BOLD%^
lkknacagld	%^YELLOW%^
ll	%^BLACK%^%^B_CYAN%^
llama	%^ORANGE%^
lleirdale	%^WHITE%^%^B_GREEN%^
llhorian	%^GREEN%^
llondkb	%^BLUE%^%^BOLD%^
llylia	%^MAGENTA%^%^BOLD%^
llyr	%^BLACK%^%^B_GREEN%^
llyrtest	%^CYAN%^
lobster	%^BLACK%^%^B_WHITE%^
locke	%^RED%^
logaer	%^GREEN%^%^BOLD%^
logras	%^BLACK%^%^B_MAGENTA%^
loki	%^GREEN%^%^BOLD%^
longrifle	%^GREEN%^%^BOLD%^
looking totally pissed, adam	%^CYAN%^%^BOLD%^
lord cron	%^BLUE%^%^BOLD%^
lord shentino	%^GREEN%^%^BOLD%^
lotherig	%^BLACK%^%^B_RED%^
lotherio	%^RED%^
lothlorian	%^CYAN%^
lotus	%^RED%^%^BOLD%^
loutre	%^BLACK%^%^BOLD%^
lovenkraft	%^BLUE%^%^BOLD%^
lox	%^GREEN%^%^BOLD%^
lpmuds	%^BLACK%^%^B_MAGENTA%^
lucid	%^WHITE%^%^B_RED%^
lucks	%^RED%^
lucretia	%^GREEN%^
luggage	%^WHITE%^%^BOLD%^
lumeria	%^BLACK%^%^B_GREEN%^
luna	%^RED%^%^BOLD%^
lunch	%^MAGENTA%^
luthor	%^BLACK%^%^BOLD%^
lutt	%^CYAN%^%^BOLD%^
ly'nx	%^GREEN%^
lyanic	%^CYAN%^
lycaon	%^BLACK%^%^B_GREEN%^
lyeith	%^GREEN%^%^BOLD%^
lynx	%^WHITE%^
lynxo	%^RED%^
lyoc	%^CYAN%^%^BOLD%^
lyra	%^BLUE%^
lysergic	%^WHITE%^%^B_BLUE%^
mactorg	%^ORANGE%^
madison	%^BLACK%^%^B_MAGENTA%^
madness	%^RED%^
madoc	%^GREEN%^
madrox	%^CYAN%^
maeko	%^GREEN%^
maelin	%^BLACK%^%^B_CYAN%^
maestro	%^GREEN%^%^BOLD%^
magearus	%^WHITE%^%^BOLD%^
maggy	%^WHITE%^%^BOLD%^
magiko	%^WHITE%^%^B_GREEN%^
magnis	%^BLACK%^%^B_WHITE%^
magstant	%^MAGENTA%^
mahain	%^WHITE%^%^B_RED%^
mahayana	%^WHITE%^%^B_MAGENTA%^
mahkefel	%^WHITE%^
maimonides	%^WHITE%^%^B_RED%^
maka	%^ORANGE%^
makan the white	%^BLACK%^%^BOLD%^
makhar	%^GREEN%^
mal	%^MAGENTA%^%^BOLD%^
malaclypse	%^MAGENTA%^%^BOLD%^
malady	%^WHITE%^%^B_MAGENTA%^
malak	%^ORANGE%^
malakanator	%^BLUE%^
malcynthis	%^WHITE%^
malegnis	%^WHITE%^%^B_BLUE%^
malek	%^WHITE%^%^BOLD%^
malic	%^RED%^%^BOLD%^
malice	%^WHITE%^%^B_BLUE%^
maliene	%^BLACK%^%^B_YELLOW%^
mallaggar	%^YELLOW%^
malthe	%^ORANGE%^
malveron	%^BLACK%^%^B_RED%^
mandos	%^GREEN%^%^BOLD%^
maniacalrage	%^BLACK%^%^B_GREEN%^
manicdep	%^CYAN%^%^BOLD%^
manious	%^WHITE%^%^B_MAGENTA%^
manwe	%^BLACK%^%^B_WHITE%^
mar	%^BLACK%^%^B_RED%^
marajin	%^GREEN%^
marbelle	%^WHITE%^%^B_BLUE%^
mardin	%^YELLOW%^
mariah	%^BLUE%^%^BOLD%^
mariana	%^BLUE%^
marianna	%^BLACK%^%^BOLD%^
marina	%^BLACK%^%^B_GREEN%^
mark	%^MAGENTA%^
marlin	%^BLACK%^%^B_WHITE%^
marqii	%^MAGENTA%^%^BOLD%^
marroc phineas sandybanks	%^WHITE%^%^B_GREEN%^
martaigne	%^BLACK%^%^B_WHITE%^
martinez	%^GREEN%^%^BOLD%^
mason	%^GREEN%^%^BOLD%^
master	%^BLACK%^%^B_RED%^
matillo	%^BLACK%^%^B_YELLOW%^
matroid	%^BLACK%^%^B_MAGENTA%^
mavven	%^YELLOW%^
maw	%^BLACK%^%^B_CYAN%^
maximus	%^GREEN%^
maya	%^RED%^
mayhem	%^BLACK%^%^B_RED%^
maynard	%^CYAN%^
mazir	%^BLACK%^%^B_MAGENTA%^
mazor	%^BLACK%^%^BOLD%^
mazu	%^CYAN%^
mbcolhfnk	%^WHITE%^%^B_BLUE%^
mc	%^WHITE%^%^B_MAGENTA%^
mcfuck	%^BLACK%^%^BOLD%^
mdagcegim	%^BLACK%^%^B_WHITE%^
mdca	%^CYAN%^
mdegakm	%^BLUE%^
me	%^RED%^%^BOLD%^
mecha	%^CYAN%^%^BOLD%^
mediok	%^MAGENTA%^%^BOLD%^
meep	%^BLUE%^%^BOLD%^
meister	%^BLUE%^
mel	%^BLACK%^%^B_YELLOW%^
mel gibson	%^BLACK%^%^B_CYAN%^
melchezidek	%^RED%^%^BOLD%^
melkor	%^BLACK%^%^B_MAGENTA%^
memnoch	%^GREEN%^%^BOLD%^
memrosh	%^WHITE%^%^BOLD%^
menouthis	%^MAGENTA%^%^BOLD%^
mercutio	%^WHITE%^
mercy	%^MAGENTA%^%^BOLD%^
merlin	%^GREEN%^%^BOLD%^
mermaid	%^RED%^
mester	%^WHITE%^%^BOLD%^
metaplace	%^BLACK%^%^B_CYAN%^
metiscus	%^RED%^
mezfir	%^RED%^%^BOLD%^
mgd	%^GREEN%^%^BOLD%^
mi	%^WHITE%^
michael	%^BLACK%^%^B_WHITE%^
micky	%^ORANGE%^
mike	%^WHITE%^
mimer	%^BLACK%^%^B_YELLOW%^
minerva	%^CYAN%^%^BOLD%^
minion	%^BLUE%^
minori	%^BLACK%^%^B_CYAN%^
miramar	%^WHITE%^%^B_GREEN%^
miranda	%^WHITE%^%^B_RED%^
mirial	%^GREEN%^
miryks	%^RED%^%^BOLD%^
misfit	%^WHITE%^%^B_GREEN%^
mishal	%^GREEN%^%^BOLD%^
miskar	%^MAGENTA%^%^BOLD%^
mister	%^BLUE%^
mistermoxxie	%^WHITE%^%^B_BLUE%^
misuzu	%^WHITE%^%^B_BLUE%^
mithleidh	%^ORANGE%^
mitonic	%^BLUE%^
mitsukai	%^BLACK%^%^B_CYAN%^
miyagi	%^BLACK%^%^B_RED%^
mizar	%^MAGENTA%^
mjbeeki	%^WHITE%^%^BOLD%^
mkmjlkoc	%^GREEN%^%^BOLD%^
ml	%^MAGENTA%^%^BOLD%^
mldbhmnaakh	%^CYAN%^%^BOLD%^
moab	%^BLACK%^%^BOLD%^
moaeadgj	%^MAGENTA%^
mod george	%^WHITE%^%^B_RED%^
mog	%^BLACK%^%^B_MAGENTA%^
mogwai	%^GREEN%^
moiroth	%^WHITE%^%^B_RED%^
mojo	%^GREEN%^%^BOLD%^
molmkhgelhe	%^CYAN%^%^BOLD%^
monad	%^YELLOW%^
money	%^CYAN%^%^BOLD%^
monika	%^BLACK%^%^B_MAGENTA%^
monkey	%^BLACK%^%^B_CYAN%^
monkeyx	%^BLACK%^%^B_MAGENTA%^
monkitron	%^GREEN%^
montresor	%^BLACK%^%^B_RED%^
monzoo	%^CYAN%^%^BOLD%^
moraless	%^WHITE%^
morbid	%^BLACK%^%^B_GREEN%^
mordain	%^GREEN%^
mordecai	%^MAGENTA%^%^BOLD%^
morfon	%^BLUE%^%^BOLD%^
morgaine	%^BLUE%^%^BOLD%^
morgan	%^BLUE%^%^BOLD%^
mori	%^YELLOW%^
moridin	%^GREEN%^
mork	%^RED%^
morkar	%^RED%^%^BOLD%^
morkin	%^WHITE%^%^BOLD%^
morlock	%^WHITE%^%^BOLD%^
morpheus	%^GREEN%^
mors	%^ORANGE%^
mortanius	%^WHITE%^%^B_RED%^
mortifi	%^WHITE%^%^B_MAGENTA%^
mortuus	%^WHITE%^%^B_RED%^
morvudd	%^RED%^
mourdrydd	%^GREEN%^
mourningstar	%^BLUE%^
mouse	%^GREEN%^%^BOLD%^
moyer	%^BLACK%^%^B_MAGENTA%^
mudadmin	%^WHITE%^
mudbytes	%^WHITE%^%^B_GREEN%^
mudbytes-files	%^RED%^%^BOLD%^
mudgamers	%^YELLOW%^
mudmagic	%^WHITE%^%^B_BLUE%^
mudportal	%^WHITE%^%^B_MAGENTA%^
mudruck	%^RED%^
mudstandards	%^MAGENTA%^%^BOLD%^
muldavia	%^BLACK%^%^B_WHITE%^
mulder	%^WHITE%^%^B_RED%^
mumpfi	%^CYAN%^
mung	%^WHITE%^%^B_MAGENTA%^
munku	%^WHITE%^%^BOLD%^
murdoc	%^ORANGE%^
muskard	%^BLACK%^%^B_WHITE%^
mylon	%^BLACK%^%^B_MAGENTA%^
myndzi	%^BLACK%^%^B_MAGENTA%^
myoko	%^RED%^%^BOLD%^
myr	%^BLACK%^%^BOLD%^
myranda	%^GREEN%^%^BOLD%^
myrddin	%^CYAN%^%^BOLD%^
myrna	%^BLACK%^%^B_MAGENTA%^
mysterio	%^RED%^
mystic spiritus	%^WHITE%^%^B_MAGENTA%^
mythica	%^WHITE%^%^B_GREEN%^
mythl	%^CYAN%^
naefar	%^CYAN%^
nafe	%^BLUE%^
namazu	%^GREEN%^
nasa	%^RED%^
nat	%^BLACK%^%^B_MAGENTA%^
nathan	%^BLACK%^%^B_YELLOW%^
natsuko	%^BLACK%^%^BOLD%^
naud	%^BLUE%^
nav	%^WHITE%^%^B_BLUE%^
navouri	%^BLACK%^%^B_WHITE%^
ndah	%^GREEN%^
needad	%^WHITE%^%^B_MAGENTA%^
nefret	%^GREEN%^%^BOLD%^
neic	%^MAGENTA%^
neinittz	%^WHITE%^%^B_GREEN%^
neo	%^BLACK%^%^BOLD%^
nerull	%^BLUE%^%^BOLD%^
neven	%^WHITE%^%^B_RED%^
nevin	%^BLACK%^%^BOLD%^
nevvyn	%^CYAN%^%^BOLD%^
newscientist	%^BLACK%^%^BOLD%^
newt	%^CYAN%^%^BOLD%^
nexes	%^BLUE%^
nexuiz	%^MAGENTA%^
niakeeh	%^BLACK%^%^B_CYAN%^
nib	%^BLACK%^%^BOLD%^
nick	%^GREEN%^
niels	%^WHITE%^%^B_MAGENTA%^
nightcrawler	%^RED%^
nightian	%^WHITE%^%^B_MAGENTA%^
nightmare	%^WHITE%^
nightmask	%^BLACK%^%^B_YELLOW%^
nightstalker	%^WHITE%^%^B_GREEN%^
niijel	%^BLACK%^%^B_WHITE%^
nika	%^BLACK%^%^B_CYAN%^
nikeos	%^BLACK%^%^B_RED%^
nilheim	%^GREEN%^%^BOLD%^
nilrin	%^WHITE%^
nim	%^WHITE%^%^B_MAGENTA%^
nimdeh	%^BLUE%^
nimrod	%^ORANGE%^
ninetenths	%^ORANGE%^
ninja	%^BLACK%^%^B_CYAN%^
ninka	%^BLACK%^%^B_WHITE%^
niraya	%^WHITE%^%^B_BLUE%^
nirethil	%^BLUE%^%^BOLD%^
nirilil	%^WHITE%^%^B_RED%^
nkidloddh	%^BLACK%^%^B_CYAN%^
nlodjcnofk	%^BLACK%^%^BOLD%^
nnbljljfb	%^RED%^
nnih	%^MAGENTA%^%^BOLD%^
nnivel	%^WHITE%^%^B_MAGENTA%^
no	%^WHITE%^
noblesse	%^ORANGE%^
nogod	%^WHITE%^%^B_BLUE%^
noibbjo	%^BLACK%^%^B_WHITE%^
noiram	%^BLACK%^%^B_WHITE%^
noise	%^BLACK%^%^B_CYAN%^
noisome	%^GREEN%^%^BOLD%^
noliojeobfg	%^WHITE%^%^B_GREEN%^
nomad	%^CYAN%^%^BOLD%^
nono	%^GREEN%^
nonx	%^WHITE%^%^B_RED%^
nosmo	%^RED%^
nova	%^RED%^
nox	%^WHITE%^%^B_BLUE%^
nslm	%^BLUE%^%^BOLD%^
nubtest	%^BLACK%^%^B_CYAN%^
nuggiewuggie	%^MAGENTA%^%^BOLD%^
nuggs	%^WHITE%^%^B_BLUE%^
nuit	%^RED%^
nullinfinite	%^GREEN%^
nulvect	%^BLACK%^%^B_RED%^
numair	%^BLACK%^%^B_WHITE%^
numhitzu	%^BLACK%^%^B_YELLOW%^
nuryan	%^BLUE%^
nvader	%^BLACK%^%^B_CYAN%^
nymeria	%^GREEN%^
nyx	%^WHITE%^%^B_GREEN%^
oarien	%^MAGENTA%^%^BOLD%^
obelix	%^WHITE%^%^B_GREEN%^
obhnmkncn	%^BLACK%^%^B_RED%^
obliki	%^BLACK%^%^B_MAGENTA%^
oblivion	%^RED%^%^BOLD%^
obsidian	%^BLUE%^%^BOLD%^
ochfkfk	%^BLACK%^%^B_CYAN%^
ochthuit	%^BLACK%^%^B_YELLOW%^
oddmint	%^MAGENTA%^%^BOLD%^
odeen	%^BLACK%^%^B_CYAN%^
odin	%^BLACK%^%^B_YELLOW%^
odoth	%^BLACK%^%^B_YELLOW%^
off-topic quixadhal	%^RED%^
ofk	%^WHITE%^
og	%^RED%^
ogguur	%^MAGENTA%^
ogma	%^YELLOW%^
ohijaebhl	%^RED%^
ohm	%^WHITE%^%^B_GREEN%^
ohtar	%^WHITE%^%^B_MAGENTA%^
oiolhnge	%^BLUE%^%^BOLD%^
oki	%^RED%^%^BOLD%^
okjcjbjk	%^ORANGE%^
okomaefbfle	%^BLACK%^%^BOLD%^
ol	%^BLACK%^%^B_WHITE%^
oldes	%^CYAN%^%^BOLD%^
olgar	%^MAGENTA%^%^BOLD%^
oliver	%^WHITE%^%^B_MAGENTA%^
oljbhffk	%^BLUE%^
omachron	%^BLACK%^%^B_GREEN%^
omega	%^WHITE%^
omni	%^YELLOW%^
omorthan	%^BLACK%^%^BOLD%^
one	%^CYAN%^%^BOLD%^
oni	%^BLACK%^%^B_MAGENTA%^
oots	%^BLACK%^%^B_YELLOW%^
ooze	%^YELLOW%^
orang	%^WHITE%^%^B_RED%^
orange	%^MAGENTA%^%^BOLD%^
orange julius	%^WHITE%^%^B_BLUE%^
ordaith	%^GREEN%^
oreo	%^ORANGE%^
oriam	%^RED%^
origen	%^RED%^%^BOLD%^
orion	%^WHITE%^%^B_MAGENTA%^
orpheo	%^WHITE%^%^B_MAGENTA%^
osiris	%^BLACK%^%^B_CYAN%^
osore	%^GREEN%^
osuvox	%^WHITE%^%^B_GREEN%^
ourania	%^BLACK%^%^B_GREEN%^
ouroboros	%^WHITE%^%^B_GREEN%^
overseer	%^WHITE%^
ow! tatianna	%^BLACK%^%^B_RED%^
paddy	%^CYAN%^
paff	%^BLUE%^%^BOLD%^
pakaran	%^RED%^%^BOLD%^
pakrat	%^RED%^%^BOLD%^
paladine	%^BLUE%^
pan	%^CYAN%^%^BOLD%^
pancracker	%^BLACK%^%^B_RED%^
panther	%^BLUE%^%^BOLD%^
paranoia	%^MAGENTA%^
parham	%^BLACK%^%^B_RED%^
pariah	%^MAGENTA%^%^BOLD%^
parillon	%^BLACK%^%^B_GREEN%^
parnell	%^CYAN%^
parthenon	%^ORANGE%^
patch	%^GREEN%^
patientfox	%^BLUE%^%^BOLD%^
patriot	%^WHITE%^%^B_GREEN%^
pauline	%^RED%^%^BOLD%^
paulkami	%^BLACK%^%^B_RED%^
paven	%^GREEN%^%^BOLD%^
pazreal	%^MAGENTA%^
peacock	%^MAGENTA%^
pellaeon	%^RED%^%^BOLD%^
pendrake	%^BLACK%^%^B_CYAN%^
penguin	%^WHITE%^%^B_RED%^
penguxn_irc	%^BLACK%^%^B_WHITE%^
perallen	%^WHITE%^%^BOLD%^
percussor	%^WHITE%^%^B_BLUE%^
pete	%^WHITE%^%^B_GREEN%^
petri	%^BLACK%^%^B_CYAN%^
petriomelony	%^BLACK%^%^B_YELLOW%^
petrov	%^GREEN%^%^BOLD%^
phabian	%^WHITE%^%^B_BLUE%^
phaedo	%^RED%^%^BOLD%^
phaerus	%^RED%^%^BOLD%^
pham	%^YELLOW%^
phanku	%^MAGENTA%^%^BOLD%^
phantom	%^WHITE%^%^BOLD%^
pharm	%^RED%^
phil	%^BLUE%^
phreak	%^WHITE%^%^B_BLUE%^
phrynicus	%^GREEN%^%^BOLD%^
pieter	%^WHITE%^
pigsty	%^WHITE%^%^B_RED%^
pinkfish	%^RED%^
pip	%^YELLOW%^
pirate	%^WHITE%^%^B_BLUE%^
pirch	%^BLACK%^%^BOLD%^
pit	%^BLACK%^%^B_GREEN%^
pite	%^BLACK%^%^B_RED%^
pius	%^MAGENTA%^
planetmuddev	%^BLACK%^%^B_CYAN%^
plato	%^CYAN%^
pli	%^YELLOW%^
ploo	%^BLACK%^%^B_WHITE%^
ploosk	%^MAGENTA%^%^BOLD%^
ploppaz	%^MAGENTA%^%^BOLD%^
plugh	%^MAGENTA%^%^BOLD%^
plugh's mom	%^RED%^%^BOLD%^
pluie	%^WHITE%^%^B_MAGENTA%^
plunderer	%^GREEN%^
pmuut	%^WHITE%^%^B_MAGENTA%^
pocok	%^WHITE%^%^BOLD%^
poet	%^YELLOW%^
poi	%^WHITE%^%^BOLD%^
polatrite	%^BLUE%^
pollux	%^BLUE%^%^BOLD%^
pommes	%^BLACK%^%^B_CYAN%^
poncho	%^WHITE%^%^B_RED%^
pondscum	%^CYAN%^
poseidon	%^BLACK%^%^B_RED%^
positrix	%^YELLOW%^
pp	%^BLACK%^%^B_YELLOW%^
prada	%^CYAN%^%^BOLD%^
pragmatic	%^MAGENTA%^
pranka	%^BLACK%^%^B_YELLOW%^
presto	%^WHITE%^%^BOLD%^
prime	%^BLACK%^%^B_YELLOW%^
princesy	%^WHITE%^%^B_BLUE%^
pringles	%^BLACK%^%^B_GREEN%^
priscilla	%^ORANGE%^
prool	%^WHITE%^%^B_GREEN%^
prophecy	%^GREEN%^%^BOLD%^
psmurf	%^WHITE%^%^BOLD%^
pso	%^BLACK%^%^B_RED%^
psycho	%^BLACK%^%^B_GREEN%^
psylo	%^MAGENTA%^
psylocybin	%^BLACK%^%^B_RED%^
ptah	%^BLACK%^%^B_GREEN%^
ptenisnet	%^YELLOW%^
pthag	%^WHITE%^%^B_GREEN%^
ptol	%^BLACK%^%^B_CYAN%^
ptoley	%^RED%^%^BOLD%^
ptorgoth	%^WHITE%^%^B_BLUE%^
pulp	%^WHITE%^%^BOLD%^
pulse	%^WHITE%^%^B_RED%^
punisher	%^BLACK%^%^B_GREEN%^
pure	%^WHITE%^%^BOLD%^
purlow	%^CYAN%^%^BOLD%^
pyromaniac	%^RED%^%^BOLD%^
pyron	%^BLUE%^%^BOLD%^
pyros	%^BLACK%^%^B_RED%^
pythium	%^BLACK%^%^B_WHITE%^
pytlak	%^WHITE%^%^B_GREEN%^
qiuyan	%^RED%^
qrzx	%^BLUE%^%^BOLD%^
qualin	%^GREEN%^
quaril	%^WHITE%^%^B_BLUE%^
quetzalcoatl	%^BLACK%^%^B_YELLOW%^
quiverclaw	%^BLUE%^%^BOLD%^
quixadhal, the lost	%^RED%^
quneloso	%^RED%^
quorthon	%^BLACK%^%^B_CYAN%^
quotid	%^WHITE%^%^B_GREEN%^
qwer	%^WHITE%^%^BOLD%^
qzxetrq	%^BLACK%^%^BOLD%^
rabenmire	%^BLACK%^%^B_MAGENTA%^
radyth	%^MAGENTA%^
rae	%^BLACK%^%^B_RED%^
rafiki	%^WHITE%^
rafjaesol	%^CYAN%^%^BOLD%^
raganim	%^WHITE%^%^BOLD%^
rain	%^WHITE%^
rainy	%^WHITE%^%^B_GREEN%^
raistlin	%^GREEN%^
raize	%^RED%^%^BOLD%^
raja	%^BLUE%^%^BOLD%^
rajana	%^MAGENTA%^%^BOLD%^
raleigh	%^WHITE%^%^B_RED%^
ralph	%^BLUE%^
ramgo	%^BLACK%^%^B_RED%^
rand	%^BLACK%^%^B_GREEN%^
randuin	%^BLACK%^%^B_YELLOW%^
ranneko	%^ORANGE%^
rarity	%^WHITE%^%^B_GREEN%^
rascal	%^BLACK%^%^BOLD%^
rasputin	%^BLUE%^
rastakwer	%^RED%^%^BOLD%^
rathe	%^WHITE%^%^B_RED%^
ratman	%^BLACK%^%^B_CYAN%^
raudhrskal	%^MAGENTA%^%^BOLD%^
raver	%^CYAN%^
ravnos	%^BLUE%^%^BOLD%^
rawls	%^WHITE%^%^B_GREEN%^
rawr	%^WHITE%^%^B_GREEN%^
raydj	%^BLACK%^%^B_WHITE%^
rayvn	%^BLACK%^%^B_GREEN%^
realedazed	%^WHITE%^%^B_GREEN%^
rebirth	%^WHITE%^%^BOLD%^
recca	%^BLACK%^%^BOLD%^
recluse	%^WHITE%^%^BOLD%^
redacted	%^GREEN%^%^BOLD%^
redhaven	%^BLACK%^%^B_MAGENTA%^
reeve	%^BLACK%^%^B_MAGENTA%^
reflection	%^BLACK%^%^BOLD%^
reflex	%^RED%^
regulus	%^RED%^%^BOLD%^
reika	%^WHITE%^
reimi	%^BLACK%^%^BOLD%^
reku	%^BLACK%^%^B_CYAN%^
relayne	%^WHITE%^
relg	%^WHITE%^%^B_RED%^
remcon	%^WHITE%^%^BOLD%^
remmy	%^BLACK%^%^B_WHITE%^
renent	%^GREEN%^
renras	%^WHITE%^%^B_GREEN%^
reseph	%^BLUE%^%^BOLD%^
retisos	%^BLUE%^
retrogt	%^WHITE%^%^B_RED%^
reuters	%^BLACK%^%^B_MAGENTA%^
revan	%^BLACK%^%^B_GREEN%^
revathiest	%^GREEN%^
reverend	%^WHITE%^%^B_MAGENTA%^
revreese	%^YELLOW%^
reyne	%^BLACK%^%^B_WHITE%^
reythirren	%^RED%^
rezeul	%^GREEN%^%^BOLD%^
rhale	%^ORANGE%^
rhavin	%^RED%^
rhayam	%^BLACK%^%^B_RED%^
rhinehold	%^WHITE%^%^BOLD%^
rhudoc	%^BLUE%^
riagn	%^BLUE%^%^BOLD%^
ricorn	%^RED%^%^BOLD%^
rijer	%^MAGENTA%^%^BOLD%^
rikluz	%^WHITE%^
rikudo	%^GREEN%^%^BOLD%^
rikudo sennin	%^BLACK%^%^B_YELLOW%^
rince	%^WHITE%^%^B_GREEN%^
riper	%^WHITE%^%^B_BLUE%^
rire makar	%^WHITE%^%^B_MAGENTA%^
riverphoenix	%^CYAN%^%^BOLD%^
rob	%^BLUE%^%^BOLD%^
robgea	%^WHITE%^%^B_GREEN%^
rock	%^BLACK%^%^B_RED%^
rodaith	%^BLACK%^%^B_CYAN%^
roger	%^GREEN%^
rogre	%^CYAN%^
roku	%^BLACK%^%^B_RED%^
roland	%^MAGENTA%^%^BOLD%^
rommel	%^WHITE%^
ron	%^CYAN%^
rondros	%^MAGENTA%^%^BOLD%^
ronny	%^BLACK%^%^B_CYAN%^
rook	%^ORANGE%^
roon	%^BLACK%^%^BOLD%^
root	%^WHITE%^
rorix	%^BLUE%^%^BOLD%^
rotas	%^GREEN%^
roy	%^BLACK%^%^B_CYAN%^
rss_bot	%^YELLOW%^
rssbot	%^MAGENTA%^
ruben	%^BLACK%^%^B_MAGENTA%^
rue	%^WHITE%^
ruhsbaar	%^MAGENTA%^
rum	%^GREEN%^%^BOLD%^
runtime	%^YELLOW%^
rushnak	%^BLUE%^%^BOLD%^
russu	%^MAGENTA%^
rusul	%^BLACK%^%^B_MAGENTA%^
ruthe	%^BLACK%^%^B_RED%^
ryan	%^CYAN%^
rylo	%^BLACK%^%^B_YELLOW%^
rynthas	%^YELLOW%^
rystal	%^WHITE%^%^B_MAGENTA%^
ryuo	%^BLACK%^%^B_YELLOW%^
ryzoth	%^YELLOW%^
saaur	%^WHITE%^
sabach	%^BLACK%^%^B_GREEN%^
sabbathiel	%^ORANGE%^
sabreur	%^MAGENTA%^%^BOLD%^
sakari	%^RED%^%^BOLD%^
sakashima	%^RED%^
sako	%^WHITE%^%^B_BLUE%^
salabena	%^CYAN%^
salanor	%^BLACK%^%^B_CYAN%^
salem	%^BLACK%^%^B_GREEN%^
salin	%^RED%^
salius	%^GREEN%^%^BOLD%^
salvatore	%^RED%^%^BOLD%^
salwork	%^MAGENTA%^%^BOLD%^
samantha	%^GREEN%^
sampedro	%^WHITE%^
samson	%^BLACK%^%^B_MAGENTA%^
samuel	%^YELLOW%^
sapid	%^BLACK%^%^B_MAGENTA%^
sapidlib	%^YELLOW%^
saquivor	%^WHITE%^%^B_GREEN%^
saradac	%^WHITE%^%^B_BLUE%^
sarak	%^BLUE%^%^BOLD%^
saraneth	%^MAGENTA%^
sarkkan	%^GREEN%^
sartok	%^ORANGE%^
sarulen	%^WHITE%^%^B_RED%^
sasquatch	%^WHITE%^
sasuke	%^BLUE%^%^BOLD%^
saul	%^WHITE%^%^BOLD%^
sbaitso	%^CYAN%^
scary face	%^RED%^
scaryface	%^BLACK%^%^BOLD%^
scathach	%^MAGENTA%^%^BOLD%^
sceir choireeil	%^RED%^
scifi	%^WHITE%^
scintill	%^BLACK%^%^B_GREEN%^
scorch	%^MAGENTA%^%^BOLD%^
scorpious	%^WHITE%^%^B_MAGENTA%^
scott	%^BLACK%^%^B_WHITE%^
scout	%^CYAN%^
scouter	%^WHITE%^%^B_BLUE%^
scoyn	%^WHITE%^%^B_RED%^
scream	%^CYAN%^%^BOLD%^
scrubb	%^WHITE%^%^B_RED%^
scruff	%^WHITE%^%^B_BLUE%^
sead	%^BLACK%^%^B_CYAN%^
seerlock	%^MAGENTA%^
sekh	%^GREEN%^
sekkite	%^WHITE%^%^BOLD%^
sektor	%^BLUE%^
selena	%^ORANGE%^
selwyn	%^WHITE%^%^B_RED%^
semei	%^WHITE%^%^B_BLUE%^
senorlarn	%^CYAN%^%^BOLD%^
seoman	%^BLACK%^%^B_MAGENTA%^
serack	%^MAGENTA%^%^BOLD%^
seraku	%^BLACK%^%^B_MAGENTA%^
seraph	%^BLACK%^%^B_GREEN%^
serenity	%^WHITE%^%^B_MAGENTA%^
sergio	%^BLACK%^%^B_YELLOW%^
serimia	%^WHITE%^
set	%^WHITE%^%^B_MAGENTA%^
setzer	%^MAGENTA%^%^BOLD%^
seuss	%^BLACK%^%^B_CYAN%^
seva	%^GREEN%^%^BOLD%^
seven	%^RED%^%^BOLD%^
shabradingo	%^MAGENTA%^
shademaster	%^BLACK%^%^B_RED%^
shadimar	%^RED%^%^BOLD%^
shadow	%^CYAN%^%^BOLD%^
shadownoichi	%^BLACK%^%^B_MAGENTA%^
shadowseeker	%^WHITE%^%^B_GREEN%^
shadox	%^MAGENTA%^%^BOLD%^
shadyfake	%^WHITE%^%^B_BLUE%^
shadyman	%^ORANGE%^
shaggy	%^BLACK%^%^B_CYAN%^
shak	%^WHITE%^%^B_GREEN%^
shamira	%^BLACK%^%^B_CYAN%^
shandara	%^GREEN%^%^BOLD%^
shandrilla	%^ORANGE%^
shaned	%^WHITE%^%^B_BLUE%^
shanrou	%^BLACK%^%^B_MAGENTA%^
shard	%^WHITE%^
shattuc	%^ORANGE%^
shayna	%^BLACK%^%^B_GREEN%^
shea	%^RED%^%^BOLD%^
sheanar	%^YELLOW%^
shem	%^BLACK%^%^B_YELLOW%^
shentino	%^BLUE%^
sheril	%^RED%^%^BOLD%^
shhasum	%^BLACK%^%^BOLD%^
shifter	%^WHITE%^
shigs	%^BLACK%^%^B_GREEN%^
shinde	%^GREEN%^%^BOLD%^
shinji	%^BLACK%^%^BOLD%^
shinobi	%^BLACK%^%^B_WHITE%^
shio	%^WHITE%^%^B_GREEN%^
shitfingers	%^WHITE%^%^B_RED%^
shodan	%^WHITE%^%^B_GREEN%^
shu	%^BLACK%^%^B_RED%^
shunamaji	%^WHITE%^%^B_MAGENTA%^
sicarius	%^WHITE%^%^B_RED%^
sickgut	%^BLACK%^%^B_WHITE%^
sid	%^BLACK%^%^B_CYAN%^
sidnee	%^BLACK%^%^B_RED%^
sifu	%^BLUE%^
sigee	%^WHITE%^
sighfigh	%^BLUE%^
sigil	%^BLUE%^%^BOLD%^
siilaan	%^BLACK%^%^B_WHITE%^
silbago	%^BLACK%^%^B_YELLOW%^
silbago doom	%^BLUE%^
silencher	%^BLACK%^%^B_YELLOW%^
silent	%^BLUE%^%^BOLD%^
silenus	%^WHITE%^%^B_GREEN%^
silvanus	%^ORANGE%^
silverblood	%^CYAN%^%^BOLD%^
silverware	%^ORANGE%^
silvyar	%^WHITE%^%^B_MAGENTA%^
sim	%^WHITE%^%^B_RED%^
simbion	%^BLACK%^%^B_WHITE%^
sined	%^BLACK%^%^B_MAGENTA%^
sinistrad	%^MAGENTA%^
sionnach	%^WHITE%^
sipher	%^WHITE%^%^B_BLUE%^
sir-hack.freenode	%^WHITE%^
sircuri	%^CYAN%^%^BOLD%^
sirdar	%^YELLOW%^
sirdude	%^BLACK%^%^B_WHITE%^
sisko	%^BLUE%^%^BOLD%^
skiboo	%^BLACK%^%^B_YELLOW%^
skitzo	%^BLACK%^%^B_WHITE%^
skiver	%^BLACK%^%^B_WHITE%^
skout	%^GREEN%^
skrin	%^MAGENTA%^
skrylar	%^BLACK%^%^B_RED%^
skullslayer	%^BLACK%^%^B_RED%^
skunkaroony	%^GREEN%^%^BOLD%^
sky	%^WHITE%^
skye	%^BLACK%^%^B_GREEN%^
skyfire	%^WHITE%^%^B_RED%^
skynet	%^GREEN%^
skysports	%^MAGENTA%^%^BOLD%^
slashdot	%^GREEN%^
slayer	%^BLACK%^%^B_WHITE%^
slayradio	%^BLUE%^
sledge	%^BLACK%^%^B_CYAN%^
sleffie	%^BLACK%^%^B_YELLOW%^
sleipnir	%^CYAN%^
slikk	%^RED%^%^BOLD%^
slip	%^BLUE%^%^BOLD%^
slitwristz	%^BLACK%^%^B_CYAN%^
sluggy	%^MAGENTA%^%^BOLD%^
slush	%^WHITE%^%^BOLD%^
slymenstra	%^RED%^%^BOLD%^
slynick	%^RED%^
smartie	%^WHITE%^%^B_BLUE%^
smaugmuds	%^BLACK%^%^B_WHITE%^
smdm	%^GREEN%^%^BOLD%^
smegg	%^WHITE%^%^B_GREEN%^
smith	%^RED%^
snert	%^GREEN%^
snivell	%^YELLOW%^
snow	%^WHITE%^%^B_BLUE%^
snowy	%^BLACK%^%^B_WHITE%^
snoyl	%^CYAN%^%^BOLD%^
so-so.freenode	%^YELLOW%^
sobowen	%^BLACK%^%^B_RED%^
sojan	%^BLUE%^
solaris	%^WHITE%^%^B_BLUE%^
solia	%^WHITE%^
solo	%^WHITE%^%^BOLD%^
solomirath	%^RED%^%^BOLD%^
solstarn	%^WHITE%^%^B_RED%^
soludix	%^WHITE%^%^B_GREEN%^
somebody	%^MAGENTA%^
someone	%^RED%^
somerville32.freenode	%^RED%^%^BOLD%^
somnar	%^MAGENTA%^
somotaw	%^WHITE%^%^B_MAGENTA%^
sonyc	%^WHITE%^%^B_GREEN%^
sorbitol	%^RED%^
sorin	%^WHITE%^%^B_RED%^
sorisor	%^WHITE%^
sorressean	%^CYAN%^%^BOLD%^
sos	%^WHITE%^%^BOLD%^
sothis	%^BLACK%^%^B_RED%^
space.com	%^BLACK%^%^B_CYAN%^
spamspanker	%^GREEN%^%^BOLD%^
sparhawk	%^BLACK%^%^B_GREEN%^
sparkles	%^BLACK%^%^B_RED%^
spartacus	%^WHITE%^%^BOLD%^
spawn	%^WHITE%^%^B_BLUE%^
spindle	%^BLACK%^%^B_RED%^
splorch	%^ORANGE%^
spontaneous	%^ORANGE%^
spontiff	%^BLACK%^%^B_YELLOW%^
spoons	%^YELLOW%^
spoonster	%^WHITE%^%^B_GREEN%^
spoony	%^WHITE%^%^B_BLUE%^
sprudleglut	%^BLACK%^%^B_MAGENTA%^
sprynx	%^GREEN%^%^BOLD%^
squ	%^BLACK%^%^BOLD%^
squid	%^GREEN%^%^BOLD%^
sryth	%^WHITE%^
ss	%^BLUE%^
stan	%^WHITE%^%^B_MAGENTA%^
stanach	%^BLACK%^%^B_YELLOW%^
stanley	%^WHITE%^%^B_BLUE%^
star	%^GREEN%^%^BOLD%^
starbuck	%^WHITE%^%^B_RED%^
stardust	%^WHITE%^%^BOLD%^
stark	%^MAGENTA%^%^BOLD%^
starker	%^WHITE%^%^BOLD%^
starr	%^BLUE%^
stavros	%^BLACK%^%^B_RED%^
stephen	%^BLACK%^%^B_MAGENTA%^
sterm	%^ORANGE%^
stern	%^BLACK%^%^BOLD%^
stofilia	%^BLACK%^%^BOLD%^
stoli	%^GREEN%^
straightwire	%^BLACK%^%^B_WHITE%^
strikehunter	%^GREEN%^
styx	%^BLACK%^%^BOLD%^
styxx	%^WHITE%^%^B_RED%^
subversion	%^WHITE%^%^B_RED%^
sumsum	%^RED%^%^BOLD%^
sunaris	%^BLACK%^%^BOLD%^
sunbeam	%^MAGENTA%^%^BOLD%^
supra	%^RED%^
supramud	%^BLACK%^%^B_YELLOW%^
surt	%^ORANGE%^
suskaforn	%^BLUE%^
swan	%^WHITE%^%^B_BLUE%^
swyrd	%^WHITE%^%^B_RED%^
sydney	%^BLACK%^%^B_RED%^
sylvan	%^WHITE%^%^B_BLUE%^
sylvani	%^WHITE%^%^BOLD%^
sylvie	%^GREEN%^
symon	%^GREEN%^
syn	%^BLACK%^%^B_YELLOW%^
syphon	%^CYAN%^%^BOLD%^
syrron	%^BLACK%^%^B_RED%^
sys	%^RED%^
system	%^MAGENTA%^
tab	%^MAGENTA%^%^BOLD%^
tacitus	%^BLACK%^%^B_WHITE%^
taepha	%^BLACK%^%^B_YELLOW%^
taffyd	%^GREEN%^
tahin	%^WHITE%^%^B_BLUE%^
taint	%^MAGENTA%^
takahari	%^CYAN%^%^BOLD%^
takhisis	%^BLACK%^%^B_YELLOW%^
tal'anis	%^GREEN%^%^BOLD%^
talanthis	%^WHITE%^%^B_BLUE%^
talder	%^BLACK%^%^B_GREEN%^
talen	%^BLACK%^%^B_RED%^
tales	%^RED%^
talge	%^BLACK%^%^BOLD%^
tali	%^RED%^
talidarania	%^CYAN%^%^BOLD%^
taliesin	%^MAGENTA%^%^BOLD%^
talin	%^CYAN%^%^BOLD%^
talis	%^BLACK%^%^B_CYAN%^
talli	%^CYAN%^
tallman	%^WHITE%^%^B_RED%^
talsin	%^BLACK%^%^B_GREEN%^
talven	%^WHITE%^%^B_MAGENTA%^
tamesis	%^CYAN%^
tamsin	%^GREEN%^
tanar	%^BLACK%^%^B_GREEN%^
tangescence	%^WHITE%^%^B_MAGENTA%^
tanis	%^BLUE%^
tanya	%^WHITE%^
tapeworm	%^WHITE%^%^B_MAGENTA%^
taplash	%^WHITE%^%^B_RED%^
tarathiel	%^BLACK%^%^B_RED%^
tareek	%^BLACK%^%^B_MAGENTA%^
tarken	%^BLACK%^%^BOLD%^
tarlyn	%^GREEN%^%^BOLD%^
tarr	%^BLACK%^%^B_RED%^
tartaros	%^BLACK%^%^B_MAGENTA%^
taruska	%^MAGENTA%^%^BOLD%^
tarvos	%^MAGENTA%^
tasm	%^BLACK%^%^B_WHITE%^
tatianna	%^MAGENTA%^
taure	%^WHITE%^%^B_MAGENTA%^
tauron	%^BLACK%^%^B_RED%^
tavish	%^GREEN%^
tay	%^CYAN%^%^BOLD%^
tbamud	%^BLACK%^%^B_RED%^
tea	%^GREEN%^
teknion	%^BLACK%^%^B_YELLOW%^
teknovyking	%^RED%^%^BOLD%^
telchur	%^BLACK%^%^B_YELLOW%^
tempest	%^WHITE%^%^B_GREEN%^
tempus	%^BLACK%^%^B_CYAN%^
temujin	%^CYAN%^%^BOLD%^
temuthril	%^GREEN%^%^BOLD%^
tenjin	%^BLACK%^%^B_MAGENTA%^
tenny	%^BLACK%^%^B_GREEN%^
teny	%^MAGENTA%^
tera	%^BLACK%^%^B_MAGENTA%^
terano	%^MAGENTA%^%^BOLD%^
terra	%^WHITE%^%^B_MAGENTA%^
terraco	%^CYAN%^
test	%^BLACK%^%^B_MAGENTA%^
test123344321_irc	%^BLUE%^
testchar	%^BLACK%^%^B_MAGENTA%^
testcharfour	%^WHITE%^%^B_RED%^
testcre	%^BLUE%^%^BOLD%^
tester	%^MAGENTA%^
testham	%^ORANGE%^
testhamlet	%^BLACK%^%^B_RED%^
testwiz	%^GREEN%^%^BOLD%^
testycre	%^BLUE%^
testylus	%^BLACK%^%^B_MAGENTA%^
tetrikitty	%^MAGENTA%^%^BOLD%^
teveshszat	%^WHITE%^%^B_RED%^
thalann	%^BLACK%^%^B_WHITE%^
thalasso	%^WHITE%^
thales	%^BLUE%^%^BOLD%^
thanatos	%^BLUE%^
thanitos	%^BLACK%^%^BOLD%^
tharios	%^GREEN%^
the bobcat	%^RED%^
the ethereal presence	%^RED%^%^BOLD%^
the jarred	%^BLACK%^%^B_WHITE%^
the kobold	%^BLACK%^%^B_GREEN%^
thebird	%^RED%^
thecreator	%^CYAN%^%^BOLD%^
thegrauniad	%^BLACK%^%^B_YELLOW%^
theinquirer	%^BLACK%^%^B_WHITE%^
theis	%^WHITE%^%^B_GREEN%^
theman	%^MAGENTA%^
theodosius	%^GREEN%^
theonion	%^WHITE%^%^B_MAGENTA%^
theophage	%^GREEN%^%^BOLD%^
theramon	%^YELLOW%^
theregister	%^RED%^
thian	%^YELLOW%^
thingol	%^WHITE%^%^BOLD%^
thirsha	%^GREEN%^%^BOLD%^
thog	%^MAGENTA%^%^BOLD%^
thoin	%^WHITE%^%^BOLD%^
thor	%^WHITE%^%^B_RED%^
thoreau	%^BLACK%^%^B_WHITE%^
thoreksken	%^GREEN%^%^BOLD%^
thorgal	%^WHITE%^%^B_GREEN%^
thrakkozz	%^GREEN%^
three	%^BLACK%^%^BOLD%^
thrym	%^GREEN%^
thulin	%^BLACK%^%^B_MAGENTA%^
thulsa	%^BLACK%^%^B_MAGENTA%^
thulsa doom	%^RED%^%^BOLD%^
thyrr	%^BLACK%^%^B_GREEN%^
tiamat	%^BLUE%^%^BOLD%^
tibalt	%^BLACK%^%^BOLD%^
tibet	%^WHITE%^%^B_BLUE%^
tibolt	%^BLACK%^%^B_GREEN%^
tidal	%^BLACK%^%^B_CYAN%^
tigran	%^WHITE%^%^B_BLUE%^
tigwyk	%^BLACK%^%^B_YELLOW%^
tijer	%^CYAN%^%^BOLD%^
tijntje	%^WHITE%^%^B_MAGENTA%^
tim	%^RED%^%^BOLD%^
timbo	%^WHITE%^%^B_BLUE%^
timbot	%^RED%^
tinfoilhat	%^CYAN%^%^BOLD%^
tique	%^WHITE%^%^B_GREEN%^
tisane	%^WHITE%^%^B_RED%^
titan	%^BLACK%^%^BOLD%^
tiz	%^CYAN%^%^BOLD%^
tizen	%^MAGENTA%^%^BOLD%^
tkkyj	%^BLACK%^%^B_YELLOW%^
tmc	%^WHITE%^%^BOLD%^
tms	%^BLACK%^%^B_GREEN%^
tms_feed	%^CYAN%^%^BOLD%^
toast	%^MAGENTA%^%^BOLD%^
toaster	%^WHITE%^%^BOLD%^
toffeecake	%^BLACK%^%^B_GREEN%^
tokie	%^ORANGE%^
tomeglow	%^CYAN%^
tomio	%^BLUE%^
toneloc	%^WHITE%^
toninno	%^BLACK%^%^B_RED%^
tony	%^BLACK%^%^B_YELLOW%^
torak	%^WHITE%^%^BOLD%^
torako	%^WHITE%^%^B_RED%^
toric	%^ORANGE%^
torture	%^WHITE%^%^B_MAGENTA%^
toshinama	%^BLACK%^%^B_YELLOW%^
towns	%^BLACK%^%^B_WHITE%^
tozan	%^WHITE%^%^BOLD%^
tragard	%^WHITE%^%^B_RED%^
tranin	%^BLACK%^%^BOLD%^
traveller	%^BLACK%^%^B_WHITE%^
traven	%^CYAN%^
treble	%^MAGENTA%^%^BOLD%^
trebor	%^MAGENTA%^
tria	%^BLUE%^
tricky	%^YELLOW%^
tricky bows to someone	%^MAGENTA%^
tricky kicks cratylus	%^WHITE%^%^B_RED%^
tricky kills drakkos	%^WHITE%^%^B_MAGENTA%^
tricky looks at shadyman	%^BLACK%^%^B_GREEN%^
tricky pats tigwyk	%^WHITE%^%^B_MAGENTA%^
tricky pats yeik	%^MAGENTA%^%^BOLD%^
tricky.freenode	%^GREEN%^%^BOLD%^
tricky1702	%^BLACK%^%^B_MAGENTA%^
trilogy	%^WHITE%^%^B_GREEN%^
trimaster	%^BLACK%^%^B_YELLOW%^
trip	%^GREEN%^
triq	%^MAGENTA%^
triskal	%^WHITE%^%^BOLD%^
tristan	%^BLACK%^%^B_CYAN%^
tristen	%^RED%^%^BOLD%^
tritan	%^YELLOW%^
triviabot	%^CYAN%^
trix	%^CYAN%^%^BOLD%^
trixar	%^GREEN%^
trixen	%^GREEN%^%^BOLD%^
troponin	%^BLUE%^%^BOLD%^
truk77	%^RED%^
trusty	%^WHITE%^
tsarenzi	%^GREEN%^%^BOLD%^
tsion	%^BLACK%^%^B_RED%^
tsme	%^BLUE%^
tsonu	%^WHITE%^%^B_GREEN%^
tuan	%^MAGENTA%^
tuck	%^BLACK%^%^B_CYAN%^
tulkas	%^CYAN%^
tumbug	%^RED%^%^BOLD%^
tumeski	%^YELLOW%^
turk	%^WHITE%^
turkey	%^WHITE%^%^B_GREEN%^
turok	%^WHITE%^
turvity	%^CYAN%^
tux	%^WHITE%^
two	%^RED%^
txiv	%^ORANGE%^
tyler	%^CYAN%^
typhon	%^YELLOW%^
tyre	%^WHITE%^%^B_MAGENTA%^
tyrin	%^WHITE%^%^B_BLUE%^
tyson	%^RED%^%^BOLD%^
tywywllch	%^WHITE%^
tzekiat	%^BLACK%^%^B_CYAN%^
uae	%^BLACK%^%^B_WHITE%^
ugh	%^WHITE%^%^BOLD%^
ulmo	%^GREEN%^%^BOLD%^
ulrik	%^RED%^%^BOLD%^
ulysses	%^WHITE%^%^B_RED%^
umbramancer	%^BLACK%^%^BOLD%^
umbrella wizard	%^BLACK%^%^B_CYAN%^
uneche	%^GREEN%^%^BOLD%^
unicoi	%^MAGENTA%^
unimatrix	%^BLACK%^%^B_CYAN%^
unloved	%^WHITE%^
unslidge	%^WHITE%^%^B_RED%^
urien	%^YELLOW%^
urlbot	%^YELLOW%^%^B_BLUE%^
uurdin	%^BLACK%^%^B_YELLOW%^
uxue	%^WHITE%^%^B_BLUE%^
vaencor	%^BLACK%^%^B_CYAN%^
val	%^BLUE%^
valandel	%^RED%^%^BOLD%^
valen	%^BLACK%^%^B_YELLOW%^
valentino	%^WHITE%^%^B_GREEN%^
valerian	%^MAGENTA%^
valerin	%^BLUE%^
valkor	%^GREEN%^
valkorian	%^WHITE%^%^BOLD%^
vance	%^RED%^%^BOLD%^
vanderbos	%^BLACK%^%^B_RED%^
vandretta	%^WHITE%^%^BOLD%^
vang	%^BLACK%^%^B_WHITE%^
vanilor	%^BLACK%^%^B_RED%^
vardogr	%^WHITE%^%^BOLD%^
vargon the insane	%^BLACK%^%^B_GREEN%^
varmel	%^WHITE%^
varreus	%^ORANGE%^
varu	%^MAGENTA%^
vash	%^BLUE%^
vashta	%^BLUE%^
vashti	%^BLACK%^%^B_YELLOW%^
vayl	%^MAGENTA%^
vega	%^MAGENTA%^%^BOLD%^
velmin	%^BLUE%^%^BOLD%^
venar	%^BLACK%^%^B_YELLOW%^
vengence	%^GREEN%^
venoch	%^MAGENTA%^
verement	%^BLACK%^%^BOLD%^
vernius	%^BLACK%^%^BOLD%^
veros	%^BLACK%^%^B_WHITE%^
viakla	%^YELLOW%^
victor	%^BLUE%^%^BOLD%^
vikiq	%^WHITE%^%^B_MAGENTA%^
viktor	%^ORANGE%^
vilci	%^BLACK%^%^BOLD%^
vilhien	%^BLACK%^%^B_MAGENTA%^
villanus	%^WHITE%^
vinez	%^CYAN%^%^BOLD%^
viper	%^BLACK%^%^B_CYAN%^
virax	%^YELLOW%^
viriato	%^WHITE%^
virus	%^GREEN%^
viscous	%^BLUE%^
visenya	%^CYAN%^
vivi	%^CYAN%^%^BOLD%^
vivid	%^BLACK%^%^B_CYAN%^
viwiel	%^RED%^%^BOLD%^
vlad	%^WHITE%^%^B_RED%^
vladaar	%^BLACK%^%^BOLD%^
vleemad	%^BLUE%^%^BOLD%^
volen	%^CYAN%^
volk	%^BLACK%^%^BOLD%^
volkov	%^YELLOW%^
volothamp	%^CYAN%^
vonkeebler	%^WHITE%^%^B_MAGENTA%^
vorlin	%^BLACK%^%^B_MAGENTA%^
vorpal	%^BLACK%^%^B_GREEN%^
vorric	%^RED%^
vortex	%^BLACK%^%^B_MAGENTA%^
vott	%^BLACK%^%^B_RED%^
vova	%^BLACK%^%^BOLD%^
vox	%^MAGENTA%^%^BOLD%^
voxoo	%^CYAN%^
vuleive	%^BLACK%^%^B_MAGENTA%^
vulture	%^WHITE%^%^BOLD%^
vulusion	%^BLUE%^
vx	%^BLACK%^%^BOLD%^
vyuk	%^ORANGE%^
wacko	%^RED%^
wade	%^CYAN%^
wallsy	%^YELLOW%^
walrii	%^WHITE%^%^BOLD%^
warcas	%^CYAN%^%^BOLD%^
warfal	%^GREEN%^
warh	%^RED%^
warhammer	%^BLACK%^%^B_GREEN%^
warmonger	%^BLACK%^%^B_YELLOW%^
warper	%^RED%^%^BOLD%^
wasdrogan	%^BLACK%^%^B_GREEN%^
watcher	%^CYAN%^%^BOLD%^
wayyne	%^MAGENTA%^%^BOLD%^
weary	%^BLACK%^%^B_WHITE%^
wedric	%^BLUE%^%^BOLD%^
weeks	%^BLUE%^%^BOLD%^
weiqi	%^WHITE%^%^B_MAGENTA%^
well blow me down! reflection	%^CYAN%^
werehamster	%^BLACK%^%^B_GREEN%^
westley	%^BLACK%^%^B_CYAN%^
whisper	%^BLACK%^%^B_GREEN%^
whisperwillow	%^WHITE%^%^B_MAGENTA%^
whome	%^WHITE%^%^B_RED%^
why	%^WHITE%^%^B_MAGENTA%^
wick	%^MAGENTA%^%^BOLD%^
wierd	%^BLACK%^%^B_GREEN%^
wildchild	%^MAGENTA%^
will	%^BLUE%^
william	%^BLACK%^%^B_YELLOW%^
willot	%^RED%^
willy	%^BLUE%^%^BOLD%^
windel	%^BLACK%^%^BOLD%^
windigo	%^BLUE%^
windrider	%^MAGENTA%^%^BOLD%^
winds	%^MAGENTA%^%^BOLD%^
wintermute	%^WHITE%^%^BOLD%^
wiz	%^YELLOW%^
wizard	%^WHITE%^%^B_GREEN%^
wobble	%^BLACK%^%^B_RED%^
wodan	%^YELLOW%^
wodantest	%^MAGENTA%^
wolf	%^CYAN%^
wolvesbane	%^GREEN%^
womble	%^WHITE%^%^B_BLUE%^
woom	%^CYAN%^%^BOLD%^
wordgod	%^CYAN%^
world as myth	%^BLUE%^
wow	%^YELLOW%^
wyre	%^GREEN%^
xadrian	%^WHITE%^%^BOLD%^
xalin	%^CYAN%^
xameil	%^WHITE%^
xanieth	%^MAGENTA%^%^BOLD%^
xanthar	%^GREEN%^%^BOLD%^
xar	%^WHITE%^%^B_MAGENTA%^
xarg	%^RED%^%^BOLD%^
xashitetsu	%^BLACK%^%^B_RED%^
xavier	%^BLUE%^%^BOLD%^
xdarkknightx	%^GREEN%^%^BOLD%^
xenophanes	%^BLACK%^%^B_RED%^
xeon	%^YELLOW%^
xerbil	%^BLACK%^%^BOLD%^
xero	%^MAGENTA%^%^BOLD%^
xii	%^BLACK%^%^B_YELLOW%^
xiron	%^BLACK%^%^BOLD%^
xirote	%^WHITE%^%^B_MAGENTA%^
xoarvvath	%^BLACK%^%^B_RED%^
xodeidhiur	%^GREEN%^
xoral	%^ORANGE%^
xytras	%^MAGENTA%^
xyzzy	%^BLUE%^
xzystance	%^RED%^
yady	%^WHITE%^%^B_BLUE%^
yakon	%^MAGENTA%^
yakuza	%^BLACK%^%^B_GREEN%^
yan	%^BLACK%^%^B_YELLOW%^
yanna	%^RED%^%^BOLD%^
yarpsnesan	%^GREEN%^
yase	%^BLACK%^%^B_GREEN%^
yeik	%^BLACK%^%^B_CYAN%^
yen	%^RED%^%^BOLD%^
ylapirrynag	%^WHITE%^%^B_GREEN%^
yldannan	%^BLUE%^%^BOLD%^
yoda	%^WHITE%^
yoinkles	%^RED%^%^BOLD%^
yorkie	%^BLUE%^%^BOLD%^
ythrangil	%^WHITE%^%^BOLD%^
yun	%^YELLOW%^
yuna	%^BLUE%^%^BOLD%^
zac	%^CYAN%^%^BOLD%^
zadious	%^ORANGE%^
zadok	%^BLACK%^%^B_CYAN%^
zaerth	%^CYAN%^
zahir	%^BLACK%^%^B_CYAN%^
zakarowyn	%^CYAN%^
zaknafein	%^BLACK%^%^B_YELLOW%^
zalbar	%^WHITE%^%^B_RED%^
zanadu	%^BLACK%^%^B_CYAN%^
zanatos	%^BLACK%^%^B_GREEN%^
zaphod	%^CYAN%^
zapper	%^BLACK%^%^B_YELLOW%^
zari	%^BLUE%^%^BOLD%^
zaroth	%^WHITE%^%^BOLD%^
zecred	%^BLACK%^%^B_MAGENTA%^
zed	%^BLACK%^%^B_MAGENTA%^
zeddicus	%^ORANGE%^
zedzo	%^BLACK%^%^B_YELLOW%^
zeke	%^MAGENTA%^
zell	%^WHITE%^%^B_RED%^
zendakath	%^GREEN%^
zengo	%^WHITE%^%^B_BLUE%^
zenik	%^CYAN%^
zennik	%^CYAN%^
zeno	%^ORANGE%^
zephran	%^WHITE%^%^B_MAGENTA%^
zephyros	%^CYAN%^%^BOLD%^
zero	%^BLACK%^%^B_WHITE%^
zeron	%^BLACK%^%^B_GREEN%^
zeus	%^MAGENTA%^
zhaan	%^WHITE%^%^BOLD%^
zhouxu	%^CYAN%^
zhuuraan	%^WHITE%^%^B_MAGENTA%^
zifnab	%^WHITE%^
zigzag	%^WHITE%^%^B_RED%^
zilch	%^RED%^%^BOLD%^
zion	%^GREEN%^%^BOLD%^
zmax	%^MAGENTA%^%^BOLD%^
zod	%^CYAN%^%^BOLD%^
zoe	%^RED%^
zolomon	%^RED%^%^BOLD%^
zolvranzix	%^MAGENTA%^%^BOLD%^
zombie	%^WHITE%^%^B_GREEN%^
zonbi	%^CYAN%^
zorn	%^CYAN%^%^BOLD%^
zorn, the old mage	%^GREEN%^
zrutu	%^WHITE%^%^B_MAGENTA%^
ztest	%^RED%^%^BOLD%^
zube	%^BLACK%^%^B_WHITE%^
zvim	%^WHITE%^%^B_GREEN%^
zyll	%^CYAN%^%^BOLD%^
zyren	%^MAGENTA%^
zyx	%^BLACK%^%^BOLD%^
§4§4tricky1702§r	%^BLACK%^%^B_GREEN%^
ãããã ã	%^BLACK%^%^B_WHITE%^
brainy	%^WHITE%^
ztilly	%^CYAN%^%^BOLD%^
flypadre	%^GREEN%^%^BOLD%^
warpdongle	%^YELLOW%^
etranger	%^MAGENTA%^%^BOLD%^
carbon	%^BLUE%^%^BOLD%^
chilidog	%^CYAN%^%^BOLD%^
ceazer	%^WHITE%^%^BOLD%^
corneliusv	%^BLACK%^%^BOLD%^
memnon	%^RED%^
muirrum	%^GREEN%^
kikusadaru no yuurie	%^BLACK%^%^B_YELLOW%^
kikusadaru no yuurei	%^BLACK%^%^B_WHITE%^
キクサダルの幽霊	%^BLACK%^%^BOLD%^
8=====d	%^RED%^
8=====d↝↝ ↝	%^GREEN%^
akana	%^ORANGE%^
kaon	%^BLUE%^
reno	%^MAGENTA%^
amariner	%^CYAN%^
a mariner	%^WHITE%^
mariner	%^WHITE%^
gesslar	%^RED%^%^BOLD%^
sarmonsiill	%^GREEN%^%^BOLD%^
gabrio	%^YELLOW%^
mrwolf	%^MAGENTA%^%^BOLD%^
succubus	%^BLUE%^%^BOLD%^
kyrrodi	%^CYAN%^%^BOLD%^
catbro	%^WHITE%^%^BOLD%^
lifeless	%^BLACK%^%^BOLD%^
swiper	%^RED%^
xrakisis	%^GREEN%^
visceral	%^ORANGE%^
curious	%^BLUE%^
ramuki	%^MAGENTA%^
chaostriad	%^CYAN%^
notnull	%^WHITE%^
tsath	%^RED%^%^BOLD%^
suturb	%^GREEN%^%^BOLD%^
the real quixadhal	%^RED%^
pridda	%^YELLOW%^
ace	%^MAGENTA%^%^BOLD%^
amazon	%^BLUE%^%^BOLD%^
exash	%^CYAN%^%^BOLD%^
cash	%^WHITE%^%^BOLD%^
troll	%^BLACK%^%^BOLD%^
rek	%^RED%^
eldermeer	%^GREEN%^
helm	%^ORANGE%^
kogar	%^BLUE%^
ignoredguy	%^BLUE%^%^BOLD%^
fire	%^MAGENTA%^
crysrothe	%^CYAN%^
majin	%^WHITE%^
ithagua	%^WHITE%^%^B_GREEN%^
gal	%^RED%^%^BOLD%^
nitupsar	%^GREEN%^%^BOLD%^
slypaw	%^YELLOW%^
dove	%^MAGENTA%^%^BOLD%^
yyy	%^BLUE%^%^BOLD%^
yucongsun	%^CYAN%^%^BOLD%^
yucong sun	%^BLACK%^%^B_YELLOW%^
circe	%^WHITE%^%^BOLD%^
galgorin	%^BLACK%^%^BOLD%^
pyra	%^RED%^
tcikoritys	%^GREEN%^
atheistx	%^ORANGE%^
tlny	%^BLUE%^
ab	%^MAGENTA%^
ageor	%^CYAN%^
mithyr	%^WHITE%^
ark	%^RED%^%^BOLD%^
storyhost shentino	%^GREEN%^%^BOLD%^
neimad	%^GREEN%^%^BOLD%^
amerikranian	%^YELLOW%^
methelius	%^MAGENTA%^%^BOLD%^
fyra	%^BLUE%^%^BOLD%^
emperor trumpu	%^WHITE%^%^BOLD%^
left	%^CYAN%^%^BOLD%^
tlaloc	%^WHITE%^%^BOLD%^
syriac	%^BLACK%^%^BOLD%^
galahad	%^RED%^
liara	%^GREEN%^
stormy	%^ORANGE%^
darb	%^BLUE%^
darih	%^MAGENTA%^
xephon	%^CYAN%^
xola	%^WHITE%^
maldrin	%^RED%^%^BOLD%^
kathaaa	%^GREEN%^%^BOLD%^
quixadhal	%^BLACK%^%^B_GREEN%^
babylon	%^YELLOW%^
creepy joe	%^ORANGE%^
'admin	%^BLUE%^
grym	%^MAGENTA%^%^BOLD%^
fizzle	%^BLUE%^%^BOLD%^
deisz	%^CYAN%^%^BOLD%^
jakalas	%^WHITE%^%^BOLD%^
virtual	%^BLACK%^%^BOLD%^
zealxp	%^RED%^
rodgerly	%^GREEN%^
wolves	%^ORANGE%^
korsario	%^BLUE%^
bambooshoots	%^MAGENTA%^
figgs	%^CYAN%^
tok	%^WHITE%^
esmene	%^RED%^%^BOLD%^
wizzard	%^GREEN%^%^BOLD%^
nikavanova	%^YELLOW%^
tbonta	%^MAGENTA%^%^BOLD%^
selfe	%^BLUE%^%^BOLD%^
atheos	%^CYAN%^%^BOLD%^
steve	%^WHITE%^%^BOLD%^
croaker	%^BLACK%^%^BOLD%^
johnny (sindome in gv)	%^RED%^
viriato (grapevine in gv)	%^GREEN%^
beandip (sindome in gv)	%^ORANGE%^
reefermadness (sindome in gv)	%^BLUE%^
stiza13 (sindome in gv)	%^MAGENTA%^
villa (sindome in gv)	%^CYAN%^
mench	%^WHITE%^
mench (sindome in gv)	%^WHITE%^
mitsuko	%^RED%^%^BOLD%^
crisis (neonmoo in gv)	%^GREEN%^%^BOLD%^
mitsuko (neonmoo in gv)	%^YELLOW%^
blizzard	%^GREEN%^%^BOLD%^
blizzard (wop in gv)	%^BLUE%^%^BOLD%^
stiza13	%^YELLOW%^
kisaki	%^MAGENTA%^%^BOLD%^
kisaki (sindome in gv)	%^WHITE%^%^B_RED%^
johnny	%^BLUE%^%^BOLD%^
ryuzaki4days	%^CYAN%^%^BOLD%^
ryuzaki4days (sindome in gv)	%^WHITE%^%^B_MAGENTA%^
bonehead	%^WHITE%^%^BOLD%^
bonehead (sindome in gv)	%^BLACK%^%^B_GREEN%^
baguette	%^BLACK%^%^BOLD%^
baguette (sindome in gv)	%^BLACK%^%^B_CYAN%^
red	%^RED%^
red (neonmoo in gv)	%^BLACK%^%^B_WHITE%^
dgleks (dgleksmoo in gv)	%^BLACK%^%^BOLD%^
lowell	%^GREEN%^
hanarra	%^ORANGE%^
driznit	%^BLUE%^
wiebman	%^MAGENTA%^
wiebman (sindome in gv)	%^MAGENTA%^
ameliabrooks	%^CYAN%^
zrothum	%^WHITE%^
zrothum (grapevine in gv)	%^RED%^%^BOLD%^
codesmith	%^RED%^%^BOLD%^
codesmith (smithymoo in gv)	%^YELLOW%^
rhea	%^GREEN%^%^BOLD%^
rhea (sindome in gv)	%^BLUE%^%^BOLD%^
enker	%^YELLOW%^
enker (ef in gv)	%^WHITE%^%^BOLD%^
urlbot3000	%^MAGENTA%^%^BOLD%^
urlbot 3000	%^WHITE%^%^B_GREEN%^
angadar	%^BLUE%^%^BOLD%^
isla	%^CYAN%^%^BOLD%^
elrad	%^WHITE%^%^BOLD%^
elrad (dev in gv)	%^BLACK%^%^B_GREEN%^
something_wicked	%^BLACK%^%^BOLD%^
something_wicked (sindome in gv)	%^BLACK%^%^B_CYAN%^
fengshui	%^RED%^
fengshui (sindome in gv)	%^BLACK%^%^B_WHITE%^
miff	%^GREEN%^
jensen	%^ORANGE%^
raycaster	%^BLUE%^
raycaster (grapevine in gv)	%^ORANGE%^
an attack droid	%^MAGENTA%^
an attack droid (wop in gv)	%^MAGENTA%^
snagglepants	%^CYAN%^
snagglepants (grapevine in gv)	%^WHITE%^
zaya	%^WHITE%^
spooky9168	%^RED%^%^BOLD%^
spooky9168 (grapevine in gv)	%^YELLOW%^
a scrawny cat	%^GREEN%^%^BOLD%^
a scrawny cat (wop in gv)	%^BLUE%^%^BOLD%^
methos	%^YELLOW%^
kevren	%^MAGENTA%^%^BOLD%^
vestian	%^BLUE%^%^BOLD%^
sandon	%^CYAN%^%^BOLD%^
sandon (myelinalpha in gv)	%^WHITE%^%^B_BLUE%^
vestian (myelinalpha in gv)	%^WHITE%^%^B_MAGENTA%^
annoyance	%^BLACK%^%^B_RED%^
nospeedy	%^WHITE%^%^BOLD%^
nospeedy (sindome in gv)	%^BLACK%^%^B_MAGENTA%^
blkcandy	%^BLACK%^%^BOLD%^
blkcandy (sindome in gv)	%^BLACK%^%^B_YELLOW%^
ikke	%^RED%^
reva	%^GREEN%^
stray	%^ORANGE%^
mirodar	%^BLUE%^
kaedok	%^MAGENTA%^
kaedok (grapevine in gv)	%^BLUE%^
gamemaster	%^CYAN%^
gorgar007	%^WHITE%^
gorgar007 (grapevine in gv)	%^WHITE%^
jude	%^RED%^%^BOLD%^
owner	%^GREEN%^%^BOLD%^
sake	%^YELLOW%^
sake (grapevine in gv)	%^MAGENTA%^%^BOLD%^
tha	%^MAGENTA%^%^BOLD%^
beth	%^BLUE%^%^BOLD%^
yebtelnyal	%^CYAN%^%^BOLD%^
yebtelnyal (myelinalpha in gv)	%^WHITE%^%^B_RED%^
sugarfi	%^WHITE%^%^BOLD%^
jezu	%^BLACK%^%^BOLD%^
reefermadness	%^RED%^
araska	%^GREEN%^
tax	%^ORANGE%^
tax (sindome in gv)	%^BLACK%^%^B_MAGENTA%^
drek	%^BLUE%^
drek (grapevine in gv)	%^BLACK%^%^B_YELLOW%^
arons	%^MAGENTA%^
ashtons	%^CYAN%^
ashtons (grapevine in gv)	%^RED%^
valyn	%^WHITE%^
nikita	%^RED%^%^BOLD%^
nikita (neonmoo in gv)	%^BLUE%^
swnh	%^GREEN%^%^BOLD%^
swnh (grapevine in gv)	%^CYAN%^
mudren	%^YELLOW%^
mudren (grapevine in gv)	%^RED%^%^BOLD%^
elrad (elvindir in gv)	%^GREEN%^%^BOLD%^
xcogi	%^MAGENTA%^%^BOLD%^
xcogi (eugormud in gv)	%^MAGENTA%^%^BOLD%^
ghaleon	%^BLUE%^%^BOLD%^
ghaleon (apotheosis in gv)	%^CYAN%^%^BOLD%^
arithorn (rm in gv)	%^WHITE%^%^BOLD%^
fal	%^CYAN%^%^BOLD%^
inome	%^WHITE%^%^BOLD%^
bruhlicious	%^BLACK%^%^BOLD%^
bruhlicious (sindome in gv)	%^WHITE%^%^B_MAGENTA%^
cheyenne	%^RED%^
cheyenne (grapevine in gv)	%^BLACK%^%^B_GREEN%^
simon	%^GREEN%^
unhappycupcake	%^ORANGE%^
unhappycupcake (grapevine in gv)	%^BLACK%^%^B_YELLOW%^
calevaro	%^BLUE%^
calevaro (neonmoo in gv)	%^BLACK%^%^BOLD%^
cole	%^MAGENTA%^
cole (apotheosisdev in gv)	%^GREEN%^
sulfurado	%^CYAN%^
sulfurado (sindome in gv)	%^BLUE%^
ceayo	%^WHITE%^
fain	%^RED%^%^BOLD%^
delphini	%^GREEN%^%^BOLD%^
aspiringhobo	%^YELLOW%^
aspiring hobo	%^GREEN%^%^BOLD%^
kabalah	%^MAGENTA%^%^BOLD%^
april	%^BLUE%^%^BOLD%^
april (neonmoo in gv)	%^BLUE%^%^BOLD%^
alt	%^CYAN%^%^BOLD%^
cayleath	%^WHITE%^%^BOLD%^
kalahami	%^BLACK%^%^BOLD%^
riker	%^RED%^
amaranth	%^GREEN%^
shaitan	%^ORANGE%^
shaitan (apotheosisdev in gv)	%^BLACK%^%^B_RED%^
scout (apotheosis in gv)	%^BLACK%^%^B_GREEN%^
rhgh	%^BLUE%^
starck	%^MAGENTA%^
leibek	%^CYAN%^
leibek (grapevine in gv)	%^BLACK%^%^B_WHITE%^
naughtymuffin	%^WHITE%^
naughtymuffin (grapevine in gv)	%^RED%^
nightdemon	%^RED%^%^BOLD%^
lynne	%^GREEN%^%^BOLD%^
lynne (grapevine in gv)	%^BLUE%^
captainstormblade	%^YELLOW%^
captainstormblade (grapevine in gv)	%^CYAN%^
a prometheus host	%^MAGENTA%^%^BOLD%^
a prometheus host (prometheus in gv)	%^RED%^%^BOLD%^
gesslar (grapevine in gv)	%^GREEN%^%^BOLD%^
jake (neonmoo in gv)	%^YELLOW%^
tokugawa	%^BLUE%^%^BOLD%^
illari	%^CYAN%^%^BOLD%^
brettkelly	%^WHITE%^%^BOLD%^
brettkelly (grapevine in gv)	%^WHITE%^%^BOLD%^
admin (myelinalpha in gv)	%^WHITE%^%^B_RED%^
izzard	%^BLACK%^%^BOLD%^
rek (grapevine in gv)	%^WHITE%^%^B_BLUE%^
swallow	%^RED%^
swallow (myelinalpha in gv)	%^BLACK%^%^B_RED%^
cylis	%^GREEN%^
luedre	%^ORANGE%^
streetpizza	%^BLUE%^
streetpizza (grapevine in gv)	%^BLACK%^%^B_YELLOW%^
surge	%^MAGENTA%^
alena03	%^CYAN%^
alena03 (grapevine in gv)	%^RED%^
sorrow	%^WHITE%^
misja	%^RED%^%^BOLD%^
rezo	%^GREEN%^%^BOLD%^
sicness	%^YELLOW%^
sicness (apotheosis in gv)	%^CYAN%^
eyediya	%^MAGENTA%^%^BOLD%^
zax	%^BLUE%^%^BOLD%^
tester (wop in gv)	%^GREEN%^%^BOLD%^
fallen	%^CYAN%^%^BOLD%^
leona	%^WHITE%^%^BOLD%^
egon	%^BLACK%^%^BOLD%^
sasha	%^RED%^
sasha (neonmoo in gv)	%^WHITE%^%^BOLD%^
aiasa	%^GREEN%^
arturos	%^ORANGE%^
breia	%^BLUE%^
breia (neonmoo in gv)	%^WHITE%^%^B_MAGENTA%^
redih	%^MAGENTA%^
crow	%^CYAN%^
snowyowl	%^WHITE%^
oblisgr	%^RED%^%^BOLD%^
oblisgr (grapevine in gv)	%^BLACK%^%^B_YELLOW%^
sapphicubus	%^GREEN%^%^BOLD%^
thresh	%^YELLOW%^
daddywolf	%^MAGENTA%^%^BOLD%^
deadcultist	%^BLUE%^%^BOLD%^
deadcultist (grapevine in gv)	%^ORANGE%^
cuddledragon	%^CYAN%^%^BOLD%^
jasih	%^WHITE%^%^BOLD%^
jasih (apotheosis in gv)	%^CYAN%^
miko	%^BLACK%^%^BOLD%^
kerrigan	%^RED%^
anodyne	%^GREEN%^
anodyne (myelinalpha in gv)	%^YELLOW%^
deminetix	%^ORANGE%^
deminetix (grapevine in gv)	%^BLUE%^%^BOLD%^
elixx	%^BLUE%^
elixx (drastical in gv)	%^WHITE%^%^BOLD%^
madoc (grapevine in gv)	%^WHITE%^%^B_RED%^
argis	%^MAGENTA%^
psitian	%^CYAN%^
rhaziel	%^WHITE%^
kedanna	%^RED%^%^BOLD%^
kedanna (grapevine in gv)	%^BLACK%^%^B_GREEN%^
wize1resurrected	%^GREEN%^%^BOLD%^
wize1resurrected (grapevine in gv)	%^BLACK%^%^B_CYAN%^
emily (neonmoo in gv)	%^BLACK%^%^B_YELLOW%^
marigold	%^YELLOW%^
marigold (neonmoo in gv)	%^BLACK%^%^BOLD%^
sasha (metalmoo in gv)	%^RED%^
zaphob	%^MAGENTA%^%^BOLD%^
zaphob (grapevine in gv)	%^ORANGE%^
ncert	%^BLUE%^%^BOLD%^
ncert (grapevine in gv)	%^MAGENTA%^
therain	%^CYAN%^%^BOLD%^
islay	%^WHITE%^%^BOLD%^
drexlor	%^BLACK%^%^BOLD%^
bozimmerman	%^RED%^
bozimmerman (grapevine in gv)	%^YELLOW%^
wolves (wop in gv)	%^MAGENTA%^%^BOLD%^
guest (neonmoo in gv)	%^BLUE%^%^BOLD%^
fenrin	%^GREEN%^
fenrin (evermore in gv)	%^WHITE%^%^BOLD%^
harrold	%^ORANGE%^
harrold (evermore in gv)	%^WHITE%^%^B_GREEN%^
walbiss	%^BLUE%^
walbiss (evermore in gv)	%^WHITE%^%^B_MAGENTA%^
fenrin@evermore	%^MAGENTA%^
fenrin@evermore (evermore in gv)	%^BLACK%^%^B_GREEN%^
vitaly	%^CYAN%^
vitaly (apotheosis in gv)	%^BLACK%^%^B_CYAN%^
kran	%^WHITE%^
kran (grapevine in gv)	%^BLACK%^%^B_WHITE%^
pool	%^RED%^%^BOLD%^
paracelsus	%^GREEN%^%^BOLD%^
doom	%^YELLOW%^
cruinne	%^MAGENTA%^%^BOLD%^
cruinne (grapevine in gv)	%^BLUE%^
lee	%^BLUE%^%^BOLD%^
blonk	%^CYAN%^%^BOLD%^
blonk (wop in gv)	%^WHITE%^
blame	%^WHITE%^%^BOLD%^
blame (wop in gv)	%^GREEN%^%^BOLD%^
tacosal	%^BLACK%^%^BOLD%^
tacosal (grapevine in gv)	%^MAGENTA%^%^BOLD%^
tacosal (wop in gv)	%^BLUE%^%^BOLD%^
barlow	%^RED%^
barlow (neonmoo in gv)	%^WHITE%^%^BOLD%^
rainbowbeard	%^GREEN%^
blystyryng	%^ORANGE%^
kelwin	%^BLUE%^
tremulo	%^MAGENTA%^
stardust (grapevine in gv)	%^BLACK%^%^B_RED%^
moltec	%^CYAN%^
rmp	%^WHITE%^
maggie560827	%^RED%^%^BOLD%^
isaak	%^GREEN%^%^BOLD%^
isaak (neonmoo in gv)	%^BLACK%^%^B_WHITE%^
maggie560827 (neonmoo in gv)	%^BLACK%^%^BOLD%^
courier	%^YELLOW%^
courier (neonmoo in gv)	%^GREEN%^
papercombo	%^MAGENTA%^%^BOLD%^
papercombo (grapevine in gv)	%^BLUE%^
damascus	%^BLUE%^%^BOLD%^
damascus (grapevine in gv)	%^CYAN%^
carolina	%^CYAN%^%^BOLD%^
carolina (neonmoo in gv)	%^RED%^%^BOLD%^
xorlarrin	%^WHITE%^%^BOLD%^
talia	%^BLACK%^%^BOLD%^
talia (neonmoo in gv)	%^MAGENTA%^%^BOLD%^
a dark shadow	%^RED%^
a dark shadow (wop in gv)	%^CYAN%^%^BOLD%^
dex	%^GREEN%^
equinox	%^ORANGE%^
skolkrusher	%^BLUE%^
skolkrusher (grapevine in gv)	%^WHITE%^%^B_BLUE%^
xenthum	%^MAGENTA%^
xenthum (grapevine in gv)	%^BLACK%^%^B_RED%^
thetinkerer	%^CYAN%^
thetinkerer (grapevine in gv)	%^BLACK%^%^B_MAGENTA%^
welsh	%^WHITE%^
welsh (evermore in gv)	%^BLACK%^%^B_YELLOW%^
payn	%^RED%^%^BOLD%^
marelle	%^GREEN%^%^BOLD%^
moonbeam	%^YELLOW%^
young warrior	%^MAGENTA%^%^BOLD%^
young warrior (evermore in gv)	%^ORANGE%^
young mage	%^BLUE%^%^BOLD%^
young mage (evermore in gv)	%^MAGENTA%^
noiotk	%^CYAN%^%^BOLD%^
noiotk (grapevine in gv)	%^WHITE%^
speedrun	%^WHITE%^%^BOLD%^
mecai	%^BLACK%^%^BOLD%^
cadence	%^RED%^
jade	%^GREEN%^
spacepup	%^ORANGE%^
santa claus	%^BLUE%^
santa claus (wop in gv)	%^WHITE%^%^BOLD%^
saint	%^MAGENTA%^
q	%^CYAN%^
q (mongoose in gv)	%^WHITE%^%^B_BLUE%^
segfault	%^WHITE%^
ekurisona	%^RED%^%^BOLD%^
ekurisona (grapevine in gv)	%^BLACK%^%^B_GREEN%^
daiverd	%^GREEN%^%^BOLD%^
daiverd (mongoose in gv)	%^BLACK%^%^B_CYAN%^
ekhart	%^YELLOW%^
jerovich	%^MAGENTA%^%^BOLD%^
lancius	%^BLUE%^%^BOLD%^
patrick w	%^CYAN%^%^BOLD%^
patrick w (mongoose in gv)	%^GREEN%^
society	%^WHITE%^%^BOLD%^
blizzard (grapevine in gv)	%^BLUE%^
daiverd dreviad	%^BLACK%^%^BOLD%^
daiverd dreviad (mongoose in gv)	%^CYAN%^
parx	%^RED%^
parx (evermore in gv)	%^RED%^%^BOLD%^
lilianaria	%^GREEN%^
kat	%^ORANGE%^
kat (mongoose in gv)	%^MAGENTA%^%^BOLD%^
hahattpro	%^BLUE%^
hahattpro (grapevine in gv)	%^CYAN%^%^BOLD%^
phrygian	%^MAGENTA%^
aldar	%^CYAN%^
zafrusteria	%^WHITE%^
zafrusteria (wop in gv)	%^WHITE%^%^B_BLUE%^
sabels	%^RED%^%^BOLD%^
sabels (grapevine in gv)	%^BLACK%^%^B_RED%^
darth molly	%^GREEN%^%^BOLD%^
darth molly (mongoose in gv)	%^BLACK%^%^B_MAGENTA%^
joe (grapevine in gv)	%^BLACK%^%^B_CYAN%^
mmiao	%^YELLOW%^
gordov	%^MAGENTA%^%^BOLD%^
gumml	%^BLUE%^%^BOLD%^
gumml (grapevine in gv)	%^RED%^
kblood	%^CYAN%^%^BOLD%^
bessie the milkcow	%^WHITE%^%^BOLD%^
bessie the milkcow (wop in gv)	%^BLUE%^
the owl	%^BLACK%^%^BOLD%^
the owl (wop in gv)	%^CYAN%^
shakath	%^RED%^
kazemi	%^GREEN%^
talon	%^ORANGE%^
kazemi (mongoose in gv)	%^YELLOW%^
talon (mongoose in gv)	%^MAGENTA%^%^BOLD%^
tyler spivey	%^BLUE%^
tyler spivey (mongoose in gv)	%^CYAN%^%^BOLD%^
nguyen oblate	%^MAGENTA%^
nguyen oblate (mongoose in gv)	%^WHITE%^%^B_RED%^
fireside huxley	%^CYAN%^
fireside huxley (mongoose in gv)	%^WHITE%^%^B_BLUE%^
itami	%^WHITE%^
stupid	%^BLACK%^%^B_RED%^
daiverd shork	%^RED%^%^BOLD%^
daiverd shork (mongoose in gv)	%^BLACK%^%^B_MAGENTA%^
gene	%^GREEN%^%^BOLD%^
gene (backrooms in gv)	%^BLACK%^%^B_YELLOW%^
maslore	%^YELLOW%^
maslore (grapevine in gv)	%^BLACK%^%^BOLD%^
rishla	%^MAGENTA%^%^BOLD%^
rishla (wop in gv)	%^GREEN%^
aeschix	%^BLUE%^%^BOLD%^
anuglyhalforc	%^CYAN%^%^BOLD%^
abrutalorc	%^WHITE%^%^BOLD%^
kazza	%^BLACK%^%^BOLD%^
kazza (mongoose in gv)	%^WHITE%^
marge300844	%^RED%^
marge300844 (neonmoo in gv)	%^GREEN%^%^BOLD%^
elwood	%^GREEN%^
tsath@goo	%^MAGENTA%^%^BOLD%^
tsath@space mud	%^BLUE%^%^BOLD%^
morris the moose	%^BLUE%^
morris the moose (mongoose in gv)	%^WHITE%^%^BOLD%^
chrnos	%^MAGENTA%^
tabasko	%^CYAN%^
tabasko (wop in gv)	%^WHITE%^%^B_BLUE%^
pgt	%^WHITE%^
an elven dancer	%^RED%^%^BOLD%^
an elven dancer (wop in gv)	%^BLACK%^%^B_GREEN%^
haplo	%^GREEN%^%^BOLD%^
saeed	%^YELLOW%^
saeed (mongoose in gv)	%^BLACK%^%^B_YELLOW%^
moustafa	%^MAGENTA%^%^BOLD%^
terin	%^BLUE%^%^BOLD%^
riverwind	%^CYAN%^%^BOLD%^
acire	%^WHITE%^%^BOLD%^
aloha	%^BLACK%^%^BOLD%^
morris the moo	%^RED%^
morris the moo (mongoose in gv)	%^MAGENTA%^
moss	%^GREEN%^
folly	%^ORANGE%^
\.


--
-- Name: channels channels_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.channels
    ADD CONSTRAINT channels_pkey PRIMARY KEY (channel);


--
-- Name: hours hours_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.hours
    ADD CONSTRAINT hours_pkey PRIMARY KEY (hour);


--
-- Name: i3log ix_i3log_row; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.i3log
    ADD CONSTRAINT ix_i3log_row UNIQUE (created, local, is_emote, is_url, is_bot, channel, speaker, mud, message);


--
-- Name: pinkfish_map pinkfish_map_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.pinkfish_map
    ADD CONSTRAINT pinkfish_map_pkey PRIMARY KEY (pinkfish);


--
-- Name: speakers speakers_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.speakers
    ADD CONSTRAINT speakers_pkey PRIMARY KEY (speaker);


--
-- Name: ix_i3_packets_created; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3_packets_created ON public.i3_packets USING btree (created);


--
-- Name: ix_i3_packets_length; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3_packets_length ON public.i3_packets USING btree (packet_length);


--
-- Name: ix_i3_packets_local; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3_packets_local ON public.i3_packets USING btree (local);


--
-- Name: ix_i3_packets_type; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3_packets_type ON public.i3_packets USING btree (packet_type);


--
-- Name: ix_i3log_bot; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3log_bot ON public.i3log USING btree (is_bot);


--
-- Name: ix_i3log_channel; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3log_channel ON public.i3log USING btree (channel);


--
-- Name: ix_i3log_local; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3log_local ON public.i3log USING btree (local);


--
-- Name: ix_i3log_speaker; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3log_speaker ON public.i3log USING btree (speaker);


--
-- Name: ix_i3log_url; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3log_url ON public.i3log USING btree (is_url);


--
-- Name: ix_i3log_username; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_i3log_username ON public.i3log USING btree (username);


--
-- Name: ix_urls_checksum; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX ix_urls_checksum ON public.urls USING btree (checksum);


--
-- Name: channels channels_pinkfish_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.channels
    ADD CONSTRAINT channels_pinkfish_fkey FOREIGN KEY (pinkfish) REFERENCES public.pinkfish_map(pinkfish);


--
-- Name: hours hours_pinkfish_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.hours
    ADD CONSTRAINT hours_pinkfish_fkey FOREIGN KEY (pinkfish) REFERENCES public.pinkfish_map(pinkfish);


--
-- Name: speakers speakers_pinkfish_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.speakers
    ADD CONSTRAINT speakers_pinkfish_fkey FOREIGN KEY (pinkfish) REFERENCES public.pinkfish_map(pinkfish);


--
-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: -
--

REVOKE USAGE ON SCHEMA public FROM PUBLIC;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

