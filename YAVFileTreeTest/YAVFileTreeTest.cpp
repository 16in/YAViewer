#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "YAVFileTree.h"


#pragma comment( lib, "Comctl32.lib" )
#pragma comment( lib, "shlwapi.lib" )


HWND hTreeWindow = NULL;


LRESULT CALLBACK wndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int sendEditor = 0;

	switch( message )
	{
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	case WM_CREATE:
		hTreeWindow = CreateTreeWindow( ((LPCREATESTRUCT)lParam)->hInstance, hWnd, 0 );
		::TreeAddFilePath( hTreeWindow, L"sampletemplate", true );
		break;
	case WM_SIZE:
		MoveWindow( hTreeWindow, 0, 0, LOWORD( lParam ), HIWORD( lParam ), TRUE );
		break;

	default:
		return DefWindowProcW( hWnd, message, wParam, lParam );
	}
	return 0;
}

HWND CreateAppWindow( HINSTANCE hInstance )
{
	HWND ret = NULL;

	WNDCLASSW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.hInstance = hInstance;
	wc.lpfnWndProc = wndProc;
	wc.style = CS_VREDRAW|CS_HREDRAW;
	wc.lpszClassName = L"AppWindow";
	wc.lpszMenuName = NULL;
	::RegisterClassW( &wc );

	ret = ::CreateWindowW( wc.lpszClassName, L"YAViwer TreeTest", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

	return ret;
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nShowCmd )
{
	InitCommonControls();


	RegisterTreeWindowClass( hInstance );


	/*---- �E�B���h�E���� ----*/
	HWND hAppWnd = CreateAppWindow( hInstance );
	ShowWindow( hAppWnd, nShowCmd );
	UpdateWindow( hAppWnd );


	/*---- ���b�Z�[�W���[�v ----*/
	MSG msg;
	while( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		::TranslateMessage( &msg );
		::DispatchMessageW( &msg );
	}


	/*---- �I���R�[�h���� ----*/
	if( msg.message == WM_QUIT ) return msg.wParam;
	return 0;
}
