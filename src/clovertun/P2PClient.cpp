#include "stdafx.h"
#include "P2PClient.h"
#include "P2PPacket.h"

CP2PClient::CP2PClient(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort, P2P_CLIENT_TYPE type)
{
    strcpy(m_szName, ClientName);
    strcpy(m_szKeyword, Keyword);
    strcpy(m_szServerIP, ServerIP);
    m_dwServerTCPPort = ServerTCPPort;
    m_dwServerUDPPort = 0;
    m_dwUDPPort = 0;
    m_dwTCPid = 0;
    m_dwPeerid = 0;

    m_eClientType = type;
    
    m_dwErrorCode = P2P_ERROR_NONE;
    m_eStatus = P2P_STATUS_NONE;

    m_pUDP = NULL;
    m_pTCP = NULL;
    m_pENet = NULL;
    
    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hStatusChange = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hConnectedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_pfnRecvFunc = NULL;
    m_pParam = NULL;

    m_eType = PCT_NONE;

    memset(&m_stRemoteClientInfo, 0, sizeof(CLIENT_INFO));

    InitializeCriticalSection(&m_csUDPLock);
    InitializeCriticalSection(&m_csTCPLock);
    InitializeCriticalSection(&m_csENetLock);
    InitializeCriticalSection(&m_csLock);
}

CP2PClient::~CP2PClient()
{
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    if (m_hStatusChange)
    {
        CloseHandle(m_hStatusChange);
    }

    if (m_hConnectedEvent)
    {
        CloseHandle(m_hConnectedEvent);
    }

    DeleteCriticalSection(&m_csUDPLock);
    DeleteCriticalSection(&m_csTCPLock);
    DeleteCriticalSection(&m_csENetLock);
    DeleteCriticalSection(&m_csLock);
}

BOOL CP2PClient::Init()
{
    EnterCriticalSection(&m_csTCPLock);

    m_pTCP = new CTCPClient(m_szServerIP, m_dwServerTCPPort);
    m_pTCP->RegisterRecvProcess(CP2PClient::RecvTCPPacketProcessDelegate, this);
    m_pTCP->RegisterEndProcess(CP2PClient::TCPEndProcessDelegate, this);

    if (!m_pTCP->Init())
    {
        LeaveCriticalSection(&m_csTCPLock);
        DBG_ERROR("TCP connect failed\r\n");
        return FALSE;
    }

    DBG_INFO("TCP connect ok\r\n");

    LeaveCriticalSection(&m_csTCPLock);

    EnterCriticalSection(&m_csUDPLock);

    m_dwUDPPort = UDP_PORT_SERVER_BASE;

    m_pUDP = new CUDPBase();
    m_pUDP->RegisterRecvProcess(CP2PClient::RecvUDPPacketProcessDelegate, this);

    while (TRUE)
    {
        if (!m_pUDP->Init(m_dwUDPPort))
        {
            m_dwUDPPort++;
        }
        else
        {
            m_pUDP->Start();
            DBG_INFO("UDP Start at Port %d\r\n", m_dwUDPPort);
            break;
        }
    }

    LeaveCriticalSection(&m_csUDPLock);

    return TRUE;
}

VOID CP2PClient::Done()
{
    EnterCriticalSection(&m_csTCPLock);
    if (m_pTCP)
    {
        m_pTCP->Done();
        m_pTCP->Release();
        m_pTCP = NULL;
    }
    LeaveCriticalSection(&m_csTCPLock);

    EnterCriticalSection(&m_csUDPLock);
    if (m_pUDP)
    {
        m_pUDP->Stop();
        m_pUDP->Release();
        m_pUDP = NULL;
    }
    LeaveCriticalSection(&m_csUDPLock);

    EnterCriticalSection(&m_csENetLock);
    if (m_pENet)
    {
        m_pENet->RegisterRecvProcess(NULL, NULL);
        m_pENet->Done();
        m_pENet = NULL;
    }
    LeaveCriticalSection(&m_csENetLock);
}

P2P_CLIENT_TYPE CP2PClient::GetClientType()
{
    return m_eClientType;
}

