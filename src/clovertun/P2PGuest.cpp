#include "stdafx.h"
#include "P2PGuest.h"
#include "P2PPacket.h"

CP2PGuest::CP2PGuest(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort)
    : CP2PClient(ClientName, Keyword, ServerIP, ServerTCPPort, P2P_CLIENT_GUEST)
{

}

CP2PGuest::~CP2PGuest()
{

}

DWORD CP2PGuest::Connect()
{
    HANDLE h[3] = {
        m_hStatusChange,
        m_hStopEvent,
        m_hConnectedEvent,
    };

    if (!StartConnect())
    {
        DBG_ERROR("Connect Failed\r\n");
        m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
        return m_dwErrorCode;
    }

    m_dwErrorCode = P2P_ERROR_NONE;

    while (TRUE)
    {
        DWORD Ret = WaitForMultipleObjects(3, h, FALSE, INFINITE);
        if (Ret != WAIT_OBJECT_0)
        {
            break;
        }

        switch (m_eStatus)
        {
            case P2P_TCP_CONNECTED:
            {
                ResetEvent(m_hStatusChange);
                TCPConnectEventProcess();
                break;
            }
            case P2P_UDP_CONNECTED:
            {
                ResetEvent(m_hStatusChange);
                UDPConnectEventProccess();
                break;
            }
            case P2P_PUNCHING:
            {
                ResetEvent(m_hStatusChange);
                UDPPunchEventProcess();
                break;
            }
            case P2P_CONNECTED:
            {
                ResetEvent(m_hStatusChange);
                UDPConnectEventProcess();
                break;
            }
            case P2P_TCP_PROXY:
            {
                ResetEvent(m_hStatusChange);
                DBG_INFO("SWITCH TO TCP RELAY\r\n");
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return m_dwErrorCode;
}

BOOL CP2PGuest::StartConnect()
{
    m_pTCP->RegisterRecvProcess(CP2PGuest::RecvTCPPacketProcessDelegate, this);
    m_pTCP->RegisterEndProcess(CP2PGuest::TCPEndProcessDelegate, this);
    m_pUDP->RegisterRecvProcess(CP2PGuest::RecvUDPPacketProcessDelegate, this);

    return Init();
}

VOID CP2PGuest::Clearup()
{
    Done();
}

VOID CP2PGuest::TCPConnectEventProcess()
{
    DBG_TRACE("Send %s Packet to Server\r\n", TCPTypeToString(TPT_CONNECT));
    BASE_PACKET_T* Packet = CreateTCPConnPkt(m_dwTCPid, m_szKeyword, m_szName);
    m_pTCP->SendPacket(Packet);
}

VOID CP2PGuest::UDPConnectEventProccess()
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
        DBG_TRACE("Send %s Packet to Server\r\n", UDPTypeToString(UPT_CONNECT));
        SendUDPToServer(FALSE);

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

    //if timeout, means udp failed, turn to tcp relay
    //otherwise, use main loop to deal with event
    if (timeout)
    {
        DBG_INFO("udp handshake failed, turn to TCP Relay\r\n");
        SetState(P2P_TCP_PROXY_REQUEST);
        BASE_PACKET_T* Packet = CreateTCPProxyRequest(m_dwTCPid, m_szKeyword, m_szName);
        m_pTCP->SendPacket(Packet);
    }
}

VOID CP2PGuest::UDPPunchEventProcess()
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
        DBG_TRACE("Send %s Packet to host\r\n", UDPTypeToString(UPT_HANDSHAKE));
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

    //if timeout, means udp failed, turn to tcp relay
    //otherwise, use main loop to deal with event
    if (timeout)
    {
        DBG_INFO("udp handshake failed, turn to TCP Relay\r\n");
        SetState(P2P_TCP_PROXY_REQUEST);
        BASE_PACKET_T* Packet = CreateTCPProxyRequest(m_dwTCPid, m_szKeyword, m_szName);
        m_pTCP->SendPacket(Packet);
    }
}

BOOL CP2PGuest::RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(udp);

    BOOL Ret = FALSE;
    CP2PGuest* Host = dynamic_cast<CP2PGuest*>(Param);

    if (Host)
    {
        Ret = Host->RecvUDPPacketProcess(Packet);
    }

    return Ret;
}

