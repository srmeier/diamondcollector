#ifndef __VIEW_H_
#define __VIEW_H_

#include "math.h"

//-----------------------------------------------------------------------------
#define SPRITE_W 8
#define SPRITE_H 8
#define SCREEN_W 480
#define SCREEN_H 288//360
#define NUM_SPRITES 960
#define SCREEN_NAME "Level Editor"
#define SCREEN_SCALE 2

//-----------------------------------------------------------------------------
TTF_Font *font;
SDL_bool running;
SDL_Window *window;
SDL_Surface *screen;
SDL_Texture *texture;
SDL_Renderer *renderer;
SDL_Surface *sprites[NUM_SPRITES];

//-----------------------------------------------------------------------------
void start(void) {
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

	// NOTE: user screen for holding pixel data
	screen = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 32, 0x00, 0x00, 0x00, 0x00);

	// NOTE: create a texture for our renderer
	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		screen->w,
		screen->h
	);

	// NOTE: init the game sprites and text font
	font = TTF_OpenFont("SDS_8x8.ttf", 8);
	SDL_Surface *tempSurface = SDL_LoadBMP("spritesheet.bmp");

	// NOTE: set the sprites from the spritesheet
	int i, x, y;
	SDL_Rect tempRect = {0, 0, SPRITE_W, SPRITE_H};
	for(i=0; i<NUM_SPRITES; i++) {
		sprites[i] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
		if(i!=0) {
			x = (i-1)%(tempSurface->w/SPRITE_W);
			y = (i-x)/(tempSurface->w/SPRITE_W);
			tempRect.x = SPRITE_W*x, tempRect.y = SPRITE_H*y;
			SDL_BlitSurface(tempSurface, &tempRect, sprites[i], NULL);
		}
	}

	// NOTE: free the temporary surface
	SDL_FreeSurface(tempSurface);
	tempSurface = NULL;

	// NOTE: start running the game
	running = SDL_TRUE;
}

//-----------------------------------------------------------------------------
void quit(void) {
	// NOTE: free the font resource and spritesheet
	int i;
	for(i=0; i<NUM_SPRITES; i++)
		SDL_FreeSurface(sprites[i]);

	// NOTE: free game font
	TTF_CloseFont(font);

	// NOTE: destroy the game texture
	SDL_DestroyTexture(texture);

	// NOTE: free the GFX resources screen
	SDL_FreeSurface(screen);

	// NOTE: free the GFX resources renderer
	SDL_DestroyRenderer(renderer);

	// NOTE: free the GFX resources window
	SDL_DestroyWindow(window);

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
			uint8_t *srcPixels = (uint8_t *)sprites[inds[w*j+i]]->pixels;

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

	SDL_Surface *text = TTF_RenderText_Solid(font, str, color);

	// NOTE: blit the text surface to the screen
	SDL_Rect rect = {x, y, text->w, text->h};
	SDL_BlitSurface(text, NULL, screen, &rect);

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
				running = SDL_FALSE;
			} break;
			// NOTE: set key down state
			case SDL_KEYDOWN: {
				switch(event.key.keysym.sym) {
					case SDLK_ESCAPE: running = SDL_FALSE; break;
				}
			} break;
		}
	}
}

#endif
