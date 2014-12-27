#ifndef __TYPES_H_
#define __TYPES_H_

//-----------------------------------------------------------------------------
struct moveState {
	int x, y;
	int i, j;
	int moveFrame;
	int moveDirec;
	SDL_bool moving;
	SDL_bool canMove;
};

//-----------------------------------------------------------------------------
struct Player {
	uint32_t x;
	uint32_t y;
	uint32_t id;
	IPaddress ip;
	uint32_t node;
	char *username;
	char *password;
	uint32_t state;
	uint32_t count;

	// NOTE: for client
	struct moveState moveState;
};

//-----------------------------------------------------------------------------
struct loginPacket {
	char *username;
	char *password;
	uint32_t unLen;
	uint32_t pwLen;
};

//-----------------------------------------------------------------------------
struct logoutPacket {
	char *username;
	char *password;
	uint32_t unLen;
	uint32_t pwLen;
};

#endif
