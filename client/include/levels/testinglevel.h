#ifndef __TESTINGLEVEL_H_
#define __TESTINGLEVEL_H_

//-----------------------------------------------------------------------------
class TestingLevel: public Level {
private:
	SDL_Surface *wallSprite00;
	SDL_Surface *floorSprite00;
	SDL_Surface *diamondSprite00;
	uint8_t tiles[12][30] = {
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01}
	};

public:
	TestingLevel(void);
	~TestingLevel(void);

	void update(void);
	void render(void);
};

TestingLevel::TestingLevel(void) {
	int wallSprite00Inds[2*3] = {
		03, 04,
		35, 36,
		67, 68
	};

	SDL_Color wall00Color = {0xFF,0xFF,0xFF,0xFF};
	wallSprite00 = buildSprite(2, 3, wall00Color, wallSprite00Inds);

	int floorSprite00Inds[2*3] = {
		17, 18,
		49, 50,
		81, 82
	};

	SDL_Color floorSprite00Color = {0x33,0x33,0x33,0xFF};
	floorSprite00 = buildSprite(2, 3, floorSprite00Color, floorSprite00Inds);

	int diamondSprite00Inds[2*3] = {
		27, 28,
		59, 60,
		91, 92
	};

	SDL_Color diamondSprite00Color = {0xFF,0xD8,0x00,0xFF};
	diamondSprite00 = buildSprite(2, 3, diamondSprite00Color, diamondSprite00Inds);
}

TestingLevel::~TestingLevel(void) {
	SDL_FreeSurface(wallSprite00);
	wallSprite00 = NULL;

	SDL_FreeSurface(floorSprite00);
	floorSprite00 = NULL;

	SDL_FreeSurface(diamondSprite00);
	diamondSprite00 = NULL;
}

void TestingLevel::update(void) {
	updateMoveState(&Game.player.moveState);

	int i = Game.player.moveState.i;
	int j = Game.player.moveState.j-3;

	if(!Game.player.moveState.moving) {
		// NOTE: player movement up
		if((Game.player.input.up_arw&&!Game.player.input.up_chk) && !tiles[j-1][i]) {
			Game.player.input.up_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_H*3;
			Game.player.moveState.movedirec = 0;

		} else if(!Game.player.input.up_arw) {
			Game.player.input.up_chk = SDL_FALSE;
		}

		// NOTE: player movement down
		if((Game.player.input.down_arw&&!Game.player.input.down_chk) && !tiles[j+1][i]) {
			Game.player.input.down_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_H*3;
			Game.player.moveState.movedirec = 1;

		} else if(!Game.player.input.down_arw) {
			Game.player.input.down_chk = SDL_FALSE;
		}

		// NOTE: player movement left
		if((Game.player.input.left_arw&&!Game.player.input.left_chk) && !tiles[j][i-1]) {
			Game.player.input.left_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_W*2;
			Game.player.moveState.movedirec = 2;

		} else if(!Game.player.input.left_arw) {
			Game.player.input.left_chk = SDL_FALSE;
		}

		// NOTE: player movement right
		if((Game.player.input.right_arw&&!Game.player.input.right_chk) && !tiles[j][i+1]) {
			Game.player.input.right_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_W*2;
			Game.player.moveState.movedirec = 3;

		} else if(!Game.player.input.right_arw) {
			Game.player.input.right_chk = SDL_FALSE;
		}
	}
}

void TestingLevel::render(void) {
	SDL_Rect tempRect;

	tempRect.w = SPRITE_W*2;
	tempRect.h = SPRITE_H*3;

	int i, j;
	for(j=3; j<15; j++) {
		for(i=0; i<30; i++) {

			tempRect.x = SPRITE_W*2*i;
			tempRect.y = SPRITE_H*3*j;

			switch(tiles[j-3][i]) {
				case 0x01: {
					SDL_BlitSurface(wallSprite00, NULL, Game.gfx.screen, &tempRect);
				} break;
				default: {
					SDL_BlitSurface(floorSprite00, NULL, Game.gfx.screen, &tempRect);
				} break;
			}
		}
	}

	tempRect.x = Game.player.moveState.x;
	tempRect.y = Game.player.moveState.y;

	SDL_BlitSurface(Game.player.shadow, NULL, Game.gfx.screen, &tempRect);
	SDL_BlitSurface(Game.player.sprite, NULL, Game.gfx.screen, &tempRect);

	// NOTE: add a diamond
	tempRect.x = SPRITE_W*2*9;
	tempRect.y = SPRITE_H*3*9;

	SDL_BlitSurface(diamondSprite00, NULL, Game.gfx.screen, &tempRect);
}

#endif
