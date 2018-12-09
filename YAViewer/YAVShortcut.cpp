/*----------------------------------------------------------------------------------------
*
* YAViewer ショートカット関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVShortcut.h"
#include "YAViewerMessage.h"
#include <commctrl.h>


#define YAVSHORTCUT_CLASSNAME		L"YAVVIEW_SHORTCUTWINDOWCLASS"


//----------------------------------------------
// ショートカットデータ
//
struct shortcutData
{
	HWND	hListViewWindow;
	HWND	hSendMessageTarget;
};


// ウィンドウプロシージャ
static LRESULT CALLBACK shortcutWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

// カラム幅を合わせる
static void setColumnWidth( HWND hListView );


//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterYAVShortcutClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= sizeof( shortcutData* );
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursorW( NULL, IDC_ARROW );
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= shortcutWindowProc;
	wc.lpszClassName	= YAVSHORTCUT_CLASSNAME;

	return (::RegisterClassExW( &wc ) != NULL);
}

//----------------------------------------------
// ショートカットウィンドウ生成
//
HWND CreateShortcutWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle )
{
	DWORD style = WS_CHILD | WS_VISIBLE | addStyle;
	DWORD exStyle = 0;

	shortcutData* data = new shortcutData;
	ZeroMemory( data, sizeof( shortcutData ) );

	HWND ret = CreateWindowExW( exStyle, YAVSHORTCUT_CLASSNAME, L"", style, 0, 0, 0, 0, hParent, NULL, hInstance, data );

	return ret;
}

//----------------------------------------------
// メッセージの送信先
//
void SetShortcutSendNotifyWindow( HWND hShortcut, HWND hTarget )
{
	shortcutData* data = (shortcutData*)GetWindowLongPtrW( hShortcut, 0 );
	if( data )
	{
		data->hSendMessageTarget = hTarget;
	}
}

//----------------------------------------------
// ショートカット生成
//
void SetShortcutFile( HWND hShortcut, aafile::AAFile* file )
{
	shortcutData* data = (shortcutData*)GetWindowLongPtrW( hShortcut, 0 );
	if( data )
	{
		ListView_DeleteAllItems( data->hListViewWindow );

		if( file )
		{
			for( unsigned long i = 0; i < file->pageCount; i++ )
			{
				if( file->pageList[ i ].value && file->pageList[ i ].valueLength > 0 )
				{
					unsigned long lf = 0;
					wchar_t* text = new wchar_t[ file->pageList[ i ].valueLength + 1 ];
					for( ; lf < file->pageList[ i ].valueLength; lf++ )
					{
						if( file->pageList[ i ].value[ lf ] == L'\n'
							|| file->pageList[ i ].value[ lf ] == L'\r' )
						{
							for( ; lf < file->pageList[ i ].valueLength; lf++ )
							{// 改行のみ判定
								if( file->pageList[ i ].value[ lf ] != L'\n'
									&& file->pageList[ i ].value[ lf ] != L'\r' )
								{
									break;
								}
								text[ lf ] = L'\0';
							}
							break;
						}
						text[ lf ] = file->pageList[ i ].value[ lf ];
					}
					text[ lf ] = L'\0';

					if( lf == file->pageList[ i ].valueLength )
					{
						LVITEMW item = { 0 };
						item.mask = LVIF_TEXT | LVIF_PARAM;
						item.pszText	= text;
						item.lParam		= (LPARAM)i;
						item.iItem		= i;

						ListView_InsertItem( data->hListViewWindow, &item );
					}
					delete[] text;
				}
			}
		}

		// カラム幅再設定
		setColumnWidth( data->hListViewWindow );
	}
}


//----------------------------------------------
// ショートカットウィンドウプロシージャ
//
static LRESULT CALLBACK shortcutWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	shortcutData* data = (shortcutData*)GetWindowLongPtrW( hWnd, 0 );
	switch( msg )
	{
	case WM_CREATE:
		{
			CREATESTRUCT* cs = (LPCREATESTRUCT)lParam;
			shortcutData* data = (shortcutData*)cs->lpCreateParams;
			SetWindowLong( hWnd, 0, (LONG)data );

			// リストビュー生成
			DWORD style = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER;
			data->hListViewWindow = CreateWindowExW( 0, WC_LISTVIEW, L"", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, 0 );
			DWORD lsvStyle = ListView_GetExtendedListViewStyle( data->hListViewWindow );
			ListView_SetExtendedListViewStyle( data->hListViewWindow, lsvStyle | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );


			LVCOLUMNW col;
			wchar_t text[] = L"名前";
			col.mask = LVCF_FMT | LVCF_TEXT;
			col.fmt = LVCFMT_LEFT;
			col.pszText = text;
			ListView_InsertColumn( data->hListViewWindow, 0, &col );
		}
		break;

	case WM_SIZE:
		if( data )
		{
			int w = LOWORD( lParam ), h = HIWORD( lParam );
			MoveWindow( data->hListViewWindow, 0, 0, w, h, TRUE );
			setColumnWidth( data->hListViewWindow );
		}
		break;

	case WM_NOTIFY:
		if( ((LPNMHDR)lParam)->hwndFrom == data->hListViewWindow )
		{
			LPNMHDR hdr = (LPNMHDR)lParam;
			switch( hdr->code )
			{
			case NM_CLICK:
				if( data->hSendMessageTarget )
				{
					LVITEMW item = { 0 };
					item.mask = LVIF_PARAM;
					item.iItem= ListView_GetNextItem( data->hListViewWindow, -1, LVNI_SELECTED );
					if( ListView_GetItem( data->hListViewWindow, &item ) )
					{
						SendMessageW( data->hSendMessageTarget, YAV_WM_PAGEJUMP, 0, item.lParam );
					}
				}
				break;
			}
		}
	}
	return ::DefWindowProcW( hWnd, msg, wParam, lParam );
}

// カラム幅を合わせる
static void setColumnWidth( HWND hListView )
{
	RECT rect;
	GetWindowRect( hListView, &rect );

	LVCOLUMNW col;
	col.mask = LVCF_WIDTH;
	col.cx = rect.right - rect.left;

	if( GetWindowLong( hListView, GWL_STYLE ) & WS_VSCROLL )
	{
		col.cx -= GetSystemMetrics( SM_CXVSCROLL );
	}
	ListView_SetColumn( hListView, 0, &col );
}
