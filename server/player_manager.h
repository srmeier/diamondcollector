#ifndef __PLAYERMANAGER_H_
#define __PLAYERMANAGER_H_

#define NODE_MAX 64
/*
- maximum "screens" which the player may travel between
*/

#define PLAYER_MAX 32
/*
- the maximum number of players on any one screen
*/

/* player database information */
//-----------------------------------------------------------------------------
typedef struct {
	uint32_t x;
	/*
	- the player's x coordinate on the screen node
	*/

	uint32_t y;
	/*
	- the player's y coordinate on the screen node
	*/

	uint32_t id;
	/*
	- the player's primary key within the SQLite database
	*/

	uint32_t host;
	/*
	- the host address (client IP) for the current player connection
	*/

	uint16_t port;
	/*
	- the port address for the current player connection
	*/

	uint32_t node;
	/*
	- the player's current node location within the game world
	*/

	char *username;
	/*
	- the player's in-game username
	*/

	char *password;
	/*
	- the player's database password
	*/

	uint32_t state;
	/*
	- the current player's state (online, offline, etc.)
	*/

	uint32_t count;
	/*
	- the number of diamonds this player currently possesses
	*/
} DB_Player;

/* player manager handle */
//-----------------------------------------------------------------------------
typedef struct {
	sqlite3 *db;
	/*
	- SQLite database connection: used to save a player's information to the
		SQLite database
	*/

	uint32_t pl_moveDelay;
	/*
	- amount of time (in milisecs) to wait before accepting another player
		movement update
	*/

	DB_Player pl_dbInfo[NODE_MAX][PLAYER_MAX]; // 64 kilobytes
	/*
	- player's database information: a pool of memory for the maximum possible
		player connections
	*/

	SDL_bool pl_indMask[NODE_MAX][PLAYER_MAX]; //  8 kilobytes
	/*
	- player's index mask: used to determine if a player slot if free for a new
		player
	*/

	uint32_t pl_lMoveTime[NODE_MAX][PLAYER_MAX]; //  8 kilobytes
	/*
	- player's last move time: this is used along with pl_moveDelay to
		determine if a player's input packet should be ignored or not
	*/
} PlayerManager;

//-----------------------------------------------------------------------------
/* initPlayerManager
*/

PlayerManager* initPlayerManager(const char *fileName) {
	/*
	- load the player manager resources
	*/

	// NOTE: clear the player manager's memory
	PlayerManager *pm = (PlayerManager *)calloc(1, sizeof(PlayerManager));

	// NOTE: open database connection
	if(sqlite3_open(fileName, &pm->db)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(pm->db));
		sqlite3_close(pm->db);
		return NULL;
	}

	// NOTE: set the move delay time
	pm->pl_moveDelay = 0xFF;

	return pm;
}

//-----------------------------------------------------------------------------
/* pl_loadCB
*/

int pl_loadCB(void *data, int argc, char *argv[], char *colName[]) {
	/*
	- run through all the database information and set the player structure
	- this is a SQLite callback function
	*/

	// NOTE: set the player's information
	DB_Player *pl = (DB_Player *)data;

	int i;
	for(i=0; i<argc; i++) {
		// NOTE: the unique player id within the database
		if(!strcmp(colName[i], "id"))
			pl->id = (uint32_t) atoi(argv[i]);

		// NOTE: the player's username
		if(!strcmp(colName[i], "username")) {
			int unLen = strlen(argv[i]);

			pl->username = (char *)malloc(sizeof(char)*(unLen+1));
			memcpy(pl->username, argv[i], sizeof(char)*unLen);
			pl->username[unLen] = '\0';
		}

		// NOTE: the player's password
		if(!strcmp(colName[i], "password")) {
			int pwLen = strlen(argv[i]);

			pl->password = (char *)malloc(sizeof(char)*(pwLen+1));
			memcpy(pl->password, argv[i], sizeof(char)*pwLen);
			pl->password[pwLen] = '\0';
		}

		// NOTE: the number of diamonds that the player possesses
		if(!strcmp(colName[i], "count"))
			pl->count = (uint32_t) atoi(argv[i]);

		// NOTE: the current state of the player
		if(!strcmp(colName[i], "state"))
			pl->state = (uint32_t) atoi(argv[i]);

		// NOTE: the IP host name for the player
		if(!strcmp(colName[i], "host"))
			pl->host = (uint32_t) atoi(argv[i]);

		// NOTE: for port for the player connection
		if(!strcmp(colName[i], "port"))
			pl->port = (uint32_t) atoi(argv[i]);

		// NOTE: the current node whic the player is on
		if(!strcmp(colName[i], "node"))
			pl->node = (uint32_t) atoi(argv[i]);

		// NOTE: the x coordinate for the player
		if(!strcmp(colName[i], "x"))
			pl->x = (uint32_t) atoi(argv[i]);

		// NOTE: the y coordinate for the player
		if(!strcmp(colName[i], "y"))
			pl->y = (uint32_t) atoi(argv[i]);
	}

	return 0;
}

