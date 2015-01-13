#ifndef __TESTINGLEVEL_H_
#define __TESTINGLEVEL_H_

//-----------------------------------------------------------------------------
class TestingLevel: public Level {
private:
	SDL_Surface *wallSprite00;
	SDL_Surface *wallSprite01;
	SDL_Surface *wallSprite02;
	SDL_Surface *doorSprite00;
	SDL_Surface *doorSprite01;
	SDL_Surface *floorSprite00;
	SDL_Surface *attackSprite00;
	SDL_Surface *attackSprite01;
	SDL_Surface *attackSprite02;
	SDL_Surface *diamondSprite00;
	uint8_t tiles[12][30] = {
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x01,0x01,0x01,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
		{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01}
	};

	int numOfHits;
	int displayAttackI;
	int displayAttackJ;
	int displayAttackFrame;
	SDL_bool displayAttack00;

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

	int doorSprite00Inds[2*3] = {
		29, 30,
		61, 62,
		93, 94
	};

	SDL_Color doorSprite00Color = {0xFF,0xFF,0xFF,0xFF};
	doorSprite00 = buildSprite(2, 3, doorSprite00Color, doorSprite00Inds);

	int doorSprite01Inds[2*3] = {
		31, 32,
		63, 64,
		95, 96
	};

	SDL_Color doorSprite01Color = {0xFF,0xFF,0xFF,0xFF};
	doorSprite01 = buildSprite(2, 3, doorSprite01Color, doorSprite01Inds);

	int wallSprite01Inds[2*3] = {
		 97,  98,
		129, 130,
		161, 162
	};

	SDL_Color wall01Color = {0xFF,0xFF,0xFF,0xFF};
	wallSprite01 = buildSprite(2, 3, wall01Color, wallSprite01Inds);

	int attackSprite00Inds[2*3] = {
		 99, 100,
		131, 132,
		163, 164
	};

	SDL_Color attackSprite00Color = {0x77,0x11,0x11,0xFF};
	attackSprite00 = buildSprite(2, 3, attackSprite00Color, attackSprite00Inds);

	int attackSprite01Inds[2*3] = {
		101, 102,
		133, 134,
		165, 166
	};

	SDL_Color attackSprite01Color = {0x77,0x11,0x11,0xFF};
	attackSprite01 = buildSprite(2, 3, attackSprite01Color, attackSprite01Inds);

	int attackSprite02Inds[2*3] = {
		103, 104,
		135, 136,
		167, 168
	};

	SDL_Color attackSprite02Color = {0x77,0x11,0x11,0xFF};
	attackSprite02 = buildSprite(2, 3, attackSprite02Color, attackSprite02Inds);

	int wallSprite02Inds[2*3] = {
		105, 106,
		137, 138,
		169, 170
	};

	SDL_Color wallSprite02Color = {0xFF,0xFF,0xFF,0xFF};
	wallSprite02 = buildSprite(2, 3, wallSprite02Color, wallSprite02Inds);
}

TestingLevel::~TestingLevel(void) {
	SDL_FreeSurface(wallSprite00);
	wallSprite00 = NULL;

	SDL_FreeSurface(floorSprite00);
	floorSprite00 = NULL;

	SDL_FreeSurface(diamondSprite00);
	diamondSprite00 = NULL;

	SDL_FreeSurface(doorSprite00);
	doorSprite00 = NULL;

	SDL_FreeSurface(doorSprite01);
	doorSprite01 = NULL;

	SDL_FreeSurface(wallSprite01);
	wallSprite01 = NULL;

	SDL_FreeSurface(attackSprite00);
	attackSprite00 = NULL;

	SDL_FreeSurface(attackSprite01);
	attackSprite01 = NULL;

	SDL_FreeSurface(attackSprite02);
	attackSprite02 = NULL;

	SDL_FreeSurface(wallSprite02);
	wallSprite02 = NULL;
}

