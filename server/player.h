#ifndef __PLAYER_H_
#define __PLAYER_H_

//-----------------------------------------------------------------------------
int getPlayerIndex(struct Player *player) {
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: will search at most PLAYER_MAX
		if(!playerIndexMask[player->node][i])
			continue;
		if(player->id == players[player->node][i].id)
			return i;
	}

	// NOTE: player not found in players array
	return -1;
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

//-----------------------------------------------------------------------------
int getPlayerNodeCB(void *data, int argc, char *argv[], char *colName[]) {
	// NOTE: return the player node
	uint32_t *node = (uint32_t *)data;

	int i;
	for(i=0; i<argc; i++) {
		if(!strcmp(colName[i], "node"))
			*node = (uint32_t) atoi(argv[i]);
	}

	return 0;
}

//-----------------------------------------------------------------------------
uint32_t getNumOnNode(uint32_t node) {
	// NOTE: the player should never be on a node that is out of bounds
	if(node>=NODE_MAX||node<0) return -2;

	int i, numOnNode = 0;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: return the number of players on a node
		if(playerIndexMask[node][i]) numOnNode++;
	}
	
	return numOnNode;
}

//-----------------------------------------------------------------------------
uint32_t getPlayerNode(IPaddress ip) {
	// NOTE: get the player's node
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE host = %d AND port = %d;",
		ip.host,
		ip.port
	);

	// NOTE: if the player isn't on the node return -1
	uint32_t node = -1;

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, getPlayerNodeCB, &node, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	return node;
}

//-----------------------------------------------------------------------------
struct Player* getPlayerFromIP(IPaddress ip) {
	// NOTE: get the player's node
	uint32_t node = getPlayerNode(ip);

	// NOTE: most likely due to player being offline or someone else
	// trying send a packet acting as them
	if(node==-1) return NULL;

	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: will search at most PLAYER_MAX
		if(!playerIndexMask[node][i])
			continue;

		int check00 = players[node][i].host==ip.host;
		int check01 = players[node][i].port==ip.port;

		if(check00 && check01)
			return &players[node][i];
	}

	// NOTE: couldn't find player - possibly due to a
	// auto-disconnect due to failed pong reply
	return NULL;
}

//-----------------------------------------------------------------------------
void playerSave(struct Player *player) {
	// NOTE: save the player's state
	char sqlCmd[0xFFFF] = {};

	// NOTE: set the sql query
	sprintf(sqlCmd,
		"UPDATE Players SET username = '%s',"\
		"password = '%s', count = %d, state = %d,"\
		"host = %d, port = %d, node = %d, x = %d, y = %d "\
		"WHERE id = %d;",
		player->username, player->password, player->count,
		player->state, player->host, player->port, player->node,
		player->x, player->y, player->id
	);

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}
}

#endif
