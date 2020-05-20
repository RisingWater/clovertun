#pragma once

#ifndef __UDP_SERVER_H__
#define __UDP_SERVER_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "UDPBase.h"

typedef struct
{
    CHAR        keyword[32];
    CLIENT_INFO info;
} WAITING_NODE;

class CUDPServer : public CUDPBase
{
public:
	CUDPServer(WORD Port);
    ~CUDPServer();

	virtual VOID RecvPacketProcess(UDP_PACKET Packet);

private:
    std::list<WAITING_NODE> m_WaitList;
};

#endif

