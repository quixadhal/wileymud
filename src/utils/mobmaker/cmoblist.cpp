#ifndef cmoblist_c
#define cmoblist_c

#include <iostream.h>
#include <stdio.h>
#include <ctype.h>

FILE *file_mob_in;
FILE *file_mob_out;

#include "cmob.h"
#include "cmoblist.h"

void cmoblist::list(void)
{
		sort();
		cmobelement *tmp = first; int lines = 0; char tmpchar;
		printf("%-15s%s\n","MOB NUMBER","MOB NAMES");
		while(tmp)
		{
				printf("#%-14d",tmp->item->get_number());
				tmp->item->display_name_list();
				printf("\n");
				if(!((lines+1)%24))
				{
						printf("MORE -- Press Any Key");
						tmpchar = getchar();
						if(toupper(tmpchar) == 'Q')
								return;
						printf("\n");
				}
				tmp = tmp->next; lines++;
		}
		printf("\n");
}


void cmoblist::sort(void)
{
		cmobelement *oldcurrent = current;
		cmobelement *b;
		int sorted = 0;

		current = first;

		while(!sorted)
		{
				sorted = 1;
				current = first;
				while(current->next)
				{
						if(current->item->get_number() < current->next->item->get_number())
						{
								current = current->next;
								continue;
						}
						// items need to be swapped.
						sorted = 0;
						b = current->next;
						if(first == current)
						{
								first = b;
						}
						if(last == b)
								last = current;
						if(current->prev)
						  current->prev->next = b;
						current->next = b->next;
						b->next = current;
						b->prev = current->prev;
						current->prev = b;
						// swapped;
						current = current->next;
				}
		}

		current = oldcurrent;
}

cmoblist::~cmoblist(void)
{
  cmobelement *tmp = first;
	while(tmp) {
			delete tmp->item;
			tmp = tmp->next;
			delete(tmp);
	}
}

cmob *cmoblist::remove(void)
{
  cmobelement *tmpprev = current->prev;
	cmobelement *tmpnext = current->next;
	delete current->item;
	delete current;
}

cmob *cmoblist::load(const char *filename)
{
		first = new(cmobelement);
		first->next = NULL;
		first->prev = NULL;
		current = first;

		first->item = new cmob(filename);
		first->item->load();
		while(!feof(file_mob_in))
		{
				cmobelement *tmp = current;
				current = (current->next = new cmobelement);
				current->prev = tmp;
				current->item = new cmob();
				current->item->load();
        printf("Loaded Mob #%d\r",current->item->get_number());
				current->next = NULL;
		}
		last = current;
		sort();
		current = first;
		return current->item;
}

void cmoblist::save(const char *filename)
{
		sort();
		cmobelement *cur = first;
		if(!first) return;
		cur->item->write(filename);
		while(cur->next)
		{
				cur = cur->next;
				cur->item->write();
		}
		fclose(file_mob_out);
} 

cmob *cmoblist::create(void)
{
		//special case, no items:
		if(!first)
		{
				first = last = current = new cmobelement;
				current->next = NULL;
				current->prev = NULL;
				current->item = new cmob;
				return current->item;
		}
		sort();
		cmobelement *newitem = new cmobelement;
		last->next = newitem;
		newitem->prev = last;
		current = (last = newitem);
		newitem->next = NULL;
		newitem->item = new cmob;
		newitem->item->set_number(last->prev->item->get_number()+1);
		// this is where code should be to assign a new number
		return newitem->item;
}

cmob *cmoblist::gomob(long mobnumber)
{
		sort();
		cmobelement *tmp = first;
		
		while(tmp)
		{
				if(tmp->item->get_number() == mobnumber)
						break;
				tmp = tmp->next;
		}
		if(!tmp)
		{
				printf("Mob not found!\n");
				return current->item;
		}
		current = tmp;
		return current->item;
}

cmob *cmoblist::gomob(const char *mobname)
{
		sort();
		cmobelement *tmp = first;

		while(tmp)
		{
				if(tmp->item->areyou(mobname))
						break;
				tmp = tmp->next;
		}
		if(!tmp)
		{
				printf("Mob not found!\n");
				return current->item;
		}
		current = tmp;
		return current->item;
}

cmob *cmoblist::next(void)
{
		if(current->next)
		{
				current = current->next;
				return current->item;
		}
		printf("No more mobs!\n");
		return current->item;
}

cmob *cmoblist::prev(void)
{
		if(current->prev)
		{
				current = current->prev;
				return current->item;
		}
		printf("No more mobs!\n");
		return current->item;
}

#endif

