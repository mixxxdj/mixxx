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
 * windriver.cpp: Responsible for implementing the IPC communication
 *      between the application and protocol.cpp
 *
 **********************************************************************/


#ifndef WIN32
#error "Invalid Arch"
#endif

#include "globaldefs.h"
#include "protocol/protocol.h"
extern "C"{
#include "utf/utstr.h"
}

#include <stdlib.h>
#include <windows.h>
#include <assert.h>

//#ifdef DEBUG
#include <stdio.h>
//#endif

#ifdef DEBUG
static const int TIME_OUT = INFINITE;
#else
#define TIME_OUT 100
#endif

//Global State Vars
static int isInitalized = FALSE;
static HANDLE hThread;
static DWORD dwThreadId;
static HANDLE hEventNewData;
static bool workerIsWaiting = false;
static int fuserVerified = false;

Protocol *pprotocol;

//IPC COMMANDS
static const int MSG_EXIT =  -5;
static const int WRITE_MSG_ASCII = 1;
static const int WRITE_MSG_UTF8 = 2;
static const int WRITE_MSG_UTF16 = 3;
static const int WRITE_MSG_UTF32 = 4;
static const int WRITE_MSG_BIN = 5;
static const int FLUSH_BUFFERS = 6;
static const int WRITE_MSG_DBG_ASCII = 7;
static const int WRITE_MSG_DBG_UTF8 = 8;
static const int WRITE_MSG_DBG_UTF16 = 9;
static const int WRITE_MSG_DBG_UTF32 = 10;
static const int SEND_MSG_DBG = 11;

typedef struct _msg{
	int command;
	int msgType;
	unsigned int msgSize;
	char *pdata;
	struct _msg *pnext;
} LIB_SHAREDMSG;

struct _libconfig{
	int maxMsgQueue;
	int maxDbgMsg;
};

static LIB_SHAREDMSG *psmsgFirst = NULL; 
static LIB_SHAREDMSG *psmsgLast = NULL;
static HANDLE hMsgQueueMutex;
//Local Prototypes
extern "C" __declspec(dllexport) int winlibentry(LPVOID lpvMaxMsgQueue);

/* Queue Manager Functions */
void freeMsg(LIB_SHAREDMSG smsg)
{
	free(smsg.pdata);
}

int smsgEnqueue(LIB_SHAREDMSG smsg)
{
	LIB_SHAREDMSG *pmsg = (LIB_SHAREDMSG*) malloc(sizeof(LIB_SHAREDMSG));
	memcpy_s(pmsg, sizeof(LIB_SHAREDMSG), &smsg, sizeof(LIB_SHAREDMSG));

	WaitForSingleObject(hMsgQueueMutex, TIME_OUT);
	pmsg->pnext = NULL;
	if(psmsgLast == NULL)
	{
		psmsgFirst = pmsg;
		psmsgLast = pmsg;
		if(workerIsWaiting)
			SetEvent(hEventNewData);
	}
	else
	{
		psmsgLast = psmsgLast->pnext = pmsg;
	}
#ifdef DEBUG
	LIB_SHAREDMSG *pmsgCur = psmsgFirst;
	if(pmsgCur)
	{
		while(pmsgCur->pnext != NULL)
			pmsgCur = pmsgCur->pnext;
	}
	assert( pmsgCur == psmsgLast );
#endif
	ReleaseMutex(hMsgQueueMutex);

	return TRUE;
}

LIB_SHAREDMSG smsgDequeue()
{
	LIB_SHAREDMSG *psmsg;
	LIB_SHAREDMSG smsgRet;

	WaitForSingleObject(hMsgQueueMutex, TIME_OUT);	

	if(psmsgFirst == NULL) //Empty List, we block until we get signalled
	{
		workerIsWaiting = true;
		ReleaseMutex(hMsgQueueMutex);
		WaitForSingleObject(hEventNewData, INFINITE);
		WaitForSingleObject(hMsgQueueMutex, INFINITE);
		assert(psmsgFirst != NULL);
		assert(psmsgLast != NULL);
		workerIsWaiting = false;
	}

	psmsg = psmsgFirst;

	if(psmsgFirst == psmsgLast)
	{
		psmsgFirst = psmsgLast = NULL;
	}
	else
	{
		psmsgFirst = psmsgFirst->pnext;
	}
	ReleaseMutex(hMsgQueueMutex);

	smsgRet = *psmsg;
	free(psmsg);
	return smsgRet;
}

