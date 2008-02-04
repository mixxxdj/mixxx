/**********************************************
 * Cmetrics.h - Case Metrics Interface
 *  Copyright 2007 John Sully.
 *
 *  This file is part of Case Metrics.
 *
 *  Case Metrics is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  Case Metrics is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Case Metrics.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************/

/**********************************************************************
 * lindriver.cpp: Responsible for implementing the IPC communication
 *      between the application and protocol.cpp
 *
 **********************************************************************/

#ifndef LINUX
#error "Invalid Arch"
#endif

#include "globaldefs.h"
#include "protocol/protocol.h"
extern "C"{
#include "utf/utstr.h"
}

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

//#ifdef DEBUG
#include <stdio.h>
//#endif

//Global State Vars
static int childPid;
static int tx;
static int rx;
Protocol *pprotocol;

//IPC COMMANDS
static const int MSG_EXIT =  -5;
static const int WRITE_MSG_ASCII = 1;
static const int WRITE_MSG_UTF8 = 2;
static const int WRITE_MSG_UTF16 = 3;
static const int WRITE_MSG_UTF32 = 4;
static const int WRITE_MSG_BIN = 5;
static const int FLUSH_BUFFERS = 6;
static const int WRITE_MSG_DBG_UTF8 = 7;
static const int SEND_MSG_DBG = 8;


/*****************************
 * Main Library Initialization Routine
 *      application must call this first
 *
 * Return: TRUE on success
 *         FAIL on failure
 *****************************/
extern "C" __attribute__ ((visibility ("default"))) int LIBINIT(int maxMsgQueue, int maxDbgMsgs)
{
    int dbgCount = 0;
    int fdAppToLib[2];
    int fdLibToApp[2];

    //Make the IPC pipe
    if(pipe(fdAppToLib) != 0)
        return FALSE;
    if(pipe(fdLibToApp) != 0)
    {
	close(fdAppToLib[0]);
	close(fdAppToLib[1]);
	return FALSE;
    }

    //fork
    childPid = fork();
    if(childPid == -1)
    {
        close(fdAppToLib[0]);
        close(fdAppToLib[1]);
	close(fdLibToApp[0]);
	close(fdLibToApp[1]);
        return FALSE;
    }

    if(childPid != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Child forked: %d\n", childPid);
#endif
	tx = fdAppToLib[1];
	rx = fdLibToApp[0];
	return TRUE;
    }
    else
    {
        //Protocol Process
	tx = fdLibToApp[1];
	rx = fdAppToLib[0];

        //Initialize Protocol Class
        while(pprotocol == NULL)    //loop until new succeeds
                                    //this is a criticle alloc
        {
            pprotocol = new Protocol(maxMsgQueue, maxDbgMsgs);
            if(pprotocol == NULL)
                sleep(1);
        }
	
	//Message Queue
#ifdef DEBUG
	fprintf(stderr, "Child: Entering message queue\n");
#endif
	int command;
	int msgtype, msgsize;
	char *msg = NULL;
	int msgAllocd = 0;
	int result;
	while(1)
	{
	    if(read(rx, &command, sizeof(int)) != sizeof(int))
	    {
#ifdef DEBUG
		fprintf(stderr,"Invalid read\n");
		perror("");
#endif
		break;
	    }
	    if(command == MSG_EXIT)
		break;

	    //Process Commands
	    switch(command)
	    {
		case WRITE_MSG_ASCII:
		case WRITE_MSG_UTF8:
		case WRITE_MSG_UTF16:
		case WRITE_MSG_UTF32:
		case WRITE_MSG_BIN:
		    read(rx, &msgtype, sizeof(int));
		    read(rx, &msgsize, sizeof(int));
		    if(msgsize > msgAllocd)
		    {
			free(msg);
			msg = (char*) malloc(msgsize * sizeof(char));
		    }
		    read(rx, msg, msgsize);
		    switch(command)	//I dont' like nesting it like this but the code above has to be done for all messages
		    {
			case WRITE_MSG_ASCII:
			    //result = pprotocol->writeMsg(msgtype, msg);
			    result = pprotocol->writeMsg(msgtype, "this is a test");
			    break;

			case WRITE_MSG_UTF8:
			    result = pprotocol->writeMsgUTF8(msgtype, msg);
			    break;

			case WRITE_MSG_UTF16:
			    result = pprotocol->writeMsgUTF16(msgtype, (XCHAR*)msg);
			    break;

			case WRITE_MSG_UTF32:
			    result = pprotocol->writeMsgUTF32(msgtype, (unsigned int*)msg);
			    break;

			case WRITE_MSG_BIN:
			    result = pprotocol->writeMsgBin(msgtype, (BYTE*)msg, msgsize);
			    break;
			
			
			default:
			    assert(FALSE);
			    break;
		    }
		    //write(tx, &result, sizeof(result));
		    break;

		case WRITE_MSG_DBG_UTF8:
                    read(rx, &msgsize, sizeof(int));
                    if(msgsize > msgAllocd)
	            {
	                free(msg);
		        msg = (char*) malloc(msgsize * sizeof(char));
		    }
		    read(rx, msg, msgsize);
    
    		    result = pprotocol->writeMsg(dbgCount++, msg, MSGSCOPE_DEBUG);
		    break;

		case SEND_MSG_DBG:
		    result = pprotocol->sendMsgDbg();
		    break;

		default:
		    assert(FALSE);
		    break;
	    }
	}
	pprotocol->flushBuff();
	delete pprotocol;
	close(tx);
	close(rx);
	free(msg);
	_exit(0);
    }
}

