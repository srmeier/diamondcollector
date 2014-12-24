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
#include "player.h"

//-----------------------------------------------------------------------------
uint8_t numChrsOnNode[32];
struct Player chrsOnline[32][4];

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
			uint8_t offset = 0;

			memcpy(&flag, packet.data, 1);
			offset += 1;

			// NOTE: process the packet
			switch(flag) {
				case 0x01: {
					// NOTE: ignore the packet if it is from a bound address
					if(packet.channel!=-1) {
						free(packet.data);
						continue;
					}

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
								printf("\nLogin success!\n");

								struct Player chr = getPlayerInfo(playerID);

								printf("X        -> %d\n", chr.x);
								printf("Y        -> %d\n", chr.y);
								printf("ID       -> %d\n", chr.id);
								printf("Node     -> %d\n", chr.node);
								printf("Username -> %s\n", chr.username);
								printf("Password -> %s\n", chr.password);
								printf("State    -> %d\n", chr.state);
								printf("Count    -> %d\n\n", chr.count);

								UDPpacket sPacket = {};

								/*
								- flag         ( 1)
								- State        ( 4)
								- PlayerID     ( 4)
								- Node         ( 4)
								- X            ( 4)
								- Y            ( 4)
								- DiamondCount ( 4)
								============== (25)
								*/

								sPacket.maxlen = 0x19; // 25 bytes
								sPacket.data = (uint8_t *)malloc(0x19);

								uint8_t offset = 0;

								memset(sPacket.data+offset, 0x03, 1);
								offset += 1;
								memcpy(sPacket.data+offset, &chr.state, 4);
								offset += 4;
								memcpy(sPacket.data+offset, &chr.id, 4);
								offset += 4;
								memcpy(sPacket.data+offset, &chr.node, 4);
								offset += 4;
								memcpy(sPacket.data+offset, &chr.x, 4);
								offset += 4;
								memcpy(sPacket.data+offset, &chr.y, 4);
								offset += 4;
								memcpy(sPacket.data+offset, &chr.count, 4);
								offset += 4;

								sPacket.len = offset;
								sPacket.address = packet.address;

								// TODO: will have to check if Node is full...
								if(!SDLNet_UDP_Send(serverFD, -1, &sPacket))
									fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

								// NOTE: set flag to 0x04 so everyone else on the node will hear about
								// the new player connection (the player won't hear this because they
								// aren't on the channel yet)
								memset(sPacket.data, 0x04, 1);

								if(!SDLNet_UDP_Send(serverFD, chr.node, &sPacket)) {
									// NOTE: could just be that there is no one on the channel
									if(strcmp(SDLNet_GetError(), ""))
										fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
								}


								// NOTE: binding the client to the channel corresponding to the
								// node they are on (maximum of 4 players per node with 32 nodes)
								int channel = SDLNet_UDP_Bind(serverFD, chr.node, &packet.address);
								if(channel==-1) {
									fprintf(stderr, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
								} else {
									int posOnNode = numChrsOnNode[chr.node];
									numChrsOnNode[chr.node]++;

									memcpy(&chrsOnline[chr.node][posOnNode], &chr, sizeof(chr));
								}

								// TODO: will have to free these char * when the player disconnects
								//free(chr.username);
								//free(chr.password);
								free(sPacket.data);
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
					// NOTE: ignore the packet if it isn't from a bound address
					if(packet.channel==-1) break;

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

				case 0x0B: {
					// NOTE: ignore the packet if it isn't from a bound address
					if(packet.channel==-1) break;

					// NOTE: notify client of how many players are being sent
					UDPpacket s1Packet = {};

					s1Packet.len = 1;
					s1Packet.maxlen = 1;
					s1Packet.address = packet.address;
					s1Packet.data = &numChrsOnNode[packet.channel];

					if(!SDLNet_UDP_Send(serverFD, -1, &s1Packet))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

					// NOTE: this flag will return all the players on a particular node
					int i;
					for(i=0; i<numChrsOnNode[packet.channel]; i++) {
						UDPpacket s2Packet = {};
						struct Player chr = chrsOnline[packet.channel][i];

						/*
						- state        ( 4)
						- PlayerID     ( 4)
						- Node         ( 4)
						- X            ( 4)
						- Y            ( 4)
						- DiamondCount ( 4)
						============== (24)
						*/

						s2Packet.maxlen = 0x18; // 24 bytes
						s2Packet.data = (uint8_t *)malloc(0x18);

						uint8_t offset = 0;

						memcpy(s2Packet.data+offset, &chr.state, 4);
						offset += 4;
						memcpy(s2Packet.data+offset, &chr.id, 4);
						offset += 4;
						memcpy(s2Packet.data+offset, &chr.node, 4);
						offset += 4;
						memcpy(s2Packet.data+offset, &chr.x, 4);
						offset += 4;
						memcpy(s2Packet.data+offset, &chr.y, 4);
						offset += 4;
						memcpy(s2Packet.data+offset, &chr.count, 4);
						offset += 4;

						s2Packet.len = offset;
						s2Packet.address = packet.address;

						if(!SDLNet_UDP_Send(serverFD, -1, &s2Packet))
							fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

						// TODO: will have to free these char * when the player disconnects
						//free(chr.username);
						//free(chr.password);
						free(s2Packet.data);
					}
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
