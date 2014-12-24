/*
gcc main.c -o client.exe -I./include -L./lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_net
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

/* TODO:
- the server should include the mainChr info with the loginSuccess packet
*/

//-----------------------------------------------------------------------------
#define SCREEN_W 320 // 40 -> 20
#define SCREEN_H 240 // 30 -> 15
#define NUM_SPRITES 1025
#define SCREEN_NAME "Prototype"
#define SCREEN_SCALE 1

/* NOTE: engine variables */
//-----------------------------------------------------------------------------
SDL_bool aChk;
SDL_bool bChk;
SDL_bool upChk;
SDL_bool downChk;
SDL_bool leftChk;
SDL_bool rightChk;

SDL_bool aBnt;
SDL_bool bBnt;
SDL_bool upBnt;
SDL_bool downBnt;
SDL_bool leftBnt;
SDL_bool rightBnt;

int gameState;
TTF_Font *font8;
SDL_bool running;
int serverChannel;
IPaddress serverIp;
UDPsocket clientFD;
SDL_Window *window;
SDL_Surface *screen;
SDL_Renderer *renderer;
SDLNet_SocketSet socketSet;
SDL_Surface *spritesheet[NUM_SPRITES];

/* NOTE: engine includes */
//-----------------------------------------------------------------------------
#include "types.h"
#include "engine.h"

/* NOTE: instance variables */
//-----------------------------------------------------------------------------
uint8_t numChrs;
struct Player mainChr;
struct Player *chrsOnline;

/* NOTE: instance includes */
//-----------------------------------------------------------------------------
#include "login.h"

//-----------------------------------------------------------------------------
void networkPoll(void);

//-----------------------------------------------------------------------------
void inputPoll(void);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	libInit();
	netInit();
	gfxInit();
	
	/* === */

	int testInds[4] = {
		 1,  2,
		33, 34
	}; SDL_Surface *test = buildSprite(2, 2, testInds);

	uint32_t unSize = 1;
	char *username = (char *)malloc(unSize+1);
	username[unSize-1] = 0x41;
	username[unSize] = 0x00;

	uint32_t pwSize = 1;
	char *password = (char *)malloc(pwSize+1);
	password[pwSize-1] = 0x41;
	password[pwSize] = 0x00;

	/* === */

	while(running) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, 0, 0xFF00FF);

		/* === */

		inputPoll();

		/* === */

		switch(gameState) {
			case 0x00: {
				// NOTE: get the players login information
				if(getPlayerLoginInfo(&username, &unSize, &password, &pwSize)) {
					printf("Username: %s (%d)\n", username, unSize);
					printf("Password: %s (%d)\n", password, pwSize);

					clearInput();
					gameState = 0x01;
				}
			} break;
			case 0x01: {
				// NOTE: connect the player with the information they provide
				struct loginPacket p = {
					username, password,
					unSize, pwSize
				};

				static uint8_t retCode = 0xFF;
				retCode = playerLogin(&p, retCode);

				switch(retCode) {
					case 0x00: {
						// NOTE: diaplay a connecting message and poll for the server response
						SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

						char *str0 = "Connecting...";
						drawText(str0, color0, 0, 0);
					} break;
					case 0x01: {
						// NOTE: incorrect password
						SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

						char *str0 = "Incorrect username or password.";
						drawText(str0, color0, 0, 0);

						// NOTE: allow player to back out to correct username/password
						if(bBnt && !bChk) {
							clearInput();
							retCode = 0xFF;
							gameState = 0x00;
						} else if(!bBnt) bChk = SDL_FALSE;
					} break;
					case 0x02: {
						// NOTE: account in use
						SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

						char *str0 = "Account in use.";
						drawText(str0, color0, 0, 0);

						// NOTE: allow player to back out to try again
						if(bBnt && !bChk) {
							clearInput();
							retCode = 0xFF;
							gameState = 0x00;
						} else if(!bBnt) bChk = SDL_FALSE;
					} break;
					case 0x03: {
						// NOTE: login success
						printf("\nlogin success\n");

						// NOTE: the other mainChr variables are set by server
						mainChr.username = username;
						mainChr.password = password;

						retCode = 0xFF;
						gameState = 0x02;
					} break;
				}
			} break;
			case 0x02: {
				// NOTE: login was a success so now we need to get all the current players info
				static uint8_t retCode = 0xFF;
				if(retCode==0xFF) {
					printf("X        -> %d\n", mainChr.x);
					printf("Y        -> %d\n", mainChr.y);
					printf("ID       -> %d\n", mainChr.id);
					printf("Node     -> %d\n", mainChr.node);
					printf("Username -> %s\n", mainChr.username);
					printf("Password -> %s\n", mainChr.password);
					printf("State    -> %d\n", mainChr.state);
					printf("Count    -> %d\n\n", mainChr.count);
				}

				retCode = getChrsOnline(&chrsOnline, &numChrs, retCode);

				switch(retCode) {
					case 0x00:
					case 0x01: {
						// NOTE: diaplay a connecting message and poll for the server response
						SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

						char *str0 = "Waiting for server status...";
						drawText(str0, color0, 0, 0);
					} break;
					case 0x02: {
						// NOTE: we got all the players and everything...
						clearInput();
						retCode = 0xFF;
						gameState = 0x03;
					} break;
				}
			} break;
			case 0x03: {
				/*
				*/

				// NOTE: main game loop
				networkPoll();

				// NOTE: any 0x04 packets at this point are new players
				int i;
				for(i=0; i<numChrs; i++) {
					// NOTE: the mainChr is in this array as well
					struct Player chr = chrsOnline[i];

					SDL_Rect rect = {8*2*chr.x, 8*2*chr.y, 8*2, 8*2};
					SDL_BlitSurface(test, NULL, screen, &rect);
				}

				/*
				*/
			} break;
			case 0xFF: {
				if(mainChr.state) {
					// NOTE: logout over network
					SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

					char *str0 = "Logging out...";
					drawText(str0, color0, 0, 0);

					// NOTE: if the retCode from server is good then close the game
					running = SDL_FALSE;
				} else running = SDL_FALSE;
			} break;
		}

		/* === */

		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, screen);
		SDL_RenderCopy(renderer, tex, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(tex);
	}

	/* === */

	int i;
	for(i=0; i<numChrs; i++) {
		// NOTE: this runs over client also
		free(chrsOnline[i].username);
		free(chrsOnline[i].password);
	} free(chrsOnline);

	SDL_FreeSurface(test);

	/* === */

	gfxQuit();
	netQuit();
	libQuit();

	return 0;
}

