#ifndef __NETWORK_H_
#define __NETWORK_H_

//-----------------------------------------------------------------------------
void networkPoll(void) {
	// NOTE: check for connections
	if(SDLNet_CheckSockets(socketSet, 0)==-1) {
		fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
		return;
	}

	if(SDLNet_SocketReady(clientFD)) {
		// NOTE: here we simply get how many players the server is going to send
		UDPpacket packet;

		// NOTE: allocate space for packet
		packet.maxlen = 0xAA; // 170 bytes
		packet.data = (uint8_t *)malloc(0xAA);

		// NOTE: get packet
		int recv = SDLNet_UDP_Recv(clientFD, &packet);
		if(!recv) {
			free(packet.data);
			return;
		}

		// NOTE: if it isn't the server then ignore it
		if(packet.channel!=serverChannel) {
			free(packet.data);
			return;
		}

		// NOTE: read the flag for packet identity
		uint8_t flag = 0;
		uint8_t offset = 0;

		memcpy(&flag, packet.data, 1);
		offset += 1;

		switch(flag) {
			case 0x04: {
				// NOTE: new player connection
				/*
				- flag         ( 1)
				- State        ( 4)
				- PlayerID     ( 4)
				- Node         ( 4)
				- X            ( 4)
				- Y            ( 4)
				- DiamondCount ( 4)
				============== (25)
				*/

				struct Player tempPlayers[numChrs];
				memcpy(tempPlayers, chrsOnline, numChrs*sizeof(struct Player));

				chrsOnline = (struct Player *)realloc(chrsOnline, ++numChrs*sizeof(struct Player));
				memset(chrsOnline, 0x00, numChrs*sizeof(struct Player));

				int i;
				for(i=0; i<(numChrs-1); i++)
					memcpy(&chrsOnline[i], &tempPlayers[i], sizeof(struct Player));
				struct Player *chr = &chrsOnline[numChrs-1];

				memcpy(&chr->state, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->id, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->node, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->x, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->y, packet.data+offset, 4);
				offset += 4;
				memcpy(&chr->count, packet.data+offset, 4);
				offset += 4;
			} break;
			case 0x05: {
				// NOTE: player disconnection
				/*
				- flag         ( 1)
				- PlayerID     ( 4)
				============== ( 5)
				*/

				uint32_t id = 0;

				memcpy(&id, packet.data+offset, 4);
				offset += 4;

				int i, ind = -1;
				for(i=0; i<numChrs; i++) {
					if(id==chrsOnline[i].id) {
						ind = i;
						continue;
					}
				} if(ind==-1) break;

				free(chrsOnline[ind].username);
				free(chrsOnline[ind].password);

				// NOTE: copy the players into a temp array
				struct Player tempPlayers[numChrs];
				memcpy(tempPlayers, chrsOnline, numChrs*sizeof(struct Player));

				// NOTE: resize and clear the actual client array
				chrsOnline = (struct Player *)realloc(chrsOnline, --numChrs*sizeof(struct Player));
				memset(chrsOnline, 0x00, numChrs*sizeof(struct Player));

				for(i=0; i<numChrs; i++) {
					if(i<ind) memcpy(&chrsOnline[i], &tempPlayers[i], sizeof(struct Player));
					else memcpy(&chrsOnline[i], &tempPlayers[i+1], sizeof(struct Player));
				}
			} break;
		}

		// NOTE: free the packet
		free(packet.data);
	}
}

#endif
