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
#define SCREEN_SCALE 3

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
struct Player mainChr;
struct Player *chrsOnline;

/* NOTE: instance includes */
//-----------------------------------------------------------------------------
#include "login.h"

//-----------------------------------------------------------------------------
uint8_t getChrsOnline(struct Player **chrsOnline, uint8_t retCode);

//-----------------------------------------------------------------------------
void inputPoll(void);
void networkPoll(void);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	libInit();
	netInit();
	gfxInit();
	
	/* === */

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
							gameState = 0x00;
						} else if(!bBnt) bChk = SDL_FALSE;
					} break;
					case 0x03: {
						// NOTE: login success
						printf("login success\n");

						// NOTE: the other mainChr variables are set by server
						mainChr.state = 0x01; // true idle
						mainChr.username = username;
						mainChr.password = password;

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
					printf("Count    -> %d\n", mainChr.count);
				}

				retCode = getChrsOnline(&chrsOnline, retCode);

				switch(retCode) {
					case 0x00: {
						// NOTE: diaplay a connecting message and poll for the server response
						SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

						char *str0 = "Connecting...";
						drawText(str0, color0, 0, 0);
					} break;
				}
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

	free(username);
	free(password);

	/* === */

	gfxQuit();
	netQuit();
	libQuit();

	return 0;
}

//-----------------------------------------------------------------------------
uint8_t getChrsOnline(struct Player **chrsOnline, uint8_t retCode) {
	// NOTE: get chrsOnline for a particular Node
	switch(retCode) {
		case 0x00: {
			//struct Player *_chrsOnline = *chrsOnline;
			//*chrsOnline = _chrsOnline;
		} break;
		case 0xFF: {
			// NOTE: set the retCode
			return 0x00;
		} break;
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

/*
void networkPoll(void) {
	// NOTE: check packet for a connection
	if(SDLNet_CheckSockets(socketSet, 0)==-1) {
		fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
		return;
	}

	if(SDLNet_SocketReady(clientFD)) {
		// NOTE: setup a packet which is big enough to store any server message
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

		// NOTE: display IPaddress infomation
		Uint32 ipaddr = SDL_SwapBE32(packet.address.host);
		printf("packet from-> %d.%d.%d.%d:%d bound to channel %d\n",
			ipaddr>>24, (ipaddr>>16)&0xFF, (ipaddr>>8)&0xFF, ipaddr&0xFF,
			SDL_SwapBE16(packet.address.port), packet.channel);

		// NOTE: if it isn't the server then ignore it
		if(packet.channel!=serverChannel) {
			free(packet.data);
			return;
		}

		// NOTE: read the flag for packet identity
		uint8_t flag;

		memcpy(&flag, packet.data, 1);
		uint8_t offset = 1;

		// NOTE: process the packet
		switch(flag) {
			case 0x01: {
				printf("Incorrect password.\n");
			} break;
			case 0x02: {
				printf("Account in use.");
			} break;
			case 0x03: {
				printf("Login success!\n");
			} break;
		}

		// NOTE: free the packet
		free(packet.data);
	}
}
*/
