/*----------------------------------------------------------------------------------------
*
* YAViewer ファイルタブ関係
*
*----------------------------------------------------------------------------------------*/
#include <tchar.h>
#include "YAVFileTab.h"
#include "YAVAppAction.h"
#include "YAVResource.h"
#include <commctrl.h>


#define TAB_BUTTON_MODE


//----------------------------------------------
// マクロ定義
//
#define YFT_STATE_SELECTED	0x0001			/* 選択されている */
#define YFT_STATE_HOVER		0x0002			/* アイテム上にマウスがある */
#define YFT_STATE_MOVING	0x0004			/* 移動中 */

#define YFT_TAB_STYLE			(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | TCS_MULTILINE)
#define YFT_TAB_BUTTON_STYLE	(YFT_TAB_STYLE | TCS_BUTTONS | TCS_HOTTRACK)


//----------------------------------------------
// 構造体定義
//
struct YavFileTabData
{
	HWND		hFileTab;					// ファイルタブ本体
	BOOL		tabButton;					// ファイルタブボタンモード
};
struct YavFileTabSubData
{
	WNDPROC		oldTabWndProc;				// 元のウィンドウプロシージャ

	// MouseMoveキャッシュ
	int			oldIndex;					// 選択アイテム
	bool		oldCloseHover;				// 閉じるの上にマウスが乗っていたか
	bool		mouseTracking;				// マウスのトラッキング処理を行ったか
	int			mmDrawCount;				// MouseMoveで描画した回数

	// アイコン関係
	HICON		hTabClose;					// 閉じるアイコン
	HICON		hTabCloseSelect;			// 選択中の閉じるアイコン

	// 作業用
	INT			moveTagIndex;				// 移動対象タブ
	INT			closeSkip;					// 一度だけタブ閉じボタンを無視するか
};
struct YavFileTabDrawParam
{
	YavFileTabSubData*	data;				// ウィンドウ所持データ
	RECT*				pRect;				// 描画矩形
	TCITEMW*			pItem;				// タブアイテム
	YavFileTabParam*	param;				// タブパラメータ
	DWORD				state;				// ステータス

	// YFT_STATE_HOVERで有効なもの
	POINT*				hoverPoint;			// カーソル位置
};

//----------------------------------------------
// グローバル関数定義
//
static YavFileTabData* getFileTabData( HWND hWnd );

// ウィンドウプロシージャ
static LRESULT CALLBACK yavFileTabProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

// メッセージクラック
static LRESULT yavFileTabCreateMessage( HWND hWnd, LPCREATESTRUCT cs );
static void yavFileTabDestroyMessage( HWND hWnd );
static LRESULT yavFileTabSizeMessage( HWND hWnd, int type, WORD width, WORD height );
static LRESULT yavFileTabNotifyMessage( HWND hWnd, UINT id, NMHDR* pNotifyHead );


//----------------------------------------------
// サブクラス関係グローバル関数定義
//
static void setupFileTab( HWND hWnd );
static YavFileTabSubData* getFileTabSubData( HWND hWnd );

// サブクラス化用ウィンドウプロシージャ
static LRESULT CALLBACK yavFileTabSubClassProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static LRESULT yavFileTabSubClassMouseMoveCrack( HWND hWnd, int x, int y, UINT keyFlags );
static LRESULT yavFileTabSubClassPaintCrack( HWND hWnd );
static void yavFileTabItemDraw( HDC hDC, int idx, YavFileTabDrawParam* drawParam, bool isButton );


//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterYAVFileTabClass( HINSTANCE hInstance )
{
	/*---- アプリケーションウィンドウクラス登録 ----*/
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize		= sizeof( wc );
	wc.cbWndExtra	= sizeof( YavFileTabData* );
	wc.hInstance	= hInstance;
	wc.style		= CS_VREDRAW | CS_HREDRAW;
	wc.hbrBackground= (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpfnWndProc	= yavFileTabProc;
	wc.lpszClassName= L"YAViewer FileTabWindowClass";

	return (RegisterClassExW( &wc ) != NULL);
}


//----------------------------------------------
// ファイルタブウィンドウ生成
//
HWND CreateFileTabWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle, DWORD id )
{
	DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	return CreateWindowW( L"YAViewer FileTabWindowClass", L"", style, 0, 0, 0, 0, hParent, 0, hInstance, (LPVOID)id );
}


