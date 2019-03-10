/*
* Web based SDR Client for SDRplay
* =============================================================
* Author: DJ0ABR
*
*   (c) DJ0ABR
*   www.dj0abr.de
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
* 
* ws_callbacks.c ... callback functions of the Websocket Server
* 
 * Websocket Server: Callback functions
 * each client get one thread and these callback functions
 * Clients are identified by fd
* 
* This WebSocketServer is a multi threaded implementation and
* opens a new thread for each client
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "websocketserver.h"

// a new browser connected
void onopen(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	printf("Connection opened, client: %d | addr: %s\n", fd, cli);
	free(cli);
}

// a browser disconnected
void onclose(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	printf("Connection closed, client: %d | addr: %s\n", fd, cli);
	free(cli);
}

// if avaiable, send data to a browser
void onwork(int fd, unsigned char *cnt0, unsigned char *cnt1)
{
    // ws_sendframe_binary(fd, pixdata+1, len-1); // send without first id counter
}

// received a Websocket Message from a browser
void onmessage(int fd, unsigned char *msg)
{
	char *cli = ws_getaddress(fd);
	printf("I receive a message: %s, from: %s/%d\n", msg, cli, fd);
	
	free(cli);
	free(msg);
}
