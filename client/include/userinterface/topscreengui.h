#ifndef __TOPSCREENGUI_H_
#define __TOPSCREENGUI_H_

//-----------------------------------------------------------------------------
class TopScreenGUI: public UserInterface {
private:
	SDL_Surface *topBotSide;
	SDL_Surface *leftRightSide;
	SDL_Surface *botLeftCorner;
	SDL_Surface *topLeftCorner;
	SDL_Surface *manaContainer;
	SDL_Surface *itemContainer;
	SDL_Surface *topRightCorner;
	SDL_Surface *botRightCorner;
	SDL_Surface *healthContainer;
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

	int healthContainerInds[2*3] = {
		21, 22,
		53, 54,
		85, 86
	};

	SDL_Color healthContainerColor = {0xFF,0x11,0x11,0xFF};
	healthContainer = buildSprite(2, 3, healthContainerColor, healthContainerInds);

	int manaContainerInds[2*3] = {
		23, 24,
		55, 56,
		87, 88
	};

	SDL_Color manaContainerColor = {0x11,0x11,0xFF,0xFF};
	manaContainer = buildSprite(2, 3, manaContainerColor, manaContainerInds);

	int itemContainerInds[2*3] = {
		25, 26,
		57, 58,
		89, 90
	};

	SDL_Color itemContainerColor = {0xFF,0xFF,0xFF,0xFF};
	itemContainer = buildSprite(2, 3, itemContainerColor, itemContainerInds);
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

	SDL_FreeSurface(healthContainer);
	healthContainer = NULL;

	SDL_FreeSurface(manaContainer);
	manaContainer = NULL;

	SDL_FreeSurface(itemContainer);
	itemContainer = NULL;
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

	char str[0xFF];
	tempRect.y = SPRITE_W*3*1;
	SDL_Color textColor = {0xFF,0xFF,0xFF,0xFF};

	int manaFrac = (int) (100.0f * (float) Game.player.mana/(float) Game.player.totMana);
	int healthFrac = (int) (100.0f * (float) Game.player.health/(float) Game.player.totHealth);

	tempRect.x = SPRITE_W*2*1;
	sprintf(str, "%3d%%", healthFrac);

	SDL_BlitSurface(healthContainer, NULL, Game.gfx.screen, &tempRect);
	drawText(SPRITE_W*2*1+16+4, SPRITE_W*3*1+8, textColor, str);

	tempRect.x = SPRITE_W*2*5;
	sprintf(str, "%3d%%", manaFrac);

	SDL_BlitSurface(manaContainer, NULL, Game.gfx.screen, &tempRect);
	drawText(SPRITE_W*2*5+16+4, SPRITE_W*3*1+8, textColor, str);

	// NOTE: item container
	tempRect.x = SPRITE_W*2*17;
	SDL_BlitSurface(itemContainer, NULL, Game.gfx.screen, &tempRect);

	tempRect.x = SPRITE_W*2*19;
	SDL_BlitSurface(itemContainer, NULL, Game.gfx.screen, &tempRect);

	tempRect.x = SPRITE_W*2*21;
	SDL_BlitSurface(itemContainer, NULL, Game.gfx.screen, &tempRect);

	tempRect.x = SPRITE_W*2*23;
	SDL_BlitSurface(itemContainer, NULL, Game.gfx.screen, &tempRect);

	tempRect.x = SPRITE_W*2*25;
	SDL_BlitSurface(itemContainer, NULL, Game.gfx.screen, &tempRect);

	tempRect.x = SPRITE_W*2*27;
	SDL_BlitSurface(itemContainer, NULL, Game.gfx.screen, &tempRect);
}

#endif
