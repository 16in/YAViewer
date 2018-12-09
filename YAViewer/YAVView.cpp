/*----------------------------------------------------------------------------------------
*
* YAViewer View関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVView.h"
#include "YAVViewDetail.h"
#include <commctrl.h>
#include <stdio.h>
#include "YAVSendSelector.h"
#include "YAVWindow.h"


#define YAVVIEW_CLASSNAME		L"YAVVIEW_VIEWWINDOWCLASS"
#define YAVVIEW_PAGE_CLASSNAME	L"YAVVIEW_VIEWPAGEWINDOWCLASS"


//----------------------------------------------
// グローバル関数定義
//
static RegViewWindowData* releaseViewWindowData( HWND hViewWnd, bool del = false );
static bool registerViewPageWindowClass( HINSTANCE hInstance );
static HWND createViewPageWindow( HWND hParent, RegViewWindowData* viewData, aafile::AAFilePage*	page );
static LRESULT CALLBACK viewPageWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static HWND getSendMessageWindow( HWND hWnd );

static LRESULT viewCreateMessage( HWND hWnd, LPCREATESTRUCTW cs );


//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterViewWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= sizeof( RegViewWindowData* );
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursorW( NULL, IDC_ARROW );
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= ViewWindowProc;
	wc.lpszClassName	= YAVVIEW_CLASSNAME;

	return (::RegisterClassExW( &wc ) != NULL) && registerViewPageWindowClass( hInstance );
}


//----------------------------------------------
// アプリケーションウィンドウ生成
//
HWND CreateViewWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle )
{
	DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | addStyle;
	DWORD exStyle = 0;

	HWND ret = CreateWindowExW( exStyle, YAVVIEW_CLASSNAME, L"", style, 0, 0, 0, 0, hParent, NULL, hInstance, new RegViewWindowData() );
	if( ret )
	{
		SetFocus( ret );
	}

	return ret;
}


//----------------------------------------------
// ビューウィンドウ標準コールバック
//
LRESULT CALLBACK ViewWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_CREATE:
		return viewCreateMessage( hWnd, (LPCREATESTRUCTW)lParam );

	case WM_DESTROY:
		releaseViewWindowData( hWnd, true );
		break;

	case WM_ERASEBKGND:
		// クリアしない
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC hDC = BeginPaint( hWnd, &paint );
			FillRect( hDC, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW + 1) );
			if( CheckViewWindowChild( hWnd ) )
			{
				EndPaint( hWnd, &paint );
				return ::DefWindowProcW( hWnd, msg, wParam, lParam );
			}
			else
			{
				EndPaint( hWnd, &paint );
			}
		}
		break;
	case WM_SIZE:
		SetViewWindowSize( hWnd, LOWORD( lParam ), HIWORD( lParam ) );
		break;

	case WM_VSCROLL:
		{
			SCROLLINFO sc;
			ZeroMemory( &sc, sizeof( sc ) );
			sc.cbSize = sizeof( sc );
			sc.fMask = SIF_ALL;
			::GetScrollInfo( hWnd, SB_VERT, &sc );

			switch( LOWORD( wParam ) )
			{
			case SB_LINEUP:			// 1 行上へスクロール
				if( --sc.nPos < sc.nMin )
				{
					sc.nPos = sc.nMin;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_LINEDOWN:		// 1 行下へスクロール
				if( (++sc.nPos + sc.nPage) >= (UINT)sc.nMax )
				{
					sc.nPos = sc.nMax - sc.nPage;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_PAGEUP:			// 1 ページ上へスクロール
				if( (sc.nPos -= sc.nPage) < sc.nMin )
				{
					sc.nPos = sc.nMin;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_PAGEDOWN:		// 1 ページ下へスクロール 
				if( ((sc.nPos += sc.nPage) + sc.nPage) >= (UINT)sc.nMax )
				{
					sc.nPos = sc.nMax - sc.nPage;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_THUMBPOSITION:	// 絶対位置へスクロール
			case SB_THUMBTRACK:		// 指定位置へスクロール ボックスをドラッグ。
				sc.nPos = HIWORD( wParam );
				sc.fMask = SIF_POS;
				break;
			case SB_TOP:			// [HOME]キーが押された
				sc.nPos = sc.nMin;
				sc.fMask = SIF_POS;
				break;
			case SB_BOTTOM:			// [END]キーが押された
				sc.nPos = sc.nMax - sc.nPage;
				sc.fMask = SIF_POS;
				break;
			case SB_ENDSCROLL:
				break;
			}
			::SetScrollInfo( hWnd, SB_VERT, &sc, TRUE );


			/*---- 表示状態を更新する ----*/
			RECT rect;
			GetClientRect( hWnd, &rect );
			SetViewWindowSize( hWnd, rect.right, rect.bottom );
			::InvalidateRect( hWnd, &rect, FALSE );
		}
		break;
		
	case WM_MOUSEWHEEL:
		if( GET_WHEEL_DELTA_WPARAM( wParam ) < 0 )
		{
			SendMessageW( hWnd, WM_VSCROLL, MAKELONG( SB_LINEDOWN, 0 ), 0 );
		}
		else
		{
			SendMessageW( hWnd, WM_VSCROLL, MAKELONG( SB_LINEUP, 0 ), 0 );
		}
		break;

	case WM_LBUTTONDOWN:
		SetFocus( hWnd );
		break;

	//---------------------------------------------
	// キー入力処理
	//
	case WM_KEYDOWN:
		switch( wParam )
		{
		case VK_PRIOR: case VK_UP:
			SendMessageW( hWnd, WM_VSCROLL, MAKELONG( SB_LINEUP, 0 ), 0 );
			break;
		case VK_NEXT: case VK_DOWN:
			SendMessageW( hWnd, WM_VSCROLL, MAKELONG( SB_LINEDOWN, 0 ), 0 );
			break;
		}
		break;

	//---------------------------------------------
	// 拡張メッセージ類
	//
	case WM_VIEW_SELECTITEM:
		SendMessageW( getSendMessageWindow( hWnd ), WM_VIEW_SELECTITEM, wParam, lParam );
		break;

	default:
		return ::DefWindowProcW( hWnd, msg, wParam, lParam );
	}

	return 0;
}
static LRESULT viewCreateMessage( HWND hWnd, LPCREATESTRUCTW cs )
{
	/*---- データを設定 ----*/
	RegViewWindowData* data = (RegViewWindowData*)cs->lpCreateParams;
	::SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	return 0;
}


