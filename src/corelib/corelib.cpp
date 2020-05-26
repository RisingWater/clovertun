#include "stdafx.h"
#include "DataStreamBuf.h"

HANDLE CreateDataStreamBuffer()
{
	CDataStreamBuf* pBuffer = new CDataStreamBuf();

	return (HANDLE)pBuffer;
}

void DataStreamBufferClearAllData(HANDLE hBuffer)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
	pBuffer->ClearAll();
}

void DataStreamBufferAddData(HANDLE hBuffer,BYTE *pData, DWORD dwSize)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
	pBuffer->AddData(pData,dwSize);
}

void DataStreamBufferAddDataFront(HANDLE hBuffer,BYTE *pData, DWORD dwSize)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
	pBuffer->AddDataFront(pData,dwSize);
}

BYTE *DataStreamBufferGetAllData(HANDLE hBuffer,DWORD *lpDataOut)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
	return pBuffer->GetAllData(lpDataOut);
}

BOOL DataStreamBufferGetData(HANDLE hBuffer,BYTE *pBuf, DWORD dwBufSize, DWORD *lpNumOfGet)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
	return pBuffer->GetData(pBuf,dwBufSize,lpNumOfGet);
}

DWORD DataStreamBufferGetCurrentDataSize(HANDLE hBuffer)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;

	return pBuffer->GetCurrentDataSize();
}

void CloseDataStreamBuffer(HANDLE hBuffer)
{
	CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
	delete pBuffer;
}