#include "stdafx.h"
#include "P2PPacket.h"

static char* TCPTypeName[] = {
    "TPT_INIT",
    "TPT_WAITING",
    "TPT_WAIT_RESULT",
    "TPT_RESET",
    "TPT_CONNECT",
    "TPT_CONNECT_RESULT",
    "TPT_START_UDP",
    "TPT_CLIENT_INFO",
    "TPT_P2P_RESULT",
    "TPT_PROXY_REQUEST",
    "TPT_PROXY_RESULT",
    "TPT_DATA_PROXY",
    "TPT_MAX",
};

char* TCPTypeToString(DWORD Type)
{
    if (Type >= TPT_MAX)
    {
        return TCPTypeName[TPT_MAX];
    }

    return TCPTypeName[Type];
}

static char* UDPTypeName[] = {
    "UPT_WAITING",
    "UPT_CONNECT",
    "UPT_HANDSHAKE",
    "UPT_KEEPALIVE",
    "UPT_P2P_SUCCESS",
    "UPT_KCP",
    "UPT_ENET",
    "UPT_MAX",
};

char* UDPTypeToString(DWORD Type)
{
    if (Type >= UPT_MAX)
    {
        return UDPTypeName[UPT_MAX];
    }

    return UDPTypeName[Type];
}

static char* P2PStatueName[] = {
    "P2P_STATUS_NONE",
    "P2P_TCP_LISTENING",
    "P2P_TCP_CONNECTED",
    "P2P_UDP_LISTENING",
    "P2P_UDP_CONNECTED",
    "P2P_PUNCHING",
    "P2P_CONNECTED",
    "P2P_TCP_PROXY_REQUEST",
    "P2P_TCP_PROXY",
    "P2P_STATUS_MAX",
};

char* P2PStatusToString(DWORD Type)
{
    if (Type >= P2P_STATUS_MAX)
    {
        return P2PStatueName[P2P_STATUS_MAX];
    }

    return P2PStatueName[Type];
}

static char* P2PErrorName[] = {
    "Success, No Error",
    "Tcpid Error",
    "Connect to Server Failed",
    "Keyword Duplicate",
    "Keyword Not Found",
    "P2P State Mismatch",
    "P2P Punching Timeout",
    "P2P Proxy Failed",
    "P2P Peer Reset",
    "P2P Error Unknow",
};

char* P2PErrorToString(DWORD Type)
{
    if (Type >= P2P_ERROR_UNKNOW)
    {
        return P2PErrorName[P2P_ERROR_UNKNOW];
    }

    return P2PErrorName[Type];
}

BASE_PACKET_T* CreateTCPInitPkt(DWORD tcpid, WORD UDPPort)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_INIT_PACKET));

    memset(Packet, 0, BASE_PACKET_HEADER_LEN + sizeof(TCP_INIT_PACKET));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_INIT_PACKET);
    Packet->Type = TPT_INIT;

    TCP_INIT_PACKET* Data = (TCP_INIT_PACKET*)Packet->Data;

    Data->tcpid = tcpid;
    Data->UDPPort = UDPPort;

    return Packet;
}

BASE_PACKET_T* CreateTCPStartUDPPkt(DWORD tcpid, CHAR* Keyword)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_START_UDP_PACKET));

    memset(Packet, 0, BASE_PACKET_HEADER_LEN + sizeof(TCP_START_UDP_PACKET));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_START_UDP_PACKET);

    Packet->Type = TPT_START_UDP;

    TCP_START_UDP_PACKET* Data = (TCP_START_UDP_PACKET*)Packet->Data;

    Data->tcpid = tcpid;

    memcpy(Data->keyword, Keyword, KEYWORD_SIZE);

    return Packet;
}

BASE_PACKET_T* CreateTCPResultPkt(DWORD type, DWORD tcpid, DWORD result)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_RESULT_PACKET));

    memset(Packet, 0, BASE_PACKET_HEADER_LEN + sizeof(TCP_RESULT_PACKET));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_RESULT_PACKET);

    Packet->Type = type;

    TCP_RESULT_PACKET* Data = (TCP_RESULT_PACKET*)Packet->Data;

    Data->tcpid = tcpid;

    Data->result = result;

    return Packet;
}

