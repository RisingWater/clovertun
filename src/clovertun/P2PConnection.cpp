#include "stdafx.h"
#include "P2PConnection.h"
#include "P2PPacket.h"

CP2PConnection::CP2PConnection(TCP_WAIT_PACKET* PacketData, CTCPServer* Server)
{
    memcpy(m_szKeyword, PacketData->keyword, KEYWORD_SIZE);
    memcpy(m_szMainTCPName, PacketData->clientName, NAME_SIZE);
    m_pMainTCPServer = Server;
    if (m_pMainTCPServer)
    {
        m_pMainTCPServer->AddRef();
    }
    m_dwMainTCPID = PacketData->tcpid;

    m_pConnTCPServer = NULL;
    m_dwConnTCPID = 0;
    memset(m_szConnTCPName, 0, NAME_SIZE);

    memset(&m_stMainUDPServer, 0, sizeof(CLIENT_INFO));
    memset(&m_stConnUDPServer, 0, sizeof(CLIENT_INFO));
    m_eStatus = P2P_TCP_LISTENING;

    InitializeCriticalSection(&m_csLock);
}

CP2PConnection::~CP2PConnection()
{
    if (m_pMainTCPServer)
    {
        m_pMainTCPServer->Release();
        m_pMainTCPServer = NULL;
    }

    if (m_pConnTCPServer)
    {
        m_pConnTCPServer->Release();
        m_pMainTCPServer = NULL;
    }

    if (m_szKeyword)
    {
        free(m_szKeyword);
    }

    DeleteCriticalSection(&m_csLock);
}

const CHAR* CP2PConnection::GetKeyword()
{
    return m_szKeyword;
}


P2P_STATUS CP2PConnection::GetStatus()
{
    P2P_STATUS Ret;

    EnterCriticalSection(&m_csLock);

    Ret = m_eStatus;

    LeaveCriticalSection(&m_csLock);

    return Ret;
}

BOOL CP2PConnection::IsMainTCPServer(CTCPServer* tcp)
{
    BOOL Ret;

    EnterCriticalSection(&m_csLock);

    Ret = (tcp == m_pMainTCPServer);

    LeaveCriticalSection(&m_csLock);

    return Ret;
}

BOOL CP2PConnection::IsConnTCPServer(CTCPServer* tcp)
{
    BOOL Ret;

    EnterCriticalSection(&m_csLock);

    Ret = (tcp == m_pConnTCPServer);

    LeaveCriticalSection(&m_csLock);

    return Ret;
}

DWORD CP2PConnection::TCPConnected(TCP_CONN_PACKET* PacketData, CTCPServer* Server)
{
    BASE_PACKET_T* Packet = NULL;

    EnterCriticalSection(&m_csLock);

    if (m_eStatus != P2P_TCP_LISTENING)
    {
        LeaveCriticalSection(&m_csLock);
        return P2P_STATE_MISMATCH;
    }

    if (m_pConnTCPServer)
    {
        m_pConnTCPServer->Release();
        m_pConnTCPServer = NULL;
    }

    m_pConnTCPServer = Server;
    if (m_pConnTCPServer)
    {
        m_pConnTCPServer->AddRef();
    }
    memcpy(m_szConnTCPName, PacketData->clientName, NAME_SIZE);
    m_dwConnTCPID = PacketData->tcpid;

    memset(&m_stMainUDPServer, 0, sizeof(CLIENT_INFO));
    memset(&m_stConnUDPServer, 0, sizeof(CLIENT_INFO));

    m_eStatus = P2P_TCP_CONNECTED;

    Packet = CreateTCPResultPkt(TPT_CONNECT_RESULT, m_dwConnTCPID, 0);
    m_pConnTCPServer->SendPacket(Packet);

    Packet = CreateTCPStartUDPPkt(m_dwMainTCPID, m_szKeyword);
    m_pMainTCPServer->SendPacket(Packet);

    LeaveCriticalSection(&m_csLock);

    return P2P_ERROR_NONE;
}

DWORD CP2PConnection::UDPListening(CLIENT_INFO* ClientInfo)
{
    BASE_PACKET_T* Packet = NULL;
    EnterCriticalSection(&m_csLock);

    if (m_eStatus != P2P_TCP_CONNECTED)
    {
        LeaveCriticalSection(&m_csLock);
        return P2P_STATE_MISMATCH;
    }

    memcpy(&m_stMainUDPServer, ClientInfo, sizeof(CLIENT_INFO));

    m_eStatus = P2P_UDP_LISTENING;

    Packet = CreateTCPStartUDPPkt(m_dwConnTCPID, m_szKeyword);
    m_pConnTCPServer->SendPacket(Packet);

    LeaveCriticalSection(&m_csLock);

    return P2P_ERROR_NONE;
}

