#include "stdafx.h"
#include "P2PServer.h"
#include "P2PPacket.h"

CP2PServer::CP2PServer(WORD TcpPort) :	CBaseObject()
{
    m_dwTCPPort = TcpPort;
    m_pUDP = new CUDPBase();
    m_pTCPService = new CTCPService(m_dwTCPPort);
    m_dwTCPid = 1;

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
            m_pUDP->Start();
            DBG_INFO("UDP Listening at Port %d\r\n", m_dwUDPPort);
            break;
        }
    }

    m_pTCPService->RegisterSocketAcceptProcess(CP2PServer::AcceptTCPSocketProcessDelegate, this);
    m_pTCPService->Init();

    return TRUE;
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
    UNREFERENCED_PARAMETER(udp);

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
    UNREFERENCED_PARAMETER(tcp);

    BOOL Ret = FALSE;
    CP2PServer* Server = dynamic_cast<CP2PServer*>(Param);

    if (Server)
    {
        Ret = Server->RecvTCPPacketProcess(Packet);
    }

    return Ret;
}

VOID CP2PServer::TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param)
{
    CP2PServer* Server = dynamic_cast<CP2PServer*>(Param);

    if (Server)
    {
        Server->AddRef();
        Server->TCPEndProcess(tcp);
        Server->Release();
    }

    return;
}

BOOL CP2PServer::AcceptTCPSocketProcess(SOCKET s)
{
    CTCPServer* tcp = new CTCPServer(s, m_dwTCPPort);
    if (tcp)
    {
        tcp->RegisterRecvProcess(CP2PServer::RecvTCPPacketProcessDelegate, this);
        tcp->RegisterEndProcess(CP2PServer::TCPEndProcessDelegate, this);
        if (tcp->Init())
        {
            DWORD tcpid = CreateTCPID(tcp);
            
            DBG_TRACE("TCP[%d] connected\r\n", tcpid);
            BASE_PACKET_T* Packet = CreateTCPInitPkt(tcpid, m_dwUDPPort);
            tcp->SendPacket(Packet);
        }

        tcp->Release();
    }

    return TRUE;
}