//-----------------------------------------------------------------------------
/* pl_allocIP
*/

DB_Player* pl_allocIP(PlayerManager *pm, IPaddress ip) {
	/*
	- load a player structure from an incoming IP address
	*/

	// NOTE: select from client IP
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE host = %d AND port = %d;",
		ip.host,
		ip.port
	);

	// NOTE: free space for the player structure
	DB_Player *pl = (DB_Player *)calloc(1, sizeof(DB_Player));

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(pm->db, sqlCmd, pl_loadCB, pl, &errorMsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	// NOTE: if the player's id is set to 0 then the player wasn't found on
	// that IP
	if(pl->id == 0) {
		free(pl->username);
		free(pl->password);

		pl->username = NULL;
		pl->password = NULL;

		free(pl);

		pl = NULL;
	}

	// NOTE: return the player, which must be free'd later
	return pl;
}

//-----------------------------------------------------------------------------
/* pl_allocUN
*/

DB_Player* pl_allocUN(PlayerManager *pm, char *username) {
	/*
	- load a player structure from a string username
	*/

	// NOTE: select from player's username
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE username = '%s';",
		username
	);

	// NOTE: free space for the player structure
	DB_Player *pl = (DB_Player *)calloc(1, sizeof(DB_Player));

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(pm->db, sqlCmd, pl_loadCB, pl, &errorMsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	// NOTE: if the player's id is set to 0 then the player wasn't found with
	// that username
	if(pl->id == 0) {
		free(pl->username);
		free(pl->password);

		pl->username = NULL;
		pl->password = NULL;

		free(pl);
		
		pl = NULL;
	}

	// NOTE: return the player, which must be free'd later
	return pl;
}

//-----------------------------------------------------------------------------
/* pl_free
*/

void pl_free(DB_Player *pl) {
	/*
	- free a player's information which has been allocated by pl_allocUN or
		pl_allocIP
	*/

	// NOTE: free the player's username and password
	free(pl->username);
	free(pl->password);

	// NOTE: set to NULL
	pl->username = NULL;
	pl->password = NULL;

	// NOTE: free the player
	free(pl);

	pl = NULL;
}

//-----------------------------------------------------------------------------
/* pl_save
*/

void pl_save(PlayerManager *pm, DB_Player *pl) {
	/*
	- save the player's information to the SQLite database
	*/

	// NOTE: save the player's state
	char sqlCmd[0xFFFF] = {};

	// NOTE: set the sql query
	sprintf(sqlCmd,
		"UPDATE Players SET username = '%s',"\
		"password = '%s', count = %d, state = %d,"\
		"host = %d, port = %d, node = %d, x = %d, y = %d "\
		"WHERE id = %d;",
		pl->username, pl->password, pl->count,
		pl->state, pl->host, pl->port, pl->node,
		pl->x, pl->y, pl->id
	);

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(pm->db, sqlCmd, NULL, NULL, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}
}

//-----------------------------------------------------------------------------
/* pl_delete
*/

