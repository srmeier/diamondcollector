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
#include "network_calls.h"

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

					net_login(plManager, &packet, &offset);

				} break;
				case 0x02: {
					
					net_logout(plManager, &packet, &offset);

				} break;
				case 0x07: {
					
					net_moveUp(plManager, &packet, &offset);

				} break;
				case 0x08: {
					
					net_moveDown(plManager, &packet, &offset);

				} break;
				case 0x09: {
					
					net_moveLeft(plManager, &packet, &offset);

				} break;
				case 0x0A: {
					
					net_moveRight(plManager, &packet, &offset);

				} break;
				case 0x0B: {

					net_sendOutPlInfo(plManager, &packet, &offset);
					
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
					
					net_sendOutNodeInfo(plManager, &packet, &offset);

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
