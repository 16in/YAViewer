/*----------------------------------------------------------------------------------------
*
* YAViewer View BitMap�ۑ��֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVBitMap.h"

static DWORD buf[ 2048 * 2048 ];


//----------------------------------------------
// DIB�Z�N�V������ۑ�����
//
bool SaveDIB( LPCWSTR filePath, HDC hdc, HBITMAP hBmp )
{
	/*---- BitMapInfo��ݒ肷�� ----*/
	BITMAPINFO bmpInfo;
	ZeroMemory( &bmpInfo, sizeof( bmpInfo ) );
	bmpInfo.bmiHeader.biSize = sizeof( bmpInfo );
	int lines = GetDIBits( hdc, hBmp, 0, 1000, NULL, &bmpInfo, DIB_RGB_COLORS );
	lines = GetDIBits( hdc, hBmp, 0, lines, buf, &bmpInfo, DIB_RGB_COLORS );


	/*---- �t�@�C������ݒ肷�� ----*/
	BITMAPFILEHEADER bmpfh;
	bmpfh.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + bmpInfo.bmiHeader.biSizeImage;
	bmpfh.bfType = ('M'<<8) + 'B';
	bmpfh.bfOffBits = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );


	/*---- �t�@�C���ɏ����o�� ----*/
	DWORD writeSize;
	HANDLE fh = CreateFile( filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	WriteFile( fh, &bmpfh, sizeof( bmpfh ), &writeSize, NULL );
	WriteFile( fh, &bmpInfo.bmiHeader, sizeof( bmpInfo.bmiHeader ), &writeSize, NULL );
	WriteFile( fh, buf, bmpInfo.bmiHeader.biSizeImage, &writeSize, NULL );

	CloseHandle( fh );


	return true;
}
