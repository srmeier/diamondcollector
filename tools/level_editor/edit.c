/*
gcc edit.c -o edit.exe -I./include -L./lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lsqlite3
*/

//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sqlite3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "engine.h"

//-----------------------------------------------------------------------------
typedef struct {
	int tiles[12][30];
	int actions[12][30];
} Level;

Level lvl;
sqlite3 *db;

//-----------------------------------------------------------------------------
void loadTiles(void);
int loadTilesCB(void *data, int argc, char *argv[], char *name[]);

//-----------------------------------------------------------------------------
int SDL_main(int argc, char *argv[]) {
	// NOTE: check for the correct number of arguements
	if(argc<2) {
		fprintf(stderr, "Error: need a level name.\n");
		return -1;
	}

	// NOTE: add the sqlite db to the levels directory
	char filename[0xFF];
	sprintf(filename, "levels/%s.db", argv[1]);

	// NOTE: open database connection
	if(sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	/* === */

	start();

	/* === */

	int wallSprite00Inds[2*3] = {
		03, 04,
		35, 36,
		67, 68
	};

	SDL_Color wall00Color = {0xFF,0xFF,0xFF,0xFF};
	SDL_Surface *wallSprite00 = buildSprite(2, 3, wall00Color, wallSprite00Inds);

	int floorSprite00Inds[2*3] = {
		17, 18,
		49, 50,
		81, 82
	};

	SDL_Color floorSprite00Color = {0x33,0x33,0x33,0xFF};
	SDL_Surface *floorSprite00 = buildSprite(2, 3, floorSprite00Color, floorSprite00Inds);

	int doorSprite00Inds[2*3] = {
		29, 30,
		61, 62,
		93, 94
	};

	SDL_Color doorSprite00Color = {0xFF,0xFF,0xFF,0xFF};
	SDL_Surface *doorSprite00 = buildSprite(2, 3, doorSprite00Color, doorSprite00Inds);

	int wallSprite01Inds[2*3] = {
		 97,  98,
		129, 130,
		161, 162
	};

	SDL_Color wall01Color = {0xFF,0xFF,0xFF,0xFF};
	SDL_Surface *wallSprite01 = buildSprite(2, 3, wall01Color, wallSprite01Inds);

	int wallSprite02Inds[2*3] = {
		105, 106,
		137, 138,
		169, 170
	};

	SDL_Color wallSprite02Color = {0xFF,0xFF,0xFF,0xFF};
	SDL_Surface *wallSprite02 = buildSprite(2, 3, wallSprite02Color, wallSprite02Inds);

	/* === */

	while(running) {
		/* === */

		pollInput();

		SDL_RenderClear(renderer);
		SDL_FillRect(screen, 0, 0x00);

		/* === */

		loadTiles();

		SDL_Rect tempRect = {
			0, 0, SPRITE_W*2, SPRITE_H*3
		};

		int i, j;
		for(j=0; j<12; j++) {
			for(i=0; i<30; i++) {
				tempRect.x = SPRITE_W*2*i;
				tempRect.y = SPRITE_H*3*j;

				switch(lvl.tiles[j][i]) {
					case 0x01: {
						SDL_BlitSurface(wallSprite00, NULL, screen, &tempRect);
					} break;
					case 0x02: {
						SDL_BlitSurface(doorSprite00, NULL, screen, &tempRect);
					} break;
					case 0x03: {
						SDL_BlitSurface(wallSprite01, NULL, screen, &tempRect);
					} break;
					case 0x04: {
						SDL_BlitSurface(wallSprite02, NULL, screen, &tempRect);
					} break;
					default: {
						SDL_BlitSurface(floorSprite00, NULL, screen, &tempRect);
					} break;
				}
			}
		}

		/* === */

		int pitch;
		void *pixels;

		// NOTE: get the pixels for the screen texture
		SDL_LockTexture(texture, NULL, &pixels, &pitch);

		// NOTE: set the pixels for the screen texture
		SDL_ConvertPixels(
			screen->w,
			screen->h,
			screen->format->format,
			screen->pixels,
			screen->pitch,
			SDL_PIXELFORMAT_RGBA8888,
			pixels, pitch
		);

		// NOTE: lock the texture so that it may be presented
		SDL_UnlockTexture(texture);

		// NOTE: present the texture
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Delay(10);

		/* === */
	}

	/* === */

	SDL_FreeSurface(wallSprite00);
	wallSprite00 = NULL;

	SDL_FreeSurface(floorSprite00);
	floorSprite00 = NULL;

	SDL_FreeSurface(doorSprite00);
	doorSprite00 = NULL;

	SDL_FreeSurface(wallSprite01);
	wallSprite01 = NULL;

	SDL_FreeSurface(wallSprite02);
	wallSprite02 = NULL;

	/* === */

	quit();

	/* === */

	// NTOE: close the database connection
	sqlite3_close(db);
	db = NULL;

	fclose(stderr);
	return 0;
}

//-----------------------------------------------------------------------------
int loadTilesCB(void *data, int argc, char *argv[], char *name[]) {
	// NTOE: collect the data from the database
	int x, y, layer, sprite, action;

	int i;
	for(i=0; i<argc; i++) {
		if(!strcmp(name[i], "x"))
			x = (uint32_t) atoi(argv[i]);
		if(!strcmp(name[i], "y"))
			y = (uint32_t) atoi(argv[i]);
		if(!strcmp(name[i], "layer"))
			layer = (uint32_t) atoi(argv[i]);
		if(!strcmp(name[i], "sprite"))
			sprite = (uint32_t) atoi(argv[i]);
		if(!strcmp(name[i], "action"))
			action = (uint32_t) atoi(argv[i]);
	}

	// NOTE: set the sprite and action enums
	lvl.tiles[y][x] = sprite;
	lvl.actions[y][x] = action;

	return 0;
}

//-----------------------------------------------------------------------------
void loadTiles(void) {
	// NOTE: select all the tiles for a level
	const char *cmd = "SELECT * FROM tiles;";

	// NOTE: execute the sql query
	char *errorMsg;
	if(sqlite3_exec(db, cmd, loadTilesCB, NULL, &errorMsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		exit(-1);
	}
}
