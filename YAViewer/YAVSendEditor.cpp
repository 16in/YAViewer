/*----------------------------------------------------------------------------------------
*
* YAViewer データ送信関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVSendEditor.h"


//----------------------------------------------
// 構造体定義
//
// enumWindowProcのLPARAM
struct enumWindowProcParam
{
	LPCWSTR		text;
	LPCWSTR		clsName;
	HWND		hResultWnd;
};


//----------------------------------------------
// グローバル関数
//
static BOOL CALLBACK enumWindowProc( HWND hWnd, LPARAM lParam );
static HWND getChildWindow( HWND hWnd, LPCWSTR text, LPCWSTR clsName );


//----------------------------------------------
// キーストロークを送る
//
void SendKeyStroke( HWND hWnd, BYTE* vkList, int count )
{
	if( hWnd )
	{
		SetForegroundWindow( hWnd );
	}
	if( vkList )
	{
		for( int i = 0; i < count; i++ )
		{
			Sleep( 5 );
			keybd_event( vkList[ i ], 0, 0, 0 );
		}
		for( int i = count - 1; i >= 0; i-- )
		{
			Sleep( 5 );
			keybd_event( vkList[ i ], 0, KEYEVENTF_KEYUP, 0 );
		}
	}
}


//----------------------------------------------
// ウィンドウを検索する
//
HWND GetWindowHandle( HWND hParent, LPCWSTR text, LPCWSTR clsName )
{
	HWND ret = NULL;

	if( !hParent )
	{
		/*---- トップレベルウィンドウの検索 ----*/
		enumWindowProcParam param;
		ZeroMemory( &param, sizeof( param ) );
		param.text = text; param.clsName = clsName;
		EnumWindows( enumWindowProc, (LPARAM)&param );
		ret = param.hResultWnd;
	}
	else
	{
		/*---- 子ウィンドウの検索 ----*/
		ret = getChildWindow( hParent, text, clsName );
	}


	return ret;
}


//----------------------------------------------
// クリップボードを経由してデータを送信する
//
bool SendClipString( HWND hWnd, HANDLE hString, SendClipStringProc proc, LPARAM lParam )
{
	bool ret = false;

	if( hWnd && hString && proc )
	{
		/*---- キャッシュを拾ってクリップボードのデータを書き換える ----*/
		HANDLE hChash = NULL;
		if( OpenClipboard( hWnd ) )
		{
			// キャッシュを確保する
			HANDLE hOldData = GetClipboardData( CF_UNICODETEXT );
			if( hOldData )
			{
				wchar_t* oldStr = (wchar_t*)GlobalLock( hOldData );
				size_t len = wcslen( oldStr ) + 1;

				hChash = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * len );
				wchar_t* chash = (wchar_t*)GlobalLock( hChash );
				wcscpy_s( chash, len, oldStr );
				GlobalUnlock( hChash );
				GlobalUnlock( hOldData );
			}

			// クリップボードのデータを書き換える
			EmptyClipboard();
			SetClipboardData( CF_UNICODETEXT, hString );
			CloseClipboard();
		}


		/*---- 割り込みを実行 ----*/
		ret = proc( hWnd, lParam );


		/*---- キャッシュをクリップボードへ戻す ----*/
		if( hChash && OpenClipboard( hWnd ) )
		{
			// クリップボードをキャッシュの値に戻す
			EmptyClipboard();
			SetClipboardData( CF_UNICODETEXT, hChash );
			CloseClipboard();
		}
	}

	return ret;
}


// 検索用
//----------------------------------------------
// トップレベルウィンドウを検索する
//
static BOOL CALLBACK enumWindowProc( HWND hWnd, LPARAM lParam )
{
	BOOL ret = TRUE;
	enumWindowProcParam* param = (enumWindowProcParam*)lParam;


	/*---- バッファを確保 ----*/
	size_t txtLength = param->text ? wcslen( param->text ) + 1 : 1;
	size_t clsLength = param->clsName ? wcslen( param->clsName ) + 1 : 1;
	wchar_t* winText = new wchar_t[ txtLength ];
	wchar_t* clsName = new wchar_t[ clsLength ];
	
	
	/*---- ウィンドウの情報を得る ----*/
	GetWindowTextW( hWnd, winText, txtLength );
	GetClassNameW( hWnd, clsName, clsLength );


	/*---- 判定 ----*/
	if( param->text == NULL || wcscmp( param->text, winText ) == 0 )
	{
		if( param->clsName == NULL || wcscmp( param->clsName, clsName ) == 0 )
		{
			param->hResultWnd = hWnd;
			ret = FALSE;
		}
	}


	/*---- バッファを開放 ----*/
	delete[] clsName;
	delete[] winText;


	return ret;
}


//----------------------------------------------
// 子ウィンドウを潜りながら検索していく
//
static HWND getChildWindow( HWND hWnd, LPCWSTR text, LPCWSTR clsName )
{
	HWND ret = NULL;


	/*---- バッファを確保 ----*/
	size_t txtLength = text ? wcslen( text ) + 1 : 1;
	size_t clsLength = clsName ? wcslen( clsName ) + 1 : 1;
	wchar_t* winText = new wchar_t[ txtLength ];
	wchar_t* winclsName = new wchar_t[ clsLength ];
	
	
	/*---- ウィンドウの情報を得る ----*/
	GetWindowTextW( hWnd, winText, txtLength );
	GetClassNameW( hWnd, winclsName, clsLength );


	/*---- 判定 ----*/
	if( text == NULL || wcscmp( text, winText ) == 0 )
	{
		if( clsName == NULL || wcscmp( clsName, winclsName ) == 0 )
		{
			ret = hWnd;
		}
	}


	/*---- バッファを開放 ----*/
	delete[] winclsName;
	delete[] winText;


	/*---- 再帰的にもぐっていくか ----*/
	if( !ret )
	{
		// 子ウィンドウから
		HWND nextWindow = GetWindow( hWnd, GW_CHILD );
		if( nextWindow )
		{
			ret = getChildWindow( nextWindow, text, clsName );
		}

		// 兄弟ウィンドウへ
		if( !ret )
		{
			nextWindow = GetWindow( hWnd, GW_HWNDNEXT );
			if( nextWindow )
			{
				ret = getChildWindow( nextWindow, text, clsName );
			}
		}
	}


	return ret;
}