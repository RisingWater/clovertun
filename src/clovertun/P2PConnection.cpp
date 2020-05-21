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
    m_eStatus = TCP_LISTENING;

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

P2P_STATUS CP2PConnection::GetStatus()
{
    return m_eStatus;
}

BOOL CP2PConnection::TCPConnected(TCP_CONN_PACKET* PacketData, CTCPServer* Server)
{
    BASE_PACKET_T* Packet = NULL;

    EnterCriticalSection(&m_csLock);

    if (m_eStatus != TCP_LISTENING)
    {
        LeaveCriticalSection(&m_csLock);
        return FALSE;
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

    m_eStatus = TCP_CONNECTED;

    Packet = CreateTCPResult(TPT_CONNECT_RESULT, m_dwConnTCPID, 0);
    m_pConnTCPServer->SendPacket(Packet);

    Packet = CreateTCPStartUDP(m_dwMainTCPID, m_szKeyword);
    m_pMainTCPServer->SendPacket(Packet);

    LeaveCriticalSection(&m_csLock);

    return TRUE;
}

BOOL CP2PConnection::UDPListening(CLIENT_INFO* ClientInfo)
{
    BASE_PACKET_T* Packet = NULL;
    EnterCriticalSection(&m_csLock);

    if (m_eStatus != TCP_CONNECTED)
    {
        LeaveCriticalSection(&m_csLock);
        return FALSE;
    }

    memcpy(&m_stMainUDPServer, ClientInfo, sizeof(CLIENT_INFO));

    m_eStatus = UDP_LISTENING;

    Packet = CreateTCPStartUDP(m_dwConnTCPID, m_szKeyword);
    m_pConnTCPServer->SendPacket(Packet);

    LeaveCriticalSection(&m_csLock);

    return TRUE;
}

BOOL CP2PConnection::UDPConnected(CLIENT_INFO* ClientInfo)
{
    BASE_PACKET_T* Packet = NULL;
    EnterCriticalSection(&m_csLock);

    if (m_eStatus != UDP_LISTENING)
    {
        LeaveCriticalSection(&m_csLock);
        return FALSE;
    }

    memcpy(&m_stConnUDPServer, ClientInfo, sizeof(CLIENT_INFO));

    m_eStatus = UDP_CONNECTED;

    Packet = CreateTCPStartUDP(m_dwConnTCPID, m_szKeyword);
    m_pConnTCPServer->SendPacket(Packet);

    m_eStatus = P2P_PUNCHING;

    LeaveCriticalSection(&m_csLock);

    return TRUE;
}