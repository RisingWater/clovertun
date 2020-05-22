#pragma once

#ifndef __P2P_CONNECTION_H__
#define __P2P_CONNECTION_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "BaseObject.h"
#include "TCPServer.h"
#include "UDPBase.h"
#include "P2PProtocol.h"

class CP2PConnection : public CBaseObject
{
public:
    CP2PConnection(TCP_WAIT_PACKET* PacketData, CTCPServer* Server);
    ~CP2PConnection();

    P2P_STATUS GetStatus();

    const CHAR* GetKeyword();

    BOOL IsMainTCPServer(CTCPServer* tcp);
    BOOL IsConnTCPServer(CTCPServer* tcp);

    VOID ConnTCPDisconnect();
    VOID MainTCPDisconnect();

    DWORD TCPConnected(TCP_CONN_PACKET* PacketData, CTCPServer* Server);
    DWORD UDPListening(CLIENT_INFO* ClientInfo);
    DWORD UDPConnected(CLIENT_INFO* ClientInfo);

    DWORD TCPProxyRequest();
    DWORD TCPDataProxy(TCP_PROXY_DATA* Data, DWORD Length);

private:
    CHAR                 m_szKeyword[KEYWORD_SIZE];
    
    CTCPServer*          m_pMainTCPServer;
    DWORD                m_dwMainTCPID;
    CHAR                 m_szMainTCPName[NAME_SIZE];

    CTCPServer*          m_pConnTCPServer;
    DWORD                m_dwConnTCPID;
    CHAR                 m_szConnTCPName[NAME_SIZE];

    CLIENT_INFO          m_stMainUDPServer;
    CLIENT_INFO          m_stConnUDPServer;
    P2P_STATUS           m_eStatus;

    CRITICAL_SECTION     m_csLock;
};

#endif