#ifndef _ACT_OBJ_H
#define _ACT_OBJ_H

void get(struct char_data *ch, struct obj_data *obj_object, struct obj_data *sub_object);
int do_get(struct char_data *ch, const char *argument, int cmd);
int do_drop(struct char_data *ch, const char *argument, int cmd);
int do_put(struct char_data *ch, const char *argument, int cmd);
int do_give(struct char_data *ch, const char *argument, int cmd);
void weight_change_object(struct obj_data *obj, int weight);
void name_from_drinkcon(struct obj_data *obj);
void name_to_drinkcon(struct obj_data *obj, int type);
int do_drink(struct char_data *ch, const char *argument, int cmd);
int do_puke(struct char_data *ch, const char *argument, int cmd);
int do_eat(struct char_data *ch, const char *argument, int cmd);
int do_pour(struct char_data *ch, const char *argument, int cmd);
int do_sip(struct char_data *ch, const char *argument, int cmd);
int do_taste(struct char_data *ch, const char *argument, int cmd);
void perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
int IsRestricted(int Mask, int Class);
void wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
int do_wear(struct char_data *ch, const char *argument, int cmd);
int do_wield(struct char_data *ch, const char *argument, int cmd);
int do_grab(struct char_data *ch, const char *argument, int cmd);
int do_remove(struct char_data *ch, const char *argument, int cmd);
int do_bury(struct char_data *ch, const char *argument, int cmd);
int do_desecrate(struct char_data *ch, const char *argument, int cmd);

#endif
