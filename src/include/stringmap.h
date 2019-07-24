#ifndef _STRINGMAP_H
#define _STRINGMAP_H

struct s_stringMap;

typedef struct s_stringMap {
    char                *key;
    void                *value;
    size_t               size;
    struct s_stringMap  *next;
} stringMap;

#define STRINGMAP_BUCKETS 100

#ifndef _STRINGMAP_C
#else
#endif

unsigned int _stringmap( const char *s );
stringMap * stringmap_init( void );
void _stringmap_recursive_nuke( stringMap *map );
void stringmap_destroy( stringMap *map );
void stringmap_add( stringMap *map, const char *k, void *v, size_t size );
void * stringmap_find(stringMap *map, const char *k);
stringMap * stringmap_walk(stringMap *map, int reset);

#endif