//----------------------------------------------
// ファイルタブウィンドウ取得
//	子ウィンドウの登録先
//
HWND GetFileTabWindow( HWND hWnd )
{
	YavFileTabData* data = getFileTabData( hWnd );
	if( data )
	{
		return data->hFileTab;
	}
	return hWnd;
}

//----------------------------------------------
// ファイルタブ - xボタン矩形取得
//
bool GetFileTabCloseRect( HWND hWnd, int idx, RECT* dst )
{
	RECT itemRect;
	if( dst == NULL || !TabCtrl_GetItemRect( hWnd, idx, &itemRect ) )
	{
		return false;
	}
	return GetFileTabCloseRectToItemRect( &itemRect, dst );
}
bool GetFileTabCloseRectToItemRect( const RECT* tabItemRect, RECT* dst )
{
	// 引数チェック
	if( !(tabItemRect && dst) )
	{
		return false;
	}


	// サイズ設定
	int padding = 2;
	int BoxSize = tabItemRect->bottom - tabItemRect->top - (padding*2);
	
	dst->right	= tabItemRect->right - padding;
	dst->top	= tabItemRect->top + padding;
	dst->left	= dst->right - BoxSize;
	dst->bottom	= dst->top + BoxSize;

	return true;
}

//----------------------------------------------
// ファイルタブ - xボタンスキップ判定
//
bool FileTabCloseSkip( HWND hWnd, int* moveIndex )
{
	bool ret = false;

	YavFileTabData* data = getFileTabData( hWnd );
	if( data && data->hFileTab )
	{
		YavFileTabSubData* sd = getFileTabSubData( data->hFileTab );
		if( sd )
		{
			ret = (sd->closeSkip == 1);
			sd->closeSkip = 0;

			if( moveIndex )
			{
				*moveIndex = sd->moveTagIndex;
			}
		}
	}

	return ret;
}

//----------------------------------------------
// ファイルタブ - タブボタンモード設定
//
void SetFileTabButtonMode( HWND hWnd, bool buttom )
{
	YavFileTabData* data = getFileTabData( hWnd );
	if( data )
	{
		data->tabButton = buttom;
		if( buttom )
		{
			SetWindowLongPtr( data->hFileTab, GWL_STYLE, YFT_TAB_BUTTON_STYLE );
			TabCtrl_SetItemSize( data->hFileTab, 10, 18 );
		}
		else
		{
			SetWindowLongPtr( data->hFileTab, GWL_STYLE, YFT_TAB_STYLE );
			TabCtrl_SetItemSize( data->hFileTab, 10, 18 );
		}
		SetWindowPos( data->hFileTab, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
	}
}
bool GetFileTabButtonMode( HWND hWnd )
{
	YavFileTabData* data = getFileTabData( hWnd );
	if( data )
	{
		return data->tabButton != FALSE;
	}
	return false;
}


//----------------------------------------------
// ファイルタブデータ取得
//
static YavFileTabData* getFileTabData( HWND hWnd )
{
	return (YavFileTabData*)GetWindowLongPtrW( hWnd, 0 );
}


//----------------------------------------------
// ウィンドウプロシージャ
//
static LRESULT CALLBACK yavFileTabProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( msg >= TCM_FIRST )
	{
		YavFileTabData* data = getFileTabData( hWnd );
		if( data )
		{
			return SendMessageW( data->hFileTab, msg, wParam, lParam );
		}
	}

	switch( msg )
	{
	default:			return ::DefWindowProcW( hWnd, msg, wParam, lParam );
	case WM_CREATE:		return yavFileTabCreateMessage( hWnd, (LPCREATESTRUCT)lParam );
	case WM_DESTROY:	yavFileTabDestroyMessage( hWnd ); break;
	case WM_SIZE:		return yavFileTabSizeMessage( hWnd, (int)wParam, LOWORD( lParam ), HIWORD( lParam ) );
	case WM_NOTIFY:		return yavFileTabNotifyMessage( hWnd, (UINT)wParam, (NMHDR*)lParam );
	}

	return 0;
}

