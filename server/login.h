#ifndef __LOGIN_H_
#define __LOGIN_H_

//-----------------------------------------------------------------------------
struct loginPacket exLoginPacket(UDPpacket *packet, uint8_t *offset) {
	// NOTE: extract the contents of a login packet
	struct loginPacket p = {};

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
int playerLoginCB(void *data, int argc, char *argv[], char *colName[]) {
	// NOTE: check if password matches
	char *password = ((struct dbData *)data)->password;
	uint8_t *retCode = &((struct dbData *)data)->retCode;
	uint32_t *playerID = &((struct dbData *)data)->playerID;

	int i;
	int isOnline = 0;
	for(i=0; i<argc; i++){
		if(!strcmp(colName[i], "PlayerID"))
			*playerID = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "Password"))
			*retCode = !strcmp(argv[i], password);
		if(!strcmp(colName[i], "State"))
			isOnline = strcmp(argv[i], "0")!=0; // NOTE: is '0' always zero?
	}

	// NOTE: if the account is in use set retCode to 0x02
	if(isOnline&&*retCode) *retCode = 0x02;

	return 0;
}

//-----------------------------------------------------------------------------
uint8_t playerLogin(struct loginPacket *p, uint32_t *playerID) {
	// NOTE: check that the player's password matches
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE Username = '%s';", p->username);
	
	struct dbData data = {};
	data.password = p->password;

	char *errorMsg;
	if(sqlite3_exec(database, sqlCmd, playerLoginCB, &data, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	*playerID = data.playerID;
	// NOTE: retCode = 0x00 on password mismatch
	// NOTE: retCode = 0x01 on password match
	// NOTE: retCode = 0x02 if password matches but the account is in use

	switch(data.retCode) {
		case 0x01: {
			// NOTE: if the password matches then set the player state to true idle (0x01)
			memset(sqlCmd, 0, 0xFF);
			sprintf(sqlCmd, "UPDATE Players SET State = 1 WHERE PlayerID = %d;", *playerID);

			if(sqlite3_exec(database, sqlCmd, NULL, 0, &errorMsg)!=SQLITE_OK) {
				fprintf(stderr, "SQL error: %s\n", errorMsg);
				sqlite3_free(errorMsg);
			}
		} break;
	}

	return data.retCode;
}

#endif
