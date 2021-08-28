#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <arpa/inet.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "base64.h"

#define _2FA_C
#include "2fa.h"

// WARNING:  This function calls base32_decode() which uses calloc().  You must free it!
unsigned char *TFA_secret(const char *rawSecret, size_t *b32Len)
{
    size_t rawSecretLen = 0;
    size_t secretLen = 0;
    static char secret[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    unsigned char *b32 = NULL;

    // Alllll this  to replace one line of python:
    // self._raw_secret = s.upper().rjust(16, 'A')[0:16]

    if (!rawSecret || !*rawSecret)
    {
        strncpy(secret, "ABCDEFGHIJKLMNOP", 16);
        secret[16] = '\0';
    }
    else
    {
        rawSecretLen = strlen(rawSecret);
        for (size_t i = 0; i < rawSecretLen; i++)
        {
            if (rawSecret[i] == ' ')
            {
                continue;
            }
            else if (rawSecret[i] == '-')
            {
                continue;
            }
            else
            {
                secret[secretLen] = toupper(rawSecret[i]);
                secretLen++;
                if (secretLen >= 16)
                {
                    break;
                }
            }
        }

        if (secretLen < 16)
        {
            // right justify with spaces
            char copy[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
            strcpy(copy, secret);
            sprintf(secret, "%16.16s", copy);
            for (size_t i = 0; i < 16; i++)
            {
                if (secret[i] == ' ')
                {
                    // The convert it to the default 'A' token
                    secret[i] = 'A';
                }
            }
        }

        secret[16] = '\0';
    }

    b32 = (unsigned char *)base32_decode(secret, strlen(secret), b32Len);
    return b32;
}

int TFA_timecode(time_t time_input, unsigned char *b32, size_t b32Len)
{
    uint64_t moment = 0;
    unsigned char time_bytes[8];
    unsigned char *hashDigest = NULL;
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int resultLen = 0;
    int offset = 0;
    uint32_t truncatedDigest = 0;

    if (time_input < 0)
    {
        time_input = time(NULL);
    }
    moment = time_input / 30;
    // log_info("DEBUG: moment: %u\n", moment);

    bzero(time_bytes, sizeof(time_bytes));
    for (int i = 7; i > 0; i--)
    {
        time_bytes[i] = (unsigned char)(moment & 0xFF);
        moment >>= 8;
    }
    // log_info("DEBUG: time_bytes: "); hex_dump((unsigned char *)time_bytes, sizeof(time_bytes));
    // log_info("DEBUG: secret: "); hex_dump((unsigned char *)b32, b32Len);

    bzero(result, EVP_MAX_MD_SIZE);
    hashDigest = HMAC(EVP_sha1(), b32, b32Len, (unsigned char *)time_bytes, sizeof(time_bytes), result, &resultLen);
    if (!hashDigest)
    {
        // Error in hashing
        return -1;
    }
    // log_info("DEBUG: hashDigest: "); hex_dump((unsigned char *)result, resultLen);

    offset = result[resultLen - 1] & 0x0F;
    // log_info("DEBUG: offset: %d\n", offset);

    for (int i = offset; i < offset + 4; ++i)
    {
        truncatedDigest <<= 8;
        truncatedDigest |= (result[i] & 0xFF);
    }
    // log_info("DEBUG: trucatedDigest: %u\n", truncatedDigest);
    truncatedDigest &= 0x7FFFFFFF;
    truncatedDigest %= 1000000;

    return truncatedDigest;
}

int TFA_verify(const char *tokenStr, unsigned char *b32, size_t b32Len)
{
    size_t tokenLen = 0;
    char tmp[256] = "\0\0\0\0\0\0\0";
    unsigned int token = 0;
    unsigned int trials[3];
    time_t now;

    if (!tokenStr || !*tokenStr)
    {
        return FALSE;
    }
    tokenLen = strlen(tokenStr);
    for (size_t i = 0; i < tokenLen; i++)
    {
        char oneChar[2] = "\0";

        oneChar[0] = tokenStr[i];

        switch (tokenStr[i])
        {
        default:
            log_error("Illegal character '%s' in authentication token.\n", oneChar);
            return FALSE;
            break;
        case ' ':
        case '-':
            // Just skip these, to be friendly.
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            oneChar[0] = tokenStr[i];
            strncat(tmp, oneChar, 255);
            break;
        }
    }

    // At this point, our token is potentially valid, so we can validate it.
    token = atoi(tokenStr);

    now = time(NULL);
    trials[0] = TFA_timecode(now - 30, b32, b32Len); // just expired edge case
    trials[1] = TFA_timecode(now + 0, b32, b32Len);  // current timecode
    trials[2] = TFA_timecode(now + 30, b32, b32Len); // future edge case
    // log_info("DEBUG: INPUT: %u TRIALS: [%u,%u,%u]\n", token, trials[0], trials[1], trials[2]);
    for (int i = 0; i < 3; i++)
    {
        if (trials[i] < 0)
        {
            log_error("DEBUG: Hash error on trial %d ???\n", i);
            return FALSE;
        }
        if (token == trials[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

int test_2fa_main(int argc, char **argv)
{
    int attempts = 3;
    unsigned char *b32 = NULL;
    size_t b32Len = 0;

    b32 = TFA_secret("appy l3en 3d7c jrru", &b32Len);
    if (!b32)
    {
        printf("Secret invalid.\n");
        exit(1);
    }

    do
    {
        char userInput[256] = "\0\0\0\0\0\0\0";
        size_t inputLen = 0;

        printf("Authenticator code: ");
        if (fgets(userInput, 256, stdin) == NULL)
        {
            printf("Error on user input!\n");
            break;
        }

        inputLen = strlen(userInput);
        if (inputLen > 0)
        {
            while (userInput[inputLen - 1] == '\r' || userInput[inputLen - 1] == '\n')
            {
                userInput[inputLen - 1] = '\0';
                inputLen--;
            }
        }

        if (TFA_verify(userInput, b32, b32Len))
        {
            printf("Code accepted.\n");
            break;
        }
        else
        {
            printf("Invalid code.\n");
            attempts--;
        }
    } while (attempts > 0);

    if (attempts < 1)
    {
        printf("Authentication failure.\n");
    }

    if (b32)
    {
        free(b32);
        b32 = NULL;
    }
    return TRUE;
}
