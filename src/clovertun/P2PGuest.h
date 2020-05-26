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
    BOOL StartConnect();
    VOID Clearup();
    
    static BOOL RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param);
    static BOOL RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param);
    static VOID TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param);

    BOOL RecvUDPPacketProcess(UDP_PACKET* Packet);
    BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet);
    VOID TCPEndProcess();
    
    VOID TCPConnectEventProcess();
    VOID UDPConnectEventProccess();
    VOID UDPPunchEventProcess();
};

#endif