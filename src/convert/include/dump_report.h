#ifndef _DUMP_REPORT_H
#define _DUMP_REPORT_H

void make_zone_report(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, char *outfile);
void make_shop_report(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outfile);
void make_room_report(zones *Zones, rooms *Rooms, char *outfile);
void make_obj_report(zones *Zones, objects *Objects, char *outfile);
void make_mob_report(zones *Zones, mobs *Mobs, char *outfile);

#endif
