#include "stdafx.h"
#include "UDPClient.h"

#ifdef WIN32

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

#endif

CUDPClient::CUDPClient(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerPort, ClientRoleType Type) : CUDPBase()
{
    strcpy(m_szName, ClientName);
    strcpy(m_szKeyword, Keyword);
    strcpy(m_szServerIP, ServerIP);
    m_dwServerPort = ServerPort;
    m_eRole = Type;

    m_hConnectServerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    memset(&m_stRemoteClientInfo, 0, sizeof(CLIENT_INFO));

#ifdef WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(m_hSock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
#endif

    //int reuseaddr = 1;
    //setsockopt(m_hSock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr, sizeof(int));

    m_dwLocalPort = UDP_PORT_BASE;
    while (TRUE)
    {
        struct sockaddr_in bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;
        bindAddr.sin_family = AF_INET;
        bindAddr.sin_port = htons(m_dwLocalPort);

        int res = bind(m_hSock, (sockaddr*)&bindAddr, sizeof(bindAddr));

        if (res < 0)
        {
            m_dwLocalPort++;
        }
        else
        {
            DBG("客户端启动成功，本地端口：%d\n", m_dwLocalPort);
            break;
        }
    }
}

CUDPClient::~CUDPClient()
{
    if (m_hConnectServerEvent)
    {
        CloseHandle(m_hConnectServerEvent);
    }
}

VOID CUDPClient::Connect()
{
    UDP_PACKET Packet;

    memset(&Packet, 0, sizeof(UDP_PACKET));
    InetPton(AF_INET, m_szServerIP, &Packet.PacketInfo.ipaddr);
    Packet.PacketInfo.port = htons(m_dwServerPort);

    if (m_eRole == CRT_SERVER)
    {
        Packet.BasePacket.type = UPT_WAITING;
    }
    else if (m_eRole == CRT_CLIENT)
    {
        Packet.BasePacket.type = UPT_CONNECT;
    }

    Packet.BasePacket.length = sizeof(CONNECT_PACKET_DATA);

    CONNECT_PACKET_DATA* ConnectData = (CONNECT_PACKET_DATA*)Packet.BasePacket.data;

    strncpy((char*)ConnectData->keyword, m_szKeyword, 32);
    strncpy((char*)ConnectData->clientName, m_szName, 32);

    SendPacket(Packet);
}

VOID CUDPClient::RecvPacketProcess(UDP_PACKET Packet)
{
    switch (Packet.BasePacket.type)
    {
        case UPT_SERVER_RESPONSE:
        {
			UDP_PACKET data;
			const char* handshakeText = "hello\n";

            DBG("recv server response pkt\n");
            SetEvent(m_hConnectServerEvent);
			memcpy(&data.BasePacket.data, handshakeText, sizeof(*handshakeText));
			data.BasePacket.length = sizeof(*handshakeText);
			data.BasePacket.type = UPT_HANDSHAKE;
            CLIENT_INFO* Info = (CLIENT_INFO*)Packet.BasePacket.data;
			memcpy(&data.PacketInfo, Info, sizeof(CLIENT_INFO));

			SendPacket(data);
            break;
        }
		case UPT_HANDSHAKE:
		case UPT_KEEPALIVE:
		{
			UDP_PACKET data;

            DBG("recv keepalive pkt\n");
			const char* handshakeText = "hello\n";
			memcpy(&data.BasePacket.data, handshakeText, sizeof(*handshakeText));
			data.BasePacket.length = sizeof(*handshakeText);
			data.BasePacket.type = UPT_KEEPALIVE;
			memcpy(&data.PacketInfo, &Packet.PacketInfo, sizeof(CLIENT_INFO));

			SendPacket(data);
            Sleep(1000);
			break;
		}
    }
}


