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

//----------------------------------------------------------------------
#define SCREEN_W 320 // 40 -> 20
#define SCREEN_H 240 // 30 -> 15
#define NUM_SPRITES 0
#define SCREEN_NAME "Prototype"
#define SCREEN_SCALE 2

//-----------------------------------------------------------------------------
int serverChannel;
IPaddress serverIp;
UDPsocket clientFD;

SDL_bool running;
SDL_Window *window;
SDL_Surface *screen;
SDL_Renderer *renderer;

//-----------------------------------------------------------------------------
void libInit(void);
void netInit(void);
void gfxInit(void);

void libQuit(void);
void netQuit(void);
void gfxQuit(void);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	libInit();
	netInit();
	gfxInit();
	
	running = SDL_TRUE;

	while(running) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, 0, 0xFF00FF);

		/* === */

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT: {
					running = SDL_FALSE;
				} break;
				
				case SDL_KEYDOWN: {
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE:  break;
						case SDLK_UP:  break;
						case SDLK_DOWN:  break;
						case SDLK_LEFT:  break;
						case SDLK_RIGHT:  break;
						case SDLK_LCTRL:  break;
						case SDLK_LALT:  break;
					}
				} break;

				case SDL_KEYUP: {
					switch(event.key.keysym.sym) {
						case SDLK_UP:  break;
						case SDLK_DOWN:  break;
						case SDLK_LEFT:  break;
						case SDLK_RIGHT:  break;
						case SDLK_LCTRL:  break;
						case SDLK_LALT:  break;
					}
				} break;
			}
		}

		/* === */

		

		/* === */

		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, screen);
		SDL_RenderCopy(renderer, tex, NULL, NULL);

		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(tex);
	}

	gfxQuit();
	netQuit();
	libQuit();

	return 0;
}

//-----------------------------------------------------------------------------
void libInit(void) {
	// NOTE: initialize the libraries
	if(SDL_Init(SDL_INIT_EVERYTHING)!=0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	if(SDLNet_Init()==-1) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}

	if(TTF_Init()==-1) {
		fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
void netInit(void) {
	// NOTE: resolve the host IP struct
	if(SDLNet_ResolveHost(&serverIp, "www.libgcw.com", 3490) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());

		TTF_Quit();
		SDLNet_Quit();
		SDL_Quit();

		exit(-1);
	}

	// NOTE: open UDP socket file descriptor
	clientFD = SDLNet_UDP_Open(0);
	if(!clientFD) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());

		TTF_Quit();
		SDLNet_Quit();
		SDL_Quit();

		exit(-1);
	}

	// NOTE: bind the server ip address to a channel on the client socket
	serverChannel = SDLNet_UDP_Bind(clientFD, -1, &serverIp);
	if(serverChannel==-1) {
		fprintf(stderr, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());

		SDLNet_UDP_Close(clientFD);

		TTF_Quit();
		SDLNet_Quit();
		SDL_Quit();
		
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
void gfxInit(void) {
	// NOTE: initialize the GFX resources
	window = SDL_CreateWindow(
		SCREEN_NAME,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_SCALE*SCREEN_W,
		SCREEN_SCALE*SCREEN_H,
		0
	);

	renderer = SDL_CreateRenderer(
		window, -1,
		SDL_RENDERER_ACCELERATED|
		SDL_RENDERER_PRESENTVSYNC
	);

	screen = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(screen, 1, 0xFF00FF);
	SDL_FillRect(screen, 0, 0xFF00FF);
}

//-----------------------------------------------------------------------------
void libQuit(void) {
	// NOTE: release the libraries
	TTF_Quit();
	SDLNet_Quit();
	SDL_Quit();
}

//-----------------------------------------------------------------------------
void netQuit(void) {
	// NOTE: unbind all ips on the server channel
	SDLNet_UDP_Unbind(clientFD, serverChannel);

	// NOTE: close UDP socket
	SDLNet_UDP_Close(clientFD);
}

//-----------------------------------------------------------------------------
void gfxQuit(void) {
	// NOTE: free the GFX resources
	SDL_FreeSurface(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