void pl_delete(PlayerManager *pm, DB_Player *pl) {
	/*
	- delete the player's information from the SQLite database
	*/
}

//-----------------------------------------------------------------------------
/* pl_getIndex
*/

int pl_getIndex(PlayerManager *pm, DB_Player *pl) {
	/*
	- get a player's index into pl_dbInfo and the various other player arrays
	- returns -1 when the player isn't found within the pl_dbInfo array
	*/

	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: if there isn't a player at the index continue onward
		if(!pm->pl_indMask[pl->node][i])
			continue;

		// NOTE: if the player IDs match then return the index at which they
		// match
		if(pl->id == pm->pl_dbInfo[pl->node][i].id)
			return i;
	}

	// NOTE: if the player isn't within the array return -1
	return -1;
}

//-----------------------------------------------------------------------------
/* pl_getFreeIndex
*/

int pl_getFreeIndex(PlayerManager *pm, uint32_t node) {
	/*
	- returns the first free index found on a particular node
	- if the node is full pl_getFreeIndex returns -1
	- if the node is out of bounds pl_getFreeIndex returns -2
	*/

	// NOTE: the player should never be on a node that is out of bounds
	if(node>=NODE_MAX || node<0) return -2;

	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: return an index which is free in the pool. the mask should
		// be set to flase elsewhere
		if(!pm->pl_indMask[node][i]) return i;
	}
	
	// NOTE: if the node is full return -1
	return -1;
}

//-----------------------------------------------------------------------------
/* pl_numOnNode
*/

int pl_numOnNode(PlayerManager *pm, uint32_t node) {
	/*
	- returns the number of players currently on a particular node
	- if the node is out of bounds pl_numOnNode returns -2
	*/

	// NOTE: the player should never be on a node that is out of bounds
	if(node>=NODE_MAX || node<0) return -2;

	int i, n = 0;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: return the number of players on a node
		if(pm->pl_indMask[node][i]) n++;
	}
	
	// NOTE: return the number of players on the node
	return n;
}

//-----------------------------------------------------------------------------
/* pl_getDBInfo
*/

DB_Player* pl_getDBInfo(PlayerManager *pm, IPaddress ip) {
	/*
	- takes a player's IPaddress and returns a pointer to their database
		information
	- pl_getDBInfo returns NULL when the player isn't found or offline
	*/

	// NOTE: load the player information from the database via their IP address
	DB_Player *pl = pl_allocIP(pm, ip);

	// NOTE: most likely due to player being offline or someone else
	// trying send a packet acting as them
	if(pl == NULL) return NULL;

	// NOTE: get the node data
	uint32_t node = pl->node;

	// NOTE: free the heap allocated player
	pl_free(pl);

	int i, ind;
	for(i=0; i<PLAYER_MAX; i++) {
		// NOTE: if there isn't a player at the index continue onward
		if(!pm->pl_indMask[node][i])
			continue;

		// NOTE: make sure the ip host and port match
		int check00 = pm->pl_dbInfo[node][i].host==ip.host;
		int check01 = pm->pl_dbInfo[node][i].port==ip.port;

		if(check00 && check01)
			return &pm->pl_dbInfo[node][i];
	}

	// NOTE: couldn't find player - possibly due to a
	// auto-disconnect due to failed pong reply
	return NULL;
}

//-----------------------------------------------------------------------------
/* pl_addPlayer
*/

