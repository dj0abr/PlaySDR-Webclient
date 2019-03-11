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
#include "../setqrg.h"

// a new browser connected
void onopen(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
    insert_socket(fd);
	printf("Connection opened, client: %d | addr: %s\n", fd, cli);
	free(cli);
}

// a browser disconnected
void onclose(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
    remove_socket(fd);
	printf("Connection closed, client: %d | addr: %s\n", fd, cli);
	free(cli);
}

// if avaiable, send data to a browser
void onwork(int fd, unsigned char *cnt0, unsigned char *cnt1)
{
    for(int i=0; i<MAX_CLIENTS; i++)
    {
        if(actsock[i].socket == fd)
        {
            // this is our entry in the socket table
            if(actsock[i].send0 == 1)
            {
                // there is something to be sent
                unsigned char pixdata[MESSAGE_LENGTH * 2];

                memcpy(pixdata, actsock[i].msg0+1, actsock[i].msglen0-1);
                int len = actsock[i].msglen0-1;
                
                if(actsock[i].send1 == 1)
                {
                    memcpy(pixdata + len, actsock[i].msg1, actsock[i].msglen1);
                    len += actsock[i].msglen1;
                }
                
                ws_sendframe_binary(fd, pixdata, len);
                actsock[i].send0 = 0;
                actsock[i].send1 = 0;
                return;
            }
        }
    }
}

// received a Websocket Message from a browser
void onmessage(int fd, unsigned char *msg)
{
	char *cli = ws_getaddress(fd);
	printf("user message: %s, from: %s/%d\n", msg, cli, fd);
    
    if(strstr((char *)msg,"mousepo:"))
    {
        freqval = atoi((char *)msg+8);
        setfreq = 1;
    }
    if(strstr((char *)msg,"mousewh:"))
    {
        freqval = atoi((char *)msg+8);
        setfreq = 2;
    }
    if(strstr((char *)msg,"bandsel:"))
    {
        freqval = atoi((char *)msg+8);
        setfreq = 3;
    }
    if(strstr((char *)msg,"ssbmode:"))
    {
        freqval = atoi((char *)msg+8);
        setfreq = 4;
    }
	
	free(cli);
	free(msg);
}