//-----------------------------------------------------------------------------
void networkPoll(void) {
	// NOTE: check for connections
	if(SDLNet_CheckSockets(socketSet, 0)==-1) {
		fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
		return;
	}

	if(SDLNet_SocketReady(clientFD)) {
		// NOTE: here we simply get how many players the server is going to send
		UDPpacket packet;

		// NOTE: allocate space for packet
		packet.maxlen = 0xAA; // 170 bytes
		packet.data = (uint8_t *)malloc(0xAA);

		// NOTE: get packet
		int recv = SDLNet_UDP_Recv(clientFD, &packet);
		if(!recv) {
			free(packet.data);
			return;
		}

		// NOTE: if it isn't the server then ignore it
		if(packet.channel!=serverChannel) {
			free(packet.data);
			return;
		}

		// NOTE: read the flag for packet identity
		uint8_t flag = 0;
		uint8_t offset = 0;

		memcpy(&flag, packet.data, 1);
		offset += 1;

		switch(flag) {
			case 0x04: {
				// NOTE: new player connection
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

				struct Player tempPlayers[numChrs];
				memcpy(tempPlayers, chrsOnline, numChrs*sizeof(struct Player));

				chrsOnline = (struct Player *)realloc(chrsOnline, ++numChrs*sizeof(struct Player));
				memset(chrsOnline, 0x00, numChrs*sizeof(struct Player));

				int i;
				for(i=0; i<(numChrs-1); i++)
					memcpy(&chrsOnline[i], &tempPlayers[i], sizeof(struct Player));
				struct Player *chr = &chrsOnline[numChrs-1];

				memcpy(&chr->state, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->id, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->node, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->x, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->y, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->count, packet.data+offset, 4);
				offset += 4;
			} break;
		}

		// NOTE: free the packet
		free(packet.data);
	}
}

//-----------------------------------------------------------------------------
/*
// NOTE: send logout packet
memset(&packet, 0, sizeof(packet));

unSize = strlen(argv[1]);
pwSize = strlen(argv[2]);

packet.maxlen = 1+4+unSize+4+pwSize;
packet.data = (uint8_t *)malloc(packet.maxlen);

offset = 0;

memset(packet.data+offset, 0x02, 1);
offset += 1;
memcpy(packet.data+offset, &unSize, 4);
offset += 4;
memcpy(packet.data+offset, argv[1], unSize);
offset += unSize;
memcpy(packet.data+offset, &pwSize, 4);
offset += 4;
memcpy(packet.data+offset, argv[2], pwSize);
offset += pwSize;

packet.len = offset;

// NOTE: SDLNet_UDP_Send returns the number of people the packet was sent to
if(!SDLNet_UDP_Send(clientFD, serverChannel, &packet))
	fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

free(packet.data);
*/
