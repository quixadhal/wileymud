#ifndef _SHOP_H
#define _SHOP_H

#define MAX_TRADE 5
#define MAX_PROD 5

struct shop_data {
  int                                     producing[MAX_PROD]; /* Which item to produce (virtual) */
  float                                   profit_buy;	       /* Factor to multiply cost with.  */
  float                                   profit_sell;	       /* Factor to multiply cost with.  */
  char                                    type[MAX_TRADE];     /* Which item to trade.  */
  char                                   *no_such_item1;       /* Message if keeper hasn't got an item */
  char                                   *no_such_item2;       /* Message if player hasn't got an item */
  char                                   *missing_cash1;       /* Message if keeper hasn't got cash */
  char                                   *missing_cash2;       /* Message if player hasn't got cash */
  char                                   *do_not_buy;	       /* If keeper dosn't buy such things.  */
  char                                   *message_buy;	       /* Message when player buys item */
  char                                   *message_sell;	       /* Message when player sells item */
  int                                     temper1;	       /* How does keeper react if no money */
  int                                     temper2;	       /* How does keeper react when attacked */
  int                                     keeper;	       /* The mobil who owns the shop (virtual) */
  int                                     with_who;	       /* Who does the shop trade with? */
  int                                     in_room;	       /* Where is the shop? */
  int                                     open1,
                                          open2;	       /* When does the shop open? */
  int                                     close1,
                                          close2;	       /* When does the shop close? */
};

#ifndef _SHOP_C
extern struct shop_data                *shop_index;
extern int                              number_of_shops;

#endif

int                                     is_ok(struct char_data *keeper, struct char_data *ch,
					      int shop_nr);
int                                     trade_with(struct obj_data *item, int shop_nr);
int                                     shop_producing(struct obj_data *item, int shop_nr);
void                                    shopping_buy(char *arg, struct char_data *ch,
						     struct char_data *keeper, int shop_nr);
void                                    shopping_sell(char *arg, struct char_data *ch,
						      struct char_data *keeper, int shop_nr);
void                                    shopping_value(char *arg, struct char_data *ch,
						       struct char_data *keeper, int shop_nr);
void                                    shopping_list(char *arg, struct char_data *ch,
						      struct char_data *keeper, int shop_nr);
void                                    shopping_kill(char *arg, struct char_data *ch,
						      struct char_data *keeper, int shop_nr);
int                                     shop_keeper(struct char_data *ch, int cmd, char *arg);
void                                    boot_the_shops(void);
void                                    assign_the_shopkeepers(void);

#endif
