#ifndef __TYPES_H_
#define __TYPES_H_

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
};

//-----------------------------------------------------------------------------
struct dbData {
	char *password;
	uint8_t retCode;
	uint32_t playerID;
	struct Player player;
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
