#include "stdafx.h"
#include "P2PProtocol.h"
#include "P2PPacket.h"

BASE_PACKET_T* CreateTCPInit(DWORD tcpid)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_INIT_PACKET));

    memset(Packet, 0, sizeof(BASE_PACKET_HEADER_LEN + sizeof(TCP_INIT_PACKET)));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_INIT_PACKET);
    Packet->Type = TPT_INIT;

    TCP_INIT_PACKET* Data = (TCP_INIT_PACKET*)Packet->Data;

    Data->tcpid = tcpid;

    return Packet;
}

BASE_PACKET_T* CreateTCPStartUDP(DWORD tcpid, CHAR* Keyword)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_START_UDP_PACKET));

    memset(Packet, 0, sizeof(BASE_PACKET_HEADER_LEN + sizeof(TCP_START_UDP_PACKET)));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_START_UDP_PACKET);

    Packet->Type = TPT_START_UDP;

    TCP_START_UDP_PACKET* Data = (TCP_START_UDP_PACKET*)Packet->Data;

    Data->tcpid = tcpid;

    memcpy(Data->keyword, Keyword, KEYWORD_SIZE);

    return Packet;
}

BASE_PACKET_T* CreateTCPResult(DWORD type, DWORD tcpid, DWORD result)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + sizeof(TCP_RESULT_PACKET));

    memset(Packet, 0, sizeof(BASE_PACKET_HEADER_LEN + sizeof(TCP_RESULT_PACKET)));

    Packet->Length = BASE_PACKET_HEADER_LEN + sizeof(TCP_RESULT_PACKET);

    Packet->Type = type;

    TCP_RESULT_PACKET* Data = (TCP_RESULT_PACKET*)Packet->Data;

    Data->tcpid = tcpid;

    Data->result = result;

    return Packet;
}