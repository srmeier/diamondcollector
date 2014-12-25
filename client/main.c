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
#include "logout.h"
#include "network.h"

//-----------------------------------------------------------------------------


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
				// NOTE: begin shutdown process
				if(mainChr.state) {
					// NOTE: logout over network
					struct logoutPacket p = {
						username, password,
						unSize, pwSize
					};

					static uint8_t retCode = 0xFF;
					retCode = playerLogout(&p, retCode);

					switch(retCode) {
						case 0x00: {
							// NOTE: waiting for logout message
							SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

							char *str0 = "Logging out...";
							drawText(str0, color0, 0, 0);
						} break;
						case 0x01: {
							// NOTE: if the retCode from server is good then close the game
							running = SDL_FALSE;
						} break;
					}
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
