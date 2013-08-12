-pmt						/* Preserve make times */

--line-length96					/* Overall width of code */
--comment-line-length120			/* width of comment lines */
--indent-level4					/* indent N spaces per level */
--case-indentation4				/* indent case statements */
--parameter-indentation4			/* indent old-style parameter declarations */
--declaration-indentation40			/* variable declarations line up here */
--tab-size8					/* tabs get expanded into N characters */

--break-before-boolean-operator			/* break before boolean operators if possible */
--dont-break-procedure-type			/* keep function type on the same line as the name */
--blank-lines-after-declarations		/* space out declarations */
--blank-lines-after-procedures			/* space out bodies */
--blank-lines-after-commas			/* seperate lines for comma seperated declarations */
--swallow-optional-blank-lines			/* remove "extra" blank lines */

--braces-on-if-line				/* place opening brace ON the if line */
--cuddle-else					/* put else with braces ala } else { */
--cuddle-do-while				/* put while inline ala do { } while */
--braces-on-struct-decl-line			/* put opening brace ON struct declaration line */
--continue-at-parentheses			/* line up continuations at parens */

--no-space-after-function-call-names		/* function() not function () */
--dont-space-special-semicolon			/* no extra spacing of semicolons */
--no-space-after-casts				/* (type)cast not (type) cast */

--comment-indentation64				/* line up comments at column N */
--declaration-comment-column64			/* line up comments after declarations in column N */
--else-endif-column64				/* preprocessor #else and #endif comments line up here */
--start-left-side-of-comments			/* add stars in block comments */
--format-all-comments				/* try to reformat comments */
--dont-format-first-column-comments		/* unless they are on the left! */
--comment-delimiters-on-blank-lines		/* one lineers with no code should be made larger */

/* Various typedefs used in our code */

-T UBYTE
-T BYTE
-T USHORT
-T SHORT
-T UINT
-T INT
-T ULONG
-T LONG
-T CHAR_DATA
-T funcp
-T ifuncp

-T bool
-T DESCRIPTOR_DATA
-T permissions
-T I3_CHANNEL
-T I3_MUD
-T I3_HEADER
-T I3_IGNORE
-T I3_BAN
-T UCACHE_DATA
-T I3_CHARDATA
-T ROUTER_DATA
-T I3_COLOR
-T I3_CMD_DATA
-T I3_HELP_DATA
-T I3_ALIAS
-T I3_FUN

-T imc_permissions
-T IMC_SPEAKER
-T IMC_CHANNEL
-T IMC_PACKET
-T IMC_PDATA
-T SITEINFO
-T REMOTEINFO
-T IMC_BAN
-T IMC_CHARDATA
-T IMC_IGNORE
-T IMCUCACHE_DATA
-T IMC_COLOR
-T IMC_CMD_DATA
-T IMC_HELP_DATA
-T IMC_ALIAS
-T IMC_PHANDLER
-T WHO_TEMPLATE
-T IMC_FUN
-T PACKET_FUN

-T SOCIAL_DATA
-T CHAR_DATA
-T DESCRIPTOR_DATA
-T genders

-T SHA256Context

-T ident_t
-T IDENT

-T Teleport_t
-T River_t
-T Exit_t
-T Extra_t
-T Room

-T afk_room_flags
-T afk_exit_flags
-T afk_sector_types
-T afk_item_types
-T afk_extra_flags

-T sector_types

-T ppmcolour
-T mapnode
-T coord_group
-T tree
-T forest
-T maptemplate

-T smaug_room_flags
-T smaug_exit_flags
-T smaug_sector_types
-T smaug_item_types
-T smaug_item_extra_flags

-T t_name

-T sh_int
-T EXT_BV

-T vnum
-T vnum_index
-T pair
-T keyword
-T zone_cmds
-T zone
-T zones
-T EXIT
-T extra
-T coordinate
-T room
-T rooms
-T shop
-T shops
-T obj_flags
-T obj_affect
-T object
-T objects
-T dice
-T attack
-T skill
-T mob
-T mobs
