#include "stdafx.h"
#include "TCPCommon.h"

#ifdef WIN32

BOOL SocketRead(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwReaded, HANDLE hStopEvent)
{
    BOOL bRet = TRUE;
    int rc;
    int err = 0;
    DWORD RecvBytes = 0, dwRet, Flags;
    WSABUF DataBuf;
    WSAOVERLAPPED RecvOverlapped;
    SecureZeroMemory((PVOID)& RecvOverlapped, sizeof(WSAOVERLAPPED));
    RecvOverlapped.hEvent = WSACreateEvent();

    DataBuf.len = dwBufferSize;
    DataBuf.buf = (CHAR*)pBuffer;
    while (1)
    {
        HANDLE hEvents[2] = { RecvOverlapped.hEvent, hStopEvent };
        Flags = 0;
        rc = WSARecv(s, &DataBuf, 1, &RecvBytes, &Flags, &RecvOverlapped, NULL);
        if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError())))
        {
            DBG_ERROR("WSARecv failed with error: %d\r\n", err);
            bRet = FALSE;
            break;
        }

        dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
        if (dwRet == WAIT_OBJECT_0)
        {
            rc = WSAGetOverlappedResult(s, &RecvOverlapped, &RecvBytes, TRUE, &Flags);
            if (rc == FALSE)
            {
                DBG_ERROR("WSARecv operation failed with error: %d\r\n", WSAGetLastError());
                bRet = FALSE;
                break;
            }
            if (RecvBytes == 0)
            {
                // connection is closed
                bRet = FALSE;
                break;
            }
            break;
        }
        else
        {
            if (dwRet == WAIT_OBJECT_0 + 1)
            {
                DBG_TRACE("get exit event \r\n");
            }

            WSASetEvent(RecvOverlapped.hEvent);
            rc = WSAGetOverlappedResult(s, &RecvOverlapped, &RecvBytes, TRUE, &Flags);
            bRet = FALSE;
            break;
        }
    }
    WSACloseEvent(RecvOverlapped.hEvent);

    *pdwReaded = RecvBytes;

    return bRet;
}

BOOL SocketWrite(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwWritten, HANDLE hStopEvent)
{
    WSAOVERLAPPED SendOverlapped;
    WSABUF DataBuf;
    DWORD SendBytes = 0;
    int err = 0;
    int rc;
    HANDLE hEvents[2] = { 0 };

    DWORD dwRet;
    DWORD Flags = 0;
    BOOL bRet = FALSE;

    SecureZeroMemory((PVOID)& SendOverlapped, sizeof(WSAOVERLAPPED));
    SendOverlapped.hEvent = WSACreateEvent();

    hEvents[0] = SendOverlapped.hEvent;
    hEvents[1] = hStopEvent;

    if (SendOverlapped.hEvent == NULL)
    {
        printf("create wsaevent fail \r\n");
        return FALSE;
    }

    DataBuf.len = dwBufferSize;
    DataBuf.buf = (char*)pBuffer;

    do
    {
        rc = WSASend(s, &DataBuf, 1,
            &SendBytes, 0, &SendOverlapped, NULL);
        if (rc == 0)
        {
            bRet = TRUE;
        }

        if ((rc == SOCKET_ERROR) &&
            (WSA_IO_PENDING != (err = WSAGetLastError()))) {
            DBG_ERROR("WSASend failed with error: %d\r\n", err);
            break;
        }

        dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        if (dwRet == WAIT_OBJECT_0)
        {
            if (WSAGetOverlappedResult(s, &SendOverlapped, &SendBytes,
                TRUE, &Flags))
            {
                bRet = TRUE;
            }
        }
        else
        {
            if (dwRet == WAIT_OBJECT_0 + 1)
            {
                DBG_TRACE("get exit event \r\n");

            }
            WSASetEvent(SendOverlapped.hEvent);

            WSAGetOverlappedResult(s, &SendOverlapped, &SendBytes,
                TRUE, &Flags);
        }

    } while (FALSE);

    WSACloseEvent(SendOverlapped.hEvent);
    *pdwWritten = SendBytes;

    return bRet;
}

