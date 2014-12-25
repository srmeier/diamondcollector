#ifndef __PLAYER_H_
#define __PLAYER_H_

//-----------------------------------------------------------------------------
int movePlayerUp(struct Player *chr) {
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "UPDATE Players SET Y = %d WHERE PlayerID = %d;", --chr->y, chr->id);

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------
int movePlayerDown(struct Player *chr) {
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "UPDATE Players SET Y = %d WHERE PlayerID = %d;", ++chr->y, chr->id);

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------
int movePlayerLeft(struct Player *chr) {
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "UPDATE Players SET X = %d WHERE PlayerID = %d;", --chr->x, chr->id);

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------
int movePlayerRight(struct Player *chr) {
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "UPDATE Players SET X = %d WHERE PlayerID = %d;", ++chr->x, chr->id);

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------
int getPlayerInfoCB(void *data, int argc, char *argv[], char *colName[]) {
	// NOTE: set the player struct
	uint8_t *retCode = &((struct dbData *)data)->retCode;
	uint32_t *playerID = &((struct dbData *)data)->playerID;
	struct Player *player = &((struct dbData *)data)->player;

	int i;
	for(i=0; i<argc; i++){
		if(!strcmp(colName[i], "PlayerID")) {
			player->id = (uint32_t) atoi(argv[i]);

		} else if(!strcmp(colName[i], "Username")) {
			int unSize = strlen(argv[i]);
			player->username = (char *)malloc(unSize+1);
			memcpy(player->username, argv[i], unSize);
			player->username[unSize] = '\0';

		} else if(!strcmp(colName[i], "Password")) {
			int pwSize = strlen(argv[i]);
			player->password = (char *)malloc(pwSize+1);
			memcpy(player->password, argv[i], pwSize);
			player->password[pwSize] = '\0';

		} else if(!strcmp(colName[i], "State")) {
			player->state = (uint32_t) atoi(argv[i]);

		} else if(!strcmp(colName[i], "Node")) {
			player->node = (uint32_t) atoi(argv[i]);

		} else if(!strcmp(colName[i], "X")) {
			player->x = (uint32_t) atoi(argv[i]);

		} else if(!strcmp(colName[i], "Y")) {
			player->y = (uint32_t) atoi(argv[i]);

		} else if(!strcmp(colName[i], "DiamondCount")) {
			player->count = (uint32_t) atoi(argv[i]);

		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
struct Player getPlayerInfo(uint32_t playerID) {
	// NOTE: get the information for a character with id = PlayerID
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE PlayerID = %d;", playerID);
	
	struct dbData data = {};
	data.playerID = playerID;

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, getPlayerInfoCB, &data, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	return data.player;
}

#endif