int pl_addPlayer(PlayerManager *pm, DB_Player *pl) {
	/*
	- adds the player to the array of managed players
	- returns -1 if the node is full
	- otherwise it returns the index of the newly added player
	*/

	// NOTE: get a free location on the node to add the player
	int ind = pl_getFreeIndex(pm, pl->node);
	if(ind == -1) return -1;

	// NOTE: copy this memory into the player pool, this overwrites anything
	// that may have been there before
	//memcpy(&pm->pl_dbInfo[pl->node][ind], pl, sizeof(DB_Player));
	pm->pl_dbInfo[pl->node][ind] = *pl;

	// NOTE: the username and password must be copied as well and pl's pointers
	// must be free'd seperately
	int unLen = strlen(pl->username);
	int pwLen = strlen(pl->password);

	pm->pl_dbInfo[pl->node][ind].username = (char *)malloc(sizeof(char)*(unLen+1));
	pm->pl_dbInfo[pl->node][ind].password = (char *)malloc(sizeof(char)*(pwLen+1));

	memcpy(pm->pl_dbInfo[pl->node][ind].username, pl->username, sizeof(char)*unLen);
	memcpy(pm->pl_dbInfo[pl->node][ind].password, pl->password, sizeof(char)*pwLen);

	pm->pl_dbInfo[pl->node][ind].username[unLen] = '\0';
	pm->pl_dbInfo[pl->node][ind].password[pwLen] = '\0';

	// NOTE: set the index mask to true so that we know a player is located
	// there
	pm->pl_indMask[pl->node][ind] = SDL_TRUE;

	// NOTE: return the index at which the new player was added
	return ind;
}

//-----------------------------------------------------------------------------
/* pl_removePlayer
*/

int pl_removePlayer(PlayerManager *pm, DB_Player *pl) {
	/*
	- removes the player from the array of managed players
	- returns -1 if the player isn't within the array of managed players
	- otherwise it returns the index of the recently removed player
	*/

	// NOTE: get the location of the player on the node
	int ind = pl_getIndex(pm, pl);
	if(ind == -1) return -1;

	// NOTE: free the player's username and password pointers. if these
	// pointers. to free pl's pointers use pl_free()
	free(pm->pl_dbInfo[pl->node][ind].username);
	free(pm->pl_dbInfo[pl->node][ind].password);

	pm->pl_dbInfo[pl->node][ind].username = NULL;
	pm->pl_dbInfo[pl->node][ind].password = NULL;

	// NOTE: the used memory is never cleared for now to increased performance
	//memset(&pm->pl_dbInfo[pl->node][ind], 0x00, sizeof(DB_Player));

	// NOTE: set the index mask to flase so that we know a player is no longer
	// located there
	pm->pl_indMask[pl->node][ind] = SDL_FALSE;

	// NOTE: return the index at which the old player existed
	return ind;
}

//-----------------------------------------------------------------------------
/* freePlayerManager
*/

SDL_bool pl_checkMoveTimer(PlayerManager *pm, DB_Player *pl) {
	/*
	- check to see if enough time has pasted since the client's last move
		request
	- returns false if it is too early to access a player movement packet
	- else return true
	*/

	// NOTE: get the player index
	int ind = pl_getIndex(pm, pl);

	// NOTE: if the player isn't within the player array return false
	if(ind == -1) return SDL_FALSE;

	// NOTE: get the time at which the server got the player's last move
	// packet
	uint32_t lTime = pm->pl_lMoveTime[pl->node][ind];

	if((SDL_GetTicks()-lTime) > pm->pl_moveDelay)
		return SDL_TRUE;
	else
		return SDL_FALSE;
}

//-----------------------------------------------------------------------------
/* freePlayerManager
*/

void freePlayerManager(PlayerManager *pm) {
	/*
	- free the player manager resources
	*/

	int i, j;
	for(j=0; j<NODE_MAX; j++) {
		for(i=0; i<PLAYER_MAX; i++) {
			if(!pm->pl_indMask[j][i])
				continue;

			// NOTE: save all the current players
			pl_save(pm, &pm->pl_dbInfo[j][i]);

			// NOTE: free their username and password strings
			free(pm->pl_dbInfo[j][i].username);
			free(pm->pl_dbInfo[j][i].password);

			// NOTE: set to NULL
			pm->pl_dbInfo[j][i].username = NULL;
			pm->pl_dbInfo[j][i].password = NULL;
		}
	}

	// NOTE: close database connection
	sqlite3_close(pm->db);

	// NOTE: clear the player manager's memory
	free(pm);

	pm = NULL;
}

#endif
