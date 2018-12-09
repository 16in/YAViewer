/*----------------------------------------------------------------------------------------
*
* YAViewer View�֌W
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
// �O���[�o���֐���`
//
static RegViewWindowData* releaseViewWindowData( HWND hViewWnd, bool del = false );
static bool registerViewPageWindowClass( HINSTANCE hInstance );
static HWND createViewPageWindow( HWND hParent, RegViewWindowData* viewData, aafile::AAFilePage*	page );
static LRESULT CALLBACK viewPageWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static HWND getSendMessageWindow( HWND hWnd );

static LRESULT viewCreateMessage( HWND hWnd, LPCREATESTRUCTW cs );


//----------------------------------------------
// �E�B���h�E�N���X�o�^
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
// �A�v���P�[�V�����E�B���h�E����
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
// �r���[�E�B���h�E�W���R�[���o�b�N
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
		// �N���A���Ȃ�
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
			case SB_LINEUP:			// 1 �s��փX�N���[��
				if( --sc.nPos < sc.nMin )
				{
					sc.nPos = sc.nMin;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_LINEDOWN:		// 1 �s���փX�N���[��
				if( (++sc.nPos + sc.nPage) >= (UINT)sc.nMax )
				{
					sc.nPos = sc.nMax - sc.nPage;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_PAGEUP:			// 1 �y�[�W��փX�N���[��
				if( (sc.nPos -= sc.nPage) < sc.nMin )
				{
					sc.nPos = sc.nMin;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_PAGEDOWN:		// 1 �y�[�W���փX�N���[�� 
				if( ((sc.nPos += sc.nPage) + sc.nPage) >= (UINT)sc.nMax )
				{
					sc.nPos = sc.nMax - sc.nPage;
				}
				sc.fMask = SIF_POS;
				break;
			case SB_THUMBPOSITION:	// ��Έʒu�փX�N���[��
			case SB_THUMBTRACK:		// �w��ʒu�փX�N���[�� �{�b�N�X���h���b�O�B
				sc.nPos = HIWORD( wParam );
				sc.fMask = SIF_POS;
				break;
			case SB_TOP:			// [HOME]�L�[�������ꂽ
				sc.nPos = sc.nMin;
				sc.fMask = SIF_POS;
				break;
			case SB_BOTTOM:			// [END]�L�[�������ꂽ
				sc.nPos = sc.nMax - sc.nPage;
				sc.fMask = SIF_POS;
				break;
			case SB_ENDSCROLL:
				break;
			}
			::SetScrollInfo( hWnd, SB_VERT, &sc, TRUE );


			/*---- �\����Ԃ��X�V���� ----*/
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
	// �L�[���͏���
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
	// �g�����b�Z�[�W��
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
	/*---- �f�[�^��ݒ� ----*/
	RegViewWindowData* data = (RegViewWindowData*)cs->lpCreateParams;
	::SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	return 0;
}