//----------------------------------------------
// ファイルタブ
//	WM_CREATE クラック
//
static LRESULT yavFileTabCreateMessage( HWND hWnd, LPCREATESTRUCT cs )
{
	/*---- 管理データ生成 ----*/
	YavFileTabData* data = new YavFileTabData;
	ZeroMemory( data, sizeof( YavFileTabData ) );
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	/*---- タブウィンドウ生成 ----*/
	DWORD tabStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | TCS_MULTILINE;
	if( data->tabButton != FALSE ) tabStyle |= TCS_BUTTONS;

	data->hFileTab = CreateWindowExW( 0, WC_TABCONTROL, L"YAViewer FileTab", tabStyle, 0, 0, 0, 0, hWnd, (HMENU)cs->lpCreateParams, cs->hInstance, NULL );
	setupFileTab( data->hFileTab );


	return 0;
}

//----------------------------------------------
// ファイルタブ
//	WM_DESTROY クラック
//
static void yavFileTabDestroyMessage( HWND hWnd )
{
	YavFileTabData* data = getFileTabData( hWnd );
	if( data )
	{
		SetWindowLongPtrW( hWnd, 0, 0 );
		delete data;
	}
}

//----------------------------------------------
// ファイルタブ
//	WM_SIZE クラック
//
static LRESULT yavFileTabSizeMessage( HWND hWnd, int type, WORD width, WORD height )
{
	YavFileTabData* data = getFileTabData( hWnd );

	if( data )
	{
		MoveWindow( data->hFileTab, 0, 0, width, height, TRUE );
	}

	return 0;
}

//----------------------------------------------
// ファイルタブ
//	WM_NOTIFY クラック
//
static LRESULT yavFileTabNotifyMessage( HWND hWnd, UINT id, NMHDR* pNotifyHead )
{
	/*---- 親ウィンドウに通知コードを送信 ----*/
	HWND hParent = ::GetParent( hWnd );
	if( hParent )
	{
		return ::SendMessageW( hParent, WM_NOTIFY, (WPARAM)id, (LPARAM)pNotifyHead );
	}

	return 0;
}



//----------------------------------------------
//
// サブクラス化関係
//
//----------------------------------------------
//----------------------------------------------
// ファイルタブサブクラスデータ取得
//
static YavFileTabSubData* getFileTabSubData( HWND hWnd )
{
	return (YavFileTabSubData*)GetWindowLongPtrW( hWnd, GWLP_USERDATA );
}


//----------------------------------------------
// ファイルタブ 初期化
//
static void setupFileTab( HWND hWnd )
{
	/*---- データ生成 ----*/
	YavFileTabSubData* data = new YavFileTabSubData;
	ZeroMemory( data, sizeof( YavFileTabSubData ) );
	data->oldIndex = -1;
	SetWindowLongPtrW( hWnd, GWLP_USERDATA, (LONG_PTR)data );


	/*---- アイコン生成 -----*/
	data->hTabClose			= LoadIcon( ::GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_TABCLOSE ) );
	data->hTabCloseSelect	= LoadIcon( ::GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_TABCLOSE_SEL ) );


	/*---- サブクラス化 ----*/
	data->oldTabWndProc = (WNDPROC)GetWindowLongPtrW( hWnd, GWLP_WNDPROC );
	SetWindowLongPtrW( hWnd, GWLP_WNDPROC, (LONG_PTR)yavFileTabSubClassProc );


	/*---- フォント設定 ----*/
	SendMessageW( hWnd, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), 0 );
}


