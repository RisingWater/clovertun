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
#include "ENetClient.h"

typedef enum
{
    PCT_NONE,
    PCT_ENET,
    PCT_TCP_RELAY,
} P2P_COMM_TYPE;

typedef enum {
    P2P_CLIENT_HOST,
    P2P_CLIENT_GUEST,
} P2P_CLIENT_TYPE;

class CP2PClient;

typedef BOOL (*_P2PRecvPacketProcess)(PBYTE Data, DWORD Len, CP2PClient* tcp, CBaseObject* Param);

class CP2PClient : public CBaseObject
{
public:
	CP2PClient(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort, P2P_CLIENT_TYPE Type);
	virtual ~CP2PClient();

    P2P_CLIENT_TYPE GetClientType();

    VOID SendPacket(PBYTE Data, DWORD Len);
    VOID RegisterRecvPacketProcess(_P2PRecvPacketProcess Func, CBaseObject* Param);

protected:
    BOOL Init();
    VOID Done();

    virtual BOOL RecvTCPPacketProcess(BASE_PACKET_T* Packet) = 0;
    BOOL RecvUDPPacketProcess(UDP_PACKET* Packet);
    VOID TCPEndProcess();

    static BOOL RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param);
    static BOOL RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param);
    static VOID TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param);

    VOID SendUDPToServer();
    VOID SendUDPToPeer(DWORD Type);

    VOID UDPInfoExchangeEventProccess();
    VOID UDPPunchEventProcess();
    VOID P2PConnectEventProcess();
    VOID TCPProxyEventProcess();

    BOOL TCPProxyPacketProcess(BASE_PACKET_T* Packet);
    

    static BOOL ENetRecvPacketProcessDelegate(PBYTE Data, DWORD Length, CENetClient* tcp, CBaseObject* Param);
    BOOL ENetRecvPacketProcess(PBYTE Data, DWORD Length, CENetClient* tcp);

	CHAR m_szName[32];
    CHAR m_szKeyword[32];
    CHAR m_szServerIP[32];
    WORD m_dwServerTCPPort;
    WORD m_dwServerUDPPort;
    WORD m_dwUDPPort;

    P2P_CLIENT_TYPE m_eClientType;

    DWORD m_dwTCPid;
    DWORD m_dwPeerid;
    CLIENT_INFO m_stRemoteClientInfo;

    CUDPBase* m_pUDP;
    CRITICAL_SECTION m_csUDPLock;

    CTCPClient* m_pTCP;
    CRITICAL_SECTION m_csTCPLock;

    CENetClient* m_pENet;
    CRITICAL_SECTION m_csENetLock;

    DWORD m_dwErrorCode;

    HANDLE m_hStopEvent;
    HANDLE m_hStatusChange;
    HANDLE m_hConnectedEvent;

    P2P_COMM_TYPE m_eType;
    P2P_STATUS m_eStatus;

    CRITICAL_SECTION m_csLock;
    _P2PRecvPacketProcess m_pfnRecvFunc;
    CBaseObject *m_pParam;
};

#define SetState(Status) \
    do {                                               \
        P2P_STATUS old = m_eStatus;                    \
        m_eStatus = Status;                            \
        DBG_TRACE("Status Change %s ==> %s\r\n",       \
            P2PStatusToString(old),                    \
            P2PStatusToString(m_eStatus));             \
    } while (FALSE);
    


#endif
