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

//-----------------------------------------------------------------------------
sqlite3 *database;
UDPsocket serverFD;
SDLNet_SocketSet socketSet;

//-----------------------------------------------------------------------------
#include "types.h"
#include "login.h"
#include "logout.h"

// NOTE: if login successes then bind client IP to channel
// NOTE: 4 possible ips per channel, 32 channels, 128 possible
// players online. I will have to keep track of which channels
// I've put players on so I can send a packet on each channel
// NOTE: packets other than login can be ignored when the sender isn't bound to
// a channel else the channel will be -1 if not bound

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

	for(;;) {
		// NOTE: wait for a connection
		int n = SDLNet_CheckSockets(socketSet, -1);

		if(n==-1) {
			fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			break;
		} if(!n) {
			// NOTE: if the server doesn't recieve anything then clean up player state
			// and send out any ping-pong packets to make sure the client is still connected
			continue;
		}

		// NOTE: only need to check the server
		if(SDLNet_SocketReady(serverFD)) {
			// NOTE: setup a packet which is big enough to store any client message
			UDPpacket packet;

			// NOTE: allocate space for packet
			packet.maxlen = 0xAA; // 170 bytes
			packet.data = (uint8_t *)malloc(0xAA);

			// NOTE: get packet
			int recv = SDLNet_UDP_Recv(serverFD, &packet);
			if(!recv) {
				free(packet.data);
				continue;
			}

			// NOTE: display IPaddress infomation
			Uint32 ipaddr = SDL_SwapBE32(packet.address.host);
			printf("packet from-> %d.%d.%d.%d:%d bound to channel %d\n",
				ipaddr>>24, (ipaddr>>16)&0xFF, (ipaddr>>8)&0xFF, ipaddr&0xFF,
				SDL_SwapBE16(packet.address.port), packet.channel);

			// NOTE: read the flag for packet identity
			uint8_t flag;

			memcpy(&flag, packet.data, 1);
			uint8_t offset = 1;

			// NOTE: process the packet
			switch(flag) {
				case 0x01: {
					// NOTE: get login packet data struct
					struct loginPacket p = exLoginPacket(&packet, &offset);

					if(offset==packet.len) {
						// NOTE: check that the player's password matches
						uint32_t playerID;

						switch(playerLogin(&p, &playerID)) {
							case 0x00: {
								// NOTE: retCode = 0x00 on password mismatch
								printf("Incorrect password.\n");

								uint8_t sFlag = 0x01;
								UDPpacket sPacket = {};

								sPacket.len = 1;
								sPacket.data = &sFlag;
								sPacket.maxlen = 1;
								sPacket.address = packet.address;

								if(!SDLNet_UDP_Send(serverFD, -1, &sPacket))
									fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
							} break;
							case 0x01: {
								// NOTE: retCode = 0x01 on password match
								printf("Login success!\n");

								uint8_t sFlag = 0x03;
								UDPpacket sPacket = {};

								sPacket.len = 1;
								sPacket.data = &sFlag;
								sPacket.maxlen = 1;
								sPacket.address = packet.address;

								if(!SDLNet_UDP_Send(serverFD, -1, &sPacket))
									fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

								// NOTE: get the player's X, Y, Node, and DiamondCount to send out
							} break;
							case 0x02: {
								// NOTE: retCode = 0x02 if password matches but the account is in use
								printf("Account in use.\n");

								uint8_t sFlag = 0x02;
								UDPpacket sPacket = {};

								sPacket.len = 1;
								sPacket.data = &sFlag;
								sPacket.maxlen = 1;
								sPacket.address = packet.address;

								if(!SDLNet_UDP_Send(serverFD, -1, &sPacket))
									fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
							} break;
						}
					}

					free(p.username);
					free(p.password);
				} break;

				case 0x02: {
					// NOTE: get logout packet data struct
					struct logoutPacket p = exLogoutPacket(&packet, &offset);

					if(offset==packet.len) {
						// NOTE: check that the player's password matches
						uint32_t playerID;

						switch(playerLogout(&p, &playerID)) {
							case 0x02: {
								printf("Logout success!\n");
							} break;
						}
					}

					free(p.username);
					free(p.password);
				} break;
			}

			// NOTE: free the packet
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
