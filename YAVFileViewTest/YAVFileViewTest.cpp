#include <Windows.h>
#include <commctrl.h>
#include "resource.h"
#include "YAVView.h"
#include "YAVSendEditor.h"
#include <stdio.h>


// �}�j���t�F�X�g���ߍ���
#pragma comment(linker, "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' version='6.0.0.0' "\
    "processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment( lib, "Comctl32.lib" )


#define APPTITLE_FACEEDIT L"YAViewer FileView Test - �i�L�t�M�jEdit���M���[�h"
#define APPTITLE_ORINEDIT L"YAViewer FileView Test - OrinrinEditor���M���[�h"

#define WM_OPEN_VIEWFILE	(WM_VIEW_SELECTITEM + 1)

static aafile::AAFile*	pViewFile = NULL;
static HWND				hViewWindow = NULL;

void openfile( HWND hWnd )
{
	static wchar_t path[ MAX_PATH ], title[ MAX_PATH ];
	static OPENFILENAMEW of;
	ZeroMemory( &of, sizeof( of ) );
	of.lStructSize		= sizeof( of );
	of.lpstrFilter		= L"MLT(*.mlt)\0*.mlt\0AllFile(*.*)\0*.*\0\0";
	of.hwndOwner		= hWnd;
	of.Flags			= OFN_FILEMUSTEXIST;
	of.lpstrFile		= path;
	of.lpstrFileTitle	= title;
	of.nMaxFile			= sizeof( path );
	of.nMaxFileTitle	= sizeof( title );
	of.lpstrTitle		= L"�t�@�C�����J��";
	of.lpstrDefExt		= L"mlt";

	if( GetOpenFileNameW( &of ) )
	{
		aafile::AAFile* pOldViewFile = pViewFile;
		if( pViewFile ) pOldViewFile = pViewFile;

		pViewFile = aafile::AAFileReader::CreateAAFileFromFile( of.lpstrFile );
		SetViewFile( hViewWindow, pViewFile );

		if( pOldViewFile ) aafile::AAFileReader::ReleaseAAFile( pOldViewFile );
	}
}

bool SendEditPaste( HWND hWnd, LPARAM lParam )
{
	switch( lParam )
	{
	default:// CTRL + V
		{
			BYTE list[ 2 ] = { VK_CONTROL, 'V' };
			SendKeyStroke( hWnd, list, 2 );
		}
		break;
	case 1:// �i�L�t�M�jEdit�̃e�L�X�g�{�b�N�X
		{
			BYTE list[ 2 ] = { VK_MENU, VK_SPACE };
			SendKeyStroke( hWnd, list, 2 );
		}
		break;
	case 2:// OrinrinEditor�̃e�L�X�g�{�b�N�X
		SetForegroundWindow( hWnd );
		SendMessageW( hWnd, WM_COMMAND, 0x00018014, 0 );
		break;
	}

	return true;
}


