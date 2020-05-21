#include "stdafx.h"
#include "P2PServer.h"
#include "P2PPacket.h"

CP2PServer::CP2PServer(WORD TcpPort) :	CBaseObject()
{
    m_dwTCPPort = TcpPort;
    m_pUDP = new CUDPBase();
    m_pTCPService = new CTCPService(m_dwTCPPort);

    InitializeCriticalSection(&m_csTCPList);
    InitializeCriticalSection(&m_csP2PConnection);
}

CP2PServer::~CP2PServer()
{
    if (m_pUDP)
    {
        m_pUDP->Release();
        m_pUDP = NULL;
    }

    if (m_pTCPService)
    {
        m_pTCPService->Release();
        m_pTCPService = NULL;
    }

    DeleteCriticalSection(&m_csTCPList);
    DeleteCriticalSection(&m_csP2PConnection);
}

BOOL CP2PServer::Init()
{
    m_dwUDPPort = UDP_PORT_SERVER_BASE;
    m_pUDP->RegisterRecvProcess(CP2PServer::RecvUDPPacketProcessDelegate, this);

    while (TRUE)
    {
        if (!m_pUDP->Init(m_dwUDPPort))
        {
            m_dwUDPPort++;
        }
        else
        {
            DBG("UDP启动监听成功，本地端口：%d\n", m_dwUDPPort);
            break;
        }
    }

    m_pTCPService->RegisterSocketAcceptProcess(CP2PServer::AcceptTCPSocketProcessDelegate, this);
    m_pTCPService->Init();
}

BOOL CP2PServer::AcceptTCPSocketProcessDelegate(SOCKET s, CBaseObject* Param)
{
    BOOL Ret = FALSE;
    CP2PServer* Server = dynamic_cast<CP2PServer*>(Param);

    if (Server)
    {
        Ret = Server->AcceptTCPSocketProcess(s);
    }

    return Ret;
}

BOOL CP2PServer::RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param)
{
    BOOL Ret = FALSE;
    CP2PServer* Server = dynamic_cast<CP2PServer*>(Param);

    if (Server)
    {
        Ret = Server->RecvUDPPacketProcess(Packet);
    }

    return Ret;
}

BOOL CP2PServer::RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param)
{
    BOOL Ret = FALSE;
    CP2PServer* Server = dynamic_cast<CP2PServer*>(Param);

    if (Server)
    {
        Ret = Server->RecvTCPPacketProcess(Packet);
    }

    return Ret;
}

BOOL CP2PServer::AcceptTCPSocketProcess(SOCKET s)
{
    CTCPServer* tcp = new CTCPServer(s, m_dwTCPPort);
    if (tcp)
    {
        tcp->RegisterRecvProcess(CP2PServer::RecvTCPPacketProcessDelegate, this);
        if (tcp->Init())
        {
            DWORD tcpid = CreateTCPID(tcp);
            BASE_PACKET_T* Packet = CreateTCPInit(tcpid);
            tcp->SendPacket(Packet);
        }

        tcp->Release();
    }
}

CTCPServer* CP2PServer::GetTCPServer(DWORD tcpip)
{
    CTCPServer* server = NULL;

    EnterCriticalSection(&m_csTCPList);
    if (m_TCPList.find(tcpip) != m_TCPList.end())
    {
        server = m_TCPList[tcpip];
        server->AddRef();
    }
    LeaveCriticalSection(&m_csTCPList);

    return server;
}

DWORD CP2PServer::CreateTCPID(CTCPServer* tcp)
{
    DWORD tcpid = 0;
    EnterCriticalSection(&m_csTCPList);

    while (TRUE)
    {
        if (m_TCPList.find(m_dwTCPid) == m_TCPList.end())
        {
            m_TCPList[m_dwTCPid] = tcp;
            tcp->AddRef();

            tcpid = m_dwTCPid;
            m_dwTCPid++;
            break;
        }
        else
        {
            m_dwTCPid++;
        }
    }

    LeaveCriticalSection(&m_csTCPList);

    return tcpid;
}

BOOL CP2PServer::RecvTCPPacketProcess(BASE_PACKET_T* Packet)
{
    BOOL Ret = TRUE;
    switch (Packet->Type)
    {
        case TPT_WAITING:
        {
            TCP_WAIT_PACKET* Data = (TCP_WAIT_PACKET*)Packet->Data;
            CTCPServer* Server = GetTCPServer(Data->tcpid);

            DWORD Result = -1;
            if (Server)
            {
                CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
                if (p2p == NULL)
                {
                    CreateP2PConnection(Data, Server);
                    Result = 0;
                }
                else
                {
                    p2p->Release();
                }

                BASE_PACKET_T* Reply = CreateTCPResult(TPT_WAIT_RESULT, Data->tcpid, Result);
                Server->SendPacket(Reply);
                Server->Release();
            }

            break;
        }
        case TPT_CONNECT:
        {
            TCP_CONN_PACKET* Data = (TCP_CONN_PACKET*)Packet->Data;
            CTCPServer* Server = GetTCPServer(Data->tcpid);

            DWORD Result = -1;
            if (Server)
            {
                CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
                if (p2p != NULL)
                {
                    if (p2p->TCPConnected(Data, Server))
                    {
                        Result = 0;
                    }
                    p2p->Release();
                }

                if (Result != 0)
                {
                    Packet = CreateTCPResult(TPT_CONNECT_RESULT, Data->tcpid, Result);
                    Server->SendPacket(Packet);
                }

                Server->Release();
            }

            break;
        }
        default:
        {
            break;
        }
    }
    return Ret;
}

BOOL CP2PServer::RecvUDPPacketProcess(UDP_PACKET* Packet)
{
	switch (Packet->BasePacket.type)
	{
        case UPT_WAITING:
        {
            UDP_WAIT_PACKET* Data = (UDP_WAIT_PACKET*)Packet->BasePacket.data;

            CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
            if (p2p != NULL)
            {
                p2p->UDPListening(&Packet->PacketInfo);
                p2p->Release();
            }

            break;
        }

		case UPT_CONNECT:
		{
            UDP_WAIT_PACKET* Data = (UDP_WAIT_PACKET*)Packet->BasePacket.data;

            CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
            if (p2p != NULL)
            {
                p2p->UDPConnected(&Packet->PacketInfo);
                p2p->Release();
            }
			break;
		}

        default:
        {
            break;
        }
	}
}

