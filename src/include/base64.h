
#ifndef _BASE64_H
#define _BASE64_H

char *base64_encode(const char *input, size_t inputLen);
char *base32_encode(const char *input, size_t inputLen);
char *base64_decode(const char *input, size_t inputLen, size_t *finalOutputLen);
char *base32_decode(const char *input, size_t inputLen, size_t *finalOutputLen);

#endif