VOID CP2PClient::SendUDPToServer()
{
    UDP_PACKET* Packet = (UDP_PACKET*)malloc(sizeof(UDP_PACKET));
    memset(Packet, 0, sizeof(UDP_PACKET));
    Packet->PacketInfo.ipaddr.S_un.S_addr = inet_addr(m_szServerIP);
    Packet->PacketInfo.port = htons(m_dwServerUDPPort);
    
    if (m_eClientType == P2P_CLIENT_HOST)
    {
        Packet->BasePacket.type = UPT_WAITING;
        Packet->BasePacket.length = sizeof(UDP_WAIT_PACKET);
    }
    else
    {
        Packet->BasePacket.type = UPT_CONNECT;
        Packet->BasePacket.length = sizeof(UDP_CONN_PACKET);
    }
    
    UDP_WAIT_PACKET* ConnectData = (UDP_WAIT_PACKET*)Packet->BasePacket.data;
    
    strncpy((char*)ConnectData->keyword, m_szKeyword, 32);
    strncpy((char*)ConnectData->clientName, m_szName, 32);
    
    EnterCriticalSection(&m_csUDPLock);
    if (m_pUDP)
    {
        m_pUDP->SendPacket(Packet);
    }
    LeaveCriticalSection(&m_csUDPLock);
}

VOID CP2PClient::SendUDPToPeer(DWORD Type)
{
    UDP_PACKET* Packet = (UDP_PACKET*)malloc(sizeof(UDP_PACKET));
    memset(Packet, 0, sizeof(UDP_PACKET));
    memcpy(&Packet->PacketInfo, &m_stRemoteClientInfo, sizeof(CLIENT_INFO));
   
    Packet->BasePacket.type = Type;
    Packet->BasePacket.length = sizeof(DWORD);

    memcpy(&Packet->BasePacket.data, &m_dwPeerid ,sizeof(DWORD));
    
    EnterCriticalSection(&m_csUDPLock);
    if (m_pUDP)
    {
        m_pUDP->SendPacket(Packet);
    }
    LeaveCriticalSection(&m_csUDPLock);
}

VOID CP2PClient::SendPacket(PBYTE Data, DWORD Len)
{
    if (m_eType == PCT_NONE)
    {
        return;
    }
    else
    {
        if (m_eType == PCT_ENET)
        {
            EnterCriticalSection(&m_csENetLock);
            if (m_pENet)
            {
                m_pENet->SendPacket(Data, Len);
            }
            LeaveCriticalSection(&m_csENetLock);
        }
        else if (m_eType == PCT_TCP_RELAY)
        {
            BASE_PACKET_T* Packet = CreateTCPProxyData(m_dwPeerid, m_eClientType == P2P_CLIENT_HOST ? TRUE : FALSE, Data, Len);

            EnterCriticalSection(&m_csTCPLock);
            if (m_pTCP)
            {
                m_pTCP->SendPacket(Packet);
            }
            LeaveCriticalSection(&m_csTCPLock);
        }
    }

    return;
}

VOID CP2PClient::RegisterRecvPacketProcess(_P2PRecvPacketProcess Func, CBaseObject* Param)
{
    CBaseObject* pOldParam = NULL;

    EnterCriticalSection(&m_csLock);
	m_pfnRecvFunc = Func;

    pOldParam = m_pParam;
    m_pParam = Param;
    if (m_pParam)
    {
        m_pParam->AddRef();
    }
    LeaveCriticalSection(&m_csLock);

    if (pOldParam)
    {
        pOldParam->Release();
    } 
}