//----------------------------------------------
// �X�N���[���ʒu�ݒ�/�擾
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
			// ���i�ڂ��Z�o���Ă���𑗐M����
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
// �\���t�H���g�ݒ�
//
void SetViewFont( HWND hViewWnd, HFONT hFont )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		data->hFont = hFont;

		// �������e�[�u���X�V
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
// �\�����[�h�ݒ�
//
void SetViewMode( HWND hViewWnd, int mode )
{
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		data->mode = mode;

		/*---- �X�N���[���o�[�̏�Ԃ�ύX ----*/
		SCROLLINFO sc;
		ZeroMemory( &sc, sizeof( sc ) );
		sc.cbSize = sizeof( sc );
		sc.fMask = SIF_POS;
		::SetScrollInfo( hViewWnd, SB_VERT, &sc, FALSE );

		/*---- �̈���ĕ`�� ----*/
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
// �c�[���q���g�\���ݒ�/�擾
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
// �J�e�S���\���T�C�Y�ݒ�
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
// �\���t�@�C���ݒ�
//
void SetViewFile( HWND hViewWnd, aafile::AAFile* file )
{
	if( hViewWnd )
	{
		/*---- �f�[�^��蒼�� ----*/
		RegViewWindowData* data = releaseViewWindowData( hViewWnd );
		SetWindowLongPtrW( hViewWnd, 0, (LONG_PTR)data );


		/*---- �X�N���[���o�[�̏�Ԃ�ύX ----*/
		SCROLLINFO sc;
		ZeroMemory( &sc, sizeof( sc ) );
		sc.cbSize = sizeof( sc );
		sc.fMask = SIF_POS;
		::SetScrollInfo( hViewWnd, SB_VERT, &sc, FALSE );


		/*---- �f�[�^���i�[���Ă��� ----*/
		data->file = file;
		if( file )
		{
			data->pages = new HWND[ data->file->pageCount ];
			for( size_t i = 0; i < data->file->pageCount; i++ )
			{
				data->pages[ i ] = createViewPageWindow( hViewWnd, data, &data->file->pageList[ i ] );
			}

			
			/*---- �y�[�W�𐶐����� ----*/
			AllPaintPage( hViewWnd );


			/*---- �E�B���h�E�T�C�Y��ݒ肷�� ----*/
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
// �g�����b�Z�[�W���M��ݒ�
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
// �r���[�E�B���h�E�o�^�f�[�^�̍폜
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
// �y�[�W�r���[�E�B���h�E�N���X�o�^
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
// �y�[�W�r���[�E�B���h�E�N���X����
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
// �y�[�W�r���[�E�B���h�E�v���V�[�W��
//
static LRESULT CALLBACK viewPageWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_CREATE:
		{
			/*---- �������p�����[�^�𓾂� ----*/
			LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
			RegViewPageData* pageData = (RegViewPageData*)cs->lpCreateParams;


			/*---- �E�B���h�E�n���h���Ƀf�[�^���֘A�t���� ----*/
			::SetWindowLongPtrW( hWnd, 0, (LONG_PTR)pageData );


			/*---- �\�����镶�����ݒ� ----*/
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
				/*---- �������f�o�C�X�R���e�L�X�g�p�� ----*/
				if( pageData->hDeviceContext )
				{
					::SelectObject( pageData->hDeviceContext, pageData->hOldDeviceContextBMP );
					if( pageData->hDeviceContextBMP )
					{
						::DeleteObject( pageData->hDeviceContextBMP );
					}
					::DeleteDC( pageData->hDeviceContext );
				}


				/*---- �������̈�̊J�� ----*/
				if( pageData->toolString )
				{
					delete[] pageData->toolString;
					pageData->toolString = NULL;
				}


				/*---- �Ńf�[�^�p�� ----*/
				delete pageData;
			}
		}
		break;

	case WM_PAINT:
		{
			/*---- �������f�o�C�X�R���e�L�X�g���R�s�[ ----*/
			PAINTSTRUCT paint;
			HDC hdc = BeginPaint( hWnd, &paint );

			RegViewWindowData*	view = GetViewWindowData( GetParent( hWnd ) );
			RegViewPageData*	page = GetViewPageData( hWnd );

			PaintPage( view, hWnd, page );
			CopyPageData( hdc, view, page );


			if( page->hover )
			{
				/*---- �I��g�\�� ----*/
				RECT rect;
				GetClientRect( hWnd, &rect );

				HBRUSH hOldBrush = (HBRUSH)SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
				HPEN hOldPen = (HPEN)SelectObject( hdc, GetStockObject( DC_PEN ) );
				COLORREF oldColor = SetDCPenColor( hdc, RGB( 0, 0, 255 ) );
				HFONT hOldFont = (HFONT)SelectObject( hdc, view->hFont );

				Rectangle( hdc, rect.left, rect.top, rect.right, rect.bottom );


				/*---- ���e�L�X�g�\�� ----*/
				COLORREF oldTextColor = SetTextColor( hdc, RGB( 255, 0, 255 ) );

				wchar_t str[ 260 ];
				swprintf_s( str, L"%d x %d�s", page->width, page->lfCount );
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
			{// �}�E�X�C�x���g���g���b�L���O����
	            TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.hwndTrack = hWnd;
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.dwHoverTime = 1;
				TrackMouseEvent( &tme );

				data->mouseTracking = true;

				// ���łɐe�E�B���h�E�Ƀt�H�[�J�X���ڂ�
				SetFocus( GetParent( hWnd ) );
			}

			if( data->hToolHint == NULL )
			{
				/*---- �c�[���q���g�E�B���h�E���� ----*/
				data->hToolHint = CreateWindowExW( 0, TOOLTIPS_CLASSW, L"", TTS_NOPREFIX | TTS_ALWAYSTIP,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, GetModuleHandle( NULL ), NULL );

				// �c�[���q���g����
				TOOLINFOW info;
				ZeroMemory( &info, sizeof( info ) );
				info.cbSize		= sizeof( info );
				info.uId		= (UINT_PTR)hWnd;
				info.uFlags		= TTF_SUBCLASS | TTF_IDISHWND | TTF_TRANSPARENT;
				info.hwnd		= hWnd;
				info.lpszText	= data->toolString;
				GetClientRect( hWnd, &info.rect );
				SendMessageW( data->hToolHint, TTM_ADDTOOLW, 0, (LPARAM)&info );


				/*---- �c�[���q���g�̐ݒ���s�� ----*/
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
// �r���[�E�B���h�E���g�����b�Z�[�W�𑗐M�������擾
//
static HWND getSendMessageWindow( HWND hWnd )
{
	HWND ret = NULL;
	RegViewWindowData* data = GetViewWindowData( hWnd );

	if( data->sendRoot ) ret = GetAncestor( hWnd, GA_ROOT );
	else				 ret = GetParent( hWnd );

	return ret;
}