static BASE_PACKET_T* CreateTCPWaitAndConnPkt(DWORD tcpid, CHAR* Keyword, CHAR* name, DWORD type)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_WAIT_PACKET));

    memset(Packet, 0, BASE_PACKET_HEADER_LEN + sizeof(TCP_WAIT_PACKET));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_WAIT_PACKET);

    Packet->Type = type;

    TCP_WAIT_PACKET* Data = (TCP_WAIT_PACKET*)Packet->Data;

    Data->tcpid = tcpid;
    strncpy((char*)Data->keyword, Keyword, KEYWORD_SIZE);
    strncpy((char*)Data->clientName, name, NAME_SIZE);

    return Packet;
}

BASE_PACKET_T* CreateTCPStartPunchingPkt(DWORD tcpid, DWORD PeerId, CLIENT_INFO* Info)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_START_PUNCHING_PACKET));

    memset(Packet, 0, BASE_PACKET_HEADER_LEN + sizeof(TCP_START_PUNCHING_PACKET));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_START_PUNCHING_PACKET);

    Packet->Type = TPT_CLIENT_INFO;

    TCP_START_PUNCHING_PACKET* Data = (TCP_START_PUNCHING_PACKET*)Packet->Data;

    Data->tcpid = tcpid;
    Data->Peerid = PeerId;
    memcpy(&Data->ClientInfo, Info, sizeof(CLIENT_INFO));

    return Packet;
}

BASE_PACKET_T* CreateTCPWaitPkt(DWORD tcpid, CHAR* Keyword, CHAR* name)
{
    return CreateTCPWaitAndConnPkt(tcpid, Keyword, name, TPT_WAITING);
}

BASE_PACKET_T* CreateTCPConnPkt(DWORD tcpid, CHAR* Keyword, CHAR* name)
{
    return CreateTCPWaitAndConnPkt(tcpid, Keyword, name, TPT_CONNECT);
}

BASE_PACKET_T* CreateTCPProxyRequest(DWORD tcpid, CHAR* Keyword, CHAR* name)
{
    return CreateTCPWaitAndConnPkt(tcpid, Keyword, name, TPT_PROXY_REQUEST);
}

BASE_PACKET_T* CreateP2PSuccessPkt(DWORD tcpid, CHAR* Keyword)
{
    CHAR Name[NAME_SIZE];
    memset(Name, 0, NAME_SIZE);

    return CreateTCPWaitAndConnPkt(tcpid, Keyword, Name, TPT_P2P_RESULT);
}

BASE_PACKET_T* CreateTCPProxyResultPkt(DWORD tcpid, DWORD peerid, DWORD result)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_PROXY_RESULT_PACKET));

    memset(Packet, 0, BASE_PACKET_HEADER_LEN + sizeof(TCP_PROXY_RESULT_PACKET));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_PROXY_RESULT_PACKET);

    Packet->Type = TPT_PROXY_RESULT;

    TCP_PROXY_RESULT_PACKET* Data = (TCP_PROXY_RESULT_PACKET*)Packet->Data;

    Data->tcpid = tcpid;
    Data->Peerid = peerid;
    Data->Result = result;

    return Packet;
}

BASE_PACKET_T* CreateTCPProxyData(TCP_PROXY_DATA* Data, DWORD Length)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + Length);

    memset(Packet, 0, sizeof(BASE_PACKET_HEADER_LEN + Length));

    Packet->Length = BASE_PACKET_HEADER_LEN + Length;

    Packet->Type = TPT_DATA_PROXY;

    memcpy(Packet->Data, Data, Length);

    return Packet;
}

BASE_PACKET_T* CreateTCPProxyData(DWORD Peerid, DWORD Host2Guest, PBYTE Data, DWORD Length)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + TCP_PROXY_DATA_HEADER_LEN + Length);

    memset(Packet, 0, sizeof(BASE_PACKET_HEADER_LEN + TCP_PROXY_DATA_HEADER_LEN + Length));

    Packet->Length = BASE_PACKET_HEADER_LEN + TCP_PROXY_DATA_HEADER_LEN + Length;

    Packet->Type = TPT_DATA_PROXY;

    TCP_PROXY_DATA* Proxy = (TCP_PROXY_DATA*)Packet->Data;

    Proxy->Host2Guest = Host2Guest;
    Proxy->Peerid = Peerid;
    Proxy->Length = Length;

    memcpy(Proxy->Data, Data, Length);

    return Packet;
}