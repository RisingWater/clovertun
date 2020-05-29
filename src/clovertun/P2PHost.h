#pragma once

#ifndef __P2P_HOST_H__
#define __P2P_HOST_H__

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

class CP2PHost : public CP2PClient
{
public:
	CP2PHost(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort);
	virtual ~CP2PHost();

    DWORD Listen();
    DWORD Accept();

    VOID WaitForStop();
    VOID Close();

private:    
    virtual BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet);

    VOID TCPListeningEventProcess();
};

#endif