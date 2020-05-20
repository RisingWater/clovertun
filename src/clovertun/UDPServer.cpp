#include "stdafx.h"
#include "UDPServer.h"

CUDPServer::CUDPServer(WORD Port) :	CUDPBase()
{
    m_dwLocalPort = Port;

	DBG("启动服务器\n");
    
    struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(struct sockaddr_in));
	servAddr.sin_family = PF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(m_dwLocalPort);

	bind(m_hSock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));
}

VOID CUDPServer::RecvPacketProcess(UDP_PACKET Packet)
{
	switch (Packet.BasePacket.type)
	{
		case UPT_WAITING:
		{
			WAITING_NODE node;
			CONNECT_PACKET_DATA* ConnectPkt = (CONNECT_PACKET_DATA*)Packet.BasePacket.data;

			memcpy(&node.keyword, ConnectPkt->keyword, 32);
			memcpy(&node.info, &Packet.PacketInfo, sizeof(CLIENT_INFO));

            DBG("recv waiting pkt %d.%d.%d.%d %d\n",
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b1,
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b2,
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b3,
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b4,
                ntohs(Packet.PacketInfo.port));
			m_WaitList.push_back(node);

			break;
		}
		case UPT_CONNECT:
		{
            DBG("recv connect pkt %d.%d.%d.%d %d\n",
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b1,
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b2,
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b3,
                Packet.PacketInfo.ipaddr.S_un.S_un_b.s_b4,
                ntohs(Packet.PacketInfo.port));

			for (std::list<WAITING_NODE>::iterator Itor = m_WaitList.begin(); Itor != m_WaitList.end(); Itor++)
			{
				if (strncmp((char*)&Packet.BasePacket.data, Itor->keyword, 32) == 0)
				{
					//发给连接客户端
					UDP_PACKET data;

					data.BasePacket.type = UPT_SERVER_RESPONSE;
					memcpy(&data.BasePacket.data, &Itor->info, sizeof(CLIENT_INFO));
					data.BasePacket.length = sizeof(CLIENT_INFO);
					memcpy(&data.PacketInfo, &Packet.PacketInfo, sizeof(CLIENT_INFO));

					SendPacket(data);

					//发给等待连接客户端
					UDP_PACKET data1;
					data1.BasePacket.type = UPT_SERVER_RESPONSE;
					memcpy(&data1.BasePacket.data, &Packet.PacketInfo, sizeof(CLIENT_INFO));
					data1.BasePacket.length = sizeof(CLIENT_INFO);
					memcpy(&data1.PacketInfo, &Itor->info, sizeof(CLIENT_INFO));

					SendPacket(data1);
					m_WaitList.erase(Itor);
					break;
				}
			}
			break;
		}
		default:
			DBG("包类型错误，无法判断类型。\n");
			break;

	}


}