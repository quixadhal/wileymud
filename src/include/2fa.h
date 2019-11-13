#ifndef _2FA_H
#define _2FA_H

unsigned char *TFA_secret(const char *rawSecret, size_t *b32Len);
int TFA_timecode(time_t time_input, unsigned char *b32, size_t b32Len);
int TFA_verify(const char *tokenStr, unsigned char *b32, size_t b32Len);

#endif
