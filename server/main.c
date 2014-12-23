/* Requires SDL2 and SDLNet to be installed
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
gcc main.c -o server -L./ -lSDL2main -lSDL2 -lSDL2_net -lsqlite3
*/

//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "sqlite3.h"

// NOTE: if login successes then bind client IP to channel
// NOTE: 4 possible ips per channel, 32 channels, 128 possible
// players online. I will have to keep track of which channels
// I've put players on so I can send a packet on each channel

//-----------------------------------------------------------------------------
sqlite3 *database;

UDPsocket serverFD;
SDLNet_SocketSet socketSet;

//-----------------------------------------------------------------------------
void libInit(void);
void libQuit(void);

//-----------------------------------------------------------------------------
struct loginData {
	uint8_t flag;
	char *password;
	uint32_t playerID;
};

uint8_t playerLogin(char *username, char *password, uint32_t *playerID);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	libInit();

	// NOTE: open database connection
	if(sqlite3_open("diamond_collector.db", &database)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(database));
		sqlite3_close(database);
		return -1;
	}

	// NOTE: open socket file descriptor
	serverFD = SDLNet_UDP_Open(3490);
	if(!serverFD) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		libQuit();
		return -1;
	}

	// NOTE: setup a socket set
	socketSet = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(socketSet, serverFD);

	for(;;) {
		// NOTE: wait for a connection
		int n = SDLNet_CheckSockets(socketSet, -1);
		if(n==-1) {
			fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			break;
		} if(!n) continue;

		// NOTE: only need to check the server
		if(SDLNet_SocketReady(serverFD)) {
			// NOTE: setup a packet which is big enough to store any client message
			UDPpacket packet;

			// NOTE: allocate space for packet
			packet.maxlen = 0xAA; // 170 bytes
			packet.data = (uint8_t *)malloc(0xAA);

			// NOTE: get packet
			int recv = SDLNet_UDP_Recv(serverFD, &packet);
			if(!recv) continue;

			// NOTE: display IPaddress infomation
			Uint32 ipaddr = SDL_SwapBE32(packet.address.host);
			printf("packet from-> %d.%d.%d.%d:%d\n", ipaddr>>24, (ipaddr>>16)&0xFF,
				(ipaddr>>8)&0xFF, ipaddr&0xFF, SDL_SwapBE16(packet.address.port));

			// NOTE: read the flag for packet identity
			uint8_t flag;
			memcpy(&flag, packet.data, 1);
			uint8_t offset = 1;

			// NOTE: process packet
			switch(flag) {
				case 0x01: {
					uint32_t unSize;
					memcpy(&unSize, packet.data+offset, 4);
					offset += 4;

					char *username = (char *)malloc(unSize+1);
					memcpy(username, packet.data+offset, unSize);
					username[unSize] = '\0';
					offset += unSize;

					uint32_t pwSize;
					memcpy(&pwSize, packet.data+offset, 4);
					offset += 4;

					char *password = (char *)malloc(pwSize+1);
					memcpy(password, packet.data+offset, pwSize);
					password[pwSize] = '\0';
					offset += pwSize;

					if(offset==packet.len) {
						// NOTE: check that the player's password matches
						uint32_t playerID;
						uint8_t flag = playerLogin(username, password, &playerID);

						if(flag==0x01) {
							printf("Login success!\n");
						}
					}

					free(username);
					free(password);
				} break;
			}

			// NOTE: free the packet
			free(packet.data);
		}
	}

	// NOTE: close the socket file descriptor
	SDLNet_UDP_Close(serverFD);

	// NOTE: close database connection
	sqlite3_close(database);

	libQuit();

	return 0;
}

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
	if(sqlite3_exec(database, sqlCmd, playerLoginCB, &data, &errorMsg)!=SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
	}

	/*
	// NOTE: if the password matches then set the player state to true idle (0x01)
	if(data.flag==0x01) {
		memset(sqlCmd, 0, 0xFF);
		sprintf(sqlCmd, "UPDATE Players SET State = 1 WHERE Username = '%s';", username);

		if(sqlite3_exec(database, sqlCmd, NULL, 0, &errorMsg)!=SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", errorMsg);
			sqlite3_free(errorMsg);
		}
	}
	*/

	*playerID = data.playerID;

	return data.flag;
}

//-----------------------------------------------------------------------------
void libInit(void) {
	// NOTE: initialize the libraries
	if(SDL_Init(SDL_INIT_TIMER)!=0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	if(SDLNet_Init()==-1) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
void libQuit(void) {
	// NOTE: release the libraries
	SDLNet_Quit();
	SDL_Quit();
}