//----------------------------------------------
// スクロール位置設定/取得
//
void SetViewScrollPosition( HWND hViewWnd, int pos )
{
	if( hViewWnd )
	{
		SendMessageW( hViewWnd, WM_VSCROLL, MAKELONG( SB_THUMBPOSITION, pos ), 0 );
	}
}
void SetViewScrollPositionByPage( HWND hViewWnd, int page )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		if( data->mode == VIEWMODE_NORMAL )
		{
			SetViewScrollPosition( hViewWnd, page );
		}
		else
		{
			// 何段目か算出してそれを送信する
			RECT rect;
			GetClientRect( hViewWnd, &rect );

			int width = rect.right - rect.left;
			width = (width / data->categorySize) * data->categorySize;

			SetViewScrollPosition( hViewWnd, page / (width / data->categorySize) );
		}
	}
}
int  GetViewScrollPosition( HWND hViewWnd )
{
	int ret = 0;
	if( hViewWnd )
	{
		SCROLLINFO sc;
		ZeroMemory( &sc, sizeof( sc ) );
		sc.cbSize = sizeof( sc );
		sc.fMask = SIF_ALL;
		::GetScrollInfo( hViewWnd, SB_VERT, &sc );
		ret = sc.nPos;
	}
	return ret;
}


//----------------------------------------------
// 表示フォント設定
//
void SetViewFont( HWND hViewWnd, HFONT hFont )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		data->hFont = hFont;

		// 文字幅テーブル更新
		HDC hdc = GetDC( hViewWnd );
		HFONT hOldFont = (HFONT)SelectObject( hdc, hFont );
		
		SIZE size;
		::GetTextExtentPoint32W( hdc, L" ", 1, &size );
		data->lfHeight = size.cy;
		GetCharWidth32W( hdc, 0x0000, 0xffff, data->charWidth );

		SelectObject( hdc, hOldFont );
		ReleaseDC( hViewWnd, hdc );
	}
}
HFONT GetViewFont( HWND hViewWnd )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		return data->hFont;
	}
	return NULL;
}


