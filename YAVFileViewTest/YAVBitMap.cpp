/*----------------------------------------------------------------------------------------
*
* YAViewer View BitMap保存関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVBitMap.h"

static DWORD buf[ 2048 * 2048 ];


//----------------------------------------------
// DIBセクションを保存する
//
bool SaveDIB( LPCWSTR filePath, HDC hdc, HBITMAP hBmp )
{
	/*---- BitMapInfoを設定する ----*/
	BITMAPINFO bmpInfo;
	ZeroMemory( &bmpInfo, sizeof( bmpInfo ) );
	bmpInfo.bmiHeader.biSize = sizeof( bmpInfo );
	int lines = GetDIBits( hdc, hBmp, 0, 1000, NULL, &bmpInfo, DIB_RGB_COLORS );
	lines = GetDIBits( hdc, hBmp, 0, lines, buf, &bmpInfo, DIB_RGB_COLORS );


	/*---- ファイル情報を設定する ----*/
	BITMAPFILEHEADER bmpfh;
	bmpfh.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + bmpInfo.bmiHeader.biSizeImage;
	bmpfh.bfType = ('M'<<8) + 'B';
	bmpfh.bfOffBits = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );


	/*---- ファイルに書き出す ----*/
	DWORD writeSize;
	HANDLE fh = CreateFile( filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	WriteFile( fh, &bmpfh, sizeof( bmpfh ), &writeSize, NULL );
	WriteFile( fh, &bmpInfo.bmiHeader, sizeof( bmpInfo.bmiHeader ), &writeSize, NULL );
	WriteFile( fh, buf, bmpInfo.bmiHeader.biSizeImage, &writeSize, NULL );

	CloseHandle( fh );


	return true;
}