VOID CP2PServer::TCPEndProcess(CTCPBase* tcp)
{
    CTCPServer* server = dynamic_cast<CTCPServer*>(tcp);
    std::map<DWORD, CP2PConnection*>::iterator Itor;
    
    EnterCriticalSection(&m_csP2PConnection);

    for (Itor = m_P2PConnectList.begin(); Itor != m_P2PConnectList.end(); Itor++)
    {
        CP2PConnection* Conn = Itor->second;
        if (Conn->IsConnTCPServer(server))
        {
            Conn->ConnTCPDisconnect();
            break;
        }

        if (Conn->IsMainTCPServer(server))
        {
            Conn->MainTCPDisconnect();
            m_P2PConnectList.erase(Itor);
            Conn->Release();
            break;
        }
    }

    LeaveCriticalSection(&m_csP2PConnection);

    RemoveTCPID(server);
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

VOID CP2PServer::RemoveTCPID(CTCPServer* server)
{
    EnterCriticalSection(&m_csTCPList);

    std::map<DWORD, CTCPServer*>::iterator TcpItor;
    for (TcpItor = m_TCPList.begin(); TcpItor != m_TCPList.end(); TcpItor++)
    {
        if (TcpItor->second == server)
        {
            DBG_TRACE("TCP[%d] disconnected\r\n", TcpItor->first);
            m_TCPList.erase(TcpItor);
            server->RegisterRecvProcess(NULL, NULL);
            server->RegisterEndProcess(NULL, NULL);
            server->Release();
            break;
        }
    }

    LeaveCriticalSection(&m_csTCPList);
}

CP2PConnection* CP2PServer::FindP2PConnection(CHAR* Keyword)
{
    CP2PConnection* Ret = NULL;
    std::map<DWORD, CP2PConnection*>::iterator Itor;
    EnterCriticalSection(&m_csP2PConnection);

    for (Itor = m_P2PConnectList.begin(); Itor != m_P2PConnectList.end(); Itor++)
    {
        CP2PConnection* Conn = Itor->second;
        if (strcmp(Conn->GetKeyword(), Keyword) == 0)
        {
            Ret = Conn;
            Ret->AddRef();
        }
    }

    LeaveCriticalSection(&m_csP2PConnection);

    return Ret;
}

CP2PConnection* CP2PServer::FindP2PConnection(DWORD Peerid)
{
    CP2PConnection* Ret = NULL;
    std::map<DWORD, CP2PConnection*>::iterator Itor;
    EnterCriticalSection(&m_csP2PConnection);

    Itor = m_P2PConnectList.find(Peerid);
    if (Itor != m_P2PConnectList.end())
    {
        Ret = Itor->second;
        Ret->AddRef();
    }

    LeaveCriticalSection(&m_csP2PConnection);

    return Ret;
}

VOID CP2PServer::CreateP2PConnection(TCP_WAIT_PACKET* Packet, CTCPServer* Server)
{
    CP2PConnection* Conn = new CP2PConnection(Packet, Server);
    EnterCriticalSection(&m_csP2PConnection);
    m_P2PConnectList[Packet->tcpid] = Conn;
    LeaveCriticalSection(&m_csP2PConnection);
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

            DBG_TRACE("Recv %s packet from tcp[%d]\n", TCPTypeToString(TPT_WAITING), Data->tcpid);

            DWORD Result = P2P_ERROR_UNKNOW;
            if (Server)
            {
                EnterCriticalSection(&m_csP2PConnection);
                CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
                if (p2p == NULL)
                {
                    CreateP2PConnection(Data, Server);
                    Result = P2P_ERROR_NONE;
                }
                else
                {
                    DBG_ERROR("keyword %s is already register to server, it duplicate!\r\n", Data->keyword);
                    Result = P2P_KEYWORD_DUPLICATE;
                    p2p->Release();
                }
                LeaveCriticalSection(&m_csP2PConnection);

                BASE_PACKET_T* Reply = CreateTCPResultPkt(TPT_WAIT_RESULT, Data->tcpid, Result);
                Server->SendPacket(Reply);
                Server->Release();
            }

            break;
        }
        case TPT_CONNECT:
        {
            TCP_CONN_PACKET* Data = (TCP_CONN_PACKET*)Packet->Data;
            CTCPServer* Server = GetTCPServer(Data->tcpid);

            DBG_TRACE("Recv %s packet from tcp[%d]\n", TCPTypeToString(TPT_WAITING), Data->tcpid);

            DWORD Result = P2P_ERROR_UNKNOW;
            if (Server)
            {
                CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
                if (p2p != NULL)
                {
                    Result = p2p->TCPConnected(Data, Server);
                    p2p->Release();
                }
                else
                {
                    DBG_ERROR("keyword %s is unregistered in server, it's not found\r\n", Data->keyword);
                    Result = P2P_KEYWORD_NOT_FOUND;
                }

                if (Result != P2P_ERROR_NONE)
                {
                    Packet = CreateTCPResultPkt(TPT_CONNECT_RESULT, Data->tcpid, Result);
                    Server->SendPacket(Packet);
                }

                Server->Release();
            }

            break;
        }

        case TPT_P2P_RESULT:
        {
            P2P_RESULT_PACKET* Data = (P2P_RESULT_PACKET*)Packet->Data;
            CTCPServer* Server = GetTCPServer(Data->tcpid);

            DBG_TRACE("Recv %s packet from tcp[%d]\n", TCPTypeToString(TPT_P2P_RESULT), Data->tcpid);

            DWORD Result = P2P_ERROR_UNKNOW;
            if (Server)
            {
                CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
                if (p2p != NULL)
                {
                    Result = p2p->SetP2POK(Server);
                    p2p->Release();
                }
                
                Server->Release();
            }
            break;
        }

        case TPT_PROXY_REQUEST:
        {
            TCP_PROXY_REQUEST_PACKET* Data = (TCP_PROXY_REQUEST_PACKET*)Packet->Data;
            CTCPServer* Server = GetTCPServer(Data->tcpid);

            DBG_TRACE("Recv %s packet from tcp[%d]\n", TCPTypeToString(TPT_WAITING), Data->tcpid);

            DWORD Result = P2P_ERROR_UNKNOW;
            if (Server)
            {
                CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->keyword);
                if (p2p != NULL)
                {
                    Result = p2p->TCPProxyRequest();
                    p2p->Release();
                }
                else
                {
                    DBG_ERROR("keyword %s is unregistered in server, it's not found\r\n", Data->keyword);
                    Result = P2P_KEYWORD_NOT_FOUND;
                }
             
                if (Result != P2P_ERROR_NONE)
                {
                    BASE_PACKET_T* Reply = CreateTCPProxyResultPkt(Data->tcpid, 0, Result);
                    Server->SendPacket(Reply);
                }

                Server->Release();
            }

            break;
        }

        case TPT_DATA_PROXY:
        {
            TCP_PROXY_DATA* Data = (TCP_PROXY_DATA*)Packet->Data;
            CP2PConnection* p2p = FindP2PConnection((CHAR*)Data->Peerid);
            if (p2p != NULL)
            {
                p2p->TCPDataProxy(Data, Packet->Length - BASE_PACKET_HEADER_LEN);
                p2p->Release();
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

    return TRUE;
}

