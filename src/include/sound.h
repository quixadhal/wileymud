#ifndef _SOUND_H
#define _SOUND_H

int                                     RecGetObjRoom(struct obj_data *obj);
void                                    MakeNoise(int room, char *local_snd, char *distant_snd);
void                                    MakeSound(int current_pulse);

#endif
