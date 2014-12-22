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
int main(int argc, char *argv[]) {
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

	// NOTE: resolve the host IP struct
	IPaddress serverIp;
	if(SDLNet_ResolveHost(&serverIp, "www.libgcw.com", 3490) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());

		TTF_Quit();
		SDLNet_Quit();
		SDL_Quit();

		return -1;
	}

	// NOTE: display IPaddress infomation
	Uint32 serverIpAddr = SDL_SwapBE32(serverIp.host);
	printf("ServerInfo-> %d.%d.%d.%d:%d\n", serverIpAddr>>24, (serverIpAddr>>16)&0xFF,
		(serverIpAddr>>8)&0xFF, serverIpAddr&0xFF, SDL_SwapBE16(serverIp.port));

	// NOTE: open UDP socket file descriptor
	UDPsocket clientFD = SDLNet_UDP_Open(0);
	if(!clientFD) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());

		TTF_Quit();
		SDLNet_Quit();
		SDL_Quit();

		return -1;
	}

	// NOTE: bind the server ip address to a channel on the client socket
	int serverChannel = SDLNet_UDP_Bind(clientFD, -1, &serverIp);
	if(serverChannel==-1) {
		fprintf(stderr, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());

		SDLNet_UDP_Close(clientFD);
		TTF_Quit();
		SDLNet_Quit();
		SDL_Quit();
		
		return -1;
	}

	// NOTE: unbind all ips on the server channel
	SDLNet_UDP_Unbind(clientFD, serverChannel);

	// NOTE: close UDP socket
	SDLNet_UDP_Close(clientFD);

	// NOTE: release the libraries
	TTF_Quit();
	SDLNet_Quit();
	SDL_Quit();

	return 0;
}
