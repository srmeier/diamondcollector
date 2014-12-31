#ifndef __NETWORKCALLS_H_
#define __NETWORKCALLS_H_

//-----------------------------------------------------------------------------
extern UDPsocket serverFD;
extern uint8_t nodes[2][15][30];

//-----------------------------------------------------------------------------
/* net_login
*/

void net_login(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: extract the information from the incoming packet
	uint32_t unLen = 0;
	uint32_t pwLen = 0;

	char *username = NULL;
	char *password = NULL;

	/*
	- flag     (1) 0x01
	- unLen    (4)
	- username (unLen)
	- pwLen    (4)
	- password (pwLen)
	========== (?)
	*/

	memcpy(&unLen, packet->data+*offset, 4);
	*offset += 4;

	username = (char *)malloc(unLen+1);
	memcpy(username, packet->data+*offset, unLen);
	username[unLen] = '\0';
	*offset += unLen;

	memcpy(&pwLen, packet->data+*offset, 4);
	*offset += 4;

	password = (char *)malloc(pwLen+1);
	memcpy(password, packet->data+*offset, pwLen);
	password[pwLen] = '\0';
	*offset += pwLen;

	DB_Player *tempPl = pl_allocUN(pm, username);

	if(tempPl == NULL) {
		// NOTE: let client know that user doesn't exist
		printf("non-existent account\n");

		uint8_t _flag = 0x01;
		UDPpacket _packet = {};

		_packet.data = &_flag;

		_packet.len = 1;
		_packet.maxlen = 1;
		_packet.address = packet->address;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(username);
		free(password);

		return;
	}

	if(tempPl->state != 0x00) {
		// NOTE: the account is currently in use
		printf("account in use\n");

		uint8_t _flag = 0x02;
		UDPpacket _packet = {};

		_packet.data = &_flag;

		_packet.len = 1;
		_packet.maxlen = 1;
		_packet.address = packet->address;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(username);
		free(password);

		pl_free(tempPl);

		return;
	}
	
	if(!strcmp(password, tempPl->password)) {

		tempPl->host = packet->address.host;
		tempPl->port = packet->address.port;

		int ind = pl_addPlayer(pm, tempPl);

		if(ind != -1) {
			DB_Player *pl = &pm->pl_dbInfo[tempPl->node][ind];

			pl->state = 0x01;
			pl_save(pm, pl);

			printf("successful login:\n");
			printf("X        -> %d\n", pl->x);
			printf("Y        -> %d\n", pl->y);
			printf("ID       -> %d\n", pl->id);
			printf("Node     -> %d\n", pl->node);
			printf("Username -> %s\n", pl->username);
			printf("Password -> %s\n", pl->password);
			printf("State    -> %d\n", pl->state);
			printf("Count    -> %d\n\n", pl->count);

			// NOTE: send a packet out to everyone on this node
			// letting them know that the player is joining
			UDPpacket _packet = {};

			/*
			- flag         (1) 0x03/0x04
			- state        (4)
			- id           (4)
			- node         (4)
			- x            (4)
			- y            (4)
			- count        (4)
			============== (25)
			*/
			
			_packet.maxlen = 0x19; // 25 bytes
			_packet.data = (uint8_t *)malloc(0x19);

			uint8_t _offset = 0;

			memset(_packet.data+_offset, 0x03, 1);
			_offset += 1;
			memcpy(_packet.data+_offset, &pl->state, 4);
			_offset += 4;
			memcpy(_packet.data+_offset, &pl->id, 4);
			_offset += 4;
			memcpy(_packet.data+_offset, &pl->node, 4);
			_offset += 4;
			memcpy(_packet.data+_offset, &pl->x, 4);
			_offset += 4;
			memcpy(_packet.data+_offset, &pl->y, 4);
			_offset += 4;
			memcpy(_packet.data+_offset, &pl->count, 4);
			_offset += 4;

			// NOTE: set the packet length to the offset point
			_packet.len = _offset;

			// NOTE: send the packet to the connecting player so
			// they know which character is theirs
			_packet.address = packet->address;

			if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
				fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

			// NOTE: set the flag to 0x04 so everyone else treats it as a
			// new connection
			memset(_packet.data, 0x04, 1);

			// NOTE: send the packet out to everyone
			int i;
			for(i=0; i<PLAYER_MAX; i++) {
				if(!pm->pl_indMask[pl->node][i])
					continue;

				_packet.address.host = pm->pl_dbInfo[pl->node][i].host;
				_packet.address.port = pm->pl_dbInfo[pl->node][i].port;

				if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
					fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
			}

			// NOTE: free the packet
			free(_packet.data);
		} else {
			// TODO: let client know that node is full
		}
	} else {
		// NOTE: let client know that the password is incorrect
		printf("incorrect password\n");

		uint8_t _flag = 0x01;
		UDPpacket _packet = {};

		_packet.data = &_flag;

		_packet.len = 1;
		_packet.maxlen = 1;
		_packet.address = packet->address;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
	}

	free(username);
	free(password);

	pl_free(tempPl);
}