//----------------------------------------------
// 表示モード設定
//
void SetViewMode( HWND hViewWnd, int mode )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		data->mode = mode;

		/*---- スクロールバーの状態を変更 ----*/
		SCROLLINFO sc;
		ZeroMemory( &sc, sizeof( sc ) );
		sc.cbSize = sizeof( sc );
		sc.fMask = SIF_POS;
		::SetScrollInfo( hViewWnd, SB_VERT, &sc, FALSE );

		/*---- 領域を再描画 ----*/
		RECT rect;
		GetClientRect( hViewWnd, &rect );
		SetViewWindowSize( hViewWnd, rect.right, rect.bottom );
	}
}
int GetViewMode( HWND hViewWnd )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		return data->mode;
	}
	return 0;
}


//----------------------------------------------
// ツールヒント表示設定/取得
//
void SetViewToolTip( HWND hViewWnd, bool enable )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		data->enableTooltip = enable;
		SetTooltipEnablePage( hViewWnd, enable );
	}
}
bool GetViewToolTip( HWND hViewWnd )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		return data->enableTooltip;
	}
	return false;
}


//----------------------------------------------
// カテゴリ表示サイズ設定
//
void SetViewCategorySize( HWND hViewWnd, int size )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		data->categorySize = size;
		
		RECT rect;
		GetClientRect( hViewWnd, &rect );
		SetViewWindowSize( hViewWnd, rect.right, rect.bottom );
	}
}
int GetViewCategorySize( HWND hViewWnd )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		return data->categorySize;
	}
	return 0;
}


//----------------------------------------------
// 表示ファイル設定
//
void SetViewFile( HWND hViewWnd, aafile::AAFile* file )
{
	if( hViewWnd )
	{
		/*---- データ作り直し ----*/
		RegViewWindowData* data = releaseViewWindowData( hViewWnd );
		SetWindowLongPtrW( hViewWnd, 0, (LONG_PTR)data );


		/*---- スクロールバーの状態を変更 ----*/
		SCROLLINFO sc;
		ZeroMemory( &sc, sizeof( sc ) );
		sc.cbSize = sizeof( sc );
		sc.fMask = SIF_POS;
		::SetScrollInfo( hViewWnd, SB_VERT, &sc, FALSE );


		/*---- データを格納していく ----*/
		data->file = file;
		if( file )
		{
			data->pages = new HWND[ data->file->pageCount ];
			for( size_t i = 0; i < data->file->pageCount; i++ )
			{
				data->pages[ i ] = createViewPageWindow( hViewWnd, data, &data->file->pageList[ i ] );
			}

			
			/*---- ページを生成する ----*/
			AllPaintPage( hViewWnd );


			/*---- ウィンドウサイズを設定する ----*/
			RECT rect;
			GetClientRect( hViewWnd, &rect );
			SetViewWindowSize( hViewWnd, rect.right, rect.bottom );
		}
	}
}
aafile::AAFile* GetViewFile( HWND hViewWnd )
{
	RegViewWindowData* data = releaseViewWindowData( hViewWnd );
	if( data ) return data->file;

	return NULL;
}


//----------------------------------------------
// 拡張メッセージ送信先設定
//
void SetViewSendMessageRootWindow( HWND hViewWnd, bool sendRoot )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data ) data->sendRoot = sendRoot;
}
bool GetViewSendMessageRootWindow( HWND hViewWnd )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data ) return data->sendRoot;

	return false;
}


