/*----------------------------------------------------------------------------------------
*
* YAViewer 送信先選択関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVSendSelector.h"
#include "YAVAppAction.h"


#define YAVVELECTOR_CLASSNAME		L"YAVVIEW_SELECTORWINDOWCLASS"


//----------------------------------------------
// 管理構造体定義
//
struct RegSelectorData
{
	HWND		hLeftComboBox;			// 左クリックコンボボックス
	HWND		hCenterComboBox;		// 中クリックコンボボックス
	HWND		hDblClickButton;		// ダブルクリックボタン
	HWND		hLeftLabel;				// 左文字ラベル
	HWND		hCenterLabel;			// 中文字ラベル
};
struct SendSelectTarget
{
	DWORD		target;			// ターゲット
	LPCWSTR		name;			// 名前
};


//----------------------------------------------
// グローバル変数定義
//
static const SendSelectTarget scgSelectTarget[] =
{
	{ SST_CLIPBOARD,						L"クリップボード" },
	{ SST_FACE_EDIT,						L"(´д｀) Edit" },
	{ SST_FACE_EDIT | SST_TEXTBOX_FLAG,		L"(´д｀) ﾃｷｽﾄﾎﾞｯｸｽ" },
	{ SST_ORINRIN_EDITOR,					L"Orinrin Editor" },
	{ SST_ORINRIN_EDITOR | SST_TEXTBOX_FLAG,L"Orinrin ﾚｲﾔﾎﾞｯｸｽ" },

	{ SST_UNKNOWN, NULL }
};


//----------------------------------------------
// グローバル関数定義
//
static void setupDefaultComboItem( HWND hComb, DWORD target );

// ウィンドウメッセージクラッカー
static LRESULT yavCreateSelectorCracker( HWND hWnd, LPCREATESTRUCT cs );
static void yavDestroySelectorCracker( HWND hWnd );
static void yavSizeSelectorCracker( HWND hWnd, WORD mode, INT cx, INT cy );


//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterSelectorWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= sizeof( RegSelectorData* );
	wc.hbrBackground	= (HBRUSH)(COLOR_MENU + 1);
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursorW( NULL, IDC_ARROW );
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= SelectorWindowProc;
	wc.lpszClassName	= YAVVELECTOR_CLASSNAME;

	return (::RegisterClassExW( &wc ) != NULL);
}


//----------------------------------------------
// 送信先選択ウィンドウ生成
//
HWND CreateSelectorWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle )
{
	DWORD style = WS_CHILD | WS_VISIBLE | addStyle;
	DWORD exStyle = 0;

	RegSelectorData* selectorData = new RegSelectorData();
	HWND ret = CreateWindowExW( exStyle, YAVVELECTOR_CLASSNAME, L"", style, 0, 0, 0, 0, hParent, NULL, hInstance, selectorData );
	return ret;
}


//----------------------------------------------
// 送信先選択ウィンドウ標準コールバック
//
LRESULT CALLBACK SelectorWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_CREATE:		return yavCreateSelectorCracker( hWnd, (LPCREATESTRUCT)lParam );
	case WM_DESTROY:	yavDestroySelectorCracker( hWnd ); break;
	case WM_SIZE:		yavSizeSelectorCracker( hWnd, wParam, LOWORD( lParam ), HIWORD( lParam ) ); return 0;
	}
	return ::DefWindowProcW( hWnd, msg, wParam, lParam );
}


//----------------------------------------------
// 送信先 設定/取得
//
void SetSelectSendSelector( HWND hWnd, SendSelector st, DWORD target )
{
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );
	if( data )
	{
		HWND hComb = NULL;
		switch( st )
		{
		case SendTargetLeftClick:
			hComb = data->hLeftComboBox;
			break;
		case SendTargetCenterClick:
			hComb = data->hCenterComboBox;
			break;
		}
		if( hComb )
		{
			for( int i = 0; scgSelectTarget[ i ].name != NULL; i++ )
			{
				if( scgSelectTarget[ i ].target == target )
				{
					SendMessageW( hComb, CB_SETCURSEL, i, 0 );
					break;
				}
			}
		}
	}
}
DWORD GetSelectSendSelector( HWND hWnd, SendSelector st )
{
	DWORD ret = SST_UNKNOWN;
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );
	if( data )
	{
		HWND hComb = NULL;
		switch( st )
		{
		case SendTargetLeftClick:
			hComb = data->hLeftComboBox;
			break;
		case SendTargetCenterClick:
			hComb = data->hCenterComboBox;
			break;
		}
		if( hComb )
		{
			ret = scgSelectTarget[ SendMessageW( hComb, CB_GETCURSEL, 0, 0 ) ].target;
		}
	}
	return ret;
}


//----------------------------------------------
// 選択方式 設定/取得
//
void SetSelectDoubleClick( HWND hWnd, bool dblclick )
{
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );
	if( data )
	{
		if( dblclick )
		{
			SendMessage( data->hDblClickButton, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendMessage( data->hDblClickButton, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
	}
}
bool GetSelectDoubleClick( HWND hWnd )
{
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );
	if( data )
	{
		return (SendMessage( data->hDblClickButton, BM_GETCHECK, 0, 0 ) != FALSE);
	}
	return false;
}


//----------------------------------------------
// コンボボックスのアイテムを追加
//
static void setupDefaultComboItem( HWND hComb, DWORD target )
{
	int selTarget = 0;
	for( int i = 0; scgSelectTarget[ i ].name != NULL; i++ )
	{
		SendMessageW( hComb, CB_ADDSTRING, 0, (LPARAM)scgSelectTarget[ i ].name );
		if( scgSelectTarget[ i ].target == target )
		{
			selTarget = i;
		}
	}
	SendMessageW( hComb, CB_SETCURSEL, selTarget, 0 );
}


//----------------------------------------------
// WM_CREATE クラッカー
//
static LRESULT yavCreateSelectorCracker( HWND hWnd, LPCREATESTRUCT cs )
{
	/*---- データをセット ----*/
	RegSelectorData* data = (RegSelectorData*)cs->lpCreateParams;
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	/*---- ラベル追加 ----*/
	data->hLeftLabel	= CreateWindowW( L"STATIC", L"左", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );
	data->hCenterLabel	= CreateWindowW( L"STATIC", L"中", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );


	/*---- コンボボックス追加 ----*/
	DWORD style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST;
	data->hLeftComboBox		= CreateWindowW( L"COMBOBOX", L"", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );
	data->hCenterComboBox	= CreateWindowW( L"COMBOBOX", L"", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );

	style = WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX;
	data->hDblClickButton	= CreateWindowW( L"BUTTON", L"ダブルクリック", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );

	/*---- アイテム設定 ----*/
	setupDefaultComboItem( data->hLeftComboBox, 0 );
	setupDefaultComboItem( data->hCenterComboBox, 0 );


	return 0;
}

//----------------------------------------------
// WM_DESTROY クラッカー
//
static void yavDestroySelectorCracker( HWND hWnd )
{
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );
	if( data )
	{
		SetWindowLongPtrW( hWnd, 0, 0 );
		delete data;
	}
}

//----------------------------------------------
// WM_SIZE クラッカー
//
static void yavSizeSelectorCracker( HWND hWnd, WORD mode, INT cx, INT cy )
{
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );


	/*---- 表示位置、サイズ変更 ----*/
	int lw = 32;
	int width = cx  / 3;
	if( width >= 240 )
	{
		width = 240;
	}
	if( width < 120 )
	{
		width = 120;
	}
	::MoveWindow( data->hLeftLabel, 8, 4, lw - 8, cy - 4, TRUE );
	::MoveWindow( data->hCenterLabel, width + 8, 4, lw - 8, cy - 4, TRUE );
	::MoveWindow( data->hLeftComboBox, lw, 0, width - lw, 300, TRUE );
	::MoveWindow( data->hCenterComboBox, width + lw, 0, width - lw, 300, TRUE );
	

	// ダブルクリック
	MoveWindow( data->hDblClickButton, width * 2 + 8, 0, width - 8, cy - 4, TRUE );
}
