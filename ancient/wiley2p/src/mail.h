#define MAX_MSGS 10                    /* Max number of messages.          */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */

struct MailRecord
{
    char from[40];
    long int date_sent;
    long int date_read;

    char msg[MAX_MAIL_LENGTH];
    int keep;
    int new;
};

struct Mail
{
    int NumberOfMessages;
    struct MailRecord *mail_record[MAX_MSGS];
    FILE *file;
    char filename[40];
};

