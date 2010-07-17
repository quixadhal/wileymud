void spell_wall_of_force(BYTE level,struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
   int to_room;
   struct room_data *room;
   struct room_direction_data *exitp;
   int dam;
   int tmp;
   int dir;

   struct char_data *targetvic;
   struct char_data *nextvic;
   char buf[80];

   if(DEBUG)
     dlog("spell_wall_of_force");
   if (!ch)
     return;
   dam=dice(1,6);
   
   for(targetvic=character_list; targetvic; targetvic=nextvic) {
     nextvic=targetvic->next;
     if((ch->in_room == targetvic->in_room) && (ch !=targetvic)
       && (!(IS_AFFECTED(tergetvic, AFF_GROUP) &&
            (targetvic->master == ch->master) || 
            (targetvic->master == ch) ||
            (targetvic = ch->master)) && CheckKill(ch,targetvic))) {
       if (IS_PC(targetvict) && IS_IMMORTAL(targetvic) && !(IS_IMMORTAL(ch))) {
            cprintf(targetvic, "Some puny mortal tried to hit you with a wall of force.\n\r"); } else {
            if(RIDDEN(targetvic)) {
              FallOffMount(RIDDEN(targetvic),targetvic);
              Dismount(RIDDEN(targetvic),targetvic,POSITION_SITTING);
            }
            if(MOUNTED(targetvic)) {
              FallOffMount(targetvic,MOUNTED(targetvic);
              Dismount(targetvic,MOUNTED(targetvic),POSITION_SITTING);
            }
            dir=number(0,(MAX_NUM_EXITS-1)*3);
            if (exit_ok(exitp=EXIT(targetvic, dir),&room) {
              /* exit exists...throw mob/pc into room */
              if(IS_SET(exitp->exit_info, EX_CLOSED)) { /* Door was closed */
                tmp=dice(1,6);
                if(tmp < 4) { /* didn't break door down */
                  damage(ch,targetvic,dam,TYPE_SUFFERING);
                  act("$n is hurled against the door.",FALSE,targetvic,0,0,TO_ROOM);
                  act("You feel a tremendous push and slam against the door.",
                       FALSE,targetvic,0,0,TO_CHAR);
                } else {
                    REMOVE_BIT(exitp->exit_info, EX_CLOSED);
                    damage(ch,targetvic,(dam/2), TYPE_SUFFERING);
                    sprintf(buf,"$n is hurled against the %s to the %s and breaks it down.",
/* Things to be added when you have more patience:
   need to finish the sprintf.  
   don't forget to give message to other room that character flies into
    and check for the death flag.
*/
