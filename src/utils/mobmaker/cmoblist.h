#ifndef cmoblist_h
#define cmoblist_h

#define NUMFLAGS 23 
#define MAX_STR_LENGTH 1500
#define MAX_SKILLS 10

#include "cmob.h"

class cmoblist
{
private:
	struct cmobelement {
			cmob *item;
			cmobelement *prev;
			cmobelement *next;
	};
	cmobelement *first;
	cmobelement *last;
	cmobelement *current;

	void sort(void);
public:
	cmoblist() { first = last = current = NULL; }
	~cmoblist();
	cmob *load(const char *);
	void save(const char *);
	cmob *create(void);
	cmob *gomob(long);
	cmob *gomob(const char *);
	cmob *next(void);
	cmob *prev(void);
  cmob *remove(void);
	void list(void);
};

#endif

