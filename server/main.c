/* Requires SDL2 and SDLNet to be installed
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
gcc main.c -o server -L./ -lSDL2main -lSDL2 -lSDL2_net -lsqlite3
*/

//-----------------------------------------------------------------------------
#define NODE_MAX 64
#define PLAYER_MAX 32

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
sqlite3 *database;
UDPsocket serverFD;
SDLNet_SocketSet socketSet;

//-----------------------------------------------------------------------------
#include "types.h"

//-----------------------------------------------------------------------------
int nodeToPing;
int playerToPing;
time_t timeOfPing;
uint8_t serverState;
SDL_bool waitingForPong;
time_t lastTimePingsWentOut;

struct Player players[NODE_MAX][PLAYER_MAX];    // 64 kilobytes
SDL_bool playerIndexMask[NODE_MAX][PLAYER_MAX]; //  8 kilobytes

//-----------------------------------------------------------------------------
#include "login.h"
#include "player.h"

//-----------------------------------------------------------------------------
void libInit(void);
void libQuit(void);

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
	printf("\nServer open on port: %d\n\n", 3490);

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
						if((time(NULL)-timeOfPing)>5) {
							// NOTE: if we hear nothing back after 5 secs then disconnect the player
							struct Player *player = &players[nodeToPing][playerToPing];
							printf("player %s didn't respond to ping logging them out.\n", player->username);

							// NOTE: set the player state and log the player out
							player->state = 0x00;

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
							memcpy(_packet.data+offset, &player->id, 4);
							offset += 4;

							// NOTE: set the packet length to the offset point
							_packet.len = offset;

							// NOTE: send the packet out to everyone but the player disconnecting
							int i;
							for(i=0; i<PLAYER_MAX; i++) {
								if(!playerIndexMask[player->node][i])
									continue;

								_packet.address.host = players[player->node][i].host;
								_packet.address.port = players[player->node][i].port;

								if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
									fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
							}

							// NOTE: free the packet
							free(_packet.data);

							// NOTE: save the new player state
							playerSave(player);

							free(player->username);
							free(player->password);

							// NOTE: get the index of the player on the node
							int ind = getPlayerIndex(player);
							if(ind>=0) {
								// NOTE: free the index in the index mask
								playerIndexMask[player->node][ind] = SDL_FALSE;

								// NOTE: remove the player from the players array
								memset(player, 0x00, sizeof(struct Player));
								printf("Logout success!\n");
							} else {
								// NOTE: player was never in the players array should
								// probably log this sort of thing
							}

							playerToPing++;
							waitingForPong = SDL_FALSE;
						}
					} else {
						// NOTE: make sure there are people on this node - else go to the
						// next node
						int numOnNode = getNumOnNode(nodeToPing);

						if(numOnNode<=0) {
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
						if(!playerIndexMask[nodeToPing][playerToPing]) {
							playerToPing++;

							if(playerToPing==PLAYER_MAX) {

								nodeToPing++;
								playerToPing = 0;

								if(nodeToPing==NODE_MAX) {
									nodeToPing = 0;
									serverState = 0x00;
								}
							}

							break;
						}

						// NOTE: get the player and send out the ping
						struct Player *player = &players[nodeToPing][playerToPing];

						// NOTE: send a ping packet
						uint8_t flag = 0x0A;
						UDPpacket _packet = {};

						_packet.data = &flag;

						_packet.len = 1;
						_packet.maxlen = 1;

						_packet.address.host = player->host;
						_packet.address.port = player->port;

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
			uint8_t offset = 0;

			memcpy(&flag, packet.data, 1);
			offset += 1;

			// NOTE: process the packet
			switch(flag) {
				case 0x01: {
					// NOTE: extract the information from the incoming packet
					uint32_t unLen = 0;
					uint32_t pwLen = 0;

					// NOTE: need to start with a fresh player struct because
					// we can't get a free index until we know which node they are on
					struct Player player = {};

					player.host = packet.address.host;
					player.port = packet.address.port;

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

					player.username = (char *)malloc(unLen+1);
					memcpy(player.username, packet.data+offset, unLen);
					player.username[unLen] = '\0';
					offset += unLen;

					memcpy(&pwLen, packet.data+offset, 4);
					offset += 4;

					player.password = (char *)malloc(pwLen+1);
					memcpy(player.password, packet.data+offset, pwLen);
					player.password[pwLen] = '\0';
					offset += pwLen;

					// NOTE: if the account doesn't exist the player state will
					// remain 0xFE
					player.state = 0xFE;

					// NOTE: check that the player's password matches and that
					// the account isn't currently in use
					switch(playerLogin(&player)) {
						case 0x00: {
							// NOTE: player.state = 0x00 on password match
							printf("\nLogin success!\n");

							// NOTE: set the player's new state to online idle
							player.state = 0x01;

							printf("X        -> %d\n", player.x);
							printf("Y        -> %d\n", player.y);
							printf("ID       -> %d\n", player.id);
							printf("Node     -> %d\n", player.node);
							printf("Username -> %s\n", player.username);
							printf("Password -> %s\n", player.password);
							printf("State    -> %d\n", player.state);
							printf("Count    -> %d\n\n", player.count);

							int ind = getFreePlayerIndex(player.node);

							if(ind==-1) {
								// NOTE: the node is full
								// TODO: handle this
							} else if(ind==-2) {
								// NOTE: the player's node location is corrupt
								// TODO: handle this
							} else {
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
								memcpy(_packet.data+offset, &player.state, 4);
								offset += 4;
								memcpy(_packet.data+offset, &player.id, 4);
								offset += 4;
								memcpy(_packet.data+offset, &player.node, 4);
								offset += 4;
								memcpy(_packet.data+offset, &player.x, 4);
								offset += 4;
								memcpy(_packet.data+offset, &player.y, 4);
								offset += 4;
								memcpy(_packet.data+offset, &player.count, 4);
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
									if(!playerIndexMask[player.node][i])
										continue;

									_packet.address.host = players[player.node][i].host;
									_packet.address.port = players[player.node][i].port;

									if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
										fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
								}

								// NOTE: free the packet
								free(_packet.data);

								// NOTE: add the player to the internal server node memory
								memcpy(&players[player.node][ind], &player, sizeof(struct Player));

								// NOTE: set the index mask to true
								playerIndexMask[player.node][ind] = SDL_TRUE;

								// NOTE: save the new player state
								playerSave(&player);
							}
						} break;
						case 0xFE:
							// NOTE: currently treating non-existent account like a password mismatch
						case 0xFF: {
							// NOTE: player.state = 0xFF on password mismatch
							printf("Incorrect password / non-existent account.\n");

							uint8_t flag = 0x01;
							UDPpacket _packet = {};

							_packet.data = &flag;

							_packet.len = 1;
							_packet.maxlen = 1;
							_packet.address = packet.address;

							if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
								fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

							free(player.username);
							free(player.password);
						} break;
						default: {
							// NOTE: player.state > 0x00 if password matches but the account is in use
							printf("Account in use.\n");

							uint8_t flag = 0x02;
							UDPpacket _packet = {};

							_packet.data = &flag;

							_packet.len = 1;
							_packet.maxlen = 1;
							_packet.address = packet.address;

							if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
								fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

							free(player.username);
							free(player.password);
						} break;
					}
				} break;
				case 0x02: {
					// NOTE: get the player which is logged in on the incoming address
					struct Player *player = getPlayerFromIP(packet.address);
					if(player == NULL) break;

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
					int check00 = !strcmp(username, player->username);
					int check01 = !strcmp(password, player->password);

					// NOTE: free the packet username and password
					free(username);
					free(password);

					// NOTE: check that the information matches
					if(check00 && check01)
						player->state = 0x00;
					else break;

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
					memcpy(_packet.data+offset, &player->id, 4);
					offset += 4;

					// NOTE: set the packet length to the offset point
					_packet.len = offset;

					// NOTE: send the packet out to everyone but the player disconnecting
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!playerIndexMask[player->node][i])
							continue;

						_packet.address.host = players[player->node][i].host;
						_packet.address.port = players[player->node][i].port;

						check00 = packet.address.host==players[player->node][i].host;
						check01 = packet.address.port==players[player->node][i].port;

						if(check00 && check01)
							continue;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
					}

					// NOTE: free the packet
					free(_packet.data);

					// NOTE: save the new player state
					playerSave(player);

					free(player->username);
					free(player->password);

					// NOTE: get the index of the player on the node
					int ind = getPlayerIndex(player);
					if(ind>=0) {
						// NOTE: free the index in the index mask
						playerIndexMask[player->node][ind] = SDL_FALSE;

						// NOTE: remove the player from the players array
						memset(player, 0x00, sizeof(struct Player));
						printf("Logout success!\n");
					} else {
						// NOTE: player was never in the players array should
						// probably log this sort of thing
					}
				} break;
				case 0x07: {
					// NOTE: get the player which is logged in on the incoming address
					struct Player *player = getPlayerFromIP(packet.address);
					if(player == NULL) break;

					// NOTE: perform a check to see if the player can move upward
					player->y--;

					// NOTE: save the new player state
					playerSave(player);

					// NOTE: let everyone know that the player has moved upward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!playerIndexMask[player->node][i])
							continue;

						UDPpacket _packet = {};
						struct Player *tempPlayer = &players[player->node][i];

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
						memcpy(_packet.data+offset, &player->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &player->y, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = players[player->node][i].host;
						_packet.address.port = players[player->node][i].port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x08: {
					// NOTE: get the player which is logged in on the incoming address
					struct Player *player = getPlayerFromIP(packet.address);
					if(player == NULL) break;

					// NOTE: perform a check to see if the player can move downward
					player->y++;

					// NOTE: save the new player state
					playerSave(player);

					// NOTE: let everyone know that the player has moved upward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!playerIndexMask[player->node][i])
							continue;

						UDPpacket _packet = {};
						struct Player *tempPlayer = &players[player->node][i];

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
						memcpy(_packet.data+offset, &player->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &player->y, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = players[player->node][i].host;
						_packet.address.port = players[player->node][i].port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x09: {
					// NOTE: get the player which is logged in on the incoming address
					struct Player *player = getPlayerFromIP(packet.address);
					if(player == NULL) break;

					// NOTE: perform a check to see if the player can move leftward
					player->x--;

					// NOTE: save the new player state
					playerSave(player);

					// NOTE: let everyone know that the player has moved upward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!playerIndexMask[player->node][i])
							continue;

						UDPpacket _packet = {};
						struct Player *tempPlayer = &players[player->node][i];

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
						memcpy(_packet.data+offset, &player->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &player->x, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = players[player->node][i].host;
						_packet.address.port = players[player->node][i].port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x0A: {
					// NOTE: get the player which is logged in on the incoming address
					struct Player *player = getPlayerFromIP(packet.address);
					if(player == NULL) break;

					// NOTE: perform a check to see if the player can move rightward
					player->x++;

					// NOTE: save the new player state
					playerSave(player);

					// NOTE: let everyone know that the player has moved upward
					int i;
					for(i=0; i<PLAYER_MAX; i++) {
						if(!playerIndexMask[player->node][i])
							continue;

						UDPpacket _packet = {};
						struct Player *tempPlayer = &players[player->node][i];

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
						memcpy(_packet.data+offset, &player->id, 4);
						offset += 4;
						memcpy(_packet.data+offset, &player->x, 4);
						offset += 4;

						_packet.len = offset;
						_packet.address.host = players[player->node][i].host;
						_packet.address.port = players[player->node][i].port;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet.data);
					}
				} break;
				case 0x0B: {
					// NOTE: get the player which is logged in on the incoming address
					struct Player *player = getPlayerFromIP(packet.address);
					if(player == NULL) break;

					// NOTE: notify client of how many players are being sent
					uint32_t numOnNode = getNumOnNode(player->node);
					UDPpacket _packet0 = {};

					_packet0.data = (uint8_t *)malloc(0x04);

					memcpy(_packet0.data, &numOnNode, 4);

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
						if(!playerIndexMask[player->node][i])
							continue;

						UDPpacket _packet1 = {};
						struct Player *tempPlayer = &players[player->node][i];

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

						memcpy(_packet1.data+offset, &tempPlayer->state, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPlayer->id, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPlayer->node, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPlayer->x, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPlayer->y, 4);
						offset += 4;
						memcpy(_packet1.data+offset, &tempPlayer->count, 4);
						offset += 4;

						_packet1.len = offset;
						_packet1.address = packet.address;

						if(!SDLNet_UDP_Send(serverFD, -1, &_packet1))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						free(_packet1.data);
					}
				} break;
				case 0x0C: {
					// NOTE: player responding with a pong packet
					playerToPing++;
					waitingForPong = SDL_FALSE;
				} break;
			}

			// NOTE: free the packet when done processing
			free(packet.data);
		}
	}

	// NOTE: free socketset
	SDLNet_FreeSocketSet(socketSet);

	// NOTE: close the socket file descriptor
	SDLNet_UDP_Close(serverFD);

	// NOTE: close database connection
	sqlite3_close(database);

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
