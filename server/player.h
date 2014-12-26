#ifndef __PLAYER_H_
#define __PLAYER_H_

//-----------------------------------------------------------------------------
void playerSave(struct Player *player) {
	// NOTE: save the player's state
	char sqlCmd[0xFF] = {};

	// NOTE: set the sql query
	sprintf(
		sqlCmd,
		"UPDATE Players SET username = '%s',"\
		"password = '%s', count = %d, state = %d,"\
		"host = %d, port = %d, node = %d, x = %d, y = %d "\
		"WHERE id = %d;",
		player->username,
		player->password,
		player->count,
		player->state,
		player->host,
		player->port,
		player->node,
		player->x,
		player->y,
		player->id
	);

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}
}

//-----------------------------------------------------------------------------
int getFreePlayerIndex(uint32_t node) {
	// NOTE: the player should never be on a node that is out of bounds
	if(node>=NODE_MAX||node<0) return -2;

	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: return an index which is free in the pool. the mask should
		// be set to flase elsewhere
		if(!playerIndexMask[node][i]) return i;
	}
	
	// NOTE: if the node is full return -1
	return -1;
}

/*
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
*/

#endif
