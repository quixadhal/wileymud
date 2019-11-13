#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#define _BASE64_C
#include "base64.h"

// WARNING:  This function allocates memory via calloc(), you must free it!
char *base64_encode(const char *input, size_t inputLen) {
    const char      *b64encodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char   *output = NULL;
    size_t          outputLen = 0;

    // input 8-bit, output 6-bit, 3 bytes IN -> 4 bytes OUT.

    size_t  padLen = inputLen % 3;
    size_t  i = 0;
    size_t  j = 0;

    if( !input || inputLen < 1 ) {
        // Make sure we are being sane here.
        return NULL;
    }

    outputLen = 4 * ((inputLen + 2) / 3);
    output = (unsigned char *)calloc(outputLen + 1, sizeof(unsigned char));
    if(!output) {
        return NULL;
    }

    while( i < inputLen ) {
        size_t      start = i;

        uint32_t    blobA = (i < inputLen) ? (unsigned char)input[i++] : 0;
        uint32_t    blobB = (i < inputLen) ? (unsigned char)input[i++] : 0;
        uint32_t    blobC = (i < inputLen) ? (unsigned char)input[i++] : 0;

        uint32_t    chunk = (blobA << 0x10) + (blobB << 0x08) + blobC;

        if( start < inputLen - padLen ) {
            output[j++] = b64encodingTable[(chunk >> 18) & 0x3F];
            output[j++] = b64encodingTable[(chunk >> 12) & 0x3F];
            output[j++] = b64encodingTable[(chunk >> 6) & 0x3F];
            output[j++] = b64encodingTable[chunk & 0x3F];
        } else {
            switch(padLen) {
                default:
                    break;
                case 1:
                    output[j++] = b64encodingTable[(chunk >> 18) & 0x3F];
                    output[j++] = b64encodingTable[(chunk >> 12) & 0x3F];
                    output[j++] = '=';
                    output[j++] = '=';
                    break;
                case 2:
                    output[j++] = b64encodingTable[(chunk >> 18) & 0x3F];
                    output[j++] = b64encodingTable[(chunk >> 12) & 0x3F];
                    output[j++] = b64encodingTable[(chunk >> 6) & 0x3F];
                    output[j++] = '=';
                    break;
            }
        }
        output[j] = '\0';
    }

    return (char *)output;
}

// WARNING:  This function allocates memory via calloc(), you must free it!
char *base32_encode(const char *input, size_t inputLen) {
    const char      *b32encodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    unsigned char   *output = NULL;
    size_t          outputLen = 0;

    // input 8-bit, output 5-bit, 5 bytes IN -> 8 bytes OUT.

    size_t  padLen = inputLen % 5;
    size_t  i = 0;
    size_t  j = 0;

    if( !input || inputLen < 1 ) {
        // Make sure we are being sane here.
        return NULL;
    }

    outputLen = 8 * ((inputLen + 6) / 5);
    output = (unsigned char *)calloc(outputLen + 1, sizeof(unsigned char));
    if(!output) {
        return NULL;
    }

    while( i < inputLen ) {
        size_t      start = i;

        uint64_t    blobA = (i < inputLen) ? (unsigned char)input[i++] : 0;
        uint64_t    blobB = (i < inputLen) ? (unsigned char)input[i++] : 0;
        uint64_t    blobC = (i < inputLen) ? (unsigned char)input[i++] : 0;
        uint64_t    blobD = (i < inputLen) ? (unsigned char)input[i++] : 0;
        uint64_t    blobE = (i < inputLen) ? (unsigned char)input[i++] : 0;

        uint64_t    chunk = (blobA << 0x20) + (blobB << 0x18)
                          + (blobC << 0x10) + (blobD << 0x08)
                          + blobE;

        if( start < inputLen - padLen ) {
            output[j++] = b32encodingTable[(chunk >> 35) & 0x1F];
            output[j++] = b32encodingTable[(chunk >> 30) & 0x1F];
            output[j++] = b32encodingTable[(chunk >> 25) & 0x1F];
            output[j++] = b32encodingTable[(chunk >> 20) & 0x1F];
            output[j++] = b32encodingTable[(chunk >> 15) & 0x1F];
            output[j++] = b32encodingTable[(chunk >> 10) & 0x1F];
            output[j++] = b32encodingTable[(chunk >> 5) & 0x1F];
            output[j++] = b32encodingTable[chunk & 0x1F];
        } else {
            switch(padLen) {
                default:
                    break;
                case 1:
                    output[j++] = b32encodingTable[(chunk >> 35) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 30) & 0x1F];
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    break;
                case 2:
                    output[j++] = b32encodingTable[(chunk >> 35) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 30) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 25) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 20) & 0x1F];
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    break;
                case 3:
                    output[j++] = b32encodingTable[(chunk >> 35) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 30) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 25) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 20) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 15) & 0x1F];
                    output[j++] = '=';
                    output[j++] = '=';
                    output[j++] = '=';
                    break;
                case 4:
                    output[j++] = b32encodingTable[(chunk >> 35) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 30) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 25) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 20) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 15) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 10) & 0x1F];
                    output[j++] = b32encodingTable[(chunk >> 5) & 0x1F];
                    output[j++] = '=';
                    break;
            }
        }
        output[j] = '\0';
    }

    return (char *)output;
}