extern "C" __attribute__ ((visibility ("default"))) void LIBCLOSE(int timeout)
{
    write(tx, &MSG_EXIT, sizeof(MSG_EXIT));
    int child = fork();	//fork watchdog
    if(child == 0)
    {
	//child
	sleep(timeout);	//close timeout
#ifdef DEBUG
	fprintf(stderr, "Timeout Exceeded: KILLING\n");
#endif
	kill(childPid, SIGKILL);
	_exit(0);
    }
    else
    {
        waitpid(childPid, NULL, 0);
	if(child != -1)
	{
	    kill(child, SIGKILL);
	    waitpid(child, NULL, 0);
	}
    }
    close(tx);
    close(rx);
}

extern "C" __attribute__ ((visibility ("default"))) void cm_set_crash_dlg(void (*pcrashDlg)(void))
{
    //TODO
}

/* EXTERNAL INTERFACE COMMANDS */
#include <string.h>

extern "C" __attribute__ ((visibility ("default"))) void WRITEMSG_ASCII(int msgType, char *pstz)
{
    int msgSize = strlen(pstz) + 1;
    
    write(tx, &WRITE_MSG_ASCII, sizeof(WRITE_MSG_ASCII));
    write(tx, &msgType, sizeof(msgType));
    write(tx, &msgSize, sizeof(msgSize));
    write(tx, pstz, msgSize);
}

extern "C" __attribute__ ((visibility ("default"))) void WRITEMSG_UTF8(int msgType, char *pstz)
{
    int msgSize = strlen(pstz) + 1;
    
    write(tx, &WRITE_MSG_UTF8, sizeof(WRITE_MSG_UTF8));
    write(tx, &msgType, sizeof(msgType));
    write(tx, &msgSize, sizeof(msgSize));
    write(tx, pstz, msgSize);
}

extern "C" __attribute__ ((visibility ("default"))) void WRITEMSG_UTF16(int msgType, XCHAR *pstz)
{
    int msgSize = xstrlen(pstz) * sizeof(XCHAR);

    write(tx, &WRITE_MSG_UTF16, sizeof(WRITE_MSG_UTF16));
    write(tx, &msgType, sizeof(msgType));
    write(tx, &msgSize, sizeof(msgSize));
    write(tx, pstz, msgSize);
}

extern "C" __attribute__ ((visibility ("default"))) void WRITEMSG_BIN(int msgType, const BYTE *pb, unsigned int size)
{
    write(tx, &WRITE_MSG_BIN, sizeof(WRITE_MSG_BIN));
    write(tx, &msgType, sizeof(msgType));
    write(tx, &size, sizeof(size));
    write(tx, pb, size);
}

extern "C" __attribute__ ((visibility ("default"))) void WRITEMSG_DBG_UTF8(const char *pstz)
{
    int msgSize = strlen(pstz) + 1;

    write(tx, &WRITE_MSG_DBG_UTF8, sizeof(WRITE_MSG_DBG_UTF8));
    write(tx, &msgSize, sizeof(msgSize));
    write(tx, pstz, msgSize);
}

extern "C" __attribute__ ((visibility ("default"))) void SENDMSG_DBG()
{
    write(tx, &SEND_MSG_DBG, sizeof(SEND_MSG_DBG));
}

#ifdef TEST
int __attribute__ ((visibility ("default"))) main()
{
    fprintf(stderr, "Server Host: %s\n", SERVER_HOST);
    if(!LIBINIT(50, 2))
	return -1;

    WRITEMSG_DBG_UTF8("Debug MSG 1");
    WRITEMSG_DBG_UTF8("Debug MSG 2");
    SENDMSG_DBG();
    WRITEMSG_DBG_UTF8("Debug MSG 3");
    WRITEMSG_DBG_UTF8("Debug MSG 4");
    WRITEMSG_DBG_UTF8("Debug MSG 5");

    WRITEMSG_ASCII(0x01, "this is a test");
    WRITEMSG_UTF8(0x02, "test2");
    WRITEMSG_ASCII(0x04, "test23");
    WRITEMSG_ASCII(0x03, "test3");
    WRITEMSG_ASCII(0x01, "test 0x01 again.");
    WRITEMSG_BIN(0x020, (const BYTE*)"abc", 3);
    //write(tx, &WRITE_MSG_BIN, 4);
    //sleep(15);
    
    SENDMSG_DBG();

    LIBCLOSE(100000);
    return 0;
}
#endif