void TestingLevel::update(void) {
	updateMoveState(&Game.player.moveState);

	int i = Game.player.moveState.i;
	int j = Game.player.moveState.j-3;

	if(displayAttackFrame>-12) displayAttackFrame--;
	else if(displayAttack00) {
		displayAttack00 = SDL_FALSE;
	}

	if(!Game.player.moveState.moving) {
		// NOTE: player movement up
		if((Game.player.input.up_arw&&!Game.player.input.up_chk) && !tiles[j-1][i]) {
			Game.player.input.up_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_H*3;
			Game.player.moveState.movedirec = 0;

		} else if((Game.player.input.up_arw&&!Game.player.input.up_chk) && tiles[j-1][i]==0x03 && !displayAttack00) {

			displayAttackI = i;
			displayAttackJ = j-1 + 3;
			displayAttack00 = SDL_TRUE;
			displayAttackFrame = 12;

			numOfHits++;
			if(numOfHits>2) {
				tiles[displayAttackJ-3][displayAttackI] = 0x04;
			}

		} else if(!Game.player.input.up_arw) {
			Game.player.input.up_chk = SDL_FALSE;
		}

		// NOTE: player movement down
		if((Game.player.input.down_arw&&!Game.player.input.down_chk) && !tiles[j+1][i]) {
			Game.player.input.down_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_H*3;
			Game.player.moveState.movedirec = 1;

		} else if((Game.player.input.down_arw&&!Game.player.input.down_chk) && tiles[j+1][i]==0x03 && !displayAttack00) {

			displayAttackI = i;
			displayAttackJ = j+1 + 3;
			displayAttack00 = SDL_TRUE;
			displayAttackFrame = 12;

			numOfHits++;
			if(numOfHits>2) {
				tiles[displayAttackJ-3][displayAttackI] = 0x04;
			}

		} else if(!Game.player.input.down_arw) {
			Game.player.input.down_chk = SDL_FALSE;
		}

		// NOTE: player movement left
		if((Game.player.input.left_arw&&!Game.player.input.left_chk) && !tiles[j][i-1]) {
			Game.player.input.left_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_W*2;
			Game.player.moveState.movedirec = 2;

		} else if((Game.player.input.left_arw&&!Game.player.input.left_chk) && tiles[j][i-1]==0x03 && !displayAttack00) {

			displayAttackI = i-1;
			displayAttackJ = j + 3;
			displayAttack00 = SDL_TRUE;
			displayAttackFrame = 12;

			numOfHits++;
			if(numOfHits>2) {
				tiles[displayAttackJ-3][displayAttackI] = 0x04;
			}

		} else if(!Game.player.input.left_arw) {
			Game.player.input.left_chk = SDL_FALSE;
		}

		// NOTE: player movement right
		if((Game.player.input.right_arw&&!Game.player.input.right_chk) && !tiles[j][i+1]) {
			Game.player.input.right_chk = SDL_TRUE;

			Game.player.moveState.moving = SDL_TRUE;
			Game.player.moveState.moveframe = SPRITE_W*2;
			Game.player.moveState.movedirec = 3;

		} else if((Game.player.input.right_arw&&!Game.player.input.right_chk) && tiles[j][i+1]==0x03 && !displayAttack00) {

			displayAttackI = i+1;
			displayAttackJ = j + 3;
			displayAttack00 = SDL_TRUE;
			displayAttackFrame = 12;

			numOfHits++;
			if(numOfHits>2) {
				tiles[displayAttackJ-3][displayAttackI] = 0x04;
			}

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
				case 0x02: {
					SDL_BlitSurface(doorSprite00, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x03: {
					SDL_BlitSurface(wallSprite01, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x04: {
					SDL_BlitSurface(wallSprite02, NULL, Game.gfx.screen, &tempRect);
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
	/*
	tempRect.x = SPRITE_W*2*9;
	tempRect.y = SPRITE_H*3*9;

	SDL_BlitSurface(diamondSprite00, NULL, Game.gfx.screen, &tempRect);
	*/

	if(displayAttack00) {
		tempRect.x = SPRITE_W*2*displayAttackI;
		tempRect.y = SPRITE_H*3*displayAttackJ;

		if(displayAttackFrame>8)
			SDL_BlitSurface(attackSprite00, NULL, Game.gfx.screen, &tempRect);
		else if(displayAttackFrame>4)
			SDL_BlitSurface(attackSprite01, NULL, Game.gfx.screen, &tempRect);
		else if(displayAttackFrame>0)
			SDL_BlitSurface(attackSprite02, NULL, Game.gfx.screen, &tempRect);
	}
}

#endif