//----------------------------------------------
// サブクラス化用ウィンドウプロシージャ
//
static LRESULT CALLBACK yavFileTabSubClassProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	YavFileTabSubData* data = getFileTabSubData( hWnd );
	switch( msg )
	{
	case WM_DESTROY:
		if( data )
		{
			SetWindowLongPtrW( hWnd, GWLP_USERDATA, 0 );

			LRESULT ret = ::CallWindowProcW( data->oldTabWndProc, hWnd, msg, wParam, lParam );
			SetWindowLongPtrW( hWnd, GWLP_WNDPROC, (LONG_PTR)data->oldTabWndProc );

			delete data;

			return ret;
		}
		break;

	case WM_PAINT:
		return yavFileTabSubClassPaintCrack( hWnd );

	case WM_MOUSEMOVE:
		if( GetCapture() != hWnd )
		{
			return yavFileTabSubClassMouseMoveCrack( hWnd, LOWORD( lParam ), HIWORD( lParam ), (UINT)wParam );
		}
		else
		{
			if( data )
			{
				/*---- 移動開始判定 ----*/
				if( data->closeSkip == 0 )
				{
					int				nIndex;
					TCHITTESTINFO	info;

					data->closeSkip = -1;

					// タッチ位置
					GetCursorPos( &info.pt );
					ScreenToClient( hWnd, &info.pt );
					nIndex = TabCtrl_HitTest( hWnd, &info );
					if( nIndex >= 0 )
					{
						data->moveTagIndex = nIndex;
						data->closeSkip = 1;

						::InvalidateRect( hWnd, NULL, FALSE );
					}
				}


				/*---- 閉じるボタン判定 ----*/
				if( data->closeSkip <= 0 )
				{
					return yavFileTabSubClassMouseMoveCrack( hWnd, LOWORD( lParam ), HIWORD( lParam ), (UINT)wParam );
				}
				else
				{
					int				nIndex;
					TCHITTESTINFO	info;

					// タッチ位置
					GetCursorPos( &info.pt );
					ScreenToClient( hWnd, &info.pt );
					nIndex = TabCtrl_HitTest( hWnd, &info );
					if( nIndex >= 0 && nIndex != data->moveTagIndex )
					{
						// データ並び替え
						wchar_t selText[ MAX_PATH ];
						TCITEMW sel;
						sel.mask = TCIF_TEXT | TCIF_PARAM | TCIF_STATE;
						sel.dwStateMask = TCIS_BUTTONPRESSED | TCIS_HIGHLIGHTED;

						sel.pszText = selText;
						sel.cchTextMax = MAX_PATH;

						TabCtrl_GetItem( hWnd, data->moveTagIndex, &sel );
						TabCtrl_DeleteItem( hWnd, data->moveTagIndex );
						TabCtrl_InsertItem( hWnd, nIndex, &sel );

						data->moveTagIndex = nIndex;

						::InvalidateRect( hWnd, NULL, FALSE );
					}
				}
			}
			break;
		}

	case WM_MOUSELEAVE:
		if( data )
		{
			data->mouseTracking = false;
			data->oldCloseHover = false;
			data->oldIndex = -1;
			data->mmDrawCount = 0;
		}
		::InvalidateRect( hWnd, NULL, FALSE );
		break;
	case WM_MOUSEHOVER:
		if( data )
		{
			data->mouseTracking = false;
		}
		break;
		
	default:
		if( data )
		{
			return ::CallWindowProcW( data->oldTabWndProc, hWnd, msg, wParam, lParam );
		}
	}
	return 0;
}

