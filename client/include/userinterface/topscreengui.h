#ifndef __TOPSCREENGUI_H_
#define __TOPSCREENGUI_H_

//-----------------------------------------------------------------------------
class TopScreenGUI: public UserInterface {
private:
	SDL_Surface *topBotSide;
	SDL_Surface *leftRightSide;
	SDL_Surface *botLeftCorner;
	SDL_Surface *topLeftCorner;
	SDL_Surface *topRightCorner;
	SDL_Surface *botRightCorner;
	uint8_t tiles[3][30] = {
		{0x01,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x02},
		{0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06},
		{0x03,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x04}
	};

public:
	TopScreenGUI(void);
	~TopScreenGUI(void);

	void update(void);
	void render(void);
};

TopScreenGUI::TopScreenGUI(void) {
	int topLeftCornerInds[2*3] = {
		 5,  6,
		37, 38,
		69, 70
	};

	SDL_Color topLeftCornerColor = {0xFF,0xFF,0xFF,0xFF};
	topLeftCorner = buildSprite(2, 3, topLeftCornerColor, topLeftCornerInds);

	int topRightCornerInds[2*3] = {
		 7,  8,
		39, 40,
		71, 72
	};

	SDL_Color topRightCornerColor = {0xFF,0xFF,0xFF,0xFF};
	topRightCorner = buildSprite(2, 3, topRightCornerColor, topRightCornerInds);

	int botLeftCornerInds[2*3] = {
		 9, 10,
		41, 42,
		73, 74
	};

	SDL_Color botLeftCornerColor = {0xFF,0xFF,0xFF,0xFF};
	botLeftCorner = buildSprite(2, 3, botLeftCornerColor, botLeftCornerInds);

	int botRightCornerInds[2*3] = {
		11, 12,
		43, 44,
		75, 76
	};

	SDL_Color botRightCornerColor = {0xFF,0xFF,0xFF,0xFF};
	botRightCorner = buildSprite(2, 3, botRightCornerColor, botRightCornerInds);

	int topBotSideInds[2*3] = {
		13, 14,
		45, 46,
		77, 78
	};

	SDL_Color topBotSideColor = {0xFF,0xFF,0xFF,0xFF};
	topBotSide = buildSprite(2, 3, topBotSideColor, topBotSideInds);

	int leftRightSideInds[2*3] = {
		15, 16,
		47, 48,
		79, 80
	};

	SDL_Color leftRightSideColor = {0xFF,0xFF,0xFF,0xFF};
	leftRightSide = buildSprite(2, 3, leftRightSideColor, leftRightSideInds);
}

TopScreenGUI::~TopScreenGUI(void) {
	SDL_FreeSurface(topLeftCorner);
	topLeftCorner = NULL;

	SDL_FreeSurface(topRightCorner);
	topRightCorner = NULL;

	SDL_FreeSurface(botLeftCorner);
	botLeftCorner = NULL;

	SDL_FreeSurface(botRightCorner);
	botRightCorner = NULL;

	SDL_FreeSurface(topBotSide);
	topBotSide = NULL;

	SDL_FreeSurface(leftRightSide);
	leftRightSide = NULL;
}

void TopScreenGUI::update(void) {
	// NOTE: this GUI updates nothing
}

void TopScreenGUI::render(void) {
	SDL_Rect tempRect = {0, 0, SPRITE_W*2, SPRITE_H*3};

	int i, j;
	for(j=0; j<3; j++) {
		for(i=0; i<30; i++) {
			tempRect.x = SPRITE_W*2*i;
			tempRect.y = SPRITE_W*3*j;

			switch(tiles[j][i]) {
				case 0x01: {
					SDL_BlitSurface(topLeftCorner, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x02: {
					SDL_BlitSurface(topRightCorner, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x03: {
					SDL_BlitSurface(botLeftCorner, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x04: {
					SDL_BlitSurface(botRightCorner, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x05: {
					SDL_BlitSurface(topBotSide, NULL, Game.gfx.screen, &tempRect);
				} break;
				case 0x06: {
					SDL_BlitSurface(leftRightSide, NULL, Game.gfx.screen, &tempRect);
				} break;
			}
		}
	}

	SDL_Color textColor = {0xFF,0xFF,0xFF,0xFF};
	drawText(16, 24, textColor, "Testing 1 2 3");
}

#endif