#else

#ifndef ANDROID
#include <execinfo.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <exception>

#ifndef MAX
#define MAX(a,b) ((a > b) ? a : b)
#endif

BOOL SocketRead(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwReaded, HANDLE hStopEvent)
{
    fd_set fdw = { 0 };

    int stopfd = 0;
    int n;
    int ret = 0;
    int dwReaded = 0;
    *pdwReaded = 0;

    if (s == -1)
    {
        return FALSE;
    }


    FD_ZERO(&fdw);

    FD_SET(s, &fdw);
    n = s + 1;
#ifndef MACOS
    if (hStopEvent)
    {
        stopfd = GetEventFileDescriptor(hStopEvent);
        FD_SET(stopfd, &fdw);

        n = MAX(s, stopfd) + 1;
    }
#endif
    ret = select(n, &fdw, NULL, NULL, NULL);

    if (ret == 0)
    {
        DBG_ERROR("should not be here !!! \r\n");
        return FALSE;
    }
    else
    {
        if (ret == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                *pdwReaded = 0;
                ret = 0;
                return TRUE;
            }
            else
            {
                DBG_ERROR("recv select error \r\n");
                return FALSE;
            }
        }
        else
        {
            if (FD_ISSET(stopfd, &fdw))
            {
                DBG_ERROR("recv get stop event \r\n");
                return FALSE;
            }
            else
                if (FD_ISSET(s, &fdw))
                {
                    dwReaded = recv(s, pBuffer, dwBufferSize, 0);
                    *pdwReaded = dwReaded;

                    if (dwReaded == 0)
                    {
                        *pdwReaded = 0;
                        return FALSE;
                    }
                    else
                        if (dwReaded == -1)
                        {
                            if (errno == EWOULDBLOCK)
                            {
                                *pdwReaded = 0;
                                return TRUE;
                            }

                            *pdwReaded = 0;
                            return FALSE;
                        }
                        else
                        {
                            return TRUE;
                        }
                }
        }
    }
}

BOOL SocketWrite(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwWritten, HANDLE hStopEvent)
{
    fd_set fdw = { 0 };

    int stopfd = 0;
    int n;
    int ret = 0;
    int dwWrite = 0;
    *pdwWritten = 0;

    if (s == -1)
    {
        return FALSE;
    }

    FD_ZERO(&fdw);

    FD_SET(s, &fdw);
    n = s + 1;
#ifndef MACOS
    if (hStopEvent)
    {
        stopfd = GetEventFileDescriptor(hStopEvent);
        FD_SET(stopfd, &fdw);

        n = MAX(s, stopfd) + 1;
    }
#endif
    ret = select(n, NULL, &fdw, NULL, NULL);

    if (ret == 0)
    {
        DBG_ERROR("should not be here !!! \r\n");
        return FALSE;
    }
    else
    {
        if (ret == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                ret = 0;
                return TRUE;
            }
            else
            {
                DBG_ERROR("recv select error \r\n");
                return FALSE;
            }
        }
        else
        {
            if (FD_ISSET(stopfd, &fdw))
            {
                DBG_ERROR("recv get stop event \r\n");
                return FALSE;
            }
            else
                if (FD_ISSET(s, &fdw))
                {
                    signal(SIGPIPE, SIG_IGN);
                    try
                    {
                        dwWrite = send(s, pBuffer, dwBufferSize, 0);
                        *pdwWritten = dwWrite;

                        if (dwWrite == 0)
                        {
                            return FALSE;
                        }
                        if (dwWrite == -1)
                        {
                            if (errno == EWOULDBLOCK)
                            {
                                *pdwWritten = 0;
                                return TRUE;
                            }
                            return FALSE;
                        }
                        return TRUE;
                    }
                    catch (std::exception& e)
                    {
                        DBG_ERROR("send data error \r\n");
                        return FALSE;
                    }
                }
        }
    }
}
#endif
