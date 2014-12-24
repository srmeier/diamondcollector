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

//-----------------------------------------------------------------------------
#define SCREEN_W 320 // 40 -> 20
#define SCREEN_H 240 // 30 -> 15
#define NUM_SPRITES 1025
#define SCREEN_NAME "Prototype"
#define SCREEN_SCALE 3

//-----------------------------------------------------------------------------
SDL_bool aBnt;
SDL_bool bBnt;
int gameState;
SDL_bool upBnt;
TTF_Font *font8;
SDL_bool downBnt;
SDL_bool leftBnt;
SDL_bool running;
SDL_bool rightBnt;
int serverChannel;
IPaddress serverIp;
UDPsocket clientFD;
SDL_Window *window;
SDL_Surface *screen;
SDL_Renderer *renderer;
SDLNet_SocketSet socketSet;
SDL_Surface *spritesheet[NUM_SPRITES];

//-----------------------------------------------------------------------------
#include "types.h"
#include "engine.h"

//-----------------------------------------------------------------------------
void inputPoll(void);
void networkPoll(void);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	libInit();
	netInit();
	gfxInit();
	
	/* === */

	SDL_bool curUnEdit = SDL_TRUE;

	int unSize = 1;
	char *username = (char *)malloc(unSize+1);
	username[unSize-1] = 0x41;
	username[unSize] = 0x00;

	int pwSize = 1;
	char *password = (char *)malloc(pwSize+1);
	password[pwSize-1] = 0x41;
	password[pwSize] = 0x00;

	SDL_bool aChk = SDL_FALSE;
	SDL_bool bChk = SDL_FALSE;
	SDL_bool upChk = SDL_FALSE;
	SDL_bool downChk = SDL_FALSE;
	SDL_bool leftChk = SDL_FALSE;
	SDL_bool rightChk = SDL_FALSE;

	/* === */

	while(running) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, 0, 0xFF00FF);

		/* === */

		inputPoll();

		/* === */

		switch(gameState) {
			case 0x00: {
				// NOTE: get the username from the player
				if(upBnt && !upChk) {
					if(curUnEdit) {
						username[unSize-1]++;
						if(username[unSize-1]==0x5B)
							username[unSize-1] = 0x41;
					} else {
						password[pwSize-1]++;
						if(password[pwSize-1]==0x5B)
							password[pwSize-1] = 0x41;
					}
					upChk = SDL_TRUE;
				} else if(!upBnt) upChk = SDL_FALSE;

				if(downBnt && !downChk) {
					if(curUnEdit) {
						username[unSize-1]--;
						if(username[unSize-1]==0x40)
							username[unSize-1] = 0x5A;
					} else {
						password[pwSize-1]--;
						if(password[pwSize-1]==0x40)
							password[pwSize-1] = 0x5A;
					}
					downChk = SDL_TRUE;
				} else if(!downBnt) downChk = SDL_FALSE;

				if(leftBnt && !leftChk && (unSize-1)>0) {
					if(curUnEdit) {
						username = (char *)realloc(username, --unSize+1);
						username[unSize] = 0x00;
					} else {
						password = (char *)realloc(password, --pwSize+1);
						password[pwSize] = 0x00;
					}
					leftChk = SDL_TRUE;
				} else if(!leftBnt) leftChk = SDL_FALSE;

				if(rightBnt && !rightChk && (unSize+1)<12) {
					if(curUnEdit) {
						username = (char *)realloc(username, ++unSize+1);
						username[unSize-1] = 0x41;
						username[unSize] = 0x00;
					} else {
						password = (char *)realloc(password, ++pwSize+1);
						password[pwSize-1] = 0x41;
						password[pwSize] = 0x00;
					}
					rightChk = SDL_TRUE;
				} else if(!rightBnt) rightChk = SDL_FALSE;

				if(aBnt && !aChk) {
					if(curUnEdit) {
						curUnEdit = SDL_FALSE;
					} else curUnEdit = SDL_TRUE;
					aChk = SDL_TRUE;
				} else if(!aBnt) aChk = SDL_FALSE;

				if(bBnt && !bChk) {
					printf("accept the info?\n");
					bChk = SDL_TRUE;
				} else if(!bBnt) bChk = SDL_FALSE;

				SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};
				SDL_Color color1 = {0xFF, 0xFF, 0xFF, 0x00};
				SDL_Color color2 = {0xFF, 0x00, 0x00, 0x00};

				char *str0 = "Username:";
				drawText(str0, color0, 0, 0);
				drawText(username, color1, 8*9, 0);

				char *str1 = "Password:";
				drawText(str1, color0, 0, 16);
				drawText(password, color1, 8*9, 16);

				if(curUnEdit)
					drawText(&username[unSize-1], color2, 8*9+8*(unSize-1), 0);
				else
					drawText(&password[pwSize-1], color2, 8*9+8*(pwSize-1), 16);
			} break;
			case 0x01: {
				// NOTE: get the password from the player
			} break;
			case 0x02: {
				// NOTE: show the connecting screen while the logging in
				// the server will send all the current players that are online
			} break;
			case 0x03: {
				// NOTE: display the game state
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

/*
// NOTE: build login packet (temp)
if(argc<3) {
	fprintf(stderr, "Error: incorrect number of arguements.\n");

	gfxQuit();
	netQuit();
	libQuit();

	return -1;
}

UDPpacket packet = {};

uint32_t unSize = strlen(argv[1]);
uint32_t pwSize = strlen(argv[2]);

packet.maxlen = 1+4+unSize+4+pwSize;
packet.data = (uint8_t *)malloc(packet.maxlen);

uint8_t offset = 0;

memset(packet.data+offset, 0x01, 1);
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
