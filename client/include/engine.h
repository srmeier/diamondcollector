#ifndef __ENGINE_H_
#define __ENGINE_H_

//-----------------------------------------------------------------------------
#define SPRITE_W 8
#define SPRITE_H 8
#define SCREEN_W 480
#define SCREEN_H 360
#define NUM_SPRITES 960
#define SCREEN_NAME "Client"
#define SCREEN_SCALE 2

//-----------------------------------------------------------------------------
typedef enum {
	INITIALIZE,
	MAIN_LOOP,
	QUIT = 0xFF
} GAME_STATE;

//-----------------------------------------------------------------------------
typedef struct {
	SDL_bool a_chk;
	SDL_bool a_bnt;
	SDL_bool b_chk;
	SDL_bool b_bnt;
	SDL_bool up_chk;
	SDL_bool up_arw;
	SDL_bool down_chk;
	SDL_bool down_arw;
	SDL_bool left_chk;
	SDL_bool left_arw;
	SDL_bool right_chk;
	SDL_bool right_arw;
} Input;

typedef struct {
	int x, y;
	int i, j;
	int moveframe;
	int movedirec;
	SDL_bool moving;
} MoveState;

//-----------------------------------------------------------------------------
class Level {
public:
	virtual void update(void) = 0;
	virtual void render(void) = 0;
};

class UserInterface {
public:
	virtual void update(void) = 0;
	virtual void render(void) = 0;
};

//-----------------------------------------------------------------------------
struct {
	Level *level;
	SDL_bool running;
	GAME_STATE state;
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct {
		Input input;
		MoveState moveState;
		SDL_Surface *sprite;
		SDL_Surface *shadow;
	} player;
	struct {
		TTF_Font *font;
		SDL_Surface *screen;
		SDL_Texture *texture;
		SDL_Surface *sprites[NUM_SPRITES];
	} gfx;
	struct {
		UDPsocket socket;
		SDLNet_SocketSet set;
		struct {
			int channel;
			IPaddress ip;
		} server;
	} net;
} Game = {};

//-----------------------------------------------------------------------------
void startGame(void) {
	// NOTE: initialize SDL2
	if(SDL_Init(SDL_INIT_EVERYTHING)!=0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	// NOTE: initialize SDL_TTF
	if(TTF_Init()==-1) {
		fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
		exit(-1);
	}

	// NOTE: initialize the GFX resources window
	Game.window = SDL_CreateWindow(
		SCREEN_NAME,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_SCALE*SCREEN_W,
		SCREEN_SCALE*SCREEN_H,
		0
	);

	// NOTE: initialize the GFX resources renderer
	Game.renderer = SDL_CreateRenderer(
		Game.window, -1,
		SDL_RENDERER_ACCELERATED|
		SDL_RENDERER_PRESENTVSYNC
	);

	// NOTE: temporary renderer information struct
	SDL_RendererInfo tempRenInfo;

	// NOTE: if Vsync didn't work then exit
	SDL_GetRendererInfo(Game.renderer, &tempRenInfo);
	if(!(tempRenInfo.flags & SDL_RENDERER_PRESENTVSYNC)) {
		fprintf(stderr, "SDL_CreateRenderer: failed to set Vsync.\n");
		exit(-1);
	}

	// NOTE: user screen for holding pixel data
	Game.gfx.screen = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 32, 0x00, 0x00, 0x00, 0x00);

	// NOTE: create a texture for our renderer
	Game.gfx.texture = SDL_CreateTexture(
		Game.renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		Game.gfx.screen->w,
		Game.gfx.screen->h
	);

	// NOTE: init the game sprites and text font
	Game.gfx.font = TTF_OpenFont("SDS_8x8.ttf", 8);
	SDL_Surface *tempSurface = SDL_LoadBMP("spritesheet.bmp");

	// NOTE: set the sprites from the spritesheet
	int i, x, y;
	SDL_Rect tempRect = {0, 0, SPRITE_W, SPRITE_H};
	for(i=0; i<NUM_SPRITES; i++) {
		Game.gfx.sprites[i] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
		if(i!=0) {
			x = (i-1)%(tempSurface->w/SPRITE_W);
			y = (i-x)/(tempSurface->w/SPRITE_W);
			tempRect.x = SPRITE_W*x, tempRect.y = SPRITE_H*y;
			SDL_BlitSurface(tempSurface, &tempRect, Game.gfx.sprites[i], NULL);
		}
	}

	// NOTE: free the temporary surface
	SDL_FreeSurface(tempSurface);
	tempSurface = NULL;

	// NOTE: initialize SDLNet
	if(SDLNet_Init()==-1) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}

	/*
	// NOTE: resolve the host IP struct
	if(SDLNet_ResolveHost(&Game.net.server.ip, "www.libgcw.com", 3490) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(-1);
	}

	// NOTE: open UDP socket file descriptor
	Game.net.socket = SDLNet_UDP_Open(0);
	if(!Game.net.socket) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		exit(-1);
	}

	// NOTE: bind the server ip address to a channel on the client socket
	Game.net.server.channel = SDLNet_UDP_Bind(Game.net.socket, -1, &Game.net.server.ip);
	if(Game.net.server.channel==-1) {
		fprintf(stderr, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		exit(-1);
	}

	// NOTE: setup the socketset
	Game.net.set = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(Game.net.set, Game.net.socket);
	*/

	// NOTE: start running the game
	Game.running = SDL_TRUE;
}

