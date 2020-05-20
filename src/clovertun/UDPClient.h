#pragma once

#ifndef __UDP_CLIENT_H__
#define __UDP_CLIENT_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#else
#include <winpr/wtypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <list>
#include "UDPBase.h"

#define UDP_PORT_BASE 30000

typedef enum {
    CRT_SERVER = 0,
    CRT_CLIENT,
} ClientRoleType;

class CUDPClient : public CUDPBase
{
public:
	CUDPClient(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerPort, ClientRoleType Type);
	~CUDPClient();

    VOID Connect();
    virtual VOID RecvPacketProcess(UDP_PACKET Packet);

private:
	CHAR m_szName[32];
    CHAR m_szKeyword[32];
    CHAR m_szServerIP[32];
    WORD m_dwServerPort;
    WORD m_dwLocalPort;
    ClientRoleType m_eRole;
    HANDLE m_hConnectServerEvent;
    CLIENT_INFO m_stRemoteClientInfo;
};

#endif
