void spell_summon_mount(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;
	int i;
	int cantload;
	int rnum;
	char buffer[40];

/* We summon a mount of a type relative to the level of the ranger
   casting it. 
      Aaaaaand the lucky mobs are:
         level 27 One of...
		huge spider		Mob #
		large stirge		Mob #
         level 28 One of...
		elephant		Mob #
		giant ant		Mob #
		unicorn			Mob #
	 level 29 One of...
		green dragon baby	Mob #
		baby brontosaur		Mob #
		giant scorpion		Mob #
	 level 30,31 One of...
		flying dragon golden	Mob #
		roc			Mob #
		lammasu			Mob #
		horse			Mob #
		pegasus			Mob #
	 level 32+ One of...
		griffon			Mob #
		giant eagle		Mob #
		small allosaur		Mob #
	 The higher levels can summon any of the mobs on levels lower than
	   their own.  If no mob is specified, a random mob appropriate to
	   the ranger's level will be loaded.  The mob gets a saving throw
	   vs. charm.  If it succeeds, it becomes agressive and hates the
	   caster.  If it fails, it becomes charmed.  
	-Sedna
*/
	if(!(arg)) {
	  If (level == 27) {
	    rnum = number(1, 2) + ??;
	    mob = read_mobile(rnum, VIRTUAL);
	  } else if (level == 28) {
	    rnum = number(1,5) + ??;
	    mob = read_mobile(rnum, VIRTUAL);
	  } else if (level == 29) {
	    rnum = number(1,8) + ??;
	    mob = read_mobile(rnum, VIRTUAL);
	  } else if (level <= 31) {
	    rnum = number(1,13) + ??;
	    mob = read_mobile(rnum, VIRTUAL);
	  } else {
	    rnum = number(1,15) + ??;
	    mob = read_mobile(rnum, VIRTUAL);
	  }
	} else {
            cantload=0;
            rnum = 0;
            for(i=??;i<(??+15);i++)
              if(isname(arg,mob_index[i].name)) {
                rnum=i;
                break;
            if(rnum) {
           

