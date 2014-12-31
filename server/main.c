/* Requires SDL2 and SDLNet to be installed
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
gcc main.c -o server -L./ -lSDL2main -lSDL2 -lSDL2_net -lsqlite3
*/

//-----------------------------------------------------------------------------
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "sqlite3.h"

//-----------------------------------------------------------------------------
#include "player_manager.h"

//-----------------------------------------------------------------------------
UDPsocket serverFD;
PlayerManager *plManager;
SDLNet_SocketSet socketSet;

//-----------------------------------------------------------------------------
int nodeToPing;
int playerToPing;
time_t timeOfPing;
uint8_t serverState;
SDL_bool waitingForPong;
time_t lastTimePingsWentOut;

uint8_t nodes[2][15][30] = {
	{
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01}
	},
	{
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01}
	}
};

//-----------------------------------------------------------------------------
void libInit(void);
void libQuit(void);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	libInit();

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
	printf("\nServer open on port: %d\n\n", 3490);

	// NOTE: initialize the player manager
	plManager = initPlayerManager("diamond_collector.db");

	/*
		LISTEN FOR PACKETS
	*/

	for(;;) {
		// NOTE: wait for a connection
		int n = SDLNet_CheckSockets(socketSet, 0);

		if(n==-1) {
			fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			break;
		} if(!n) {
			// NOTE: if the server doesn't have anything to do then run through
			// a few regular routines
			switch(serverState) {
				case 0x00: {
					if((time(NULL)-lastTimePingsWentOut)>20) {
						// NOTE: if the server is idle in its freetime then start sending out
						// ping packets for the client to respond to
						serverState = 0x01;
						lastTimePingsWentOut = time(NULL);
					}
				} break;
				case 0x01: {
					if(waitingForPong) {
						if(!plManager->pl_indMask[nodeToPing][playerToPing]) {
							playerToPing++;
							waitingForPong = SDL_FALSE;
						} else if((time(NULL)-timeOfPing)>120) {
							// NOTE: if we hear nothing back after 5 secs then disconnect the player
							DB_Player *pl = &plManager->pl_dbInfo[nodeToPing][playerToPing];
							printf("player %s didn't respond to ping logging them out.\n", pl->username);

							// NOTE: set the player state and log the player out
							pl->state = 0x00;

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

							uint8_t offset = 0;

							memset(_packet.data+offset, 0x05, 1);
							offset += 1;
							memcpy(_packet.data+offset, &pl->id, 4);
							offset += 4;

							// NOTE: set the packet length to the offset point
							_packet.len = offset;

							// NOTE: send the packet out to everyone but the player disconnecting
							int i;
							for(i=0; i<PLAYER_MAX; i++) {
								if(!plManager->pl_indMask[pl->node][i])
									continue;

								_packet.address.host = plManager->pl_dbInfo[pl->node][i].host;
								_packet.address.port = plManager->pl_dbInfo[pl->node][i].port;

								if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
									fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
							}

							// NOTE: free the packet
							free(_packet.data);

							// NOTE: save the new player state
							pl_save(plManager, pl);

							if(pl_removePlayer(plManager, pl) != -1) {
								printf("Logout success!\n");
							} else {
								// NOTE: player was never in the players array should
								// probably log this sort of thing
							}

							playerToPing++;
							waitingForPong = SDL_FALSE;
						} else {
							// TODO: keep sending the ping packet
						}
					} else {
						// NOTE: make sure there are people on this node - else go to the
						// next node
						int n = pl_numOnNode(plManager, nodeToPing);

						if(n <= 0) {
							nodeToPing++;
							playerToPing = 0;

							if(nodeToPing==NODE_MAX) {
								nodeToPing = 0;
								serverState = 0x00;
							}

							break;
						}

						// NOTE: if there isn't a player at this point in the pool then
						// go to the next point in the pool
						if(!plManager->pl_indMask[nodeToPing][playerToPing]) {
							playerToPing++;

							if(playerToPing == PLAYER_MAX) {

								nodeToPing++;
								playerToPing = 0;

								if(nodeToPing == NODE_MAX) {
									nodeToPing = 0;
									serverState = 0x00;
								}
							}

							break;
						}

						// NOTE: get the player and send out the ping
						DB_Player *pl = &plManager->pl_dbInfo[nodeToPing][playerToPing];

						// NOTE: send a ping packet
						uint8_t flag = 0x0A;
						UDPpacket _packet = {};

						_packet.data = &flag;

						_packet.len = 1;
						_packet.maxlen = 1;

						_packet.address.host = pl->host;
						_packet.address.port = pl->port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						timeOfPing = time(NULL);
						waitingForPong = SDL_TRUE;
					}
				} break;
			}
			continue;
		}

		// NOTE: does the server have packets waiting?
		if(SDLNet_SocketReady(serverFD)) {
			// NOTE: setup a packet which is big enough to store any client message
			UDPpacket packet;

			// NOTE: allocate space for packet
			packet.maxlen = 0xAA; // 170 bytes
			packet.data = (uint8_t *)malloc(0xAA);

			// NOTE: get the packet
			int recv = SDLNet_UDP_Recv(serverFD, &packet);
			if(!recv) {
				free(packet.data);
				continue;
			}

			// NOTE: read the flag for packet identity
			uint8_t flag = 0;
			uint32_t offset = 0;

			memcpy(&flag, packet.data, 1);
			offset += 1;

			// NOTE: process the packet
			switch(flag) {
				case 0x01: {
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

					memcpy(&unLen, packet.data+offset, 4);
					offset += 4;

					username = (char *)malloc(unLen+1);
					memcpy(username, packet.data+offset, unLen);
					username[unLen] = '\0';
					offset += unLen;

					memcpy(&pwLen, packet.data+offset, 4);
					offset += 4;

					password = (char *)malloc(pwLen+1);
					memcpy(password, packet.data+offset, pwLen);
					password[pwLen] = '\0';
					offset += pwLen;

					DB_Player *tempPl = pl_allocUN(plManager, username);

					if(tempPl == NULL) {
						// NOTE: let client know that user doesn't exist
						printf("non-existent account\n");

						uint8_t _flag = 0x01;
						UDPpacket _packet = {};

						_packet.data = &_flag;

						_packet.len = 1;
						_packet.maxlen = 1;
						_packet.address = packet.address;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(username);
						free(password);

						break;
					}

					if(tempPl->state != 0x00) {
						// NOTE: the account is currently in use
						printf("account in use\n");

						uint8_t _flag = 0x02;
						UDPpacket _packet = {};

						_packet.data = &_flag;

						_packet.len = 1;
						_packet.maxlen = 1;
						_packet.address = packet.address;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(username);
						free(password);

						pl_free(tempPl);

						break;
					}
					
					if(!strcmp(password, tempPl->password)) {

						tempPl->host = packet.address.host;
						tempPl->port = packet.address.port;

						int ind = pl_addPlayer(plManager, tempPl);

						if(ind != -1) {
							DB_Player *pl = &plManager->pl_dbInfo[tempPl->node][ind];

							pl->state = 0x01;
							pl_save(plManager, pl);

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

							uint8_t offset = 0;

							memset(_packet.data+offset, 0x03, 1);
							offset += 1;
							memcpy(_packet.data+offset, &pl->state, 4);
							offset += 4;
							memcpy(_packet.data+offset, &pl->id, 4);
							offset += 4;
							memcpy(_packet.data+offset, &pl->node, 4);
							offset += 4;
							memcpy(_packet.data+offset, &pl->x, 4);
							offset += 4;
							memcpy(_packet.data+offset, &pl->y, 4);
							offset += 4;
							memcpy(_packet.data+offset, &pl->count, 4);
							offset += 4;

							// NOTE: set the packet length to the offset point
							_packet.len = offset;

							// NOTE: send the packet to the connecting player so
							// they know which character is theirs
							_packet.address = packet.address;

							if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
								fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

							// NOTE: set the flag to 0x04 so everyone else treats it as a
							// new connection
							memset(_packet.data, 0x04, 1);

							// NOTE: send the packet out to everyone
							int i;
							for(i=0; i<PLAYER_MAX; i++) {
								if(!plManager->pl_indMask[pl->node][i])
									continue;

								_packet.address.host = plManager->pl_dbInfo[pl->node][i].host;
								_packet.address.port = plManager->pl_dbInfo[pl->node][i].port;

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
						_packet.address = packet.address;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
					}

					free(username);
					free(password);

					pl_free(tempPl);
				} break;
				case 0x02: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					uint32_t unLen = 0;
					uint32_t pwLen = 0;

					char *username = NULL;
					char *password = NULL;

					memcpy(&unLen, packet.data+offset, 4);
					offset += 4;

					username = (char *)malloc(unLen+1);
					memcpy(username, packet.data+offset, unLen);
					username[unLen] = '\0';
					offset += unLen;

					memcpy(&pwLen, packet.data+offset, 4);
					offset += 4;

					password = (char *)malloc(pwLen+1);
					memcpy(password, packet.data+offset, pwLen);
					password[pwLen] = '\0';
					offset += pwLen;

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
						pl_free(pl);
						break;
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

					uint8_t offset = 0;

					memset(_packet.data+offset, 0x05, 1);
					offset += 1;
					memcpy(_packet.data+offset, &pl->id, 4);
					offset += 4;

					// NOTE: set the packet length to the offset point
					_packet.len = offset;

					// NOTE: send the packet out to everyone but the player disconnecting
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!plManager->pl_indMask[pl->node][i])
							continue;

						_packet.address.host = plManager->pl_dbInfo[pl->node][i].host;
						_packet.address.port = plManager->pl_dbInfo[pl->node][i].port;

						check00 = packet.address.host==plManager->pl_dbInfo[pl->node][i].host;
						check01 = packet.address.port==plManager->pl_dbInfo[pl->node][i].port;

						if(check00 && check01)
							continue;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
					}

					// NOTE: free the packet
					free(_packet.data);

					// NOTE: save the disconnecting player state
					pl_save(plManager, pl);

					// NOTE: remove the player from the array of managed players
					if(pl_removePlayer(plManager, pl) != -1) {
						printf("Logout success!\n");
					} else {
						// NOTE: player was never added
					}
				} break;
				case 0x07: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					if(!pl_checkMoveTimer(plManager, pl)) break;

					// NOTE: perform a check to see if the player can move upward
					if(nodes[pl->node][(pl->y-1)][pl->x]==1) break;

					pl->y--;

					int ind = pl_getIndex(plManager, pl);
					plManager->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

					// NOTE: save the new player state
					pl_save(plManager, pl);

					// NOTE: let everyone know that the player has moved upward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!plManager->pl_indMask[pl->node][i])
							continue;

						UDPpacket _packet = {};
						DB_Player *tempPl = &plManager->pl_dbInfo[pl->node][i];

						/*
						- flag (1) 0x06
						- id   (4)
						- y    (4)
						====== (9)
						*/

						_packet.maxlen = 0x09; // 9 bytes
						_packet.data = (uint8_t *)malloc(0x09);

						uint8_t offset = 0;

						memset(_packet.data+offset, 0x06, 1);
						offset += 1;
						memcpy(_packet.data+offset, &pl->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &pl->y, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = tempPl->host;
						_packet.address.port = tempPl->port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x08: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					if(!pl_checkMoveTimer(plManager, pl)) break;

					// NOTE: perform a check to see if the player can move downward
					if(nodes[pl->node][(pl->y+1)][pl->x]==1) break;

					pl->y++;

					int ind = pl_getIndex(plManager, pl);
					plManager->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

					// NOTE: save the new player state
					pl_save(plManager, pl);

					// NOTE: let everyone know that the player has moved downward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!plManager->pl_indMask[pl->node][i])
							continue;

						UDPpacket _packet = {};
						DB_Player *tempPl = &plManager->pl_dbInfo[pl->node][i];

						/*
						- flag (1) 0x07
						- id   (4)
						- y    (4)
						====== (9)
						*/

						_packet.maxlen = 0x09; // 9 bytes
						_packet.data = (uint8_t *)malloc(0x09);

						uint8_t offset = 0;

						memset(_packet.data+offset, 0x07, 1);
						offset += 1;
						memcpy(_packet.data+offset, &pl->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &pl->y, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = tempPl->host;
						_packet.address.port = tempPl->port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x09: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					if(!pl_checkMoveTimer(plManager, pl)) break;

					// NOTE: perform a check to see if the player can move leftward
					if(nodes[pl->node][pl->y][(pl->x-1)]==1) break;

					pl->x--;

					int ind = pl_getIndex(plManager, pl);
					plManager->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

					// NOTE: save the new player state
					pl_save(plManager, pl);

					// NOTE: let everyone know that the player has moved leftward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!plManager->pl_indMask[pl->node][i])
							continue;

						UDPpacket _packet = {};
						DB_Player *tempPl = &plManager->pl_dbInfo[pl->node][i];

						/*
						- flag (1) 0x08
						- id   (4)
						- x    (4)
						====== (9)
						*/

						_packet.maxlen = 0x09; // 9 bytes
						_packet.data = (uint8_t *)malloc(0x09);

						uint8_t offset = 0;

						memset(_packet.data+offset, 0x08, 1);
						offset += 1;
						memcpy(_packet.data+offset, &pl->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &pl->x, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = tempPl->host;
						_packet.address.port = tempPl->port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x0A: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					if(!pl_checkMoveTimer(plManager, pl)) break;

					// NOTE: perform a check to see if the player can move rightward
					if(nodes[pl->node][pl->y][(pl->x+1)]==1) break;

					pl->x++;

					int ind = pl_getIndex(plManager, pl);
					plManager->pl_lMoveTime[pl->node][ind] = SDL_GetTicks();

					// NOTE: save the new player state
					pl_save(plManager, pl);

					// NOTE: let everyone know that the player has moved rightward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!plManager->pl_indMask[pl->node][i])
							continue;

						UDPpacket _packet = {};
						DB_Player *tempPl = &plManager->pl_dbInfo[pl->node][i];

						/*
						- flag (1) 0x09
						- id   (4)
						- x    (4)
						====== (9)
						*/

						_packet.maxlen = 0x09; // 9 bytes
						_packet.data = (uint8_t *)malloc(0x09);

						uint8_t offset = 0;

						memset(_packet.data+offset, 0x09, 1);
						offset += 1;
						memcpy(_packet.data+offset, &pl->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &pl->x, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = tempPl->host;
						_packet.address.port = tempPl->port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x0B: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					// NOTE: notify client of how many players are being sent
					int n = pl_numOnNode(plManager, pl->node);
					UDPpacket _packet0 = {};

					_packet0.data = (uint8_t *)malloc(0x04);

					memcpy(_packet0.data, &n, 4);

					_packet0.len = 4;
					_packet0.maxlen = 4;
					_packet0.address = packet.address;

					if(!SDLNet_UDP_Send(serverFD, -1, &_packet0))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
					free(_packet0.data);

					// NOTE: send out the player information to the newly connected player
					// (could just be new to the node)
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!plManager->pl_indMask[pl->node][i])
							continue;

						UDPpacket _packet1 = {};
						DB_Player *tempPl = &plManager->pl_dbInfo[pl->node][i];

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

						uint8_t offset = 0;

						memcpy(_packet1.data+offset, &tempPl->state, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPl->id, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPl->node, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPl->x, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPl->y, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPl->count, 4);
						offset += 4;

						_packet1.len = offset;
						_packet1.address = packet.address;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet1))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet1.data);
					}
				} break;
				case 0x0C: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					// NOTE: need to make sure the pong is from the right player
					int ind = pl_getIndex(plManager, pl);

					// NOTE: player responding with a pong packet
					if(ind==playerToPing) {
						playerToPing++;
						waitingForPong = SDL_FALSE;
					}
				} break;
				case 0x0D: {
					// NOTE: get the player which is logged in on the incoming address
					DB_Player *pl = pl_getDBInfo(plManager, packet.address);
					if(pl == NULL) break;

					// NOTE: send the spriteIDs for the node position to the client
					UDPpacket _packet = {};

					/*
					- flag     (1) 0x0B
					- node[][] (450)
					========== (301)
					*/

					_packet.maxlen = 30*15+1; // 20*15+1 bytes
					_packet.data = (uint8_t *)malloc(30*15+1);

					uint32_t offset = 0;

					memset(_packet.data+offset, 0x0B, 1);
					offset += 1;

					int i, j;
					for(j=0; j<15; j++) {
						for(i=0; i<30; i++) {
							memcpy(_packet.data+offset, &nodes[pl->node][j][i], 1);
							offset += 1;
						}
					}

					// NOTE: set the packet length to the offset point
					_packet.len = offset;

					// NOTE: send the packet to the connecting player
					_packet.address = packet.address;

					if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

					// NOTE: free the packet
					free(_packet.data);
				} break;
			}

			// NOTE: free the packet when done processing
			free(packet.data);
		}
	}

	// NOTE: free the player manager
	freePlayerManager(plManager);

	// NOTE: free socketset
	SDLNet_FreeSocketSet(socketSet);

	// NOTE: close the socket file descriptor
	SDLNet_UDP_Close(serverFD);

	libQuit();

	return 0;
}

//-----------------------------------------------------------------------------
void libInit(void) {
	// NOTE: initialize SDL2
	if(SDL_Init(SDL_INIT_TIMER)!=0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	// NOTE: initialize SDLNet
	if(SDLNet_Init()==-1) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
void libQuit(void) {
	// NOTE: release SDL2
	SDLNet_Quit();

	// NOTE: release SDLNet
	SDL_Quit();
}
