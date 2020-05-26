#ifndef _UNIT_H_
#define _UNIT_H_

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "corelib.h"
#include "BasePacket.h"

__inline PBASE_PACKET_T GetPacketFromBuffer(HANDLE hDataBuffer)
{
	BASE_PACKET_T* pPacket = NULL;
	DWORD dwReaded = 0;
	DWORD dwCurrentSize = DataStreamBufferGetCurrentDataSize(hDataBuffer);
	if (dwCurrentSize >= BASE_PACKET_HEADER_LEN)
	{
		BASE_PACKET_T header;

		DataStreamBufferGetData(hDataBuffer,(BYTE*)&header,BASE_PACKET_HEADER_LEN, &dwReaded);
		DataStreamBufferAddDataFront(hDataBuffer,(BYTE*)&header,dwReaded);

		if (dwReaded == BASE_PACKET_HEADER_LEN)
		{
			if (header.Length <= dwCurrentSize)
			{
				pPacket = (BASE_PACKET_T*)malloc(header.Length);
				DataStreamBufferGetData(hDataBuffer,(BYTE*)pPacket,header.Length,&dwReaded);

				if(dwReaded != header.Length)
				{
					DataStreamBufferAddDataFront(hDataBuffer,(BYTE*)pPacket,dwReaded);
					free(pPacket);
					pPacket = NULL;
				}
			}
		}
	}
	return pPacket;

}

#endif