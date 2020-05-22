#pragma once

#ifndef __P2P_CLIENT_H__
#define __P2P_CLIENT_H__

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

#include "UDPBase.h"
#include "TCPClient.h"

class CP2PClient : public CBaseObject
{
public:
	CP2PClient(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort);
	~CP2PClient();

protected:
    BOOL Init();
    VOID Done();

    VOID SendUDPToServer(BOOL IsHost);
    VOID SendUDPToPeer(DWORD Type);

	CHAR m_szName[32];
    CHAR m_szKeyword[32];
    CHAR m_szServerIP[32];
    WORD m_dwServerTCPPort;
    WORD m_dwServerUDPPort;
    WORD m_dwUDPPort;

    DWORD m_dwTCPid;
    DWORD m_dwPeerid;
    CLIENT_INFO m_stRemoteClientInfo;

    CUDPBase* m_pUDP;
    CTCPClient* m_pTCP;

    DWORD m_dwErrorCode;

    P2P_STATUS m_eStatus;
};


#endif
