#ifndef _ACT_OBJ_H
#define _ACT_OBJ_H

void get(struct char_data *ch, struct obj_data *obj_object, struct obj_data *sub_object) ;
void do_get(struct char_data *ch, char *argument, int cmd);
void do_drop(struct char_data *ch, char *argument, int cmd) ;
void do_put(struct char_data *ch, char *argument, int cmd);
void do_give(struct char_data *ch, char *argument, int cmd);
void weight_change_object(struct obj_data *obj, int weight);
void name_from_drinkcon(struct obj_data *obj);
void name_to_drinkcon(struct obj_data *obj,int type);
void do_drink(struct char_data *ch, char *argument, int cmd);
void do_eat(struct char_data *ch, char *argument, int cmd);
void do_pour(struct char_data *ch, char *argument, int cmd);
void do_sip(struct char_data *ch, char *argument, int cmd);
void do_taste(struct char_data *ch, char *argument, int cmd);
perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
int IsRestricted(int Mask, int Class);
void wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
void do_wear(struct char_data *ch, char *argument, int cmd) ;
void do_wield(struct char_data *ch, char *argument, int cmd) ;
void do_grab(struct char_data *ch, char *argument, int cmd);
void do_remove(struct char_data *ch, char *argument, int cmd);

#endif
