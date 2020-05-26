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

private:
    BOOL StartListening();
    VOID Clearup();
    
    static BOOL RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param);
    static BOOL RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param);
    static VOID TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param);

    BOOL RecvUDPPacketProcess(UDP_PACKET* Packet);
    BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet);
    VOID TCPEndProcess();

    VOID TCPListeningEventProcess();
    VOID UDPListeningEventProccess();
    VOID UDPPunchEventProcess();
};

#endif