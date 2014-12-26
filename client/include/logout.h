#ifndef __LOGOUT_H_
#define __LOGOUT_H_

//-----------------------------------------------------------------------------
uint8_t playerLogout(struct logoutPacket *p, uint8_t retCode) {
	// NOTE: switch based on previous retCode
	switch(retCode) {
		case 0x00: {
			// NOTE: for now don't wait for an OK from server - just exit
			return 0x01;
		} break;
		case 0xFF: {
			// NOTE: send out the logout packet
			UDPpacket packet = {};

			packet.maxlen = 1+4+p->unLen+4+p->pwLen;
			packet.data = (uint8_t *)malloc(packet.maxlen);

			uint8_t offset = 0;

			memset(packet.data+offset, 0x02, 1);
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
