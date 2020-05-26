#include "stdafx.h"
#include "XGetOpt.h"

#include "P2PServer.h"
#include "P2PHost.h"
#include "P2PGuest.h"
#include "P2PPacket.h"

BOOL P2PRecvPacketProcess(PBYTE Data, DWORD Len, CP2PClient* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(Param);

    DWORD ID = *(DWORD*)Data;
    if (tcp->GetClientType() == P2P_CLIENT_HOST)
    {
        DBG_TRACE("host recv data [%d] %d\r\n", ID, Len);
        tcp->SendPacket(Data, Len);
    }
    else
    {
        DBG_TRACE("guest recv data [%d] %d\r\n", ID, Len);
        ID++;
        tcp->SendPacket((PBYTE)&ID, sizeof(DWORD));
    }

    return TRUE;
}

int main(int argc,char * argv[])
{
	int c;
	char name[64] = {0};
    char addr[64] = {0};
	int port = 0;
	char keyword[32] = {0};
	BOOL isServer = FALSE;
    BOOL isHost = FALSE;
    BOOL isGuest = FALSE;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	while ((c = getopt(argc, argv, _T("hgsn:k:a:p:"))) != -1)
    {
        switch (c)
        {
            case _T('h'):
                DBG_INFO("I am Host Client\n");
				isHost = TRUE;
                isGuest = FALSE;
                isServer = FALSE;
                break;

            case _T('g'):
                DBG_INFO("I am Guest Client\n");
				isHost = FALSE;
                isGuest = TRUE;
                isServer = FALSE;
                break;

            case _T('s'):
                DBG_INFO("I am Server\n");
				isHost = FALSE;
                isGuest = FALSE;
                isServer = TRUE;
                break;

            case _T('n'):
				if (!isServer)
				{
					DBG_INFO("name: %s\n", optarg);
					strcpy(name, optarg);
				}
                break;
            case _T('p'):
				DBG_INFO("port: %d\n", atoi(optarg));
				port = atoi(optarg);
                break;

            case _T('a'):
				if (!isServer)
				{
					DBG_INFO("addr: %s\n", optarg);
					strcpy(addr, optarg);
				}
                break;

			case _T('k'):
				if (!isServer)
				{
					DBG_INFO("keyword: %s\n", optarg);
					strcpy(keyword, optarg);
				}
                break;

            default:
                DBG_ERROR("WARNING: no handler for option %c\n", c);
                return -1;
                break;
        }
    }
	
    if (isServer)
    {
        CP2PServer* server = new CP2PServer((WORD)port);
        server->Init();
    }
    else if (isHost)
    {
        CP2PHost* host = new CP2PHost(name, keyword, addr, (WORD)port);
        host->RegisterRecvPacketProcess(P2PRecvPacketProcess, host);
        DWORD ErrorCode = host->Listen();
        DBG_ERROR("Host Run %s\r\n", P2PErrorToString(ErrorCode));
    }
    else if (isGuest)
    {
        CP2PGuest* guest = new CP2PGuest(name, keyword, addr, (WORD)port);
        guest->RegisterRecvPacketProcess(P2PRecvPacketProcess, guest);
        DWORD ErrorCode = guest->Connect();
        DBG_ERROR("Guest Run %s\r\n", P2PErrorToString(ErrorCode));
        DWORD id = 0;
        guest->SendPacket((PBYTE)&id, sizeof(DWORD));
    }

	getchar();
}


