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
    dlog("called %s with %08x, %d, %d", __PRETTY_FUNCTION__, ht, rec_size, table_size);

  /*
   * int size; 
   */
  ht->rec_size = rec_size;
  ht->table_size = table_size;
  /*
   * if(!(ht->buckets = (void *)calloc(sizeof(struct hash_link **), table_size))) { bug("Cannot allocate hash bucket
   * list"); exit(1); } 
   */
  CREATE_VOID(ht->buckets, struct hash_link **, table_size);

  /*
   * if(!(ht->keylist = (void *)malloc(sizeof(*ht->keylist) * (ht->klistsize = 128)))) { bug("Cannot allocate hash key
   * list"); exit(1); } 
   */
  ht->klistsize = 128;
  CREATE_VOID(ht->keylist, *ht->keylist, ht->klistsize);
  ht->klistlen = 0;
}

void destroy_hash_table(struct hash_header *ht, funcp gman)
{
  int                                     i = 0;
  struct hash_link                       *scan = NULL;
  struct hash_link                       *temp = NULL;

  if (DEBUG > 2)
    dlog("called %s with %08x, %08x", __PRETTY_FUNCTION__, ht, gman);

  for (i = 0; i < ht->table_size; i++)
    for (scan = ht->buckets[i]; scan;) {
      temp = scan->next;
      (*gman) (scan->data);
      free(scan);
      scan = temp;
    }
  free(ht->buckets);
  free(ht->keylist);
}

static void _hash_enter(struct hash_header *ht, int key, void *data)
{							       /* precondition: there is no entry for <key> yet */
  struct hash_link                       *temp = NULL;
  int                                     i = 0;

  if (DEBUG > 2)
    dlog("called %s with %08x, %d, %08x", __PRETTY_FUNCTION__, ht, key, data);
  /*
   * if(!(temp = (void *)malloc(sizeof(*temp)))) { bug("Cannot allocate hash entry"); exit(1); } 
   */
  CREATE_VOID(temp, *temp, 1);
  temp->key = key;
  temp->next = ht->buckets[HASH_KEY(ht, key)];
  temp->data = data;
  ht->buckets[HASH_KEY(ht, key)] = temp;
  if (ht->klistlen >= ht->klistsize) {
    if (!(ht->keylist = (void *)realloc(ht->keylist, sizeof(*ht->keylist) *
					(ht->klistsize *= 2)))) {
      bug("Cannot grow hash entry");
      exit(1);
    }
  }
  for (i = ht->klistlen; i > 0; i--) {			       /* In empty lists, >= was causing access to element -1 */
    if (ht->keylist[i - 1] < key) {
      ht->keylist[i] = key;
      break;
    }
    ht->keylist[i] = ht->keylist[i - 1];
  }
  ht->klistlen++;
}

void                                   *hash_find(struct hash_header *ht, int key)
{
  struct hash_link                       *scan = NULL;

  if (DEBUG > 3)
    dlog("called %s with %08x, %d", __PRETTY_FUNCTION__, ht, key);

  scan = ht->buckets[HASH_KEY(ht, key)];

  while (scan && scan->key != key)
    scan = scan->next;

  return scan ? scan->data : NULL;
}

int hash_enter(struct hash_header *ht, int key, void *data)
{
  void                                   *temp = NULL;

  if (DEBUG > 2)
    dlog("called %s with %08x, %d, %08x", __PRETTY_FUNCTION__, ht, key, data);

  temp = hash_find(ht, key);
  if (DEBUG)
    dlog("hash_enter");
  if (temp)
    return 0;

  _hash_enter(ht, key, data);
  return 1;
}

void                                   *hash_find_or_create(struct hash_header *ht, int key)
{
  void                                   *rval = NULL;

  if (DEBUG > 2)
    dlog("called %s with %08x, %d", __PRETTY_FUNCTION__, ht, key);

  rval = hash_find(ht, key);
  if (rval)
    return rval;

  /*
   * if(!(rval = (void *)malloc(ht->rec_size))) { bug("Cannot allocate return for hash search"); exit(1); } 
   */
  CREATE_VOID(rval, char, ht->rec_size);
  _hash_enter(ht, key, rval);
  return rval;
}

void                                   *hash_remove(struct hash_header *ht, int key)
{
  struct hash_link                      **scan = NULL;

  if (DEBUG > 2)
    dlog("called %s with %08x, %d", __PRETTY_FUNCTION__, ht, key);

  scan = ht->buckets + HASH_KEY(ht, key);

  while (*scan && (*scan)->key != key)
    scan = &(*scan)->next;

  if (*scan) {
    int                                     i = 0;

    struct hash_link                       *temp = NULL;
    struct hash_link                       *aux = NULL;

    temp = (*scan)->data;
    aux = *scan;
    *scan = aux->next;
    free(aux);

    for (i = 0; i < ht->klistlen; i++)
      if (ht->keylist[i] == key)
	break;

    if (i < ht->klistlen) {
      bcopy(ht->keylist + i + 1, ht->keylist + i, (ht->klistlen - i) * sizeof(*ht->keylist));
      ht->klistlen--;
    }
    return temp;
  }
  return NULL;
}

void hash_iterate(struct hash_header *ht, funcp func, void *cdata)
{
  int                                     i = 0;

  if (DEBUG > 2)
    dlog("called %s with %08x, %08x, %08x", __PRETTY_FUNCTION__, func, cdata);

  for (i = 0; i < ht->klistlen; i++) {
    void                                   *temp = NULL;
    int                                     key = 0;

    key = ht->keylist[i];
    temp = hash_find(ht, key);
    (*func) (key, temp, cdata);
    if (ht->keylist[i] != key)				       /* They must have deleted this room */
      i--;						       /* Hit this slot again. */
  }
}
