#ifndef __LOGOUT_H_
#define __LOGOUT_H_

//-----------------------------------------------------------------------------
struct logoutPacket exLogoutPacket(UDPpacket *packet, uint8_t *offset) {
	// NOTE: extract the contents of a logout packet
	struct logoutPacket p = {};

	memcpy(&p.unLen, packet->data+*offset, 4);
	*offset += 4;

	p.username = (char *)malloc(p.unLen+1);
	memcpy(p.username, packet->data+*offset, p.unLen);
	p.username[p.unLen] = '\0';
	*offset += p.unLen;

	memcpy(&p.pwLen, packet->data+*offset, 4);
	*offset += 4;

	p.password = (char *)malloc(p.pwLen+1);
	memcpy(p.password, packet->data+*offset, p.pwLen);
	p.password[p.pwLen] = '\0';
	*offset += p.pwLen;

	return p;
}

//-----------------------------------------------------------------------------
int playerLogoutCB(void *data, int argc, char *argv[], char *colName[]) {
	// NOTE: make sure player is online
	char *password = ((struct dbData *)data)->password;
	uint8_t *retCode = &((struct dbData *)data)->retCode;
	uint32_t *playerID = &((struct dbData *)data)->playerID;
	struct Player *player = &((struct dbData *)data)->player;

	int i;
	int isOnline = 0;
	for(i=0; i<argc; i++){
		if(!strcmp(colName[i], "PlayerID"))
			*playerID = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "Password"))
			*retCode = !strcmp(argv[i], password);
		if(!strcmp(colName[i], "State"))
			isOnline = strcmp(argv[i], "0")!=0;
		if(!strcmp(colName[i], "Node"))
			player->node = (uint32_t) atoi(argv[i]);

		// NOTE: if the account is in use set retCode to 0x02
		if(isOnline) *retCode = 0x02;
	}

	return 0;
}

//-----------------------------------------------------------------------------
uint8_t playerLogout(struct logoutPacket *p, uint32_t *playerID) {
	// NOTE: check that the player's password matches
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE Username = '%s';", p->username);

	struct dbData data = {};
	data.password = p->password;

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, playerLogoutCB, &data, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	// NOTE: if the password matches then set the player state to offline (0x00)
	if(data.retCode==0x02) {
		memset(sqlCmd, 0, 0xFF);
		sprintf(sqlCmd, "UPDATE Players SET State = 0 WHERE Username = '%s';", p->username);

		if(sqlite3_exec(database, sqlCmd, NULL, 0, &errorMsg)!=SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", errorMsg);
			sqlite3_free(errorMsg);
		}

		// NOTE: rebind the IPaddresses on the node for this client
		SDLNet_UDP_Unbind(serverFD, data.player.node);

		int i, ind;
		for(i=0; i<numChrsOnNode[data.player.node]; i++) {
			struct Player chr = chrsOnline[data.player.node][i];

			if(data.playerID==chr.id) {
				ind = i;
				continue;
			}

			if(SDLNet_UDP_Bind(serverFD, data.player.node, &chr.ip)==-1) {
				fprintf(stderr, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
			}
		}

		free(chrsOnline[data.player.node][ind].username);
		free(chrsOnline[data.player.node][ind].password);

		// NOTE: remove the client from the array of clients
		numChrsOnNode[data.player.node]--;
		if(ind<3) memcpy(
			&chrsOnline[data.player.node][ind],
			&chrsOnline[data.player.node][ind+1],
			(3-ind)*sizeof(struct Player)
		);

		// NOTE: let the players know the client left
		UDPpacket packet = {};

		packet.maxlen = 0x05;
		packet.data = (uint8_t *)malloc(0x05);

		uint8_t offset = 0;

		memset(packet.data+offset, 0x05, 1);
		offset += 1;
		memcpy(packet.data+offset, &data.playerID, 4);
		offset += 4;

		packet.len = offset;
		if(!SDLNet_UDP_Send(serverFD, data.player.node, &packet)) {
			// NOTE: could just be that there is no one on the channel
			if(strcmp(SDLNet_GetError(), ""))
				fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
		}
	}

	*playerID = data.playerID;

	return data.retCode;
}

#endif
