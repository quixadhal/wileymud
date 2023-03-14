#ifndef __DISCORD_H__
#define __DISCORD_H__

#include "discord_bot.h"

void allchan_discord(int is_emote, const char *channel, const char *speaker, const char *username, const char *mud, const char *message);
void discord_startup(void);
void discord_poll(void);
void discord_shutdown(void);

#endif