LRESULT CALLBACK wndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int sendEditor = 0;

	switch( message )
	{
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	case WM_CREATE:
		hViewWindow = CreateViewWindow( NULL, hWnd, 0 );

		/*---- �t�H���g���� ----*/
		{
			HFONT hFont;
			LOGFONTW logFont;
			ZeroMemory( &logFont, sizeof( logFont ) );
			wcscpy_s( logFont.lfFaceName, L"�l�r �o�S�V�b�N" );
			logFont.lfHeight = -16;

			hFont = ::CreateFontIndirectW( &logFont );
			SetViewFont( hViewWindow, hFont );
		}
		SetViewMode( hViewWindow, 1 );

		/*---- �e�X�g�p�Ƀt�@�C�����J�� ----*/
		openfile( hWnd );
		break;
	case WM_SIZE:
		MoveWindow( hViewWindow, 0, 0, LOWORD( lParam ), HIWORD( lParam ), TRUE );
		break;

	case WM_COMMAND:
		switch( LOWORD( wParam ) )
		{
		case ID_40001:// �I��
			DestroyWindow( hWnd );
			break;
		case ID_40002:// �t�@�C�����J��
			openfile( hWnd );
			break;

		// �\��
		case ID_40003:// �ʏ�\��
			SetViewMode( hViewWindow, 0 );
			break;
		case ID_40004:// �ʏ�\��
			SetViewMode( hViewWindow, 1 );
			break;
			
		// �\���T�C�Y�ύX
		case ID_40006:
			SetViewCategorySize( hViewWindow, 32 );
			break;
		case ID_40007:
			SetViewCategorySize( hViewWindow, 64 );
			break;
		case ID_40008:
			SetViewCategorySize( hViewWindow, 128 );
			break;
		case ID_40009:
			SetViewCategorySize( hViewWindow, 256 );
			break;

		// ���M��ύX
		case ID_40010:
			SetWindowTextW( hWnd, APPTITLE_FACEEDIT );
			sendEditor = 0;
			break;
		case ID_40011:
			SetWindowTextW( hWnd, APPTITLE_ORINEDIT );
			sendEditor = 1;
			break;
		}
		break;

	case WM_VIEW_SELECTITEM:
		{
			ViewSelectItem* item = (ViewSelectItem*)lParam;
			HGLOBAL hString = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * (item->itemLength + 1) );
			wchar_t* str = (wchar_t*)GlobalLock( hString );
			wcsncpy_s( str, item->itemLength + 1, item->itemString, item->itemLength );
			GlobalUnlock( hString );

			bool textbox = false;
			switch( LOWORD( wParam ) )
			{
			case 0x01:
				return 0;
			case 0x02:
				textbox = true;
				break;
			}


			HWND hTopWnd = NULL;
			HWND hEditWnd = NULL;
			int sendtype = !textbox ? 0 : sendEditor + 1;
			switch( sendEditor )
			{
			case 0:
				if( (hTopWnd = GetWindowHandle( NULL, L"�i�L�t�M�jEdit", L"TEditFrame" )) != NULL )
				{
					hEditWnd = GetWindowHandle( GetWindow( hTopWnd, GW_CHILD ), NULL, L"TChonEdit" );
				}
				break;
			case 1:
				if( (hTopWnd = GetWindowHandle( NULL, L"OrinrinEditor", L"ORINRINEDITOR" )) != NULL )
				{
					hEditWnd = GetWindowHandle( GetWindow( hTopWnd, GW_CHILD ), NULL, L"EDIT_VIEW" );
				}
				break;
			}
			SendClipString( hEditWnd, hString, SendEditPaste, sendtype );
		}
		break;

	case WM_SETFOCUS:
		SetFocus( hViewWindow );
		break;

	default:
		return DefWindowProcW( hWnd, message, wParam, lParam );
	case WM_ERASEBKGND:
		break;
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
	wc.lpszMenuName = L"IDR_MENU1";
	::RegisterClassW( &wc );

	HMENU hMenu = LoadMenuW( hInstance, MAKEINTRESOURCEW( IDR_MENU1 ) );
	ret = ::CreateWindowW( wc.lpszClassName, APPTITLE_FACEEDIT, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, hMenu, hInstance, NULL );

	return ret;
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nShowCmd )
{
	InitCommonControls();

	/*---- �E�B���h�E�N���X���� ----*/
	RegisterViewWindowClass( hInstance );


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


	/*---- �t�@�C�����c���Ă�����J�� ----*/
	if( pViewFile ) aafile::AAFileReader::ReleaseAAFile( pViewFile );


	/*---- �I���R�[�h���� ----*/
	if( msg.message == WM_QUIT ) return msg.wParam;
	return 0;
}

typedef struct _tagRegYavAppData RegYavAppData;
RegYavAppData* GetYAVAppData( HWND hWnd )
{
	return NULL;
}
bool GetSelectDoubleClick( HWND hWnd )
{
	return false;
}
