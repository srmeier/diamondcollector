#ifndef __LOGIN_H_
#define __LOGIN_H_

//-----------------------------------------------------------------------------
int getPlayerLoginInfo(char **username, uint32_t *unSize, char **password, uint32_t *pwSize) {
	// NOTE: copy memory locations
	uint32_t _unSize = *unSize;
	uint32_t _pwSize = *pwSize;
	char *_username = *username;
	char *_password = *password;

	// NOTE: toggle between username and password fields
	static SDL_bool curUnEdit = SDL_TRUE;

	// NOTE: get the username and password info from player
	if(upBnt && !upChk) {
		if(curUnEdit) {
			_username[_unSize-1]++;
			if(_username[_unSize-1]==0x5B)
				_username[_unSize-1] = 0x41;
		} else {
			_password[_pwSize-1]++;
			if(_password[_pwSize-1]==0x5B)
				_password[_pwSize-1] = 0x41;
		}
		upChk = SDL_TRUE;
	} else if(!upBnt) upChk = SDL_FALSE;

	if(downBnt && !downChk) {
		if(curUnEdit) {
			_username[_unSize-1]--;
			if(_username[_unSize-1]==0x40)
				_username[_unSize-1] = 0x5A;
		} else {
			_password[_pwSize-1]--;
			if(_password[_pwSize-1]==0x40)
				_password[_pwSize-1] = 0x5A;
		}
		downChk = SDL_TRUE;
	} else if(!downBnt) downChk = SDL_FALSE;

	if(leftBnt && !leftChk) {
		if(curUnEdit && (_unSize-1)>0) {
			_username = (char *)realloc(_username, --_unSize+1);
			_username[_unSize] = 0x00;
		} else if(!curUnEdit && (_pwSize-1)>0) {
			_password = (char *)realloc(_password, --_pwSize+1);
			_password[_pwSize] = 0x00;
		}
		leftChk = SDL_TRUE;
	} else if(!leftBnt) leftChk = SDL_FALSE;

	if(rightBnt && !rightChk) {
		if(curUnEdit && (_unSize+1)<12) {
			_username = (char *)realloc(_username, ++_unSize+1);
			_username[_unSize-1] = 0x41;
			_username[_unSize] = 0x00;
		} else if(!curUnEdit && (_pwSize+1)<12) {
			_password = (char *)realloc(_password, ++_pwSize+1);
			_password[_pwSize-1] = 0x41;
			_password[_pwSize] = 0x00;
		}
		rightChk = SDL_TRUE;
	} else if(!rightBnt) rightChk = SDL_FALSE;

	if(aBnt && !aChk) {
		if(curUnEdit) {
			curUnEdit = SDL_FALSE;
		} else curUnEdit = SDL_TRUE;
		aChk = SDL_TRUE;
	} else if(!aBnt) aChk = SDL_FALSE;

	if(bBnt && !bChk) {
		return 1;
		bChk = SDL_TRUE;
	} else if(!bBnt) bChk = SDL_FALSE;

	SDL_Color color0 = {0x73, 0x73, 0x73, 0x00};
	SDL_Color color1 = {0xFF, 0xFF, 0xFF, 0x00};
	SDL_Color color2 = {0xFF, 0x00, 0x00, 0x00};

	char *str0 = "Username:";
	drawText(str0, color0, 0, 0);
	drawText(_username, color1, 8*9, 0);

	char *str1 = "Password:";
	drawText(str1, color0, 0, 16);
	drawText(_password, color1, 8*9, 16);

	if(curUnEdit)
		drawText(&_username[_unSize-1], color2, 8*9+8*(_unSize-1), 0);
	else
		drawText(&_password[_pwSize-1], color2, 8*9+8*(_pwSize-1), 16);

	// NOTE: set memory locations
	*unSize = _unSize;
	*pwSize = _pwSize;
	*username = _username;
	*password = _password;

	return 0;
}

//-----------------------------------------------------------------------------
uint8_t playerLogin(struct loginPacket *p, uint8_t retCode) {
	// NOTE: switch based on previous retCode
	switch(retCode) {
		case 0x00: {
			// NOTE: check for connections
			if(SDLNet_CheckSockets(socketSet, 0)==-1) {
				fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
				return;
			}

			if(SDLNet_SocketReady(clientFD)) {
				// NOTE: setup a packet which is big enough to store the server return
				UDPpacket packet;

				// NOTE: allocate space for packet
				packet.maxlen = 0x15; // 21 bytes
				packet.data = (uint8_t *)malloc(0x15);
				
				/*
				- flag         ( 1)
				- PlayerID     ( 4)
				- Node         ( 4)
				- X            ( 4)
				- Y            ( 4)
				- DiamondCount ( 4)
				============== (21)
				*/

				// NOTE: get packet
				int recv = SDLNet_UDP_Recv(clientFD, &packet);
				if(!recv) {
					free(packet.data);
					return;
				}

				// NOTE: display IPaddress infomation
				Uint32 ipaddr = SDL_SwapBE32(packet.address.host);
				printf("packet from-> %d.%d.%d.%d:%d bound to channel %d\n",
					ipaddr>>24, (ipaddr>>16)&0xFF, (ipaddr>>8)&0xFF, ipaddr&0xFF,
					SDL_SwapBE16(packet.address.port), packet.channel);

				// NOTE: if it isn't the server then ignore it
				if(packet.channel!=serverChannel) {
					free(packet.data);
					return;
				}

				// NOTE: on a successful flag set the mainChr
				uint8_t flag;
				uint32_t unSize;
				uint8_t offset = 0;

				memcpy(&flag, packet.data+offset, 1);
				offset += 1;

				if(flag==0x03) {
					// NOTE: login success - get the rest of the packet
					memcpy(&mainChr.id, packet.data+offset, 4);
					offset += 4;
					memcpy(&mainChr.node, packet.data+offset, 4);
					offset += 4;
					memcpy(&mainChr.x, packet.data+offset, 4);
					offset += 4;
					memcpy(&mainChr.y, packet.data+offset, 4);
					offset += 4;
					memcpy(&mainChr.count, packet.data+offset, 4);
				}

				// NOTE: free the packet
				free(packet.data);

				// NOTE: set the retCode
				return flag;
			}
		} break;
		case 0xFF: {
			// NOTE: send out the login packet
			UDPpacket packet = {};

			packet.maxlen = 1+4+p->unLen+4+p->pwLen;
			packet.data = (uint8_t *)malloc(packet.maxlen);

			uint8_t offset = 0;

			memset(packet.data+offset, 0x01, 1);
			offset += 1;
			memcpy(packet.data+offset, &p->unLen, 4);
			offset += 4;
			memcpy(packet.data+offset, p->username, p->unLen);
			offset += p->unLen;
			memcpy(packet.data+offset, &p->pwLen, 4);
			offset += 4;
			memcpy(packet.data+offset, p->password, p->pwLen);
			offset += p->pwLen;

			packet.len = offset;

			// NOTE: SDLNet_UDP_Send returns the number of people the packet was sent to
			if(!SDLNet_UDP_Send(clientFD, serverChannel, &packet))
				fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

			free(packet.data);

			// NOTE: set the retCode
			return 0x00;
		}
	}
}

#endif