//----------------------------------------------
// ビューウィンドウ登録データの削除
//
static RegViewWindowData* releaseViewWindowData( HWND hViewWnd, bool del )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		if( data->pages )
		{
			for( size_t i = 0; i < data->file->pageCount; i++ )
			{
				::DestroyWindow( data->pages[ i ] );
			}
			delete[] data->pages;
			data->pages = NULL;
		}
		if( del )
		{
			SetWindowLongPtrW( hViewWnd, 0, 0 );
			delete data;
			data = NULL;
		}
	}

	return data;
}


//----------------------------------------------
// ページビューウィンドウクラス登録
//
static bool registerViewPageWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= sizeof( RegViewWindowData* );
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursorW( NULL, IDC_ARROW );
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= viewPageWindowProc;
	wc.lpszClassName	= YAVVIEW_PAGE_CLASSNAME;

	return (::RegisterClassExW( &wc ) != NULL);
}

//----------------------------------------------
// ページビューウィンドウクラス生成
//
static HWND createViewPageWindow( HWND hParent, RegViewWindowData* viewData, aafile::AAFilePage* page )
{
	DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER;
	DWORD exStyle = 0;

	RegViewPageData* pageData = new RegViewPageData( viewData->charWidth, &viewData->lfHeight );
	pageData->page = page;

	HWND ret = CreateWindowExW( exStyle, YAVVIEW_PAGE_CLASSNAME, L"", style, 0, 0, 0, 0, hParent, NULL, NULL, pageData );
	if( !ret )
	{
		delete pageData;
	}

	return ret;
}

