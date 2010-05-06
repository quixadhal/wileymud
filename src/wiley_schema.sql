--
-- log_types
--
-- This table describes the various types of log messages which
-- can appear in the logfile table.
--

CREATE TABLE log_types (
  log_type_id		INTEGER NOT NULL PRIMARY KEY,
  name			TEXT NOT NULL UNIQUE,
  description		TEXT
);

COMMENT ON TABLE	log_types				IS 'Log message types';
COMMENT ON COLUMN	log_types.log_type_id			IS 'What kind of log message is this?';
COMMENT ON COLUMN	log_types.name				IS 'What do we call it?';
COMMENT ON COLUMN	log_types.description			IS 'What does it get used for?';

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
-- logfile
--
-- The logfile table is the actual table of all log events.
-- I called it logfile, because it reflects what it really replaces.
--

CREATE TABLE logfile (
  log_type_id		INTEGER REFERENCES log_types (log_type_id),
  log_date		TIMESTAMP DEFAULT now(),
  log_entry		TEXT,
  log_file		TEXT,
  log_function		TEXT,
  log_line		INTEGER,
  log_areafile		TEXT,
  log_arealine		INTEGER,
  log_pc_actor		TEXT,		-- This will eventually be a player id reference
  log_pc_victim		TEXT,		-- This will eventually be a player id reference
  log_npc_actor		INTEGER,	-- This will eventually be a mob id reference
  log_npc_victim	INTEGER,	-- This will eventually be a mob id reference
  log_obj		INTEGER,	-- This will eventually be a object id reference
  log_area		INTEGER,	-- This will eventually be an area table reference
  log_room		INTEGER		-- This will eventually be a room reference
);

COMMENT ON TABLE	logfile					IS 'Log messages';
COMMENT ON COLUMN	logfile.log_type_id			IS 'What kind of log message is this?';
COMMENT ON COLUMN	logfile.log_date			IS 'Time this log entry was created';
COMMENT ON COLUMN	logfile.log_entry			IS 'Actual message';
COMMENT ON COLUMN	logfile.log_file			IS 'C source file of error call';
COMMENT ON COLUMN	logfile.log_function			IS 'C function of error caller';
COMMENT ON COLUMN	logfile.log_line			IS 'Line number of error call in C source';
COMMENT ON COLUMN	logfile.log_areafile			IS 'Area file being loaded at error point';
COMMENT ON COLUMN	logfile.log_arealine			IS 'Error point line number in area file';
COMMENT ON COLUMN	logfile.log_pc_actor			IS 'Player which caused the event';
COMMENT ON COLUMN	logfile.log_pc_victim			IS 'Player that the event happened to';
COMMENT ON COLUMN	logfile.log_npc_actor			IS 'Mobile which caused the event';
COMMENT ON COLUMN	logfile.log_npc_victim			IS 'Mobile that the event happened to';
COMMENT ON COLUMN	logfile.log_obj				IS 'Object which caused the error';
COMMENT ON COLUMN	logfile.log_area			IS 'Area the actor was in when the error happened';
COMMENT ON COLUMN	logfile.log_room			IS 'Room the actor was in when the error happened';

CREATE INDEX ix_logfile_date ON logfile (log_date);

CREATE VIEW log_today AS 
       SELECT   to_char(date_trunc('second', log_date), 'HH24:MI:SS') AS log_date, log_types.name AS log_type, log_entry
       FROM     logfile join log_types USING (log_type_id)
       WHERE    logfile.log_date > now() - interval '1 day'
       ORDER BY logfile.log_date DESC;




CREATE TABLE banned (
  banned_name		TEXT,
  banned_ip		INET UNIQUE,
  banned_by		TEXT DEFAULT 'SYSTEM',
  banned_date		TIMESTAMP DEFAULT now()
);

CREATE INDEX ix_banned_name ON banned (banned_name); 
CREATE UNIQUE INDEX ix_banned_name_lc ON banned (lower(banned_name)); 

COMMENT ON TABLE	banned					IS 'IP and Name bans';
COMMENT ON COLUMN	banned.banned_name			IS 'Text name that has been banned';
COMMENT ON COLUMN	banned.banned_ip			IS 'IP address that has been banned';
COMMENT ON COLUMN	banned.banned_by			IS 'Wizard that added this ban record';
COMMENT ON COLUMN	banned.banned_date			IS 'Time the ban was implemented';

COPY banned (banned_name, banned_ip, banned_by, banned_date) FROM stdin;
fuck	\N	SYSTEM	2008-10-02 05:07:09.606346
shit	\N	SYSTEM	2008-10-02 05:07:09.606346
asshole	\N	SYSTEM	2008-10-02 05:07:09.606346
fucker	\N	SYSTEM	2008-10-02 05:07:09.606346
\.

-- banned_name():  SELECT 1 FROM banned WHERE lower(banned_name) = lower(?) AND banned_ip IS NULL;
-- banned_ip():    SELECT 1 FROM banned WHERE host(banned_ip) = lower(?) AND banned_name IS NULL;
-- banned_at():    SELECT 1 FROM banned WHERE lower(banned_name) = lower(?) AND host(banned_ip) = lower(?);
-- is_banned():    banned_name(name) OR banned_ip(ip) OR banned_at(name,ip)
--
-- Wiley doesn't support specific name@ip bans, but it could easily enough by reworking using is_banned().




CREATE TABLE alignment (
  alignment_id		INTEGER NOT NULL PRIMARY KEY,
  lower_bound		INTEGER,
  upper_bound		INTEGER,
  name			TEXT NOT NULL,
  description		TEXT
);

COPY alignment (alignment_id, lower_bound, upper_bound, name, description) FROM stdin;
0	\N	-1000	REALLY VILE	\N
1	-999	-900	VILE	\N
2	-899	-700	VERY EVIL	\N
3	-699	-350	EVIL	\N
4	-349	-100	WICKED	\N
5	-99	99	NEUTRAL	\N
6	100	349	NICE	\N
7	350	699	GOOD	\N
8	700	899	VERY GOOD	\N
9	900	999	HOLY	\N
10	1000	\N	REALLY HOLY	\N
\.