//-----------------------------------------------------------------------------
/* net_logout
*/

void net_logout(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	uint32_t unLen = 0;
	uint32_t pwLen = 0;

	char *username = NULL;
	char *password = NULL;

	memcpy(&unLen, packet->data+*offset, 4);
	*offset += 4;

	username = (char *)malloc(unLen+1);
	memcpy(username, packet->data+*offset, unLen);
	username[unLen] = '\0';
	*offset += unLen;

	memcpy(&pwLen, packet->data+*offset, 4);
	*offset += 4;

	password = (char *)malloc(pwLen+1);
	memcpy(password, packet->data+*offset, pwLen);
	password[pwLen] = '\0';
	*offset += pwLen;

	// NOTE: check that the information matches
	int check00 = !strcmp(username, pl->username);
	int check01 = !strcmp(password, pl->password);

	// NOTE: free the packet username and password
	free(username);
	free(password);

	// NOTE: check that the information matches
	if(check00 && check01)
		pl->state = 0x00;
	else {
		// NOTE: username/password mismatch
		return;
	}

	// NOTE: send a packet out to everyone on this node
	// letting them know that the player is leaving.
	UDPpacket _packet = {};

	/*
	- flag (1) 0x05
	- id   (4)
	====== (5)
	*/

	_packet.maxlen = 0x05; // 5 bytes
	_packet.data = (uint8_t *)malloc(0x05);

	uint8_t _offset = 0;

	memset(_packet.data+_offset, 0x05, 1);
	_offset += 1;
	memcpy(_packet.data+_offset, &pl->id, 4);
	_offset += 4;

	// NOTE: set the packet length to the offset point
	_packet.len = _offset;

	// NOTE: send the packet out to everyone but the player disconnecting
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		if(!pm->pl_indMask[pl->node][i])
			continue;

		_packet.address.host = pm->pl_dbInfo[pl->node][i].host;
		_packet.address.port = pm->pl_dbInfo[pl->node][i].port;

		check00 = packet->address.host==pm->pl_dbInfo[pl->node][i].host;
		check01 = packet->address.port==pm->pl_dbInfo[pl->node][i].port;

		if(check00 && check01)
			continue;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
	}

	// NOTE: free the packet
	free(_packet.data);

	// NOTE: save the disconnecting player state
	pl_save(pm, pl);

	// NOTE: remove the player from the array of managed players
	if(pl_removePlayer(pm, pl) != -1) {
		printf("Logout success!\n");
	} else {
		// NOTE: player was never added
	}
}

//-----------------------------------------------------------------------------
/* net_moveUp
*/