//----------------------------------------------
// ページビューウィンドウプロシージャ
//
static LRESULT CALLBACK viewPageWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_CREATE:
		{
			/*---- 初期化パラメータを得る ----*/
			LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
			RegViewPageData* pageData = (RegViewPageData*)cs->lpCreateParams;


			/*---- ウィンドウハンドルにデータを関連付ける ----*/
			::SetWindowLongPtrW( hWnd, 0, (LONG_PTR)pageData );


			/*---- 表示する文字列を設定 ----*/
			pageData->toolString = new wchar_t[ pageData->page->valueLength + 1 ];
			wcsncpy_s( pageData->toolString, pageData->page->valueLength + 1, pageData->page->value, pageData->page->valueLength );
		}
		break;

	case WM_DESTROY:
		{
			RegViewPageData* pageData = GetViewPageData( hWnd );
			SetWindowLongPtrW( hWnd, 0, 0 );
			if( pageData )
			{
				/*---- メモリデバイスコンテキスト廃棄 ----*/
				if( pageData->hDeviceContext )
				{
					::SelectObject( pageData->hDeviceContext, pageData->hOldDeviceContextBMP );
					if( pageData->hDeviceContextBMP )
					{
						::DeleteObject( pageData->hDeviceContextBMP );
					}
					::DeleteDC( pageData->hDeviceContext );
				}


				/*---- メモリ領域の開放 ----*/
				if( pageData->toolString )
				{
					delete[] pageData->toolString;
					pageData->toolString = NULL;
				}


				/*---- 頁データ廃棄 ----*/
				delete pageData;
			}
		}
		break;

	case WM_PAINT:
		{
			/*---- メモリデバイスコンテキストをコピー ----*/
			PAINTSTRUCT paint;
			HDC hdc = BeginPaint( hWnd, &paint );

			RegViewWindowData*	view = GetViewWindowData( GetParent( hWnd ) );
			RegViewPageData*	page = GetViewPageData( hWnd );

			PaintPage( view, hWnd, page );
			CopyPageData( hdc, view, page );


			if( page->hover )
			{
				/*---- 選択枠表示 ----*/
				RECT rect;
				GetClientRect( hWnd, &rect );

				HBRUSH hOldBrush = (HBRUSH)SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
				HPEN hOldPen = (HPEN)SelectObject( hdc, GetStockObject( DC_PEN ) );
				COLORREF oldColor = SetDCPenColor( hdc, RGB( 0, 0, 255 ) );
				HFONT hOldFont = (HFONT)SelectObject( hdc, view->hFont );

				Rectangle( hdc, rect.left, rect.top, rect.right, rect.bottom );


				/*---- 情報テキスト表示 ----*/
				COLORREF oldTextColor = SetTextColor( hdc, RGB( 255, 0, 255 ) );

				wchar_t str[ 260 ];
				swprintf_s( str, L"%d x %d行", page->width, page->lfCount );
				TextOutW( hdc, 1, 1, str, (int)wcslen( str ) );

				SetTextColor( hdc, oldTextColor );

				SelectObject( hdc, hOldFont );
				SetDCPenColor( hdc, oldColor );
				SelectObject( hdc, hOldPen );
				SelectObject( hdc, hOldBrush );
			}

			EndPaint( hWnd, &paint );
		}
		return ::DefWindowProcW( hWnd, msg, wParam, lParam );
		break;

	case WM_MOUSEMOVE:
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data && !data->mouseTracking )
			{// マウスイベントをトラッキングする
	            TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.hwndTrack = hWnd;
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.dwHoverTime = 1;
				TrackMouseEvent( &tme );

				data->mouseTracking = true;

				// ついでに親ウィンドウにフォーカスを移す
				SetFocus( GetParent( hWnd ) );
			}

			if( data->hToolHint == NULL )
			{
				/*---- ツールヒントウィンドウ生成 ----*/
				data->hToolHint = CreateWindowExW( 0, TOOLTIPS_CLASSW, L"", TTS_NOPREFIX | TTS_ALWAYSTIP,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, GetModuleHandle( NULL ), NULL );

				// ツールヒント生成
				TOOLINFOW info;
				ZeroMemory( &info, sizeof( info ) );
				info.cbSize		= sizeof( info );
				info.uId		= (UINT_PTR)hWnd;
				info.uFlags		= TTF_SUBCLASS | TTF_IDISHWND | TTF_TRANSPARENT;
				info.hwnd		= hWnd;
				info.lpszText	= data->toolString;
				GetClientRect( hWnd, &info.rect );
				SendMessageW( data->hToolHint, TTM_ADDTOOLW, 0, (LPARAM)&info );


				/*---- ツールヒントの設定を行う ----*/
				RegViewWindowData*	viewData = GetViewWindowData( GetParent( hWnd ) );
				SendMessage( data->hToolHint, WM_SETFONT, (WPARAM)GetViewFont( GetParent( hWnd ) ), 0 );
				SendMessage( data->hToolHint, TTM_SETMAXTIPWIDTH, 0, 0 );
				SendMessageW( data->hToolHint, TTM_ACTIVATE, viewData->enableTooltip, 0 );
			}
		}
		break;

	case WM_MOUSEHOVER:
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				data->hover = true;
			}
			::InvalidateRect( hWnd, NULL, TRUE );
		}
		break;
	case WM_MOUSELEAVE:
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				data->hover = false;
				data->mouseTracking = false;
			}
			::InvalidateRect( hWnd, NULL, TRUE );
		}
		break;

	case WM_LBUTTONUP:
		if( !GetSelectDoubleClick( GetYAVAppData( GetAncestor( hWnd, GA_ROOTOWNER ) )->hSelector ) )
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				ViewSelectItem selItem;
				selItem.itemLength = data->page->valueLength;
				selItem.itemString = data->page->value;

				WORD key = 0;
				if( wParam & MK_SHIFT )		key |= 0x0001;
				if( wParam & MK_CONTROL )	key |= 0x0002;
				SendMessageW( GetParent( hWnd ), WM_VIEW_SELECTITEM, MAKELONG( 0, key ), (LONG_PTR)&selItem );
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		if( GetSelectDoubleClick( GetYAVAppData( GetAncestor( hWnd, GA_ROOTOWNER ) )->hSelector ) )
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				ViewSelectItem selItem;
				selItem.itemLength = data->page->valueLength;
				selItem.itemString = data->page->value;

				WORD key = 0;
				if( wParam & MK_SHIFT )		key |= 0x0001;
				if( wParam & MK_CONTROL )	key |= 0x0002;
				SendMessageW( GetParent( hWnd ), WM_VIEW_SELECTITEM, MAKELONG( 0, key ), (LONG_PTR)&selItem );
			}
		}
		break;

	case WM_RBUTTONUP:
		if( !GetSelectDoubleClick( GetYAVAppData( GetAncestor( hWnd, GA_ROOTOWNER ) )->hSelector ) )
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				ViewSelectItem selItem;
				selItem.itemLength = data->page->valueLength;
				selItem.itemString = data->page->value;

				WORD key = 0;
				if( wParam & MK_SHIFT )		key |= 0x0001;
				if( wParam & MK_CONTROL )	key |= 0x0002;
				SendMessageW( GetParent( hWnd ), WM_VIEW_SELECTITEM, MAKELONG( 1, key ), (LONG_PTR)&selItem );
			}
		}
		break;
	case WM_RBUTTONDBLCLK:
		if( GetSelectDoubleClick( GetYAVAppData( GetAncestor( hWnd, GA_ROOTOWNER ) )->hSelector ) )
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				ViewSelectItem selItem;
				selItem.itemLength = data->page->valueLength;
				selItem.itemString = data->page->value;

				WORD key = 0;
				if( wParam & MK_SHIFT )		key |= 0x0001;
				if( wParam & MK_CONTROL )	key |= 0x0002;
				SendMessageW( GetParent( hWnd ), WM_VIEW_SELECTITEM, MAKELONG( 1, key ), (LONG_PTR)&selItem );
			}
		}
		break;

	case WM_MBUTTONUP:
		if( !GetSelectDoubleClick( GetYAVAppData( GetAncestor( hWnd, GA_ROOTOWNER ) )->hSelector ) )
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				ViewSelectItem selItem;
				selItem.itemLength = data->page->valueLength;
				selItem.itemString = data->page->value;

				WORD key = 0;
				if( wParam & MK_SHIFT )		key |= 0x0001;
				if( wParam & MK_CONTROL )	key |= 0x0002;
				SendMessageW( GetParent( hWnd ), WM_VIEW_SELECTITEM, MAKELONG( 2, key ), (LONG_PTR)&selItem );
			}
		}
		break;
	case WM_MBUTTONDBLCLK:
		if( GetSelectDoubleClick( GetYAVAppData( GetAncestor( hWnd, GA_ROOTOWNER ) )->hSelector ) )
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				ViewSelectItem selItem;
				selItem.itemLength = data->page->valueLength;
				selItem.itemString = data->page->value;

				WORD key = 0;
				if( wParam & MK_SHIFT )		key |= 0x0001;
				if( wParam & MK_CONTROL )	key |= 0x0002;
				SendMessageW( GetParent( hWnd ), WM_VIEW_SELECTITEM, MAKELONG( 2, key ), (LONG_PTR)&selItem );
			}
		}
		break;

	case WM_SIZE:
		{
			RegViewPageData* data = GetViewPageData( hWnd );
			if( data )
			{
				TOOLINFOW info;
				ZeroMemory( &info, sizeof( info ) );
				info.cbSize = sizeof( info );
				info.uId	= 1;
				info.hwnd	= hWnd;
				GetClientRect( hWnd , &info.rect );
				SendMessage( data->hToolHint, TTM_NEWTOOLRECT, 0, (LPARAM)&info );
			}
		}
		break;

	default:
		return ::DefWindowProcW( hWnd, msg, wParam, lParam );
	}

	return 0;
}

//----------------------------------------------
// ビューウィンドウが拡張メッセージを送信する先を取得
//
static HWND getSendMessageWindow( HWND hWnd )
{
	HWND ret = NULL;
	RegViewWindowData* data = GetViewWindowData( hWnd );

	if( data->sendRoot ) ret = GetAncestor( hWnd, GA_ROOT );
	else				 ret = GetParent( hWnd );

	return ret;
}