DWORD CP2PConnection::UDPConnected(CLIENT_INFO* ClientInfo)
{
    BASE_PACKET_T* Packet = NULL;
    EnterCriticalSection(&m_csLock);

    if (m_eStatus != P2P_UDP_LISTENING)
    {
        LeaveCriticalSection(&m_csLock);
        return P2P_STATE_MISMATCH;
    }

    memcpy(&m_stConnUDPServer, ClientInfo, sizeof(CLIENT_INFO));

    m_eStatus = P2P_UDP_CONNECTED;

    Packet = CreateTCPStartPunchingPkt(m_dwMainTCPID, m_dwMainTCPID, &m_stConnUDPServer);
    m_pMainTCPServer->SendPacket(Packet);

    Packet = CreateTCPStartPunchingPkt(m_dwConnTCPID, m_dwMainTCPID, &m_stMainUDPServer);
    m_pConnTCPServer->SendPacket(Packet);

    m_eStatus = P2P_PUNCHING;

    LeaveCriticalSection(&m_csLock);

    return P2P_ERROR_NONE;
}

VOID CP2PConnection::ConnTCPDisconnect()
{
    EnterCriticalSection(&m_csLock);

    if (m_pConnTCPServer)
    {
        m_pConnTCPServer->Release();
        m_pConnTCPServer = NULL;
    }
    m_dwConnTCPID = 0;
    memset(m_szConnTCPName, 0, NAME_SIZE);

    memset(&m_stMainUDPServer, 0, sizeof(CLIENT_INFO));
    memset(&m_stConnUDPServer, 0, sizeof(CLIENT_INFO));

    m_eStatus = P2P_TCP_LISTENING;

    BASE_PACKET_T* Reply = CreateTCPResultPkt(TPT_WAIT_RESULT, m_dwMainTCPID, 0);
    m_pMainTCPServer->SendPacket(Reply);

    LeaveCriticalSection(&m_csLock);

    return;
}

VOID CP2PConnection::MainTCPDisconnect()
{
    EnterCriticalSection(&m_csLock);

    if (m_pMainTCPServer)
    {
        m_pMainTCPServer->Release();
        m_pMainTCPServer = NULL;
    }

    m_dwMainTCPID = 0;
    memset(m_szMainTCPName, 0, NAME_SIZE);

    if (m_pConnTCPServer)
    {
        m_pConnTCPServer->Done();
        m_pConnTCPServer->Release();
        m_pMainTCPServer = NULL;
    }

    m_dwConnTCPID = 0;
    memset(m_szConnTCPName, 0, NAME_SIZE);

    memset(&m_stMainUDPServer, 0, sizeof(CLIENT_INFO));
    memset(&m_stConnUDPServer, 0, sizeof(CLIENT_INFO));

    LeaveCriticalSection(&m_csLock);
}

DWORD CP2PConnection::TCPProxyRequest()
{
    BASE_PACKET_T* Packet = NULL;

    EnterCriticalSection(&m_csLock);

    if (m_pConnTCPServer == NULL)
    {
        LeaveCriticalSection(&m_csLock);
        return P2P_PROXY_FAIL;
    }

    Packet = CreateTCPProxyResultPkt(m_dwMainTCPID, m_dwMainTCPID, P2P_ERROR_NONE);
    m_pMainTCPServer->SendPacket(Packet);
    
    Packet = CreateTCPProxyResultPkt(m_dwConnTCPID, m_dwMainTCPID, P2P_ERROR_NONE);
    m_pConnTCPServer->SendPacket(Packet);
        
    LeaveCriticalSection(&m_csLock);

    return P2P_ERROR_NONE;
}

DWORD CP2PConnection::TCPDataProxy(TCP_PROXY_DATA* Data, DWORD Length)
{
    BASE_PACKET_T* Packet = CreateTCPProxyData(Data, Length);
    CTCPServer* Server = NULL;

    EnterCriticalSection(&m_csLock);

    if (Data->Host2Guest)
    {
        Server = m_pConnTCPServer;
    }
    else
    {
        Server = m_pMainTCPServer;
    }

    LeaveCriticalSection(&m_csLock);

    if (Server)
    {
        Server->SendPacket(Packet);
        Server->Release();
    }

    return P2P_ERROR_NONE;
}