

#include "stdafx.h"
#include <windows.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <math.h>


#define XXX TEXT("BITMAP_XXX")        // mem name
#define XXX_MUTEX_NAME TEXT("BITMAP_XXX_MUTEX_EXE")
#define MEMORY_SIZE 1024 * 1024 * 6   // mem size

int width = 320;
int height = 480;

int  MEM_Lock(WCHAR* str, HANDLE* handle);	// lock mem
void MEM_UnLock(HANDLE handle);			    // unlock mem

void MAKE_MATRIX(LPBYTE bitmap24, LPBYTE bitmap8);

int main(int argc, char* argv[])
{
	LPBYTE bitmap24, bitmap8;

	HANDLE mutex_SharedMem = NULL;			// Mutex
	HANDLE hShare = NULL;					// mem handle

	// take a lock

	DWORD iRet = MEM_Lock(XXX_MUTEX_NAME, &mutex_SharedMem);
	if (iRet != 0) {
		return -1;
	}

	// open share mem
	hShare = ::OpenFileMapping(FILE_MAP_WRITE, FALSE, XXX);
	if (hShare == NULL)
	{
		// unlock
		MEM_UnLock(mutex_SharedMem);
		return -1;
	}

	// share mem mapping
	LPBYTE shmem = (LPBYTE) ::MapViewOfFile(hShare, FILE_MAP_WRITE, 0, 0, MEMORY_SIZE);
	if (shmem == NULL) {

		// release
		UnmapViewOfFile(shmem);
		CloseHandle(hShare);

		// unlock
		MEM_UnLock(mutex_SharedMem);
		return -1;
	}

	// write share mem
	memset(shmem, 0x00, MEMORY_SIZE);
	bitmap24 = shmem;
	bitmap8 = shmem + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3;
	MAKE_MATRIX(bitmap24, bitmap8);

	// release
	UnmapViewOfFile(shmem);
	CloseHandle(hShare);

	// unlock
	MEM_UnLock(mutex_SharedMem);

    return 0;
}

int MEM_Lock(WCHAR* str, HANDLE* handle) {

	char cErrMsg[255] = { "\0" };

	HANDLE hMutex = NULL;

	hMutex = CreateMutex(NULL, FALSE, str);

	if (hMutex == NULL)
	{

		//sprintf(cErrMsg, "CreateMutex ERROR[%d]", GetLastError());
		return -1;
	}

	DWORD iRet = WaitForSingleObject( hMutex, INFINITE );
	if (iRet == WAIT_FAILED)
	{

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

		//sprintf(cErrMsg, "WaitForSingleObject ERROR[%d]", GetLastError());
		return -1;
	}

	*handle = hMutex;
	return 0;
}

void MEM_UnLock(HANDLE handle)
{
	ReleaseMutex(handle);
	CloseHandle(handle);
}


void MAKE_MATRIX(LPBYTE bitmap24, LPBYTE bitmap8) {


	((LPBITMAPFILEHEADER)bitmap24)->bfType = 0x4D42;
	((LPBITMAPFILEHEADER)bitmap24)->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3;
	((LPBITMAPFILEHEADER)bitmap24)->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	((LPBITMAPFILEHEADER)bitmap24)->bfReserved1 = 0;
	((LPBITMAPFILEHEADER)bitmap24)->bfReserved2 = 0;

	((BITMAPINFOHEADER*)(bitmap24 + sizeof(BITMAPFILEHEADER)))->biSize = sizeof(BITMAPINFOHEADER);
	((BITMAPINFOHEADER*)(bitmap24 + sizeof(BITMAPFILEHEADER)))->biWidth = width;
	((BITMAPINFOHEADER*)(bitmap24 + sizeof(BITMAPFILEHEADER)))->biHeight = height;
	((BITMAPINFOHEADER*)(bitmap24 + sizeof(BITMAPFILEHEADER)))->biPlanes = 0x1;
	((BITMAPINFOHEADER*)(bitmap24 + sizeof(BITMAPFILEHEADER)))->biBitCount = 24;         // 8bit(256-color),24bit(full-color)
	((BITMAPINFOHEADER*)(bitmap24 + sizeof(BITMAPFILEHEADER)))->biSizeImage = width * height; // サイズ

	for (int i = 0; i < width * height; i++)
	{
		(bitmap24 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))[i] = i%width / 2;
	}

	for (int i = width * height * 3 - 1; i >= 0; i--)
	{
		(bitmap24 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))[i] = (bitmap24 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))[(int)floor(i / 3)];
	}

	memcpy(bitmap8, bitmap24, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

	((LPBITMAPFILEHEADER)bitmap8)->bfType = 0x4d42;
	((LPBITMAPFILEHEADER)bitmap8)->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256 + width * height;
	((LPBITMAPFILEHEADER)bitmap8)->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
	((LPBITMAPFILEHEADER)bitmap8)->bfReserved1 = 0;
	((LPBITMAPFILEHEADER)bitmap8)->bfReserved2 = 0;

	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biSize = sizeof(BITMAPINFOHEADER);
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biWidth = width;
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biHeight = height;
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biPlanes = 0x1;
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biBitCount = 8;          // 8bit(256-color),24bit(full-color)
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biSizeImage = width * height; // サイズ

																							 // default setting;
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biCompression = 0;
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biXPelsPerMeter = 0;//基本：0 0x4ce5;//19685
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biYPelsPerMeter = 0;//基本：0 0x4ce5;//19685

	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biClrImportant = 0;
	((BITMAPINFOHEADER*)(bitmap8 + sizeof(BITMAPFILEHEADER)))->biClrUsed = 0;



	for (size_t i = 0; i < 256; i++)
	{
		/*((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)))[i].rgbBlue = i;
		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)))[i].rgbGreen = i;
		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)))[i].rgbRed = i;
		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)))[i].rgbReserved = 0;*/

		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + i * sizeof(LPRGBQUAD)))->rgbBlue = i;
		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + i * sizeof(LPRGBQUAD)))->rgbGreen = i;
		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + i * sizeof(LPRGBQUAD)))->rgbRed = i;
		((LPRGBQUAD)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + i * sizeof(LPRGBQUAD)))->rgbReserved = 0;
	}

	BYTE *p = (LPBYTE)(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
	for (int i = 0; i < width * height; i++)
	{
		int c = i % width / 2;
		(bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256)[i] = c;
		*p ^= 0xff;
		p++;
	}

}
