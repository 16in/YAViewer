/*----------------------------------------------------------------------------------------
*
* アスキーアートファイルビューア YAViewer
*
*----------------------------------------------------------------------------------------*/
#include <Windows.h>
#include <Shlwapi.h>
#include <commctrl.h>
#include "YAVWindow.h"
#include "YAVSendSelector.h"


#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "Shlwapi.lib" )


//----------------------------------------------
// エントリポイント
//
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR cmdline, int nShowCmd )
{
	RegYavAppData	appData;


	/*---- コモンコントロール初期化 ----*/
	InitCommonControls();


	/*---- アプリケーションディレクトリのパスを設定 ----*/
	wchar_t path[ MAX_PATH ];
	GetModuleFileNameW( NULL, path, MAX_PATH );
	PathRemoveFileSpecW( path );
	YavSetAppDirectory( path );


	/*---- 初期化状態を読み込む ----*/
	YavLoadAppSetting( &appData, NULL );


	/*---- ウィンドウクラス登録 ----*/
	if( !RegisterYAVWindowClass( hInstance ) )
	{
		MessageBoxW( NULL, L"ウィンドウクラスの登録に失敗しました。", L"Error Register WindowClass", MB_OK | MB_ICONERROR );
		return 0;
	}


	/*---- アプリケーションウィンドウ生成 ----*/
	HWND hAppWnd = CreateYAVWindow( hInstance, &appData );
	if( !hAppWnd )
	{
		MessageBoxW( NULL, L"ウィンドウの生成に失敗しました。", L"Error CreateWindow", MB_OK | MB_ICONERROR );
		return 0;
	}
	ShowWindow( hAppWnd, nShowCmd );
	UpdateWindow( hAppWnd );


	/*---- メッセージループ ----*/
	MSG msg;
	while( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		::TranslateMessage( &msg );
		::DispatchMessageW( &msg );
	}


	/*---- 終了コード処理 ----*/
	if( msg.message == WM_QUIT )
	{
		YavSaveAppSetting( &appData );
		return (int)msg.wParam;
	}


	return 0;
}
