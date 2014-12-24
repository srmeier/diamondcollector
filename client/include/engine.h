#ifndef __ENGINE_H_
#define __ENGINE_H_

//-----------------------------------------------------------------------------
void libInit(void) {
	// NOTE: initialize SDL2
	if(SDL_Init(SDL_INIT_EVERYTHING)!=0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	// NOTE: initialize SDLNet
	if(SDLNet_Init()==-1) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}

	// NOTE: initialize SDL_TTF
	if(TTF_Init()==-1) {
		fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
		exit(-1);
	}

	// NOTE: start running the game
	running = SDL_TRUE;
}

//-----------------------------------------------------------------------------
void libQuit(void) {
	// NOTE: release SDL_TTF
	TTF_Quit();

	// NOTE: release SDLNet
	SDLNet_Quit();

	// NOTE: release SDL2
	SDL_Quit();
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

	// NOTE: setup the socketset
	socketSet = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(socketSet, clientFD);
}

//-----------------------------------------------------------------------------
void netQuit(void) {
	// NOTE: free the socketset
	SDLNet_FreeSocketSet(socketSet);

	// NOTE: unbind all ips on the server channel
	SDLNet_UDP_Unbind(clientFD, serverChannel);

	// NOTE: close UDP socket
	SDLNet_UDP_Close(clientFD);
}

//-----------------------------------------------------------------------------
void gfxInit(void) {
	// NOTE: initialize the GFX resources window
	window = SDL_CreateWindow(
		SCREEN_NAME,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_SCALE*SCREEN_W,
		SCREEN_SCALE*SCREEN_H,
		0
	);

	// NOTE: initialize the GFX resources renderer
	renderer = SDL_CreateRenderer(
		window, -1,
		SDL_RENDERER_ACCELERATED|
		SDL_RENDERER_PRESENTVSYNC
	);

	// NOTE: if Vsync didn't work then exit
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	if(!(info.flags & SDL_RENDERER_PRESENTVSYNC)) {
		fprintf(stderr, "SDL_CreateRenderer: failed to set Vsync.\n");

		netQuit();
		libQuit();

		exit(-1);
	}

	// NOTE: user screen for holding pixel data
	screen = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(screen, 1, 0xFF00FF);
	SDL_FillRect(screen, 0, 0xFF00FF);

	// NOTE: init the game sprites and text font
	font8 = TTF_OpenFont("SDS_8x8.ttf", 8);
	SDL_Surface *surface = SDL_LoadBMP("spritesheet.bmp");

	int i, x, y;
	SDL_Rect rect = {0, 0, 8, 8};
	for(i=0; i<NUM_SPRITES; i++) {
		spritesheet[i] = SDL_CreateRGBSurface(0, 8, 8, 24, 0x00, 0x00, 0x00, 0x00);
		SDL_SetColorKey(spritesheet[i], 1, 0xFF00FF);
		SDL_FillRect(spritesheet[i], 0, 0xFF00FF);
		if(i!=0) {
			x = (i-1)%(surface->w/8);
			y = (i-x)/(surface->w/8);
			rect.x = x*8, rect.y = y*8;
			SDL_BlitSurface(surface, &rect, spritesheet[i], NULL);
		}
	}

	SDL_FreeSurface(surface);
}

//-----------------------------------------------------------------------------
void gfxQuit(void) {
	// NOTE: free the GFX resources screen
	SDL_FreeSurface(screen);

	// NOTE: free the GFX resources renderer
	SDL_DestroyRenderer(renderer);

	// NOTE: free the GFX resources window
	SDL_DestroyWindow(window);

	// NOTE: free the font resource and spritesheet
	int i;
	for(i=0; i<NUM_SPRITES; i++)
		SDL_FreeSurface(spritesheet[i]);
	TTF_CloseFont(font8);
}

//-----------------------------------------------------------------------------
SDL_Surface* buildSprite(int w, int h, int inds[]) {
	// NOTE: build a sprite surface to be drawn later
	SDL_Surface *surface = SDL_CreateRGBSurface(0, 8*w, 8*h, 24, 0x00, 0x00, 0x00, 0x00);

	SDL_SetColorKey(surface, 1, 0xFF00FF);
	SDL_FillRect(surface, 0, 0xFF00FF);

	int i, j;
	SDL_Rect rect = {0, 0, 8, 8};
	for(j=0; j<h; j++) {
		for(i=0; i<w; i++) {
			rect.x = i*8, rect.y = j*8;
			SDL_BlitSurface(spritesheet[inds[w*j+i]], NULL, surface, &rect);
		}
	}

	return surface;
}

//-----------------------------------------------------------------------------
void drawText(const char *str, SDL_Color color, int x, int y) {
	// NOTE: draw text to the screen
	if(!str) return;

	SDL_Surface *text = TTF_RenderText_Solid(font8, str, color);

	SDL_Rect rect = {x, y, text->w, text->h};
	SDL_BlitSurface(text, NULL, screen, &rect);

	SDL_FreeSurface(text);
}

//-----------------------------------------------------------------------------
void inputPoll(void) {
	// NOTE: poll for events and set the global state
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT: {
				gameState = 0xFF;
			} break;
			case SDL_KEYDOWN: {
				switch(event.key.keysym.sym) {
					case SDLK_ESCAPE: gameState = 0xFF; break;
					case SDLK_UP: upBnt = SDL_TRUE; break;
					case SDLK_DOWN: downBnt = SDL_TRUE; break;
					case SDLK_LEFT: leftBnt = SDL_TRUE; break;
					case SDLK_RIGHT: rightBnt = SDL_TRUE; break;
					case SDLK_LCTRL: aBnt = SDL_TRUE; break;
					case SDLK_LALT: bBnt = SDL_TRUE; break;
				}
			} break;
			case SDL_KEYUP: {
				switch(event.key.keysym.sym) {
					case SDLK_UP: upBnt = SDL_FALSE; break;
					case SDLK_DOWN: downBnt = SDL_FALSE; break;
					case SDLK_LEFT: leftBnt = SDL_FALSE; break;
					case SDLK_RIGHT: rightBnt = SDL_FALSE; break;
					case SDLK_LCTRL: aBnt = SDL_FALSE; break;
					case SDLK_LALT: bBnt = SDL_FALSE; break;
				}
			} break;
		}
	}
}

//-----------------------------------------------------------------------------
void clearInput(void) {
	aChk = SDL_FALSE;
	bChk = SDL_FALSE;
	upChk = SDL_FALSE;
	downChk = SDL_FALSE;
	leftChk = SDL_FALSE;
	rightChk = SDL_FALSE;

	aBnt = SDL_FALSE;
	bBnt = SDL_FALSE;
	upBnt = SDL_FALSE;
	downBnt = SDL_FALSE;
	leftBnt = SDL_FALSE;
	rightBnt = SDL_FALSE;
}

#endif
