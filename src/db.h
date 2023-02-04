#ifndef __DB_H__
#define __DB_H__

#include "network.h"
#include <sqlite3.h>

sqlite3 *Create_room_sqlite(Package *pkg);
void save_chat(Package *pkg);
void drop_table(int group_id);

#endif
