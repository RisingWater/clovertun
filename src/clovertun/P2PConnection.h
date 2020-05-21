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

typedef enum
{
    TCP_LISTENING = 0,
    TCP_CONNECTED,
    UDP_LISTENING,
    UDP_CONNECTED,
    P2P_PUNCHING,
    P2P_CONNECTED,
} P2P_STATUS;

class CP2PConnection : public CBaseObject
{
public:
    CP2PConnection(TCP_WAIT_PACKET* PacketData, CTCPServer* Server);
    ~CP2PConnection();

    P2P_STATUS GetStatus();

    int TCPConnected(TCP_CONN_PACKET* PacketData, CTCPServer* Server);
    int UDPListening(CLIENT_INFO* ClientInfo);
    int UDPConnected(CLIENT_INFO* ClientInfo);

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