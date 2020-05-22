#include "stdafx.h"
#include "XGetOpt.h"

#include "P2PServer.h"
#include "P2PHost.h"

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
                printf("I am Host Client\n");
				isHost = TRUE;
                isGuest = FALSE;
                isServer = FALSE;
                break;

            case _T('g'):
                printf("I am Guest Client\n");
				isHost = FALSE;
                isGuest = TRUE;
                isServer = FALSE;
                break;

            case _T('s'):
                printf("I am Server\n");
				isHost = FALSE;
                isGuest = FALSE;
                isServer = TRUE;
                break;

            case _T('n'):
				if (!isServer)
				{
					printf("name: %s\n", optarg);
					strcpy(name, optarg);
				}
                break;
            case _T('p'):
				printf("port: %d\n", atoi(optarg));
				port = atoi(optarg);
                break;

            case _T('a'):
				if (!isServer)
				{
					printf("addr: %s\n", optarg);
					strcpy(addr, optarg);
				}
                break;

			case _T('k'):
				if (!isServer)
				{
					printf("keyword: %s\n", optarg);
					strcpy(keyword, optarg);
				}
                break;

            default:
                printf("WARNING: no handler for option %c\n", c);
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
        host->Run();
    }
    else if (isGuest)
    {

    }

	getchar();
}


