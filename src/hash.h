
struct hash_link {
  int                              key;
  struct hash_link                *next;
  void                            *data;
};

struct hash_header {
  int                              rec_size;
  int                              table_size;
  int                             *keylist,
                                   klistsize,
                                   klistlen;	/*
						 * this is really lame,
						 * AMAZINGLY lame 
						 */
  struct hash_link               **buckets;
};

void                            *hash_find(struct hash_header *ht, int key);
int                              hash_enter(struct hash_header *ht, int key, void *data);
void                            *hash_find_or_create(struct hash_header *ht, int key);
void                            *hash_remove(struct hash_header *ht, int key);
