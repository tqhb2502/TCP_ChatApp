#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "network.h"
#include "server.h"
#include "db.h"
#include <sqlite3.h>

#define MAX_SQL_SIZE 3072

sqlite3 *Create_room_sqlite(Package *pkg)
{
    sqlite3 *database;
    char name[23];
    sprintf(name, "%d.db", pkg->group_id);
    int ret = sqlite3_open(name, &database);
    if (ret != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        exit(0);
    }
    return database;
}

void save_chat(Package *pkg)
{
    sqlite3 *database = Create_room_sqlite(pkg);

    char *errmsg = NULL;
    char buf[MAX_SQL_SIZE] = "create table if not exists chat(time TEXT, sender TEXT, message TEXT)";
    int ret = sqlite3_exec(database, buf, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("Open table failed\n");
        return;
    }

    time_t t;
    t = time(&t);
    char time[100];
    strcpy(time, ctime(&t));
    int len = strlen(time);
    time[len - 1] = '\0';
    sprintf(buf, "insert into chat values('%s','%s','%s')", time, pkg->sender, pkg->msg);
    ret = sqlite3_exec(database, buf, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("Failed to insert data\n");
        return;
    }
    sqlite3_close(database);
}

void see_chat(Package *pkg)
{
    sqlite3 *database = Create_room_sqlite(pkg);
    int ret;
    char *errmsg = NULL;
    char buf[MAX_SQL_SIZE] = "create table if not exists chat(time TEXT, sender TEXT, message TEXT)";
    ret = sqlite3_exec(database, buf, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("Open table failed\n");
        return;
    }

    // xem lich su
    char **resultp = NULL;
    int nrow, ncolumn;
    char *sq1 = "select * from chat";
    ret = sqlite3_get_table(database, sq1, &resultp, &nrow, &ncolumn, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("Database operation failed\n");
        printf("sqlite3_get_table: %s\n", errmsg);
        return;
    }
    int i;
    for (i = 3; i < (nrow + 1) * ncolumn; i++)
    {
        if (i % 3 == 0)
        {
            printf("\n");
            printf("%-25s", resultp[i]);
        }
        else
            printf("%15s", resultp[i]);
    }
    printf("\n");

    sqlite3_free_table(resultp);

    sqlite3_close(database);

    sleep(1);
}

void drop_table(int group_id)
{
    sqlite3 *database;
    char name[23];
    sprintf(name, "%d.db", group_id);
    int ret = sqlite3_open(name, &database);
    if (ret != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        exit(0);
    }

    char *errmsg = NULL;

    ret = sqlite3_exec(database, "drop table IF EXISTS chat", 0, 0, &errmsg);

    if (ret != SQLITE_OK)
    {

        printf("Error in SQL statement: %s\n", errmsg);

        sqlite3_free(errmsg);
        sqlite3_close(database);
        return;
    }
    printf("Chat table dropped successfully\n");
    sqlite3_close(database);
}
// int main(int argc, char *argv[])
// {
//     sqlite3 *db;
//     Package pkg1;
//     strcpy(pkg1.sender, "tuanchibi");
//     strcpy(pkg1.msg, "hello");
//     pkg1.group_id = 1;
//     char *zErrMsg = 0;

//     save_chat(&pkg1);
//     see_chat(&pkg1);
//     drop_table(pkg1.group_id);
//     see_chat(&pkg1);
//     return 0;
// }