// WARNING:  This function allocates memory via calloc(), you must free it!
char *base64_decode(const char *input, size_t inputLen, size_t *finalOutputLen) {
    const char      *b64decodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char   *output = NULL;
    size_t          outputLen = 0;

    // input 6-bit, output 8-bit, 4 bytes IN -> 3 bytes OUT.

    size_t  i = 0;
    size_t  j = 0;
    size_t  padLen = 0;

    if( !input || inputLen < 1 || inputLen % 4 != 0 ) {
        // Not properly padded
        return NULL;
    }

    for( size_t k = 0; k < inputLen; k++ ) {
        if( input[k] == '=' ) {
            // Pad byte
            continue;
        } else if( !strchr(b64decodingTable, (int)input[k]) ) {
            // Illegal character
            return NULL;
        }
    }

    outputLen = (inputLen / 4) * 3;
    output = (unsigned char *)calloc(outputLen + 1, sizeof(unsigned char));
    if(!output) {
        return NULL;
    }

    while( inputLen > 0 ) {
        // Remove pad bytes
        if( input[inputLen-1] == '=' ) {
            inputLen--;
            padLen++;
        } else {
            break;
        }
    }

    while( i < inputLen ) {
        size_t          start = i;
        uint64_t        chunk = 0;
        int             shiftAmounts[4] = { 18, 12, 6, 0 };

        for( size_t k = 0; k < 4; k++ ) {
            // We expect the input to be padded to chunks of 4 characters
            if( i < inputLen ) {
                char        *pos = NULL;
                size_t      x = 0;

                pos = strchr(b64decodingTable, (int)input[i]);
                x = (size_t)(pos - b64decodingTable);
                chunk |= x << shiftAmounts[k];
                i++;
            }
        }

        if( start <= inputLen - 4 ) {
            // Output 3 characters
            output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
            output[j++] = (unsigned char)((chunk >> 8) & 0xFF);
            output[j++] = (unsigned char)(chunk & 0xFF);
            output[j] = '\0';
        } else {
            // Output fewer characters due to the padding
            switch(padLen) {
                default:
                    break;
                case 1:
                    // The first 2 characters are normal, the 3rd still has 2 bits
                    output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 8) & 0xFF);
                    output[j++] = (unsigned char)(chunk & 0xFF);
                    break;
                case 2:
                    // The first character is normal, the 2nd still has 4 bits
                    output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 8) & 0xFF);
                    break;
            }
            output[j] = '\0';
        }
    }
    if(finalOutputLen) {
        *finalOutputLen = j;
    }

    return (char *)output;
}

// WARNING:  This function allocates memory via calloc(), you must free it!
char *base32_decode(const char *input, size_t inputLen, size_t *finalOutputLen) {
    const char      *b32decodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    unsigned char   *output = NULL;
    size_t          outputLen = 0;

    // input 5-bit, output 8-bit, 8 bytes IN -> 5 bytes OUT.

    size_t  i = 0;
    size_t  j = 0;
    size_t  padLen = 0;

    if( !input || inputLen < 1 || inputLen % 4 != 0 ) {
        // Not properly padded
        return NULL;
    }

    for( size_t k = 0; k < inputLen; k++ ) {
        if( input[k] == '=' ) {
            // Pad byte
            continue;
        } else if( !strchr(b32decodingTable, (int)input[k]) ) {
            // Illegal character
            return NULL;
        }
    }

    outputLen = (inputLen / 8) * 5;
    output = (unsigned char *)calloc(outputLen + 1, sizeof(unsigned char));
    if(!output) {
        return NULL;
    }

    while( inputLen > 0 ) {
        // Remove pad bytes
        if( input[inputLen-1] == '=' ) {
            inputLen--;
            padLen++;
        } else {
            break;
        }
    }

    while( i < inputLen ) {
        size_t          start = i;
        uint64_t        chunk = 0;
        int             shiftAmounts[8] = { 35, 30, 25, 20, 15, 10, 5, 0 };

        for( size_t k = 0; k < 8; k++ ) {
            // We expect the input to be padded to chunks of 8 characters
            if( i < inputLen ) {
                char        *pos = NULL;
                size_t      x = 0;

                pos = strchr(b32decodingTable, (int)input[i]);
                x = (size_t)(pos - b32decodingTable);
                chunk |= x << shiftAmounts[k];
                i++;
            }
        }

        if( start <= inputLen - 8 ) {
            // Output 5 characters
            output[j++] = (unsigned char)((chunk >> 32) & 0xFF);
            output[j++] = (unsigned char)((chunk >> 24) & 0xFF);
            output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
            output[j++] = (unsigned char)((chunk >> 8) & 0xFF);
            output[j++] = (unsigned char)(chunk & 0xFF);
            output[j] = '\0';
        } else {
            // Output fewer characters due to the padding
            switch(padLen) {
                default:
                    break;
                case 1:
                    // The first 4 characters are normal, the 5th still has 3 bits
                    output[j++] = (unsigned char)((chunk >> 32) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 24) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 8) & 0xFF);
                    output[j++] = (unsigned char)(chunk & 0xFF);
                    break;
                case 3:
                    // The first 3 characters are normal, the 4th still has 1 bit
                    output[j++] = (unsigned char)((chunk >> 32) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 24) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 8) & 0xFF);
                    break;
                case 4:
                    // The first 2 characters are normal, the 3rd still has 4 bits
                    output[j++] = (unsigned char)((chunk >> 32) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 24) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 16) & 0xFF);
                    break;
                case 6:
                    // The first character is normal, the 2nd still has 2 bits
                    output[j++] = (unsigned char)((chunk >> 32) & 0xFF);
                    output[j++] = (unsigned char)((chunk >> 24) & 0xFF);
                    break;
            }
            output[j] = '\0';
        }
    }
    if(finalOutputLen) {
        *finalOutputLen = j;
    }

    return (char *)output;
}
