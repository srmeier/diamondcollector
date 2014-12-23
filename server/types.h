#ifndef __TYPES_H_
#define __TYPES_H_

//-----------------------------------------------------------------------------
struct dbData {
	char *password;
	uint8_t retCode;
	uint32_t playerID;
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
