/*
gcc main.c -o client.exe -I./include -L./lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_net
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

/* NOTE: engine includes */
//-----------------------------------------------------------------------------
#include "types.h"
#include "engine.h"

/* NOTE: instance variables */
//-----------------------------------------------------------------------------
uint32_t numChrs;
struct Player mainChr;
uint8_t nodeGrid[15][30];
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

	int testInds0[4] = {
		 3,  4,
		35, 36
	}; SDL_Surface *test0 = buildSprite(2, 2, testInds0);

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

						char *str0 = "Waiting for player information...";
						drawText(str0, color0, 0, 0);
					} break;
					case 0x02: {
						// NOTE: we got all the players and everything...
						clearInput();
						retCode = 0xFF;
						gameState = 0x04;
					} break;
				}
			} break;
			case 0x03: {
				/*
				*/

				// NOTE: main game loop
				networkPoll();

				if(mainChr.moveState.moveFrame==0) {
				// NOTE: if the up key is pressed send a request to move up
				if(upBnt && !upChk && !mainChr.moveState.moving) {
					UDPpacket packet = {};

					uint8_t flag = 0x07;
					packet.data = &flag;

					packet.len = 1;
					packet.maxlen = 1;

					if(!SDLNet_UDP_Send(clientFD, serverChannel, &packet))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

					upChk = SDL_TRUE;
					mainChr.moveState.moving = SDL_TRUE;

					int i;
					for(i=0; i<numChrs; i++) {
						if(chrsOnline[i].id==mainChr.id)
							chrsOnline[i].moveState.moving = SDL_TRUE;
					}
				} else if(!upBnt && !mainChr.moveState.moving) upChk = SDL_FALSE;

				// NOTE: if the down key is pressed send a request to move down
				if(downBnt && !downChk && !mainChr.moveState.moving) {
					UDPpacket packet = {};

					uint8_t flag = 0x08;
					packet.data = &flag;

					packet.len = 1;
					packet.maxlen = 1;

					if(!SDLNet_UDP_Send(clientFD, serverChannel, &packet))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

					downChk = SDL_TRUE;
					mainChr.moveState.moving = SDL_TRUE;

					int i;
					for(i=0; i<numChrs; i++) {
						if(chrsOnline[i].id==mainChr.id)
							chrsOnline[i].moveState.moving = SDL_TRUE;
					}
				} else if(!downBnt && !mainChr.moveState.moving) downChk = SDL_FALSE;

				// NOTE: if the left key is pressed send a request to move left
				if(leftBnt && !leftChk && !mainChr.moveState.moving) {
					UDPpacket packet = {};

					uint8_t flag = 0x09;
					packet.data = &flag;

					packet.len = 1;
					packet.maxlen = 1;

					if(!SDLNet_UDP_Send(clientFD, serverChannel, &packet))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

					leftChk = SDL_TRUE;
					mainChr.moveState.moving = SDL_TRUE;

					int i;
					for(i=0; i<numChrs; i++) {
						if(chrsOnline[i].id==mainChr.id)
							chrsOnline[i].moveState.moving = SDL_TRUE;
					}
				} else if(!leftBnt && !mainChr.moveState.moving) leftChk = SDL_FALSE;

				// NOTE: if the right key is pressed send a request to move right
				if(rightBnt && !rightChk && !mainChr.moveState.moving) {
					UDPpacket packet = {};

					uint8_t flag = 0x0A;
					packet.data = &flag;

					packet.len = 1;
					packet.maxlen = 1;

					if(!SDLNet_UDP_Send(clientFD, serverChannel, &packet))
						fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

					rightChk = SDL_TRUE;
					mainChr.moveState.moving = SDL_TRUE;

					int i;
					for(i=0; i<numChrs; i++) {
						if(chrsOnline[i].id==mainChr.id)
							chrsOnline[i].moveState.moving = SDL_TRUE;
					}
				} else if(!rightBnt && !mainChr.moveState.moving) rightChk = SDL_FALSE;
				}

				int i, j;

				// NOTE: update moveState
				for(i=0; i<numChrs; i++) {
					updateMoveState(&chrsOnline[i]);
				}

				// NOTE: draw everything
				for(j=0; j<15; j++) {
					for(i=0; i<30; i++) {
						if(nodeGrid[j][i]==0x01) {
							SDL_Rect rect = {8*2*i, 12*2*j, 8*2, 12*2};
							SDL_BlitSurface(test0, NULL, screen, &rect);
						}
					}
				}

				for(i=0; i<numChrs; i++) {
					// NOTE: the mainChr is in this array as well
					struct Player chr = chrsOnline[i];

					int xOffset = 0;
					int yOffset = 0;

					if(chr.moveState.moveDirec==0) {
						yOffset = 24+chr.moveState.y;
					} else if(chr.moveState.moveDirec==1) {
						yOffset = chr.moveState.y-24;
					} else if(chr.moveState.moveDirec==2) {
						xOffset = 16+chr.moveState.x;
					} else if(chr.moveState.moveDirec==3) {
						xOffset = chr.moveState.x-16;
					}

					SDL_Rect rect = {8*2*chr.x+xOffset, 12*2*chr.y+yOffset, 8*2, 12*2};
					SDL_BlitSurface(test, NULL, screen, &rect);

					/*
					SDL_Rect rect2 = {8*2*chr.x, 8*2*chr.y, 8*2, 8*2};
					SDL_BlitSurface(test, NULL, screen, &rect2);
					*/
				}

				/*
				*/
			} break;
			case 0x04: {
				// NOTE: login was a success so now we need to get all the spriteIDs for the node
				static uint8_t retCode = 0x00;
				retCode = getNodeSpriteIDs(retCode);

				switch(retCode) {
					case 0x00:
					case 0x01: {
						// NOTE: diaplay a connecting message and poll for the server response
						SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};

						char *str0 = "Loading node information...";
						drawText(str0, color0, 0, 0);
					} break;
					case 0x02: {
						// NOTE: we got all the node sprites...
						clearInput();
						retCode = 0x00;
						gameState = 0x03;
					} break;
				}
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
