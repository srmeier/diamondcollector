/*
g++ main.cpp -std=c++11 -o client.exe -I./include -L./lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_net
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

//-----------------------------------------------------------------------------
#include "engine.h"
#include "levels/testinglevel.h"
#include "userinterface/topscreengui.h"

/*
- b/a+arrow key = continuously move in that direction
- add the defense "meter"
*/

//-----------------------------------------------------------------------------
int SDL_main(int argc, char *argv[]) {

	startGame();

	/* === */

	TopScreenGUI *topScreenGUI = NULL;

	/* === */

	while(Game.running) {
		/* === */

		pollInput();

		SDL_RenderClear(Game.renderer);
		SDL_FillRect(Game.gfx.screen, 0, 0x00);

		/* === */

		switch(Game.state) {
			case INITIALIZE: {
				/* === */

				// NOTE: create top-screen GUI
				topScreenGUI = new TopScreenGUI();

				// NOTE: create player sprite
				int playerSpriteInds[2*3] = {
					 1,  2,
					33, 34,
					65, 66
				};

				SDL_Color playerColor = {0xFF,0xFF,0xFF,0xFF};
				Game.player.sprite = buildSprite(2, 3, playerColor, playerSpriteInds);

				// NOTE: create player's shadow
				int playerShadowInds[2*3] = {
					19, 20,
					51, 52,
					83, 84
				};

				SDL_Color playerShadowColor = {0x00,0x00,0x00,0xFF};
				Game.player.shadow = buildSprite(2, 3, playerShadowColor, playerShadowInds);

				// NOTE: set the player information
				Game.player.mana = 100;
				Game.player.health = 100;
				Game.player.totMana = 100;
				Game.player.totHealth = 100;

				// NOTE: set the player's position
				Game.player.moveState.x = SPRITE_W*2*6;
				Game.player.moveState.y = SPRITE_H*3*6;

				// NOTE: create a testing level
				Game.level = new TestingLevel();

				// NOTE: move to the main loop
				Game.state = MAIN_LOOP;

				/* === */
			} break;
			case MAIN_LOOP: {
				/* === */

				// NOTE: update and draw the level
				Game.level->update();
				Game.level->render();

				// NOTE: draw the top-screen GUI
				topScreenGUI->render();

				/* === */
			} break;
			case QUIT: {
				/* === */

				// NOTE: free the current level
				delete Game.level;
				Game.level = NULL;

				// NOTE: free the player's sprite
				SDL_FreeSurface(Game.player.sprite);
				Game.player.sprite = NULL;

				// NOTE: free the player's shadow
				SDL_FreeSurface(Game.player.shadow);
				Game.player.shadow = NULL;

				// NOTE: free the top-screen GUI
				delete topScreenGUI;
				topScreenGUI = NULL;

				// NOTE: stop running the game
				Game.running = SDL_FALSE;

				/* === */
			} break;
		}

		/* === */

		int pitch;
		void *pixels;

		// NOTE: get the pixels for the screen texture
		SDL_LockTexture(Game.gfx.texture, NULL, &pixels, &pitch);

		// NOTE: set the pixels for the screen texture
		SDL_ConvertPixels(
			Game.gfx.screen->w,
			Game.gfx.screen->h,
			Game.gfx.screen->format->format,
			Game.gfx.screen->pixels,
			Game.gfx.screen->pitch,
			SDL_PIXELFORMAT_RGBA8888,
			pixels, pitch
		);

		// NOTE: lock the texture so that it may be presented
		SDL_UnlockTexture(Game.gfx.texture);

		// NOTE: present the texture
		SDL_RenderCopy(Game.renderer, Game.gfx.texture, NULL, NULL);
		SDL_RenderPresent(Game.renderer);

		/* === */
	}

	quitGame();

	return 0;
}
