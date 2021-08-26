#ifndef _HASH_H
#define _HASH_H

struct hash_link
{
    int key;
    struct hash_link *next;
    void *data;
};

struct hash_header
{
    int rec_size;
    int table_size;
    int *keylist, klistsize, klistlen; /* this is really lame, AMAZINGLY lame */
    struct hash_link **buckets;
};

#define HASH_KEY(ht, key) ((((unsigned int)(key)) * 17) % (ht)->table_size)

void init_hash_table(struct hash_header *ht, int rec_size, int table_size);
void destroy_hash_table(struct hash_header *ht, funcp gman);

/* static void _hash_enter(struct hash_header *ht, int key, void *data); */
void *hash_find(struct hash_header *ht, int key);
int hash_enter(struct hash_header *ht, int key, void *data);
void *hash_find_or_create(struct hash_header *ht, int key);
void *hash_remove(struct hash_header *ht, int key);
void hash_iterate(struct hash_header *ht, rfuncp f, void *cdata);

#endif
