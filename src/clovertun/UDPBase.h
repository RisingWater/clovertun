#pragma once

#ifndef __UDP_BASE_H__
#define __UDP_BASE_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "P2PProtocol.h"
#include "BaseObject.h"
#include <list>

#define BASE_BUFF_SIZE 256

class CUDPBase;

typedef BOOL (*_UDPRecvPacketProcess)(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param);

class CUDPBase : public CBaseObject
{
public :
	CUDPBase();
	virtual ~CUDPBase();

    SOCKET GetSocket();

    BOOL Init(WORD UdpPort);
    VOID Start();
    VOID Stop();

    VOID SendPacket(UDP_PACKET* Packet);
    VOID RegisterRecvProcess(_UDPRecvPacketProcess Process, CBaseObject* Param);

protected:
	SOCKET m_hSock;
	WORD  m_dwLocalPort;

private:
    static DWORD WINAPI RecvProc(void* pParam);
    static DWORD WINAPI SendProc(void* pParam);

	BOOL SendProcess(HANDLE StopEvent);
	BOOL RecvProcess(HANDLE StopEvent);

    HANDLE m_hRecvThread;
    HANDLE m_hSendThread;
    
    HANDLE m_hStopEvent;
    HANDLE m_hSendEvent;

    CRITICAL_SECTION m_csSendLock;   
	std::list<UDP_PACKET*> m_SendList;

    CRITICAL_SECTION m_csRecvFunc;
    _UDPRecvPacketProcess m_pfnRecvFunc;
    CBaseObject* m_pRecvParam;

};

#endif

