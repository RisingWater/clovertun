#pragma once

#ifndef __KCP_CLIENT_H__
#define __KCP_CLIENT_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "ikcp.h"
#include "BasePacket.h"
#include "P2PProtocol.h"
#include "BaseObject.h"
#include <list>

#define BASE_BUFF_SIZE 256

typedef struct
{
    PBYTE Data;
    DWORD Len;
} DATA_CB;

class CKCPClient;
typedef BOOL (*_KCPRecvPacketProcess)(PBYTE Data, DWORD Length, CKCPClient* tcp, CBaseObject* Param);

class CKCPClient : public CBaseObject
{
public :
	CKCPClient(SOCKET socket, DWORD PeerId, CLIENT_INFO info);
	virtual ~CKCPClient();

    BOOL Init();
    VOID Done();

    VOID SendPacket(PBYTE Data, DWORD Length);
    VOID RegisterRecvProcess(_KCPRecvPacketProcess Process, CBaseObject* Param);

private:

    static int KCPOutputDelegate(const char *buf, int len, struct IKCPCB *kcp, void *user);

    static DWORD WINAPI MainProc(void* pParam);

	BOOL KCPProcess(HANDLE StopEvent);

    VOID KCPOutput(const char* buf, int len);

    VOID KCPInit();
    VOID KCPDone();

    HANDLE m_hMainThread;
    HANDLE m_hStopEvent;
    HANDLE m_hDataStreamBuffer;

    CRITICAL_SECTION m_csSendLock;   
	std::list<BASE_PACKET_T*> m_SendList;

    CRITICAL_SECTION m_csRecvFunc;
    _KCPRecvPacketProcess m_pfnRecvFunc;
    CBaseObject* m_pRecvParam;

    ikcpcb* m_pKcp;

#ifdef WIN32
    HANDLE m_hSocketEvent;
#endif
    
    DWORD m_dwPeerId;
    SOCKET m_hSock;
    CLIENT_INFO m_stPeerInfo;
};

#endif

