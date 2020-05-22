#include "stdafx.h"
#include "P2PPacket.h"

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

BASE_PACKET_T* CreateTCPWaitConn(DWORD tcpid, CHAR* Keyword, CHAR* name)
{
    return CreateTCPWaitAndConnPkt(tcpid, Keyword, name, TPT_CONNECT);
}

BASE_PACKET_T* CreateTCPProxyRequest(DWORD tcpid, CHAR* Keyword, CHAR* name)
{
    return CreateTCPWaitAndConnPkt(tcpid, Keyword, name, TPT_PROXY_REQUEST);
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

    memset(Packet, 0, sizeof(BASE_PACKET_HEADER_LEN + sizeof(TCP_START_PUNCHING_PACKET)));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_START_PUNCHING_PACKET);

    Packet->Type = TPT_DATA_PROXY;

    memcpy(Packet->Data, Data, Length);

    return Packet;
}