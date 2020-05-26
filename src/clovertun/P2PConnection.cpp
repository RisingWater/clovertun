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
    m_bMainUDPOK = FALSE;

    m_pConnTCPServer = NULL;
    m_dwConnTCPID = 0;
    m_bConnUDPOK = FALSE;
    memset(m_szConnTCPName, 0, NAME_SIZE);

    memset(&m_stMainUDPServer, 0, sizeof(CLIENT_INFO));
    memset(&m_stConnUDPServer, 0, sizeof(CLIENT_INFO));
    m_eStatus = P2P_TCP_LISTENING;

    InitializeCriticalSection(&m_csLock);

    DumpP2PConnection("P2PConnection Host Connected");
}

CP2PConnection::~CP2PConnection()
{
    EnterCriticalSection(&m_csLock);
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
    LeaveCriticalSection(&m_csLock);

    DeleteCriticalSection(&m_csLock);
}

void CP2PConnection::DumpP2PConnection(char* message)
{
    CHAR AddrTmp[512];
    WORD PortTmp;
    EnterCriticalSection(&m_csLock);
    if (message)
    {
        DBG_TRACE("%s\n", message);
    }
    DBG_TRACE("P2PConnection [%s] State %s\r\n", m_szKeyword, P2PStatusToString(m_eStatus));
    DBG_TRACE("===> Host[%s] tcpid %d udp\r\n", m_szMainTCPName, m_dwMainTCPID);

    memset(AddrTmp, 0, 512);
    m_pMainTCPServer->GetDstPeer(AddrTmp, 512, &PortTmp);

    DBG_TRACE("     TCPInfo %s:%d\r\n", AddrTmp, PortTmp);
    DBG_TRACE("     UDPInfo %s:%d\r\n", inet_ntoa(m_stMainUDPServer.ipaddr), ntohs(m_stMainUDPServer.port));
    DBG_TRACE("     P2P[%s] peerid: %d\r\n", m_bMainUDPOK ? "OK" : "Failed", m_dwMainTCPID);

    if (m_eStatus >= P2P_TCP_CONNECTED)
    {
        DBG_TRACE("===> Guest[%s] tcpid %d\r\n", m_szMainTCPName, m_dwMainTCPID);

        memset(AddrTmp, 0, 512);
        m_pConnTCPServer->GetDstPeer(AddrTmp, 512, &PortTmp);

        DBG_TRACE("     TCPInfo %s:%d\r\n", AddrTmp, PortTmp);
        DBG_TRACE("     UDPInfo %s:%d\r\n", inet_ntoa(m_stConnUDPServer.ipaddr), ntohs(m_stConnUDPServer.port));
        DBG_TRACE("     P2P[%s] peerid: %d\r\n", m_bConnUDPOK ? "OK" : "Failed", m_dwMainTCPID);
    }

    LeaveCriticalSection(&m_csLock);
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

    DumpP2PConnection("P2PConnection Guest Conected");

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

    DumpP2PConnection("P2PConnection Host UDP Connected");

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

    DumpP2PConnection("P2PConnection Guest UDP Connected");

    Packet = CreateTCPStartPunchingPkt(m_dwMainTCPID, m_dwMainTCPID, &m_stConnUDPServer);
    m_pMainTCPServer->SendPacket(Packet);

    Packet = CreateTCPStartPunchingPkt(m_dwConnTCPID, m_dwMainTCPID, &m_stMainUDPServer);
    m_pConnTCPServer->SendPacket(Packet);

    m_eStatus = P2P_PUNCHING;

    LeaveCriticalSection(&m_csLock);

    return P2P_ERROR_NONE;
}

DWORD CP2PConnection::SetP2POK(CTCPServer* Server)
{
    BASE_PACKET_T* Packet = NULL;
    EnterCriticalSection(&m_csLock);

    if (m_eStatus != P2P_PUNCHING)
    {
        LeaveCriticalSection(&m_csLock);
        return P2P_STATE_MISMATCH;
    }

    if (m_pMainTCPServer == Server)
    {
        DBG_TRACE("Host p2p connect ok\r\n");
        m_bMainUDPOK = TRUE;
    }

    if (m_pConnTCPServer == Server)
    {
        DBG_TRACE("Guest p2p connect ok\r\n");
        m_bConnUDPOK = TRUE;
    }

    if (m_bMainUDPOK && m_bConnUDPOK)
    {
        m_eStatus = P2P_CONNECTED;
    }

    DumpP2PConnection("P2PConnection P2P Result recviced");

    if (m_bMainUDPOK && m_bConnUDPOK)
    {
        Packet = CreateP2PSuccessPkt(m_dwMainTCPID, m_szKeyword);
        m_pMainTCPServer->SendPacket(Packet);

        Packet = CreateP2PSuccessPkt(m_dwConnTCPID, m_szKeyword);
        m_pConnTCPServer->SendPacket(Packet);
    }

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

    DumpP2PConnection("P2PConnection Guest Disconnected");

    BASE_PACKET_T* Reply = CreateTCPResultPkt(TPT_WAIT_RESULT, m_dwMainTCPID, 0);
    m_pMainTCPServer->SendPacket(Reply);

    LeaveCriticalSection(&m_csLock);

    return;
}

VOID CP2PConnection::MainTCPDisconnect()
{
    EnterCriticalSection(&m_csLock);

    DBG_TRACE("P2PConnection Host Disconnected\r\n");

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
        m_pConnTCPServer = NULL;
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

    DBG_TRACE("P2PConnection One of Peer Request Proxy\r\n");

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