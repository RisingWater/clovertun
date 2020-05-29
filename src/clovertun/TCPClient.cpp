#include "stdafx.h"
#include "TCPClient.h"

CTCPClient::CTCPClient(CHAR* szRemoteAddress, WORD dwPort) : CTCPBase()
{
    m_hSock = INVALID_SOCKET;
    strcpy(m_szDstAddress, szRemoteAddress);
    m_dwDstPort = dwPort;
}

CTCPClient::~CTCPClient()
{

}

BOOL CTCPClient::Init()
{
    int Ret;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        return FALSE;
    }

    struct sockaddr_in  DstAddress;
    struct sockaddr_in  SrcAddress;

    memset(&DstAddress, 0, sizeof(sockaddr_in));
    DstAddress.sin_family = AF_INET;
    DstAddress.sin_port = htons(m_dwDstPort);
    DstAddress.sin_addr.s_addr = inet_addr(m_szDstAddress);

    Ret = connect(sock, (struct sockaddr *)&DstAddress, sizeof(sockaddr_in));
    if (Ret < 0)
    {
        closesocket(sock);
        return FALSE;
    }

    m_hSock = sock;

#ifdef WIN32    
    int len = sizeof(struct sockaddr_in);
#else
    socklen_t len = sizeof(struct sockaddr_in);
#endif    
    if (getpeername(m_hSock, (struct sockaddr*)&SrcAddress, &len) >= 0)
    {
        m_dwSrcPort = SrcAddress.sin_port;
        strcpy(m_szSrcAddress, inet_ntoa(SrcAddress.sin_addr));
    }

    return InitBase();
}

VOID CTCPClient::Done()
{
    DBG_TRACE("ctcpclient done\r\n");
    RegisterRecvProcess(NULL, NULL);
    RegisterEndProcess(NULL, NULL);

    DoneBase();

    if (m_hSock != INVALID_SOCKET)
    {
        shutdown(m_hSock, SD_BOTH);
        closesocket(m_hSock);
        m_hSock = INVALID_SOCKET;
    }
}