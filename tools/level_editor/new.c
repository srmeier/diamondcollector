/*
gcc new.c -o new.exe -I./include -L./lib -lsqlite3
- create a new sqlite database to store all the level information
*/

//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sqlite3.h"

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	sqlite3 *db = NULL;

	// NOTE: check for the correct number of arguements
	if(argc<2) {
		fprintf(stderr, "Error: need a level name.\n");
		return -1;
	}

	// NOTE: add the sqlite db to the levels directory
	char filename[0xFF];
	sprintf(filename, "levels/%s.db", argv[1]);

	// NOTE: open database connection
	if(sqlite3_open(filename, &db)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	// NOTE: setup the tiles table
	const char *cmd = ""\
	"CREATE TABLE tiles ("\
		"x        INTEGER,"\
		"y        INTEGER,"\
		"layer    INTEGER,"\
		"sprite   INTEGER,"\
		"[action] INTEGER"\
	");";

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(db, cmd, NULL, NULL, &errorMsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		return -1;
	}

	// NOTE: database created!
	printf("\nNOTE: created a database for level %s.\n\n", argv[1]);

	// NTOE: close the database connection
	sqlite3_close(db);
	db = NULL;

	return 0;
}
