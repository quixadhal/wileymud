--
-- log_types
--
-- This table describes the various types of log messages which
-- can appear in the logfile table.
--

--   LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM,
--   LOG_WARN, LOG_ALL

-- CHANNEL_LOG, CHANNEL_HIGHGOD, CHANNEL_BUILD, CHANNEL_HIGH,
-- CHANNEL_COMM, CHANNEL_AUTH

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

