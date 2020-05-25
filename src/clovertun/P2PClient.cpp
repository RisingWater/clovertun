#include "stdafx.h"
#include "P2PClient.h"

CP2PClient::CP2PClient(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort)
{
    strcpy(m_szName, ClientName);
    strcpy(m_szKeyword, Keyword);
    strcpy(m_szServerIP, ServerIP);
    m_dwServerTCPPort = ServerTCPPort;
    m_dwServerUDPPort = 0;
    m_dwUDPPort = 0;
    m_dwTCPid = 0;
    m_dwPeerid = 0;

    m_dwErrorCode = P2P_ERROR_NONE;
    m_eStatus = P2P_STATUS_NONE;

    m_pUDP = new CUDPBase();
    m_pTCP = new CTCPClient(m_szServerIP, m_dwServerTCPPort);
    
    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hStatusChange = CreateEvent(NULL, TRUE, FALSE, NULL);

    memset(&m_stRemoteClientInfo, 0, sizeof(CLIENT_INFO));
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
