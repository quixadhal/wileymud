extern int DEBUG;

void Teleport( int pulse )
{
   struct char_data *ch;
   struct obj_data *obj_object, *temp_obj;
   int or;

   if(DEBUG) dlog("Teleport");
   if (pulse < 0) 
      return;

   if(MOUNTED(ch))
   {
     FallOffMount(ch,MOUNTED(ch));
     Dismount(ch,MOUNTED(ch),POSITION_SITTING);
   }
   else
   if(RIDDEN(ch))
   {
     FallOffMount(RIDDEN(ch),ch);
     Dismount(RIDDEN(ch),ch,POSITION_SITTING);
   }

   for (ch = character_list; ch; ch = ch->next) 
   {
     if (ch->in_room != NOWHERE) 
     {
	if (real_roomp(ch->in_room)->tele_targ > 0) 
	{
           if (real_roomp(ch->in_room)->tele_time > 0) 
	   {
              if ((pulse % real_roomp(ch->in_room)->tele_time)==0) 
	      {
		 obj_object = real_roomp(ch->in_room)->contents;
		 while (obj_object) 
		 {
		   temp_obj = obj_object->next_content;
                   obj_from_room(obj_object);
   	           obj_to_room(obj_object, real_roomp(ch->in_room)->tele_targ);
		   obj_object = temp_obj;
		 }

		 or = ch->in_room;
		 char_from_room(ch); 
		 char_to_room(ch, real_roomp(or)->tele_targ);
       	         if (real_roomp(or)->tele_look) 
		 {
                   do_look(ch, "\0",15);
		 }
	       }
	    }
	 }
      }
   }
}