/*****************************
 * Main Library Initialization Routine
 *      application must call this first
 *
 * Return: TRUE on success
 *         FAIL on failure
 *****************************/
extern "C" __declspec(dllexport) int LIBINIT(int maxMsgQueue, int maxDbgMsg, int finUserVerified)
{
	struct _libconfig *pconf;
	fuserVerified = finUserVerified;
	if(fuserVerified)
	{
		pconf = (struct _libconfig*) malloc(sizeof(struct _libconfig));
		pconf->maxMsgQueue = maxMsgQueue;
		pconf->maxDbgMsg = maxDbgMsg;

		hMsgQueueMutex = CreateMutex(NULL, FALSE, NULL);
		if(hMsgQueueMutex == NULL)
			return FALSE;

		hEventNewData = CreateEvent(NULL, FALSE, FALSE, NULL);
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)winlibentry, (LPVOID)pconf, 0, &dwThreadId);
		
		if(hThread == NULL)
		{
			CloseHandle(hMsgQueueMutex);
			return FALSE;
		}

		return TRUE;
	}
	return FALSE;
}

extern "C" __declspec(dllexport) int winlibentry(LPVOID lpvpconf)
{
	LIB_SHAREDMSG localMsg;
	struct _libconfig *pconf = (struct _libconfig*) lpvpconf;
	int maxMsgQueue = pconf->maxMsgQueue;
	int maxDbgMsg = pconf->maxDbgMsg;
	int cDbgMsg = 0;
	free(pconf);

	//BOOTSTRAP Protocol, and wait for messages
	do{
		pprotocol = new Protocol(maxMsgQueue, maxDbgMsg);
	}while(pprotocol == NULL);

	while(TRUE)
	{

		//memcpy(&localMsg, &smsg, sizeof(LIB_SHAREDMSG));
		localMsg = smsgDequeue();

		switch(localMsg.command)
		{
			case WRITE_MSG_ASCII:
				pprotocol->writeMsg(localMsg.msgType, localMsg.pdata);
				break;

			case WRITE_MSG_UTF8:
				pprotocol->writeMsgUTF8(localMsg.msgType, localMsg.pdata);
				break;

			case WRITE_MSG_UTF16:
				pprotocol->writeMsgUTF16(localMsg.msgType, (XCHAR*)localMsg.pdata);
				break;

			case WRITE_MSG_BIN:
				pprotocol->writeMsgBin(localMsg.msgType, (BYTE*)localMsg.pdata, localMsg.msgSize);
				break;

			case MSG_EXIT:
				delete pprotocol;
				return 0;
			
			case WRITE_MSG_DBG_ASCII:
				pprotocol->writeMsg(cDbgMsg++, localMsg.pdata, MSGSCOPE_DEBUG);
				break;

			case WRITE_MSG_DBG_UTF8:
				pprotocol->writeMsgUTF8(cDbgMsg++, localMsg.pdata, MSGSCOPE_DEBUG);
				break;

			case SEND_MSG_DBG:
				pprotocol->sendMsgDbg();
				break;

			default:
				assert(FALSE);
		};
	}
	freeMsg(localMsg);
}

extern "C" __declspec(dllexport) void LIBCLOSE(int timeout)
{
	LIB_SHAREDMSG smsg;
	if(fuserVerified)
	{
		smsg.command = MSG_EXIT;
		smsg.pdata = NULL;
		smsgEnqueue(smsg);
		WaitForSingleObject(hThread, timeout * 100);
		CloseHandle(hMsgQueueMutex);
	}
}

/* EXTERNAL INTERFACE COMMANDS */
#include <string.h>

extern "C" __declspec(dllexport) void WRITEMSG_ASCII(int msgType, char *pstz)
{
   LIB_SHAREDMSG smsg;
   int size;

   if(fuserVerified)
   {
	   smsg.command = WRITE_MSG_ASCII;
	   smsg.msgType = msgType;
	   size = strlen(pstz) + 1;
	   smsg.pdata = (char*) malloc(size);
	   memcpy_s(smsg.pdata, size, pstz, size);

	   smsgEnqueue(smsg);
   }
}

