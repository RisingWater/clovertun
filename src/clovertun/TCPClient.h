#pragma once

#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "TCPBase.h"

class CTCPClient : public CTCPBase
{
public :
	CTCPClient(CHAR* szRemoteAddress, WORD dwPort);
	virtual ~CTCPClient();

    BOOL Init();
    VOID Done();
};

#endif

