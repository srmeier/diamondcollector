#ifndef __TYPES_H_
#define __TYPES_H_

//-----------------------------------------------------------------------------
struct Player {
	uint32_t x;
	uint32_t y;
	uint32_t id;
	uint32_t node;
	char *username;
	char *password;
	uint32_t state;
	uint32_t count;
};

//-----------------------------------------------------------------------------
struct loginPacket {
	char *username;
	char *password;
	uint32_t unLen;
	uint32_t pwLen;
};

#endif
