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

    m_pUDP = new CUDPBase();
    m_pTCP = new CTCPClient(m_szServerIP, m_dwServerTCPPort);
    m_pKCP = NULL;
    
    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hStatusChange = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hConnectedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_pfnRecvFunc = NULL;
    m_pParam = NULL;

    m_eType = PCT_NONE;

    memset(&m_stRemoteClientInfo, 0, sizeof(CLIENT_INFO));
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

    DeleteCriticalSection(&m_csLock);
}

BOOL CP2PClient::Init()
{
    if (!m_pTCP->Init())
    {
        DBG_ERROR("TCP connect failed\r\n");
        return FALSE;
    }

    DBG_INFO("TCP connect ok\r\n");

    m_dwUDPPort = UDP_PORT_SERVER_BASE;

    while (TRUE)
    {
        if (!m_pUDP->Init(m_dwUDPPort))
        {
            m_dwUDPPort++;
        }
        else
        {
            DBG_INFO("UDP Start at Port %d\r\n", m_dwUDPPort);
            break;
        }
    }

    return TRUE;
}

VOID CP2PClient::Done()
{
    m_pTCP->RegisterRecvProcess(NULL, NULL);
    m_pTCP->RegisterEndProcess(NULL, NULL);
    m_pUDP->RegisterRecvProcess(NULL, NULL);

    m_pTCP->Done();
    m_pUDP->Done();

    if (m_pKCP)
    {
        m_pKCP->RegisterRecvProcess(NULL, NULL);
        m_pKCP->Done();
    }
}

P2P_CLIENT_TYPE CP2PClient::GetClientType()
{
    return m_eClientType;
}

VOID CP2PClient::SendUDPToServer(BOOL IsHost)
{
    UDP_PACKET* Packet = (UDP_PACKET*)malloc(sizeof(UDP_PACKET));
    memset(Packet, 0, sizeof(UDP_PACKET));
    InetPton(AF_INET, m_szServerIP, &Packet->PacketInfo.ipaddr);
    Packet->PacketInfo.port = htons(m_dwServerUDPPort);
    
    if (IsHost)
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
    
    m_pUDP->SendPacket(Packet);
}

VOID CP2PClient::SendUDPToPeer(DWORD Type)
{
    UDP_PACKET* Packet = (UDP_PACKET*)malloc(sizeof(UDP_PACKET));
    memset(Packet, 0, sizeof(UDP_PACKET));
    memcpy(&Packet->PacketInfo, &m_stRemoteClientInfo, sizeof(CLIENT_INFO));
   
    Packet->BasePacket.type = Type;
    Packet->BasePacket.length = sizeof(DWORD);

    memcpy(&Packet->BasePacket.data, &m_dwPeerid ,sizeof(DWORD));
    
    m_pUDP->SendPacket(Packet);
}

VOID CP2PClient::SendPacket(PBYTE Data, DWORD Len)
{
    if (m_eType == PCT_NONE)
    {
        return;
    }
    else
    {
        if (m_eType == PCT_KCP)
        {
            m_pKCP->SendPacket(Data, Len);
        }
        else if (m_eType == PCT_TCP_RELAY)
        {
            BASE_PACKET_T* Packet = CreateTCPProxyData(m_dwPeerid, m_eClientType == P2P_CLIENT_HOST ? TRUE : FALSE, Data, Len);
            m_pTCP->SendPacket(Packet);
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

VOID CP2PClient::UDPConnectEventProcess()
{
    DBG_TRACE("UDP Connect ok, start kcp ...\r\n");

    m_pUDP->Done();
    m_pKCP = new CKCPClient(m_pUDP->GetSocket(), m_dwPeerid, m_stRemoteClientInfo);
    m_pKCP->RegisterRecvProcess(CP2PClient::KCPRecvPacketProcessDelegate, this);
    m_pKCP->Init();

    m_eType = PCT_KCP;

    SetEvent(m_hConnectedEvent);
}

VOID CP2PClient::TCPProxyEventProcess()
{
    DBG_TRACE("TCP Proxy start ...\r\n");
    m_pUDP->Done();

    m_eType = PCT_TCP_RELAY;

    SetEvent(m_hConnectedEvent);
}

BOOL CP2PClient::KCPRecvPacketProcessDelegate(PBYTE Data, DWORD Length, CKCPClient* tcp, CBaseObject* Param)
{
    BOOL Ret = TRUE;
    CP2PClient* kcp = dynamic_cast<CP2PClient*>(Param);

    if (kcp)
    {
        Ret = kcp->KCPRecvPacketProcess(Data, Length, tcp);
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

BOOL CP2PClient::KCPRecvPacketProcess(PBYTE Data, DWORD Length, CKCPClient* tcp)
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