void net_moveUp(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	if(!pl_checkMoveTimer(pm, pl)) return;

	// NOTE: perform a check to see if the player can move upward
	if(nodes[pl->node][(pl->y-1)][pl->x]==1) return;

	pl->y--;

	int ind = pl_getIndex(pm, pl);
	pm->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

	// NOTE: save the new player state
	pl_save(pm, pl);

	// NOTE: let everyone know that the player has moved upward
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		if(!pm->pl_indMask[pl->node][i])
			continue;

		UDPpacket _packet = {};
		DB_Player *tempPl = &pm->pl_dbInfo[pl->node][i];

		/*
		- flag (1) 0x06
		- id   (4)
		- y    (4)
		====== (9)
		*/

		_packet.maxlen = 0x09; // 9 bytes
		_packet.data = (uint8_t *)malloc(0x09);

		uint8_t _offset = 0;

		memset(_packet.data+_offset, 0x06, 1);
		_offset += 1;
		memcpy(_packet.data+_offset, &pl->id, 4);
		_offset += 4;
		memcpy(_packet.data+_offset, &pl->y, 4);
		_offset += 4;

		_packet.len = _offset;
		_packet.address.host = tempPl->host;
		_packet.address.port = tempPl->port;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(_packet.data);
	}
}

//-----------------------------------------------------------------------------
/* net_moveDown
*/

void net_moveDown(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	if(!pl_checkMoveTimer(pm, pl)) return;

	// NOTE: perform a check to see if the player can move downward
	if(nodes[pl->node][(pl->y+1)][pl->x]==1) return;

	pl->y++;

	int ind = pl_getIndex(pm, pl);
	pm->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

	// NOTE: save the new player state
	pl_save(pm, pl);

	// NOTE: let everyone know that the player has moved downward
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		if(!pm->pl_indMask[pl->node][i])
			continue;

		UDPpacket _packet = {};
		DB_Player *tempPl = &pm->pl_dbInfo[pl->node][i];

		/*
		- flag (1) 0x07
		- id   (4)
		- y    (4)
		====== (9)
		*/

		_packet.maxlen = 0x09; // 9 bytes
		_packet.data = (uint8_t *)malloc(0x09);

		uint8_t _offset = 0;

		memset(_packet.data+_offset, 0x07, 1);
		_offset += 1;
		memcpy(_packet.data+_offset, &pl->id, 4);
		_offset += 4;
		memcpy(_packet.data+_offset, &pl->y, 4);
		_offset += 4;

		_packet.len = _offset;
		_packet.address.host = tempPl->host;
		_packet.address.port = tempPl->port;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(_packet.data);
	}
}

//-----------------------------------------------------------------------------
/* net_moveLeft
*/

void net_moveLeft(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	if(!pl_checkMoveTimer(pm, pl)) return;

	// NOTE: perform a check to see if the player can move leftward
	if(nodes[pl->node][pl->y][(pl->x-1)]==1) return;

	pl->x--;

	int ind = pl_getIndex(pm, pl);
	pm->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

	// NOTE: save the new player state
	pl_save(pm, pl);

	// NOTE: let everyone know that the player has moved leftward
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		if(!pm->pl_indMask[pl->node][i])
			continue;

		UDPpacket _packet = {};
		DB_Player *tempPl = &pm->pl_dbInfo[pl->node][i];

		/*
		- flag (1) 0x08
		- id   (4)
		- x    (4)
		====== (9)
		*/

		_packet.maxlen = 0x09; // 9 bytes
		_packet.data = (uint8_t *)malloc(0x09);

		uint8_t _offset = 0;

		memset(_packet.data+_offset, 0x08, 1);
		_offset += 1;
		memcpy(_packet.data+_offset, &pl->id, 4);
		_offset += 4;
		memcpy(_packet.data+_offset, &pl->x, 4);
		_offset += 4;

		_packet.len = _offset;
		_packet.address.host = tempPl->host;
		_packet.address.port = tempPl->port;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(_packet.data);
	}
}

//-----------------------------------------------------------------------------
/* net_moveRight
*/

