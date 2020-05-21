#pragma once

#ifndef __P2P_SERVER_H__
#define __P2P_SERVER_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "UDPBase.h"
#include "TCPServer.h"
#include "P2PConnection.h"
#include <map>

class CP2PServer : public CBaseObject
{
public:
	CP2PServer(WORD TCPPort);
    ~CP2PServer();

    BOOL Init();
    VOID Done();

private:
	static BOOL RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param);
    static BOOL RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param);
    static BOOL TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param);

    static BOOL AcceptTCPSocketProcessDelegate(SOCKET s, CBaseObject* Param);

    BOOL RecvUDPPacketProcess(UDP_PACKET* Packet);
    BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet);
    BOOL AcceptTCPSocketProcess(SOCKET s);
    
    CTCPServer* GetTCPServer(DWORD tcpip);
    DWORD CreateTCPID(CTCPServer* server);

    CP2PConnection* FindP2PConnection(CHAR* Keyword);
    VOID CreateP2PConnection(TCP_WAIT_PACKET* Packet, CTCPServer* Server);
    
    std::list<CP2PConnection*> m_P2PConnectList;
    std::map<DWORD, CTCPServer*> m_TCPList;

    DWORD m_dwTCPid;

    CRITICAL_SECTION m_csTCPList;
    CRITICAL_SECTION m_csP2PConnection;

    WORD m_dwTCPPort;
    WORD m_dwUDPPort;

    CUDPBase* m_pUDP;
    CTCPService* m_pTCPService;

    SOCKET m_hListenSock;

};

#endif

