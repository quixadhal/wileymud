#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <strings.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _DUMP_ISLES_C
#include "include/dump_isles.h"

const int                               isles_convertsectors[] = { ISLES_SECT_INSIDE,
    ISLES_SECT_INSIDE,
    ISLES_SECT_CITY,
    ISLES_SECT_FIELD,
    ISLES_SECT_FOREST,
    ISLES_SECT_HILLS,
    ISLES_SECT_MOUNTAIN,
    ISLES_SECT_WATER_SWIM,
    ISLES_SECT_WATER_NOSWIM,
    ISLES_SECT_AIR,
    ISLES_SECT_UNDERWATER
};

const int                              *isles_convertsector = &isles_convertsectors[1];

extern const char                       terrains[];
extern const char                      *terrain;
extern const int                        exit_dir_to_bit[];

int isles_qcmp_zone_vnum(const void *a, const void *b)
{
    return (((const pair *)b)->y - ((const pair *)a)->y);
}

void dump_as_isles(zones *Zones, rooms *Rooms, shops *Shops)
{
    FILE                                   *ofp,
                                           *listfp;
    char                                    filename[256],
                                            tmpstr[256];
    int                                     i,
                                            j,
                                            k,
                                            x;

    /*
     * int LastMob, LastLoc; 
     */
    int                                     LowRoom,
                                            HighRoom;
    pair                                   *ZoneSort;

    ZoneSort = (pair *)get_mem(Zones->Count, sizeof(pair));
    bzero(ZoneSort, sizeof(pair) * Zones->Count);
    for (i = 0; i < Zones->Count; i++) {
	sprintf(filename, "%s/%s/%s_%d.zone", OutputDir, ISLES_SUBDIR,
		remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
	ofp = open_file(filename, "w");
	if (Verbose)
	    fprintf(stderr, "Dump of Zone \"%s\"[#%d]...\n", remap_name(Zones->Zone[i].Name),
		    Zones->Zone[i].Number);
	else if (!Quiet) {
	    sprintf(tmpstr, "#%d Dump of Zone \"%s\"[#%d]...", i + 1,
		    remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
	    fprintf(stderr, "%s", tmpstr);
	    for (x = strlen(tmpstr); x < 79; x++)
		fprintf(stderr, " ");
	    for (x = strlen(tmpstr); x < 79; x++)
		fprintf(stderr, "\b");
	    fflush(stderr);
	}
	LowRoom = INT_MAX;
	HighRoom = INT_MIN;
	for (j = 0; j < Rooms->Count; j++) {
	    if ((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
		((Rooms->Room[j].Number >= (!i ? 0 : Zones->Zone[i - 1].Top + 1))&&
		 (Rooms->Room[j].Number <= Zones->Zone[i].Top)
		)) {
		LowRoom = min(LowRoom, Rooms->Room[j].Number);
		HighRoom = max(HighRoom, Rooms->Room[j].Number);
	    }
	}
	ZoneSort[i].x = i;
	ZoneSort[i].y = LowRoom;

/* ZONE */

	fprintf(ofp, "#ZONEDATA\n");
	fprintf(ofp, "Name\t%s~\n", Zones->Zone[i].Name);
	fprintf(ofp, "Author\tThe Wiley Gang~\n");
	fprintf(ofp, "VNUMs\t%d %d\n", LowRoom, HighRoom);
	fprintf(ofp, "Time\t%d\n", Zones->Zone[i].Time);
	fprintf(ofp, "Mode\t%d\n", Zones->Zone[i].Mode);
	fprintf(ofp, "End\n\n");
	fprintf(ofp, "#MOBDATA\n#0\n\n");
	fprintf(ofp, "#OBJDATA\n#0\n\n");
	fprintf(ofp, "#ROOMDATA\n");
	for (j = 0; j < Rooms->Count; j++) {
	    if ((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
		((Rooms->Room[j].Number >= (!i ? 0 : Zones->Zone[i - 1].Top + 1))&&
		 (Rooms->Room[j].Number <= Zones->Zone[i].Top)
		)) {
		long                                    OldValue = 0,
		    NewValue = 0;

		if (Verbose)
		    fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", i,
			    Rooms->Room[i].Name, Rooms->Room[i].Number);
		else if (!Quiet)
		    spin(stderr);

		fprintf(ofp, "#%d\n", Rooms->Room[j].Number);
		fprintf(ofp, "Name\t%s~\n", Rooms->Room[j].Name);
		fprintf(ofp, "Descr\n%s~\n", Rooms->Room[j].Description);

/*
 * flag and sectors need translation
 * a nice routine should be written... I'm lazy... this is cheap and cheezy!
 */
		OldValue = Rooms->Room[j].              Flags;

		if (OldValue & ROOM_DARK)
		    NewValue |= ISLES_ROOM_DARK;
		if (OldValue & ROOM_DEATH)
		    NewValue |= ISLES_ROOM_IMMORTAL;
		if (OldValue & ROOM_NOMOB)
		    NewValue |= ISLES_ROOM_NO_MOB;
		if (OldValue & ROOM_INDOORS)
		    NewValue |= ISLES_ROOM_INDOORS;
		if (OldValue & ROOM_NOATTACK)
		    NewValue |= ISLES_ROOM_SAFE;
		/*
		 * if(OldValue & ROOM_NOSTEAL) NewValue |= 
		 */
		if (OldValue & ROOM_NOSUMMON)
		    NewValue |= ISLES_ROOM_NO_RECALL;
		/*
		 * if(OldValue & ROOM_NOMAGIC) NewValue |= 
		 */
		if (OldValue & ROOM_PRIVATE)
		    NewValue |= ISLES_ROOM_PRIVATE;
		/*
		 * if(OldValue & ROOM_SOUND) NewValue |= 
		 */
		fprintf(ofp, "Flags\t%ld\n", NewValue);
		fprintf(ofp, "Sector\t%d\n", isles_convertsector[Rooms->Room[j].Sector]);

		for (k = 0; k < Rooms->Room[j].ExtraCount; k++) {
		    fprintf(ofp, "ExtraDescr\t");
		    if (Rooms->Room[j].Extra[k].Keyword->Count > 0) {
			fprintf(ofp, "%s", Rooms->Room[j].Extra[k].Keyword->Word[0]);

			for (x = 0; x < Rooms->Room[j].Extra[k].Keyword->Count; x++) {
			    fprintf(ofp, " %s", Rooms->Room[j].Extra[k].Keyword->Word[x]);
			}
		    }
		    fprintf(ofp, "~\n%s~\n", Rooms->Room[j].Extra[k].Description);
		}
/* Resets need to be done here.... later... */
		for (k = 0; k < Rooms->Room[j].ExitCount; k++) {
		    switch (Rooms->Room[j].Exit[k].Error) {
			case EXIT_OK:
			case EXIT_NON_EUCLIDEAN:
			case EXIT_ONE_WAY:
			    fprintf(ofp, "Door\t%d %d %d %d\n%s~\n", Rooms->Room[j].Exit[k].Direction, 0,	/* reset 
														 * flags... 
														 * later... 
														 */
				    Rooms->Room[j].Exit[k].KeyNumber,
				    Rooms->Room[j].Exit[k].Room,
				    Rooms->Room[j].Exit[k].Description);
			    if (Rooms->Room[j].Exit[k].Keyword->Count > 0) {
				fprintf(ofp, "%s", Rooms->Room[j].Exit[k].Keyword->Word[0]);

				for (x = 0; x < Rooms->Room[j].Exit[k].Keyword->Count; x++) {
				    fprintf(ofp, " %s",
					    Rooms->Room[j].Exit[k].Keyword->Word[x]);
				}
			    }
			    fprintf(ofp, "~\n");
		    }
		}
		fprintf(ofp, "End\n\n");
	    }
	}
	fprintf(ofp, "#0\n\n");
	fprintf(ofp, "#$\n");
	fclose(ofp);
	if (Verbose)
	    fprintf(stderr, "done.\n");
	else if (!Quiet) {
	    fprintf(stderr, "done.\r");
	    fflush(stderr);
	}
    }
    if (!Quiet)
	fprintf(stderr, "\n");
    if (!Quiet)
	fprintf(stderr, "Generating zone list...");
    qsort(ZoneSort, Zones->Count, sizeof(pair), isles_qcmp_zone_vnum);
    sprintf(filename, "%s/%s/zone.list", OutputDir, ISLES_SUBDIR);
    listfp = open_file(filename, "w");
    for (i = Zones->Count - 1; i >= 0; i--) {
	if (!Quiet)
	    spin(stderr);
	sprintf(filename, "%s_%d.zone", remap_name(Zones->Zone[ZoneSort[i].x].Name),
		Zones->Zone[i].Number);
	fprintf(listfp, "%s\n", filename);
    }
    fprintf(listfp, "$\n");
    fclose(listfp);
    if (!Quiet)
	fprintf(stderr, "done.\n");
}