extern "C" __declspec(dllexport) void WRITEMSG_UTF8(int msgType, char *pstz)
{
   LIB_SHAREDMSG smsg;
	int size;
	
   if(fuserVerified)
   {
	   smsg.command = WRITE_MSG_UTF8;
	   smsg.msgType = msgType;
	   size = strlen(pstz) + 1;
	   smsg.pdata = (char*) malloc(size);
	   memcpy_s(smsg.pdata, size, pstz, size);

	   smsgEnqueue(smsg);
   }
}

extern "C" __declspec(dllexport) void WRITEMSG_UTF16(int msgType, XCHAR *pstz)
{
   LIB_SHAREDMSG smsg;
	int size;
   
   if(fuserVerified)
   {
	   smsg.command = WRITE_MSG_UTF16;
	   smsg.msgType = msgType;
	   size = (xstrlen(pstz) + 1) * sizeof(XCHAR);
	   smsg.pdata = (char*) malloc(size);
	   memcpy_s(smsg.pdata, size, pstz, size);
	   
	   smsgEnqueue(smsg);
   }
}

extern "C" __declspec(dllexport) void WRITEMSG_BIN(int msgType, const BYTE *pb, unsigned int size)
{
   LIB_SHAREDMSG smsg;

   if(fuserVerified)
   {
	   smsg.command = WRITE_MSG_BIN;
	   smsg.msgType = msgType;
	   smsg.pdata = (char*) malloc(size);
	   memcpy_s(smsg.pdata, size, pb, size);
	   smsg.msgSize = size;

	   smsgEnqueue(smsg);
   }
}

extern "C" __declspec(dllexport) void WRITEMSG_DBG_ASCII(char *pstz)
{
   LIB_SHAREDMSG smsg;
   int size;

   if(fuserVerified)
   {
	   smsg.command = WRITE_MSG_DBG_ASCII;
	   size = strlen(pstz);
	   smsg.pdata = (char*) malloc(size);
	   memcpy_s(smsg.pdata, size, pstz, size);

	   smsgEnqueue(smsg);
   }
}

extern "C" __declspec(dllexport) void WRITEMSG_DBG_UTF8(char *pstz)
{
	LIB_SHAREDMSG smsg;
	int size;

	if(fuserVerified)
	{
		smsg.command = WRITE_MSG_DBG_UTF8;
		size = strlen(pstz);
		smsg.pdata = (char*) malloc(size);
		memcpy_s(smsg.pdata, size, pstz, size);
		
		smsgEnqueue(smsg);
	}
}

extern "C" __declspec(dllexport) void SENDMSG_DBG()
{
	LIB_SHAREDMSG smsg;

	if(fuserVerified)
	{
		smsg.command = SEND_MSG_DBG;
		smsg.pdata = NULL;
		smsgEnqueue(smsg);
	}
}

#ifdef TEST
int main()
{
    LIBINIT(100, 2, TRUE); 

	WRITEMSG_ASCII(0x01, "FIRST MESSAGE");
	WRITEMSG_DBG_ASCII("Debug ASCII 0");
	SENDMSG_DBG();
    WRITEMSG_DBG_UTF8("Debug MSG 1");
    WRITEMSG_DBG_UTF8("Debug MSG 2");
    WRITEMSG_DBG_UTF8("Debug MSG 3");
    WRITEMSG_DBG_UTF8("Debug MSG 4");
    WRITEMSG_DBG_ASCII("Debug MSG 5");
	SENDMSG_DBG();

    WRITEMSG_ASCII(0x01, "this is a test");
    WRITEMSG_UTF8(0x02, "test2");
    WRITEMSG_ASCII(0x04, "test23");
    WRITEMSG_ASCII(0x03, "test3");
    WRITEMSG_ASCII(0x01, "test 0x01 again.");
    WRITEMSG_BIN(0x020, (const BYTE*)"abc", 3);
    
	SENDMSG_DBG();
	int count = 0;
	while(1)
	{
		if((count % 500) == 0)
		{
			Sleep(500);	//flush the list
		
		}
		if(count > 2000)
			break;
		WRITEMSG_ASCII(0x050, "STRESS TEST");
		count++;
	}
    LIBCLOSE(10000);
}
#endif