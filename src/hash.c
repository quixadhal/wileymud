#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include "global.h"
#include "bug.h"
#include "comm.h"
#include "utils.h"
#define _HASH_C
#include "hash.h"

void init_hash_table(struct hash_header *ht, int rec_size, int table_size)
{
    if (DEBUG > 2)
        log_info("called %s with %08zx, %d, %d", __PRETTY_FUNCTION__, (size_t)ht, rec_size, table_size);

    /*
     * int size;
     */
    ht->rec_size = rec_size;
    ht->table_size = table_size;
    /*
     * if(!(ht->buckets = (void *)calloc(sizeof(struct hash_link **), table_size))) { log_error("Cannot allocate hash
     * bucket list"); proper_exit(MUD_HALT); }
     */
    CREATE(ht->buckets, struct hash_link *, table_size);

    /*
     * if(!(ht->keylist = (void *)malloc(sizeof(*ht->keylist) * (ht->klistsize = 128)))) { log_error("Cannot allocate
     * hash key list"); proper_exit(MUD_HALT); }
     */
    ht->klistsize = 128;
    CREATE(ht->keylist, int, ht->klistsize);
    ht->klistlen = 0;
}

void destroy_hash_table(struct hash_header *ht, hfuncp gman)
{
    int i = 0;
    struct hash_link *scan = NULL;
    struct hash_link *temp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)ht, (size_t)gman);

    for (i = 0; i < ht->table_size; i++)
        for (scan = ht->buckets[i]; scan;)
        {
            temp = scan->next;
            (*gman)(scan->data);
            free(scan);
            scan = temp;
        }
    free(ht->buckets);
    free(ht->keylist);
}

static void _hash_enter(struct hash_header *ht, int key, void *data)
{ /* precondition: there is no entry for <key> yet */
    struct hash_link *temp = NULL;
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %d, %08zx", __PRETTY_FUNCTION__, (size_t)ht, key, (size_t)data);
    /*
     * if(!(temp = (void *)malloc(sizeof(*temp)))) { log_error("Cannot allocate hash entry"); proper_exit(MUD_HALT); }
     */
    CREATE(temp, struct hash_link, 1);
    temp->key = key;
    temp->next = ht->buckets[HASH_KEY(ht, key)];
    temp->data = data;
    ht->buckets[HASH_KEY(ht, key)] = temp;
    if (ht->klistlen >= ht->klistsize)
    {
        if (!(ht->keylist = (int *)realloc(ht->keylist, sizeof(*ht->keylist) * (ht->klistsize *= 2))))
        {
            log_fatal("Cannot grow hash entry");
            proper_exit(MUD_HALT);
        }
    }
    for (i = ht->klistlen; i > 0; i--)
    { /* In empty lists, >= was causing access to element -1 */
        if (ht->keylist[i - 1] < key)
        {
            ht->keylist[i] = key;
            break;
        }
        ht->keylist[i] = ht->keylist[i - 1];
    }
    ht->klistlen++;
}

void *hash_find(struct hash_header *ht, int key)
{
    struct hash_link *scan = NULL;

    if (DEBUG > 3)
        log_info("called %s with %08zx, %d", __PRETTY_FUNCTION__, (size_t)ht, key);

    scan = ht->buckets[HASH_KEY(ht, key)];

    while (scan && scan->key != key)
        scan = scan->next;

    return scan ? scan->data : NULL;
}

int hash_enter(struct hash_header *ht, int key, void *data)
{
    void *temp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %d, %08zx", __PRETTY_FUNCTION__, (size_t)ht, key, (size_t)data);

    temp = hash_find(ht, key);
    if (DEBUG)
        log_info("hash_enter");
    if (temp)
        return FALSE;

    _hash_enter(ht, key, data);
    return TRUE;
}

void *hash_find_or_create(struct hash_header *ht, int key)
{
    void *rval = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %d", __PRETTY_FUNCTION__, (size_t)ht, key);

    rval = hash_find(ht, key);
    if (rval)
        return rval;

    /*
     * if(!(rval = (void *)malloc(ht->rec_size))) { log_error("Cannot allocate return for hash search");
     * proper_exit(MUD_HALT); }
     */
    CREATE_VOID(rval, char, ht->rec_size);
    _hash_enter(ht, key, rval);
    return rval;
}

void *hash_remove(struct hash_header *ht, int key)
{
    struct hash_link **scan = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %d", __PRETTY_FUNCTION__, (size_t)ht, key);

    scan = ht->buckets + HASH_KEY(ht, key);

    while (*scan && (*scan)->key != key)
        scan = &(*scan)->next;

    if (*scan)
    {
        int i = 0;

        struct hash_link *temp = NULL;
        struct hash_link *aux = NULL;

        temp = (struct hash_link *) (*scan)->data;
        aux = *scan;
        *scan = aux->next;
        free(aux);

        for (i = 0; i < ht->klistlen; i++)
            if (ht->keylist[i] == key)
                break;

        if (i < ht->klistlen)
        {
            bcopy(ht->keylist + i + 1, ht->keylist + i, (ht->klistlen - i) * sizeof(*ht->keylist));
            ht->klistlen--;
        }
        return temp;
    }
    return NULL;
}

void hash_iterate(struct hash_header *ht, rfuncp f, void *cdata)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)ht, (size_t)f, (size_t)cdata);

    for (i = 0; i < ht->klistlen; i++)
    {
        void *temp = NULL;
        int key = 0;

        key = ht->keylist[i];
        temp = hash_find(ht, key);
        (*f)(key, (ROOM_DATA *)temp, cdata);
        if (ht->keylist[i] != key) /* They must have deleted this room */
            i--;                   /* Hit this slot again. */
    }
}
