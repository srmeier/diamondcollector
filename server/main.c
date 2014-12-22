/*
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
gcc main.c -o server -lSDL2main -lSDL2 -lSDL2_net
*/

//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

//-----------------------------------------------------------------------------
void libInit(void);
void libQuit(void);

//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
	libInit();

	/*
	// NOTE: get IPaddress struct
	IPaddress ip;
	if(SDLNet_ResolveHost(&ip, NULL, 3490) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		libQuit();
		return -1;
	}

	// NOTE: display IPaddress infomation
	Uint32 ipaddr = SDL_SwapBE32(ip.host);
	printf("ServerInfo-> %d.%d.%d.%d:%d\n", ipaddr>>24, (ipaddr>>16)&0xFF,
		(ipaddr>>8)&0xFF, ipaddr&0xFF, SDL_SwapBE16(ip.port));
	*/

	// NOTE: open socket file descriptor
	UDPsocket server = SDLNet_UDP_Open(3490);
	if(!server) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		libQuit();
		return -1;
	}

	// NOTE: setup a socket set
	int numClients = 0;
	SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(socketSet, server);

	// NOTE: wait for a connection
	for(;;) {
		int numRdy = SDLNet_CheckSockets(socketSet, -1);
		if(numRdy == -1) {
			fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			break;
		} if(!numRdy) continue;

		if(SDLNet_SocketReady(server)) {
			numRdy--;

			UDPpacket packet;
			packet.maxlen = 0xFF;
			packet.data = (uint8_t *)malloc(0xFF);

			int numrecv = SDLNet_UDP_Recv(server, &packet);
			printf("got a packet of size: %d\n", packet.len);

			free(packet.data);

			/*
			TCPsocket sock = SDLNet_TCP_Accept(server);
			if(sock) {
				unsigned char data[4];
				int bsent = SDLNet_TCP_Recv(sock, data, sizeof data);

				if(bsent < sizeof data) {
					if(SDLNet_GetError() && strlen(SDLNet_GetError())) {
						printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
					}
				} else if(bsent == 0) {
					// NOTE: connection closed
					printf("recv-> connection closed\n");
					SDLNet_TCP_Close(sock);
				} else {
					// NOTE: print out the data
					printf("recv-> %d\n", (int) *data);
				}
			} else SDLNet_TCP_Close(sock);
			SDLNet_TCP_Close(sock);
			*/
		}
	}

	// NOTE: close the socket file descriptor
	SDLNet_UDP_Close(server);

	libQuit();

	return 0;
}

//-----------------------------------------------------------------------------
void libInit(void) {
	// NOTE: initialize the libraries
	if(SDL_Init(SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	if(SDLNet_Init() == -1) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
void libQuit(void) {
	// NOTE: release the libraries
	SDLNet_Quit();
	SDL_Quit();
}
