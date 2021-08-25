#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <pcre.h>
#include "global.h"
#include "bug.h"
#include "sql.h"
#define _STRINGMAP_C
#include "stringmap.h"

unsigned int _stringmap(const char *s)
{
    unsigned int hash = 0;

    if (!s || !*s)
        return 0;
    do
    {
        hash += *s;
        hash *= 13;
        s++;
    } while (*s);

    return hash % STRINGMAP_BUCKETS;
}

void _stringmap_recursive_nuke(stringMap *map)
{
    if (!map)
        return;

    if (map->key)
    {
        free(map->key);
        map->key = NULL;
    }
    if (map->value)
    {
        free(map->value);
        map->value = NULL;
    }
    map->size = 0;
    if (map->next)
    {
        _stringmap_recursive_nuke(map->next);
        free(map->next);
        map->next = NULL;
    }
}

void stringmap_destroy(stringMap *map)
{
    if (!map)
        return;

    for (int i = 0; i < STRINGMAP_BUCKETS; i++)
    {
        if (map[i].key)
        {
            free(map[i].key);
            map[i].key = NULL;
        }
        if (map[i].value)
        {
            free(map[i].value);
            map[i].value = NULL;
        }
        map[i].size = 0;
        if (map[i].next)
        {
            _stringmap_recursive_nuke(map->next);
            map[i].next = NULL;
        }
    }
}

stringMap *stringmap_init(void)
{
    int i;
    stringMap *map = (stringMap *)calloc(STRINGMAP_BUCKETS, sizeof(stringMap));

    for (i = 0; i < STRINGMAP_BUCKETS; i++)
    {
        map[i].key = NULL;
        map[i].value = NULL;
        map[i].size = 0;
        map[i].next = NULL;
    }

    return map;
}

void stringmap_add(stringMap *map, const char *k, void *v, size_t size)
{
    unsigned int hashcode;
    stringMap *p;

    // We won't allow NULL keys
    if (!k || !*k)
        return;

    hashcode = _stringmap(k);
    p = &map[hashcode];
    while (p->key && strcasecmp(p->key, k) && p->next)
        p = p->next;

    if (!p->key)
    {
        /* First node? */
        p->key = (char *)strdup(k);
        p->value = (void *)calloc(1, size);
        memcpy(p->value, v, size);
        p->size = size;
        p->next = NULL;
    }
    else if (!strcasecmp(p->key, k))
    {
        /* Found our match! */
        if (p->value)
            free((void *)p->value);
        p->value = (void *)calloc(1, size);
        memcpy(p->value, v, size);
        p->size = size;
    }
    else
    {
        /* New key */
        p->next = (stringMap *)calloc(1, sizeof(stringMap));
        p = p->next;
        p->key = (char *)strdup(k);
        p->value = (void *)calloc(1, size);
        memcpy(p->value, v, size);
        p->size = size;
        p->next = NULL;
    }
}

void *stringmap_find(stringMap *map, const char *k)
{
    unsigned int hashcode;
    stringMap *p;

    hashcode = _stringmap(k);
    p = &map[hashcode];
    while (p->key && strcasecmp(p->key, k) && p->next)
        p = p->next;

    if (!p->key)
        return NULL;

    if (!strcasecmp(p->key, k))
        return p->value;

    return NULL;
}

/*
 * This will walk the stringMap structure, returning an entry
 * each call until it hits the end, where it will return NULL.
 *
 * This is NOT thread safe, and is also not safe if you make
 * any modifications to the stringMap structure before finishing.
 *
 * It should be called with reset TRUE the first time, and then
 * called with reset FALSE until NULL is returned.
 *
 * The stringMap entry's key and value elements can be used
 * but any other fields are subject to internal changes.  They
 * should not be modified, or the find functions will fail.
 */
stringMap *stringmap_walk(stringMap *map, int reset)
{
    static int bucket = 0;
    static stringMap *listptr = NULL;

    if (!map)
        return NULL;

    if (reset)
    {
        bucket = 0;
        listptr = &map[bucket];
        for (int i = 0; i < STRINGMAP_BUCKETS; i++)
        {
            if (map[i].key)
            {
                // There is something in this bucket, so let's start here.
                bucket = i;
                listptr = &map[bucket];
                return listptr;
            }
        }
        listptr = NULL;
        return NULL; // We found no valid entries at all.
    }

    if (!listptr)
        return NULL; // We already hit the end, reset or stop.

    if (listptr->next)
    {
        // Walk the bucket
        listptr = listptr->next;
        return listptr;
    }
    else
    {
        // No more nodes in this bucket.
        bucket++;
        for (int i = bucket; i < STRINGMAP_BUCKETS; i++)
        {
            if (map[i].key)
            {
                // There is something in this bucket, so let's continue from here.
                bucket = i;
                listptr = &map[bucket];
                return listptr;
            }
        }
        bucket = STRINGMAP_BUCKETS;
        listptr = NULL;
        return NULL; // We found no more valid entries.
    }

    return NULL;
}