//-----------------------------------------------------------------------------
void quitGame(void) {
	/*
	// NOTE: free the socketset
	SDLNet_FreeSocketSet(Game.net.set);

	// NOTE: unbind all ips on the server channel
	SDLNet_UDP_Unbind(Game.net.socket, Game.net.server.channel);

	// NOTE: close UDP socket
	SDLNet_UDP_Close(Game.net.socket);
	*/

	// NOTE: free the font resource and spritesheet
	int i;
	for(i=0; i<NUM_SPRITES; i++)
		SDL_FreeSurface(Game.gfx.sprites[i]);

	// NOTE: free game font
	TTF_CloseFont(Game.gfx.font);

	// NOTE: destroy the game texture
	SDL_DestroyTexture(Game.gfx.texture);

	// NOTE: free the GFX resources screen
	SDL_FreeSurface(Game.gfx.screen);

	// NOTE: free the GFX resources renderer
	SDL_DestroyRenderer(Game.renderer);

	// NOTE: free the GFX resources window
	SDL_DestroyWindow(Game.window);

	// NOTE: release SDLNet
	SDLNet_Quit();

	// NOTE: release SDL_TTF
	TTF_Quit();

	// NOTE: release SDL2
	SDL_Quit();
}

//-----------------------------------------------------------------------------
SDL_Surface* buildSprite(int w, int h, SDL_Color color, int inds[]) {
	// NOTE: build a sprite surface to be drawn later
	SDL_Surface *spr = SDL_CreateRGBSurface(
		0,
		SPRITE_W*w,
		SPRITE_H*h,
		8, 0x00, 0x00, 0x00, 0x00
	);

	SDL_SetPaletteColors(spr->format->palette, &color, 1, 1);
	SDL_SetColorKey(spr, SDL_TRUE, 0x00);

	int i, j;
	SDL_Rect rect = {0, 0, SPRITE_W, SPRITE_H};
	for(j=0; j<h; j++) {
		for(i=0; i<w; i++) {
			rect.x = SPRITE_W*i, rect.y = SPRITE_H*j;

			// NOTE: get the pixel pointers
			uint8_t *desPixels = (uint8_t *)spr->pixels;
			uint8_t *srcPixels = (uint8_t *)Game.gfx.sprites[inds[w*j+i]]->pixels;

			SDL_LockSurface(spr);

			int m, k;
			for(k=0; k<rect.h; k++) {
				for(m=0; m<rect.w; m++) {
					// NOTE: the blue value in the spritesheet is responsible
					// for setting the pallette color
					desPixels[(SPRITE_W*w)*(rect.y+k)+(rect.x+m)] = srcPixels[3*(rect.w*k+m)];
				}
			}

			SDL_UnlockSurface(spr);
		}
	}

	return spr;
}

//-----------------------------------------------------------------------------
void drawText(int x, int y, SDL_Color color, const char *str) {
	// NOTE: draw text to the screen
	if(!str) return;

	SDL_Surface *text = TTF_RenderText_Solid(Game.gfx.font, str, color);

	// NOTE: blit the text surface to the screen
	SDL_Rect rect = {x, y, text->w, text->h};
	SDL_BlitSurface(text, NULL, Game.gfx.screen, &rect);

	SDL_FreeSurface(text);
}

//-----------------------------------------------------------------------------
void pollInput(void) {
	// NOTE: poll for events and set the global state
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			// NOTE: set quit state
			case SDL_QUIT: {
				Game.state = QUIT;
			} break;
			// NOTE: set key down state
			case SDL_KEYDOWN: {
				switch(event.key.keysym.sym) {
					case SDLK_ESCAPE: Game.state = QUIT; break;

					// NOTE: input arrow keys
					case SDLK_UP: Game.player.input.up_arw = SDL_TRUE; break;
					case SDLK_DOWN: Game.player.input.down_arw = SDL_TRUE; break;
					case SDLK_LEFT: Game.player.input.left_arw = SDL_TRUE; break;
					case SDLK_RIGHT: Game.player.input.right_arw = SDL_TRUE; break;

					// NOTE: input button keys
					case SDLK_LALT: Game.player.input.b_bnt = SDL_TRUE; break;
					case SDLK_LCTRL: Game.player.input.a_bnt = SDL_TRUE; break;
				}
			} break;
			// NOTE: set key up state
			case SDL_KEYUP: {
				switch(event.key.keysym.sym) {
					// NOTE: input arrow keys
					case SDLK_UP: Game.player.input.up_arw = SDL_FALSE; break;
					case SDLK_DOWN: Game.player.input.down_arw = SDL_FALSE; break;
					case SDLK_LEFT: Game.player.input.left_arw = SDL_FALSE; break;
					case SDLK_RIGHT: Game.player.input.right_arw = SDL_FALSE; break;

					// NOTE: input button keys
					case SDLK_LALT: Game.player.input.b_bnt = SDL_FALSE; break;
					case SDLK_LCTRL: Game.player.input.a_bnt = SDL_FALSE; break;
				}
			} break;
		}
	}
}

//-----------------------------------------------------------------------------
void updateMoveState(MoveState *state) {
	// NOTE: set the state coordinates
	state->i = floor(state->x/(float) (SPRITE_W*2));
	state->j = floor(state->y/(float) (SPRITE_H*3));

	// NOTE: update the movement frame
	if(state->moveframe>0) state->moveframe--;
	else if(state->moving) {
		state->moving = SDL_FALSE;
	}

	// NOTE: move the state forward in the right direction
	if(state->moving) {
		switch(state->movedirec) {
			case 0: state->y--; break;
			case 1: state->y++; break;
			case 2: state->x--; break;
			case 3: state->x++; break;
		}
	}
}

//-----------------------------------------------------------------------------
/*
void clearInput(Input *input) {
	memset(input, 0x00, sizeof(Input));
}
*/

//-----------------------------------------------------------------------------
/*
void changeColor(SDL_Surface *spr, SDL_Color color) {
	SDL_SetPaletteColors(spr->format->palette, &color, 1, 1);
}
*/

#endif
