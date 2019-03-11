/*
* Web based SDR Client for SDRplay
* =============================================================
* Author: DJ0ABR
*
*   (c) DJ0ABR
*   www.dj0abr.de
* 
* most websocket related function are written by
* Davidson Francis <davidsondfgl@gmail.com>
* under the same license (available at github)
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
* websocketserver.c ... sends data to a web browser
* 
* if a web browser is logged into this WebSocketServer then
* we send the pixel data (of one line) and header data to this browser.
* Its the job of the browser to draw the waterfall picture.
* 
* This WebSocketServer is a multi threaded implementation and
* opens a new thread for each client
* 
* ! THIS implementation of a WebSocketServer DOES NOT require any
* modifications of the (Apache) Webserver. It has nothing to do with
* the webserver because it handles all Websocket tasks by itself !
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "websocketserver.h"
#include "../playSDRweb.h"

WS_SOCK actsock[MAX_CLIENTS];

void *wsproc(void *pdata);

// initialise the WebSocketServer
// run the WebSocketServer in a new thread
void ws_init()
{
    for(int i=0; i<MAX_CLIENTS; i++)
    {
        actsock[i].socket = -1;
        actsock[i].send0 = 0;
        actsock[i].send1 = 0;
    }
            
    pthread_t ws_pid = 0;
    
    int ret = pthread_create(&ws_pid,NULL,wsproc, NULL);
    if(ret)
    {
        printf("wf_drawline: proc NOT started\n\r");
    }
}

void *wsproc(void *pdata)
{
    // this thread must terminate itself because
    // the parent does not want to wait
    pthread_detach(pthread_self());

    struct ws_events evs;
	evs.onopen    = &onopen;
	evs.onclose   = &onclose;
	evs.onmessage = &onmessage;
    evs.onwork    = &onwork;
	ws_socket(&evs, WEBSOCK_PORT);
    
    // never comes here because ws_socket runs in an infinite loop
    pthread_exit(NULL); // self terminate this thread
}

/*
 * save the data in an array and set a flag.
 * the transmission to the web browser is done by the threads
 * handling all logged-in web browsers.
 * 
 * this function is thread safe by a LOCK
 */
pthread_mutex_t crit_sec;
#define LOCK	pthread_mutex_lock(&crit_sec)
#define UNLOCK	pthread_mutex_unlock(&crit_sec)

void ws_send(unsigned char *pwfdata, int idx, int wf_id)
{
    // insert the new message into the buffer of each active client
    LOCK;
    for(int i=0; i<MAX_CLIENTS; i++)
    {
        if(actsock[i].socket != -1)
        {
            if(wf_id == 0 && actsock[i].send0 == 0)
            {
                memcpy(actsock[i].msg0, pwfdata, idx);
                actsock[i].msglen0 = idx;
                actsock[i].send0 = 1;
            }
            
            if(wf_id == 1 && actsock[i].send1 == 0)
            {
                memcpy(actsock[i].msg1, pwfdata, idx);
                actsock[i].msglen1 = idx;
                actsock[i].send1 = 1;
            }
        }
    }
    UNLOCK;
}

// insert a socket into the socket-list
void insert_socket(int fd)
{
    LOCK;
    for(int i=0; i<MAX_CLIENTS; i++)
    {
        if(actsock[i].socket == -1)
        {
            actsock[i].socket = fd;
            actsock[i].send0 = 0;
            actsock[i].send1 = 0;
            UNLOCK;
            return;
        }
    }
    UNLOCK;
    printf("all sockets in use !!!\n");
}

// remove a socket from the socket-list
void remove_socket(int fd)
{
    LOCK;
    for(int i=0; i<MAX_CLIENTS; i++)
    {
        if(actsock[i].socket == fd)
            actsock[i].socket = -1;
    }
    UNLOCK;
}