//----------------------------------------------
// サブクラス化 WM_MOUSEMOVEハンドリング
//
static LRESULT yavFileTabSubClassMouseMoveCrack( HWND hWnd, int x, int y, UINT keyFlags )
{
	YavFileTabSubData* data = getFileTabSubData( hWnd );


	/*---- マウスのトラッキング処理 ----*/
	if( data && !data->mouseTracking )
	{// マウスイベントをトラッキングする
        TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = hWnd;
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		TrackMouseEvent( &tme );

		data->mouseTracking = true;
	}


	/*---- カーソル位置取得 ----*/
	TCHITTESTINFO	info;
	info.pt.x = x; info.pt.y = y;
	int nIndex = TabCtrl_HitTest( hWnd, &info );
	bool hover = false;


	/*---- ボタンモードなら再描画 ----*/
	if( GetFileTabButtonMode( ::GetParent( hWnd ) ) )
	{
		if( data->oldIndex != nIndex )
		{
			::InvalidateRect( hWnd, NULL, FALSE );
		}
	}


	/*---- 閉じるアイコン表示 ----*/
	if( nIndex >= 0 )
	{
		HICON hIcon = data->hTabClose;
		HDC hDC = GetDC( hWnd );

		// 閉じるアイコン座標
		RECT rect, itemRect;
		TabCtrl_GetItemRect( hWnd, nIndex, &itemRect );
		GetFileTabCloseRectToItemRect( &itemRect, &rect );

		// 矩形内にあるか判定
		HBRUSH hBrush = NULL;
		HPEN hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_ACTIVEBORDER ) );
		POINT& pt = info.pt;
		if( rect.left <= pt.x && pt.x < rect.right && rect.top <= pt.y && pt.y < rect.bottom )
		{
			hBrush = ::CreateSolidBrush( RGB( 255, 64, 64 ) );
			hIcon = data->hTabCloseSelect;
			hover = true;
		}

		// 条件判定
		if( data->mmDrawCount == 1 )
		{
			// 背景描画
			if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );
			if( hPen )	 hPen = (HPEN)::SelectObject( hDC, hPen );
			::Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
			if( hPen )	 hPen = (HPEN)::SelectObject( hDC, hPen );
			if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );

			// アイコン描画
			::DrawIconEx( hDC, rect.left, rect.top, hIcon, rect.right-rect.left, rect.bottom-rect.top, 0, NULL, DI_NORMAL );
		}

		// ブラシ廃棄
		if( hPen )
		{
			::DeleteObject( hPen );
		}
		if( hBrush )
		{
			::DeleteObject( hBrush );
		}

		::ReleaseDC( hWnd, hDC );
	}


	/*---- 状態を格納しておく ----*/
	if( data->oldIndex != nIndex || data->oldCloseHover != hover )
	{
		if( data->mmDrawCount < 3 )	data->mmDrawCount++;
	}
	else
	{
		data->mmDrawCount = 0;
	}
	data->oldIndex = nIndex;
	data->oldCloseHover = hover;


	return ::CallWindowProcW( data->oldTabWndProc, hWnd, WM_MOUSEMOVE, (WPARAM)keyFlags, (LPARAM)MAKELONG( x, y ) );
}