VOID CP2PClient::UDPPunchEventProcess()
{
    DWORD Ret = 0;
    DWORD Retry = 0;
    BOOL timeout = FALSE;

    HANDLE h[2] = {
        m_hStatusChange,
        m_hStopEvent,
    };

    while (TRUE)
    {
        //send udp to peer
        DBG_TRACE("Send %s Packet to other peer\r\n", UDPTypeToString(UPT_HANDSHAKE));
        SendUDPToPeer(UPT_HANDSHAKE);

        //wait 500ms to retry
        Ret = WaitForMultipleObjects(2, h, FALSE, 200);
        if (Ret != WAIT_TIMEOUT)
        {
            break;
        }
        else
        {
            Retry++;
        }

        //wait for total 2s
        if (Retry >= 10)
        {
            timeout = TRUE;
            break;
        }
    }

    if (Ret != WAIT_OBJECT_0 || Ret != WAIT_TIMEOUT)
    {
        return;
    }

    if (timeout)
    {
        DBG_INFO("udp handshake failed, turn to TCP Relay\r\n");
        SetState(P2P_TCP_PROXY_REQUEST);
        BASE_PACKET_T* Packet = CreateTCPProxyRequest(m_dwTCPid, m_szKeyword, m_szName);

        EnterCriticalSection(&m_csTCPLock);
        if (m_pTCP)
        {
            m_pTCP->SendPacket(Packet);
        }
        else
        {
            free(Packet);
        }
        LeaveCriticalSection(&m_csTCPLock);
    }
}

VOID CP2PClient::UDPInfoExchangeEventProccess()
{
    DWORD Ret = 0;
    DWORD Retry = 0;
    BOOL timeout = FALSE;

    HANDLE h[2] = {
        m_hStatusChange,
        m_hStopEvent,
    };

    while (TRUE)
    {
        //send udp to server
        DBG_TRACE("Send %s Packet to Server\r\n", m_eClientType == P2P_CLIENT_HOST ? UDPTypeToString(UPT_WAITING) : UDPTypeToString(UPT_CONNECT));
        SendUDPToServer();

        //wait 500ms to retry
        Ret = WaitForMultipleObjects(2, h, FALSE, 200);
        if (Ret != WAIT_TIMEOUT)
        {
            break;
        }
        else
        {
            Retry++;
        }

        //wait for total 2s
        if (Retry >= 10)
        {
            timeout = TRUE;
            break;
        }
    }

    if (Ret != WAIT_OBJECT_0 && Ret != WAIT_TIMEOUT)
    {
        return;
    }

    if (timeout)
    {
        DBG_INFO("udp handshake failed, turn to TCP Relay\r\n");
        SetState(P2P_TCP_PROXY_REQUEST);
        BASE_PACKET_T* Packet = CreateTCPProxyRequest(m_dwTCPid, m_szKeyword, m_szName);

        EnterCriticalSection(&m_csTCPLock);
        if (m_pTCP)
        {
            m_pTCP->SendPacket(Packet);
        }
        else
        {
            free(Packet);
        }
        LeaveCriticalSection(&m_csTCPLock);
    }
}

VOID CP2PClient::P2PConnectEventProcess()
{
    DBG_TRACE("P2P Connect ok, start kcp ...\r\n");

    EnterCriticalSection(&m_csUDPLock);
    if (m_pUDP)
    {
        m_pUDP->Stop();
    }
    LeaveCriticalSection(&m_csUDPLock);

    EnterCriticalSection(&m_csENetLock);
    m_pENet = new CENetClient(m_pUDP->GetSocket(), m_dwPeerid, m_stRemoteClientInfo, m_eClientType == P2P_CLIENT_HOST);
    m_pENet->RegisterRecvProcess(CP2PClient::ENetRecvPacketProcessDelegate, this);
    m_pENet->Init();
    LeaveCriticalSection(&m_csENetLock);
    m_eType = PCT_ENET;

    SetEvent(m_hConnectedEvent);
}

VOID CP2PClient::TCPProxyEventProcess()
{
    DBG_TRACE("TCP Proxy start ...\r\n");
    EnterCriticalSection(&m_csUDPLock);
    if (m_pUDP)
    {
        m_pUDP->Stop();
    }
    LeaveCriticalSection(&m_csUDPLock);

    m_eType = PCT_TCP_RELAY;

    SetEvent(m_hConnectedEvent);
}

BOOL CP2PClient::ENetRecvPacketProcessDelegate(PBYTE Data, DWORD Length, CENetClient* tcp, CBaseObject* Param)
{
    BOOL Ret = TRUE;
    CP2PClient* kcp = dynamic_cast<CP2PClient*>(Param);

    if (kcp)
    {
        Ret = kcp->ENetRecvPacketProcess(Data, Length, tcp);
    }

    return Ret;
}

