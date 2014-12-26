#ifndef __LOGIN_H_
#define __LOGIN_H_

//-----------------------------------------------------------------------------
int playerLoginCB(void *data, int argc, char *argv[], char *colName[]) {
	// NOTE: check that the player's password matches and that
	// the account isn't currently in use
	struct Player *player = (struct Player *)data;

	int i;
	for(i=0; i<argc; i++) {
		if(!strcmp(colName[i], "id"))
			player->id = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "count"))
			player->count = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "state"))
			player->state = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "node"))
			player->node = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "x"))
			player->x = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "y"))
			player->y = (uint32_t) atoi(argv[i]);

		// NOTE: check that the password matches and set internal failure
		// state on mismatch
		if(!strcmp(colName[i], "password")) {
			if(strcmp(player->password, argv[i]))
				player->state = 0xFF;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
uint8_t playerLogin(struct Player *player) {
	// NOTE: check that the player's password matches
	char sqlCmd[0xFF] = {};

	sprintf(sqlCmd, "SELECT * FROM Players WHERE username = '%s';",
		player->username
	);

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, playerLoginCB, player, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	/*
	- player->state == 0xFF  (password did not match)
	- player->state == 0x00  (password match, currently offline)
	- player->state == other (player is currently online)
	*/

	return player->state;
}

#endif