//----------------------------------------------
// サブクラス化 WM_PAINTハンドリング
//
static LRESULT yavFileTabSubClassPaintCrack( HWND hWnd )
{
	YavFileTabSubData* data = getFileTabSubData( hWnd );
	int nMaxIndex = TabCtrl_GetItemCount( hWnd );
	bool isButton = GetFileTabButtonMode( ::GetParent( hWnd ) );


	/*---- 描画処理 ----*/
	PAINTSTRUCT paint;
	HDC hDC = BeginPaint( hWnd, &paint );


	/*---- デバイスコンテキストを設定 ----*/
	HFONT hOldFont = (HFONT)::SelectObject( hDC, (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 ) );


	/*---- 背景塗りつぶし ----*/
	FillRect( hDC, &paint.rcPaint, ::GetSysColorBrush( COLOR_MENU ) );


	/*---- 表示領域塗りつぶし処理 ----*/
	RECT rect;
	LONG itemHeight, itemPaddingY = 2, itemPaddingX = 4;
	GetClientRect( hWnd, &rect );
	{
		RECT rc;
		TabCtrl_GetItemRect( hWnd, 0, &rc );
		itemHeight = rc.bottom - rc.top;
	}
	rect.top = itemHeight * TabCtrl_GetRowCount( hWnd );
	if( isButton )
	{
		rect.top += itemPaddingY * TabCtrl_GetRowCount( hWnd );
		rect.right -= itemPaddingX;
	}
	else
	{
		rect.top += itemPaddingY;
		rect.right -= itemPaddingX;
	}
	FillRect( hDC, &rect, ::GetSysColorBrush( COLOR_WINDOW ) );


	/*---- 描画範囲矩形 ----*/
	HPEN hBorderPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_ACTIVEBORDER ) );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hBorderPen );
	if( !isButton )
	{
		::Rectangle( hDC, rect.left, rect.top-1, rect.right+1, rect.bottom-1 );
	}


	/*---- 現在のカーソル位置を取得 ----*/
	TCHITTESTINFO info;
	GetCursorPos( &info.pt );
	ScreenToClient( hWnd, &info.pt );
	int nHoverIndex = TabCtrl_HitTest( hWnd, &info );


	/*---- アイテムを全件表示 ----*/
	for( int i = 0; i < nMaxIndex; i++ )
	{
		/*---- 表示パラメータ ----*/
		YavFileTabDrawParam drawParam;
		ZeroMemory( &drawParam, sizeof( drawParam ) );
		drawParam.data = data;


		/*---- アイテムの情報を取得 ----*/
		wchar_t text[ MAX_PATH ];
		TCITEMW item;
		item.mask = TCIF_TEXT | TCIF_PARAM | TCIF_STATE;
		item.pszText = text;
		item.cchTextMax = MAX_PATH;
		item.dwStateMask = TCIS_BUTTONPRESSED | TCIS_HIGHLIGHTED;
		TabCtrl_GetItem( hWnd, i, &item );
		drawParam.pItem = &item;
		

		/*---- 拡張ステータス適用 ----*/
		if( data->closeSkip > 0 )
		{
			if( data->moveTagIndex == i )
			{
				drawParam.state = YFT_STATE_MOVING;
			}
		}
		else
		{
			if( item.dwState & TCIS_BUTTONPRESSED )
			{
				drawParam.state |= YFT_STATE_SELECTED;
			}
			if( i == nHoverIndex )
			{
				drawParam.state |= YFT_STATE_HOVER;
				drawParam.hoverPoint = &info.pt;
			}
		}


		/*---- パラメータ取得 ----*/
		if( item.lParam )
		{
			drawParam.param = (YavFileTabParam*)item.lParam;
		}


		/*---- 表示矩形取得 ----*/
		RECT itemRect;
		TabCtrl_GetItemRect( hWnd, i, &itemRect );
		drawParam.pRect = &itemRect;


		/*---- 表示 ----*/
		yavFileTabItemDraw( hDC, i, &drawParam, isButton );
	}


	/*---- デバイスコンテキストを復帰 ----*/
	::SelectObject( hDC, hOldFont );
	::SelectObject( hDC, hOldPen );


	/*---- 制御をOSへ返す ----*/
	::EndPaint( hWnd, &paint );
	return 0;
}

