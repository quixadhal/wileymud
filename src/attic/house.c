
int House(struct char_data *ch, int cmd, char *arg) 
{
  struct obj_cost cost;
  int i, save_room;
  char * tmp_desc;
  struct extra_descr_data *ext;
  int found = 0;

  if (IS_NPC(ch)) return(FALSE);
  
  if (cmd != 92) 
  {
    return(FALSE);
  }
  else
  {

  /*
  // Verify that a person can rent here, the name of the character wil
  // be found in the extra description of the room itself, thus leaving
  // the name of the room to be what ever the owner wishes.
  */

    found = 0;
    for(ext = real_roomp(ch->in_room)->ex_description; ext && !found; ext = ext->next)
    if(str_cmp(GET_NAME(ch), ext->keyword) == 0)
    {
      found = 1;
      send_to_char("Okay, found your name in the anals.\n\r",ch);
    }

    if(!found)
    {
      if(strncmp(GET_NAME(ch),real_roomp(ch->in_room)->name,strlen(GET_NAME(ch))))
      {
        send_to_char("Sorry, you'll have to find your own house.\n\r",ch);
        return(FALSE);
      }
      else
      {
        send_to_char("Ah, you own this room.\n\r",ch);
      }
    }
    cost.total_cost = 0; /* Minimum cost */
    cost.no_carried = 0;
    cost.ok = TRUE; /* Use if any "-1" objects */
    
    add_obj_cost(ch, 0, ch->carrying, &cost);
    for(i = 0; i<MAX_WEAR; i++)
      add_obj_cost(ch, 0, ch->equipment[i], &cost);
    
    if(!cost.ok) {
      return(FALSE);
    }      
    cost.total_cost = 0;
    
    GET_HOME(ch) = ch->in_room;
    save_obj(ch, &cost,1);
    save_room = ch->in_room;
    extract_char(ch);
    ch->in_room = save_room;
    save_char(ch, ch->in_room);
    return( TRUE );
  }
}