BOOL CP2PClient::TCPProxyPacketProcess(BASE_PACKET_T* Packet)
{
    BOOL Ret = TRUE;

    TCP_PROXY_DATA* Proxy = (TCP_PROXY_DATA*)Packet;

    EnterCriticalSection(&m_csLock);

    if (m_pfnRecvFunc)
    {
        Ret = m_pfnRecvFunc(Proxy->Data, Proxy->Length, this, m_pParam);
    }

    LeaveCriticalSection(&m_csLock);

    return Ret;
}

BOOL CP2PClient::ENetRecvPacketProcess(PBYTE Data, DWORD Length, CENetClient* tcp)
{
    BOOL Ret = TRUE;

    UNREFERENCED_PARAMETER(tcp);

    EnterCriticalSection(&m_csLock);

    if (m_pfnRecvFunc)
    {
        Ret = m_pfnRecvFunc(Data, Length, this, m_pParam);
    }

    LeaveCriticalSection(&m_csLock);

    return Ret;
}

BOOL CP2PClient::RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(udp);

    BOOL Ret = FALSE;
    CP2PClient* Host = dynamic_cast<CP2PClient*>(Param);

    if (Host)
    {
        Ret = Host->RecvUDPPacketProcess(Packet);
    }

    return Ret;
}

BOOL CP2PClient::RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(tcp);

    BOOL Ret = FALSE;
    CP2PClient* Host = dynamic_cast<CP2PClient*>(Param);

    if (Host)
    {
        Ret = Host->RecvTCPPacketProcess(Packet);
    }

    return Ret;
}

VOID CP2PClient::TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(tcp);

    CP2PClient* Host = dynamic_cast<CP2PClient*>(Param);

    if (Host)
    {
        Host->TCPEndProcess();
    }

    return;
}

BOOL CP2PClient::RecvUDPPacketProcess(UDP_PACKET* Packet)
{
    BOOL Ret = TRUE;
    switch (Packet->BasePacket.type)
    {
        case UPT_HANDSHAKE:
        {
            DWORD Peerid;
            memcpy(&Peerid, Packet->BasePacket.data, sizeof(DWORD));

            DBG_TRACE("Recv %s packet from %s: %d\r\n", UDPTypeToString(UPT_HANDSHAKE), inet_ntoa(Packet->PacketInfo.ipaddr), ntohs(Packet->PacketInfo.port));
            DBG_TRACE("===> Peerid %d\r\n", Peerid);
            
            if (Peerid == m_dwPeerid)
            {
                DBG_TRACE("===> Send %s Packet to other peer\r\n", UDPTypeToString(UPT_KEEPALIVE));
                SendUDPToPeer(UPT_KEEPALIVE);
            }
            else
            {
                DBG_ERROR("===> Peerid mismatch, skip this packet\r\n");
            }
            break;
        }

        case UPT_KEEPALIVE:
        {
            DWORD Peerid;
            memcpy(&Peerid, Packet->BasePacket.data, sizeof(DWORD));

            DBG_TRACE("Recv %s packet from %s: %d\r\n", UDPTypeToString(UPT_KEEPALIVE), inet_ntoa(Packet->PacketInfo.ipaddr), ntohs(Packet->PacketInfo.port));
            DBG_TRACE("===> Peerid %d\r\n", Peerid);

            if (Peerid == m_dwPeerid)
            {
                if (m_eStatus == P2P_PUNCHING)
                {
                    BASE_PACKET_T* Packet = CreateP2PSuccessPkt(m_dwTCPid, m_szKeyword);
                    EnterCriticalSection(&m_csTCPLock);
                    if (m_pTCP)
                    {
                        m_pTCP->SendPacket(Packet);
                    }
                    else
                    {
                        free(Packet);
                    }
                    LeaveCriticalSection(&m_csTCPLock);
                }
            }
            else
            {
                DBG_ERROR("===> Peerid mismatch, skip this packet\r\n");
            }
            break;
        }
    }

    return Ret;
}

VOID CP2PClient::TCPEndProcess()
{
    DBG_INFO("TCP Disconnect\r\n");
    m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
    SetEvent(m_hStopEvent);
}