//----------------------------------------------
// タブ表示関数
//
static void yavFileTabItemDraw( HDC hDC, int idx, YavFileTabDrawParam* drawParam, bool isButton )
{
	COLORREF oldTextColor = ::GetTextColor( hDC );
	HPEN hPen = NULL;


	/*---- ペンの生成 ----*/
	if( drawParam->state & YFT_STATE_SELECTED && drawParam->param->useTabColor )
	{
		hPen = ::CreatePen( PS_SOLID, 1, drawParam->param->tabColor );
	}


	/*---- 背景表示 ----*/
	if( drawParam->state & YFT_STATE_MOVING )
	{
		::FillRect( hDC, drawParam->pRect, ::GetSysColorBrush( COLOR_HOTLIGHT ) );

		// タブ色から文字色を設定
		COLORREF textColor = RGB( 255, 255, 255 );
		oldTextColor = ::SetTextColor( hDC, textColor );
	}
	else if( drawParam->state & YFT_STATE_SELECTED )
	{
		::FillRect( hDC, drawParam->pRect, ::GetSysColorBrush( COLOR_WINDOW ) );
	}
	else if( drawParam->state & YFT_STATE_HOVER )
	{
		::FillRect( hDC, drawParam->pRect, ::GetSysColorBrush( COLOR_MENUHILIGHT ) );
	}
	else if( drawParam->param->useTabColor )
	{
		HBRUSH hBrush = ::CreateSolidBrush( drawParam->param->tabColor );
		::FillRect( hDC, drawParam->pRect, hBrush );
		::DeleteObject( hBrush );

		// タブ色から文字色を設定
		COLORREF textColor = RGB( 0, 0, 0 ), c = drawParam->param->tabColor;
		if( ((GetRValue( c ) + GetGValue( c ) + GetBValue( c )) / 3) <= 162 )
		{
			textColor = RGB( 255, 255, 255 );
		}
		oldTextColor = ::SetTextColor( hDC, textColor );
	}


	/*---- タブ領域区切り線 ----*/
	if( isButton )
	{
		POINT points[] =
		{
			{ drawParam->pRect->left, drawParam->pRect->bottom },
			{ drawParam->pRect->left, drawParam->pRect->top },
			{ drawParam->pRect->right-1,drawParam->pRect->top },
			{ drawParam->pRect->right-1,drawParam->pRect->bottom },
			{ drawParam->pRect->left, drawParam->pRect->bottom },
		};
		if( hPen ) hPen = (HPEN)::SelectObject( hDC, hPen );
		::Polyline( hDC, points, sizeof( points ) / sizeof( points[0] ) );
		if( hPen )
		{
			hPen = (HPEN)::SelectObject( hDC, hPen );
			::DeleteObject( hPen );
		}
	}
	else
	{
		POINT points[] =
		{
			{ drawParam->pRect->left, drawParam->pRect->bottom-1 },
			{ drawParam->pRect->left, drawParam->pRect->top },
			{ drawParam->pRect->right-1,drawParam->pRect->top },
			{ drawParam->pRect->right-1,drawParam->pRect->bottom }
		};
		if( hPen ) hPen = (HPEN)::SelectObject( hDC, hPen );
		::Polyline( hDC, points, sizeof( points ) / sizeof( points[0] ) );
		if( hPen )
		{
			hPen = (HPEN)::SelectObject( hDC, hPen );
			::DeleteObject( hPen );
		}
	}



	/*---- タブ文字表示 ----*/
	RECT textRect = *(drawParam->pRect);
	textRect.bottom -= 2;
	::SetBkMode( hDC, TRANSPARENT );
	::DrawTextW( hDC, drawParam->pItem->pszText, -1, &textRect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE );
	::SetTextColor( hDC, oldTextColor );


	/*---- 閉じるアイコン表示 ----*/
	if( !drawParam->param->disableClose && (drawParam->state & YFT_STATE_HOVER) )
	{
		HICON hIcon = drawParam->data->hTabClose;

		// 閉じるアイコン座標
		RECT rect;
		GetFileTabCloseRectToItemRect( drawParam->pRect, &rect );

		// 矩形内にあるか判定
		HBRUSH hBrush = NULL;
		POINT pt = *(drawParam->hoverPoint);
		if( rect.left <= pt.x && pt.x < rect.right && rect.top <= pt.y && pt.y < rect.bottom )
		{
			hBrush = ::CreateSolidBrush( RGB( 255, 64, 64 ) );
			hIcon = drawParam->data->hTabCloseSelect;
		}

		// 背景描画
		if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );
		::Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
		if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );

		// アイコン描画
		::DrawIconEx( hDC, rect.left, rect.top, hIcon, rect.right-rect.left, rect.bottom-rect.top, 0, NULL, DI_NORMAL );

		// ブラシ廃棄
		if( hBrush )
		{
			::DeleteObject( hBrush );
		}
	}



	/*---- 文字色戻す ----*/
	::SetTextColor( hDC, oldTextColor );
}

