#include "UDPClient.h"
#include "XGetOpt.h"
#include <Windows.h>
#include <map>

int main(int argc,char * argv[])
{
	int c;
	char name[64];
	char addr[64];
	int port;
	char keyword[32];
	int type = 0;
	BOOL isServer = FALSE;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	while ((c = getopt(argc, argv, _T("csn:k:a:p:t:"))) != -1)
    {
        switch (c)
        {
            case _T('c'):
                printf("I am Client\n");
				isServer = FALSE;
                break;
            case _T('s'):
                printf("I am Server\n");
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

			case _T('t'):
				if (!isServer)
				{
					printf("addr: %s\n", optarg);
					type = atoi(optarg);
				}
                break;

            default:
                printf("WARNING: no handler for option %c\n", c);
                return -1;
                break;
        }
    }
	
	

	getchar();
}