BOOL CP2PGuest::RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(tcp);

    BOOL Ret = FALSE;
    CP2PGuest* Host = dynamic_cast<CP2PGuest*>(Param);

    if (Host)
    {
        Ret = Host->RecvTCPPacketProcess(Packet);
    }

    return Ret;
}

VOID CP2PGuest::TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(tcp);

    CP2PGuest* Host = dynamic_cast<CP2PGuest*>(Param);

    if (Host)
    {
        Host->TCPEndProcess();
    }

    return;
}


BOOL CP2PGuest::RecvUDPPacketProcess(UDP_PACKET* Packet)
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
                DBG_TRACE("===> Send %s Packet to host\r\n", UDPTypeToString(UPT_KEEPALIVE));
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
                    BASE_PACKET_T* P2PSuccess = CreateP2PSuccessPkt(m_dwTCPid, m_szKeyword);
                    m_pTCP->SendPacket(P2PSuccess);
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

BOOL CP2PGuest::RecvTCPPacketProcess(BASE_PACKET_T* Packet)
{
    BOOL Ret = TRUE;
    switch (Packet->Type)
    {
        case TPT_INIT:
        {
            TCP_INIT_PACKET* Data = (TCP_INIT_PACKET*)Packet->Data;
            m_dwTCPid = Data->tcpid;
            m_dwServerUDPPort = Data->UDPPort;

            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_INIT));
            DBG_TRACE("===> Tcpid %d udpport %d\r\n", m_dwTCPid, m_dwUDPPort);

            SetState(P2P_TCP_CONNECTED);
            SetEvent(m_hStatusChange);
            break;
        }
        case TPT_WAIT_RESULT:
        {
            TCP_RESULT_PACKET* Data = (TCP_RESULT_PACKET*)Packet->Data;

            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_WAIT_RESULT));
            DBG_TRACE("===> result %s\r\n", P2PErrorToString(Data->result));
            if (Data->result != 0)
            {
                m_dwErrorCode = Data->result;
                SetEvent(m_hStopEvent);
            }

            break;
        }
        case TPT_START_UDP:
        {
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_START_UDP));
            SetState(P2P_UDP_CONNECTED);
            SetEvent(m_hStatusChange);
            break;
        }
        case TPT_CLIENT_INFO:
        {
            //only set when state is udp listening
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_CLIENT_INFO));
            if (m_eStatus == P2P_UDP_CONNECTED)
            {
                TCP_START_PUNCHING_PACKET* Data = (TCP_START_PUNCHING_PACKET*)Packet->Data;
                m_dwPeerid = Data->Peerid;
                memcpy(&m_stRemoteClientInfo, &Data->ClientInfo, sizeof(CLIENT_INFO));

                DBG_TRACE("===> Peerid %d guset addr %s:%d %d\r\n", m_dwPeerid, inet_ntoa(m_stRemoteClientInfo.ipaddr), ntohs(m_stRemoteClientInfo.port));
                SetState(P2P_PUNCHING);
                SetEvent(m_hStatusChange);
            }
            else
            {
                DBG_ERROR("===> State is not TPT_CLIENT_INFO, skip this packet\r\n");
            }
            break;
        }
        case TPT_P2P_RESULT:
        {
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_P2P_RESULT));
            if (m_eStatus == P2P_PUNCHING)
            {
                SetState(P2P_CONNECTED);
                SetEvent(m_hStatusChange);
            }
            else
            {
                DBG_ERROR("===> State is not P2P_PUNCHING, skip this packet\r\n");
            }
            break;
        }
        case TPT_PROXY_RESULT:
        {
            TCP_PROXY_RESULT_PACKET* Data = (TCP_PROXY_RESULT_PACKET*)Packet->Data;
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_PROXY_RESULT));
            DBG_TRACE("===> result %s\r\n", P2PErrorToString(Data->Result));

            if (Data->Result != P2P_ERROR_NONE)
            {
                m_dwErrorCode = P2P_PEER_RESET;
                SetEvent(m_hStopEvent);
            }
            else
            {
                SetState(P2P_TCP_PROXY);
                SetEvent(m_hStatusChange);
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

VOID CP2PGuest::TCPEndProcess()
{
    DBG_INFO("TCP Disconnect\r\n");
    m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
    SetEvent(m_hStopEvent);
}
