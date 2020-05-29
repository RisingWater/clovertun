#pragma once

#ifndef __P2P_SERVER_H__
#define __P2P_SERVER_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include <map>

class CTCPServer;
class CTCPService;
class CP2PConnection;
class CUDPBase;

class CP2PServer : public CBaseObject
{
public:
	CP2PServer(WORD TCPPort);
    virtual ~CP2PServer();

    BOOL Init();
    VOID Done();

private:
	static BOOL RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param);
    static BOOL RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param);
    static VOID TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param);

    static BOOL AcceptTCPSocketProcessDelegate(SOCKET s, CBaseObject* Param);

    BOOL RecvUDPPacketProcess(UDP_PACKET* Packet);
    BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet);

    VOID TCPEndProcess(CTCPBase* tcp);
    BOOL AcceptTCPSocketProcess(SOCKET s);
    
    CTCPServer* GetTCPServer(DWORD tcpip);
    DWORD CreateTCPID(CTCPServer* server);
    VOID RemoveTCPID(CTCPServer* server);

    CP2PConnection* FindP2PConnection(CHAR* Keyword);
    CP2PConnection* FindP2PConnection(DWORD Peerid);
    VOID CreateP2PConnection(TCP_WAIT_PACKET* Packet, CTCPServer* Server);
    
    std::map<DWORD, CP2PConnection*> m_P2PConnectList;
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

