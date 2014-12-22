/*
gcc login.c -o login.exe -I./ -L./ -lsqlite3
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "sqlite3.h"

//-----------------------------------------------------------------------------
struct loginData {
	uint8_t flag;
	char *password;
	uint32_t playerID;
};

sqlite3 *dataBase;

//-----------------------------------------------------------------------------
int playerLoginCB(void *data, int argc, char *argv[], char *colName[]) {
	// NOTE: check if password matches
	uint8_t *flag = &((struct loginData *)data)->flag;
	char *password = ((struct loginData *)data)->password;
	uint32_t *playerID = &((struct loginData *)data)->playerID;

	int i;
	int isOnline = 0;
	for(i=0; i<argc; i++){
		if(!strcmp(colName[i], "PlayerID"))
			*playerID = (uint32_t) atoi(argv[i]);
		if(!strcmp(colName[i], "Password"))
			*flag = !strcmp(argv[i], password);
		if(!strcmp(colName[i], "State"))
			isOnline = strcmp(argv[i], "0")!=0;

		// NOTE: if the account is in use set flag to 0x02
		if(isOnline) *flag = 0x02;
	}

	return 0;
}

//-----------------------------------------------------------------------------
uint8_t playerLogin(char *username, char *password, uint32_t *playerID) {
	// NOTE: check that the player's password matches
	char sqlCmd[0xFF] = {};
	sprintf(sqlCmd, "SELECT * FROM Players WHERE Username = '%s';", username);
	struct loginData data = {0, password, 0};

	char *errorMsg;
	if(sqlite3_exec(dataBase, sqlCmd, playerLoginCB, &data, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	// NOTE: if the password matches then set the player state to true idle (0x01)
	if(data.flag==0x01) {
		memset(sqlCmd, 0, 0xFF);
		sprintf(sqlCmd, "UPDATE Players SET State = 1 WHERE Username = '%s';", username);

		if(sqlite3_exec(dataBase, sqlCmd, NULL, 0, &errorMsg)!=SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", errorMsg);
			sqlite3_free(errorMsg);
		}
	}

	*playerID = data.playerID;

	return data.flag;
}

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {

	// NOTE: open database connection
	if(sqlite3_open("diamond_collector.db", &dataBase)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(dataBase));
		sqlite3_close(dataBase);
		return -1;
	}

	// NOTE: get player's username and password
	char *username = "Rickert";
	char *password = "test";

	// NOTE: incoming login packet from client (the client shouldn't know their PlayerID)
	/*
	- (1) flag (0x01)
	- (4) strlen(username)
	- (?) username
	- (4) strlen(password)
	- (?) password
	*/

	// NOTE: info on client side
	uint8_t flagIn = 0x01;
	int unSize = strlen(username);
	int pwSize = strlen(password);

	int pInLen = 1+4+unSize+4+pwSize;
	unsigned char *pIn = (unsigned char *)malloc(pInLen);

	memcpy(pIn, &flagIn, 1);
	memcpy((pIn+1), &unSize, 4);
	memcpy((pIn+5), username, unSize);
	memcpy((pIn+5+unSize), &pwSize, 4);
	memcpy((pIn+5+unSize+4), password, pwSize);

	// NOTE: send the packet over the nets
	
	// NOTE: info on server side
	uint8_t flagOut;
	memcpy(&flagOut, pIn, 1);

	int unSizeOut;
	memcpy(&unSizeOut, (pIn+1), 4);

	char *usernameOut = (char *)malloc(unSizeOut+1);
	memcpy(usernameOut, (pIn+5), unSizeOut);
	usernameOut[unSizeOut] = '\0';
	
	int pwSizeOut;
	memcpy(&pwSizeOut, (pIn+5+unSizeOut), 4);

	char *passwordOut = (char *)malloc(pwSizeOut+1);
	memcpy(passwordOut, (pIn+5+unSizeOut+4), pwSizeOut);
	passwordOut[pwSizeOut] = '\0';

	printf("flag-> 0x%02x\n", flagOut);
	printf("username-> %s (%d)\n", usernameOut, unSizeOut);
	printf("password-> %s (%d)\n\n", passwordOut, pwSizeOut);

	free(passwordOut);
	free(usernameOut);
	free(pIn);

	// NOTE: check that the player's password matches
	uint32_t playerID;
	uint8_t flag = playerLogin(username, password, &playerID);

	if(flag==0x01) {
		printf("Login success!\n");

		// NOTE: send out a new connection packet
		int packetLen = 0;
		unsigned char packet[5];
		memcpy(packet, &flag, 1);
		packetLen += 1;
		memcpy((packet+packetLen), &playerID, 4);
		packetLen += 4;

		// NOTE: will need to think of what to put here
		printf("sending out packet:\n");

		int i;
		for(i=0; i<packetLen; i++) {
			printf("(byte %d)-> 0x%02x\n", i, *(packet+i));
		}
	} else if(flag==0x02) {
		printf("Account is currently in use.\n");
	} else {
		printf("Login failed: unknown username or invalid password.\n");
		sqlite3_close(dataBase);
		return 0;
	}

	// NOTE: close database connection
	sqlite3_close(dataBase);

	return 0;
}