void net_moveRight(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	if(!pl_checkMoveTimer(pm, pl)) return;

	// NOTE: perform a check to see if the player can move rightward
	if(nodes[pl->node][pl->y][(pl->x+1)]==1) return;

	pl->x++;

	int ind = pl_getIndex(pm, pl);
	pm->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

	// NOTE: save the new player state
	pl_save(pm, pl);

	// NOTE: let everyone know that the player has moved rightward
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		if(!pm->pl_indMask[pl->node][i])
			continue;

		UDPpacket _packet = {};
		DB_Player *tempPl = &pm->pl_dbInfo[pl->node][i];

		/*
		- flag (1) 0x09
		- id   (4)
		- x    (4)
		====== (9)
		*/

		_packet.maxlen = 0x09; // 9 bytes
		_packet.data = (uint8_t *)malloc(0x09);

		uint8_t _offset = 0;

		memset(_packet.data+_offset, 0x09, 1);
		_offset += 1;
		memcpy(_packet.data+_offset, &pl->id, 4);
		_offset += 4;
		memcpy(_packet.data+_offset, &pl->x, 4);
		_offset += 4;

		_packet.len = _offset;
		_packet.address.host = tempPl->host;
		_packet.address.port = tempPl->port;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(_packet.data);
	}
}

//-----------------------------------------------------------------------------
/* net_sendOutPlInfo
*/

void net_sendOutPlInfo(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	// NOTE: notify client of how many players are being sent
	int n = pl_numOnNode(pm, pl->node);
	UDPpacket _packet0 = {};

	_packet0.data = (uint8_t *)malloc(0x04);

	memcpy(_packet0.data, &n, 4);

	_packet0.len = 4;
	_packet0.maxlen = 4;
	_packet0.address = packet->address;

	if(!SDLNet_UDP_Send(serverFD, -1, &_packet0))
		fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
	free(_packet0.data);

	// NOTE: send out the player information to the newly connected player
	// (could just be new to the node)
	int i;
	for(i=0; i<PLAYER_MAX; i++) {
		if(!pm->pl_indMask[pl->node][i])
			continue;

		UDPpacket _packet1 = {};
		DB_Player *tempPl = &pm->pl_dbInfo[pl->node][i];

		/*
		- state (4)
		- id    (4)
		- node  (4)
		- x     (4)
		- y     (4)
		- count (4)
		======= (24)
		*/

		_packet1.maxlen = 0x18;
		_packet1.data = (uint8_t *)malloc(0x18); // 24 bytes

		uint8_t _offset = 0;

		memcpy(_packet1.data+_offset, &tempPl->state, 4);
		_offset += 4;
		memcpy(_packet1.data+_offset, &tempPl->id, 4);
		_offset += 4;
		memcpy(_packet1.data+_offset, &tempPl->node, 4);
		_offset += 4;
		memcpy(_packet1.data+_offset, &tempPl->x, 4);
		_offset += 4;
		memcpy(_packet1.data+_offset, &tempPl->y, 4);
		_offset += 4;
		memcpy(_packet1.data+_offset, &tempPl->count, 4);
		_offset += 4;

		_packet1.len = _offset;
		_packet1.address = packet->address;

		if(!SDLNet_UDP_Send(serverFD, -1, &_packet1))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

		free(_packet1.data);
	}
}

//-----------------------------------------------------------------------------
/* net_sendOutNodeInfo
*/

void net_sendOutNodeInfo(PlayerManager *pm, UDPpacket *packet, uint32_t *offset) {
	// NOTE: get the player which is logged in on the incoming address
	DB_Player *pl = pl_getDBInfo(pm, packet->address);
	if(pl == NULL) return;

	// NOTE: send the spriteIDs for the node position to the client
	UDPpacket _packet = {};

	/*
	- flag     (1) 0x0B
	- node[][] (450)
	========== (301)
	*/

	_packet.maxlen = 30*15+1; // 20*15+1 bytes
	_packet.data = (uint8_t *)malloc(30*15+1);

	uint32_t _offset = 0;

	memset(_packet.data+_offset, 0x0B, 1);
	_offset += 1;

	int i, j;
	for(j=0; j<15; j++) {
		for(i=0; i<30; i++) {
			memcpy(_packet.data+_offset, &nodes[pl->node][j][i], 1);
			_offset += 1;
		}
	}

	// NOTE: set the packet length to the offset point
	_packet.len = _offset;

	// NOTE: send the packet to the connecting player
	_packet.address = packet->address;

	if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
		fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

	// NOTE: free the packet
	free(_packet.data);
}

#endif
