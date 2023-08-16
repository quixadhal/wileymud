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

DROP INDEX IF EXISTS public.ix_logfile_logtype;
DROP INDEX IF EXISTS public.ix_logfile_created;
DROP VIEW IF EXISTS public.logtail;
DROP TABLE IF EXISTS public.logfile;
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
-- Name: logfile; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.logfile (
    created timestamp with time zone DEFAULT now() NOT NULL,
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
-- Name: logtail; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW public.logtail AS
 SELECT to_char(logfile.created, 'YYYY-MM-DD HH24:MI:SS'::text) AS created,
    logfile.logtype,
    logfile.message
   FROM public.logfile
  ORDER BY (to_char(logfile.created, 'YYYY-MM-DD HH24:MI:SS'::text))
 OFFSET (( SELECT count(*) AS count
           FROM public.logfile logfile_1) - 20);


--
-- Name: ix_logfile_created; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_logfile_created ON public.logfile USING btree (created);


--
-- Name: ix_logfile_logtype; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_logfile_logtype ON public.logfile USING btree (logtype);


--
-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: -
--

REVOKE USAGE ON SCHEMA public FROM PUBLIC;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

