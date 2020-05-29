#pragma once

#ifndef __P2P_GUEST_H__
#define __P2P_GUEST_H__

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

#include "P2PClient.h"

class CP2PGuest : public CP2PClient
{
public:
    CP2PGuest(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort);
    virtual ~CP2PGuest();

    DWORD Connect();

private:  
    virtual BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet);
    
    VOID TCPConnectEventProcess();
};

#endif