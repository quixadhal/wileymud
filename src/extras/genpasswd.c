#include <stdio.h>
#include <string.h>
#define _XOPEN_SOURCE
#include <unistd.h>

extern char *crypt(const char *key, const char *salt);

int main(int argc, char **argv)
{
    char key[9];
    char salt[3];

    if (argc != 3)
    {
        fprintf(stderr, "usage:  %s <key> <salt>\n", argv[0]);
        return 1;
    }
    salt[0] = argv[2][0];
    salt[1] = argv[2][1];
    salt[2] = '\0';
    strncpy(key, argv[1], 8);
    key[8] = '\0';
    printf("%s\n", crypt(key, salt));
    return 0;
}
