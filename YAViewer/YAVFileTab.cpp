/*----------------------------------------------------------------------------------------
*
* YAViewer �t�@�C���^�u�֌W
*
*----------------------------------------------------------------------------------------*/
#include <tchar.h>
#include "YAVFileTab.h"
#include "YAVAppAction.h"
#include "YAVResource.h"
#include <commctrl.h>


#define TAB_BUTTON_MODE


//----------------------------------------------
// �}�N����`
//
#define YFT_STATE_SELECTED	0x0001			/* �I������Ă��� */
#define YFT_STATE_HOVER		0x0002			/* �A�C�e����Ƀ}�E�X������ */
#define YFT_STATE_MOVING	0x0004			/* �ړ��� */

#define YFT_TAB_STYLE			(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | TCS_MULTILINE)
#define YFT_TAB_BUTTON_STYLE	(YFT_TAB_STYLE | TCS_BUTTONS | TCS_HOTTRACK)


//----------------------------------------------
// �\���̒�`
//
struct YavFileTabData
{
	HWND		hFileTab;					// �t�@�C���^�u�{��
	BOOL		tabButton;					// �t�@�C���^�u�{�^�����[�h
};
struct YavFileTabSubData
{
	WNDPROC		oldTabWndProc;				// ���̃E�B���h�E�v���V�[�W��

	// MouseMove�L���b�V��
	int			oldIndex;					// �I���A�C�e��
	bool		oldCloseHover;				// ����̏�Ƀ}�E�X������Ă�����
	bool		mouseTracking;				// �}�E�X�̃g���b�L���O�������s������
	int			mmDrawCount;				// MouseMove�ŕ`�悵����

	// �A�C�R���֌W
	HICON		hTabClose;					// ����A�C�R��
	HICON		hTabCloseSelect;			// �I�𒆂̕���A�C�R��

	// ��Ɨp
	INT			moveTagIndex;				// �ړ��Ώۃ^�u
	INT			closeSkip;					// ��x�����^�u���{�^���𖳎����邩
};
struct YavFileTabDrawParam
{
	YavFileTabSubData*	data;				// �E�B���h�E�����f�[�^
	RECT*				pRect;				// �`���`
	TCITEMW*			pItem;				// �^�u�A�C�e��
	YavFileTabParam*	param;				// �^�u�p�����[�^
	DWORD				state;				// �X�e�[�^�X

	// YFT_STATE_HOVER�ŗL���Ȃ���
	POINT*				hoverPoint;			// �J�[�\���ʒu
};

//----------------------------------------------
// �O���[�o���֐���`
//
static YavFileTabData* getFileTabData( HWND hWnd );

// �E�B���h�E�v���V�[�W��
static LRESULT CALLBACK yavFileTabProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

// ���b�Z�[�W�N���b�N
static LRESULT yavFileTabCreateMessage( HWND hWnd, LPCREATESTRUCT cs );
static void yavFileTabDestroyMessage( HWND hWnd );
static LRESULT yavFileTabSizeMessage( HWND hWnd, int type, WORD width, WORD height );
static LRESULT yavFileTabNotifyMessage( HWND hWnd, UINT id, NMHDR* pNotifyHead );


//----------------------------------------------
// �T�u�N���X�֌W�O���[�o���֐���`
//
static void setupFileTab( HWND hWnd );
static YavFileTabSubData* getFileTabSubData( HWND hWnd );

// �T�u�N���X���p�E�B���h�E�v���V�[�W��
static LRESULT CALLBACK yavFileTabSubClassProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static LRESULT yavFileTabSubClassMouseMoveCrack( HWND hWnd, int x, int y, UINT keyFlags );
static LRESULT yavFileTabSubClassPaintCrack( HWND hWnd );
static void yavFileTabItemDraw( HDC hDC, int idx, YavFileTabDrawParam* drawParam, bool isButton );


//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterYAVFileTabClass( HINSTANCE hInstance )
{
	/*---- �A�v���P�[�V�����E�B���h�E�N���X�o�^ ----*/
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
// �t�@�C���^�u�E�B���h�E����
//
HWND CreateFileTabWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle, DWORD id )
{
	DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	return CreateWindowW( L"YAViewer FileTabWindowClass", L"", style, 0, 0, 0, 0, hParent, 0, hInstance, (LPVOID)id );
}


//----------------------------------------------
// �t�@�C���^�u�E�B���h�E�擾
//	�q�E�B���h�E�̓o�^��
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
// �t�@�C���^�u - x�{�^����`�擾
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
	// �����`�F�b�N
	if( !(tabItemRect && dst) )
	{
		return false;
	}


	// �T�C�Y�ݒ�
	int padding = 2;
	int BoxSize = tabItemRect->bottom - tabItemRect->top - (padding*2);
	
	dst->right	= tabItemRect->right - padding;
	dst->top	= tabItemRect->top + padding;
	dst->left	= dst->right - BoxSize;
	dst->bottom	= dst->top + BoxSize;

	return true;
}

//----------------------------------------------
// �t�@�C���^�u - x�{�^���X�L�b�v����
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
// �t�@�C���^�u - �^�u�{�^�����[�h�ݒ�
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
// �t�@�C���^�u�f�[�^�擾
//
static YavFileTabData* getFileTabData( HWND hWnd )
{
	return (YavFileTabData*)GetWindowLongPtrW( hWnd, 0 );
}


//----------------------------------------------
// �E�B���h�E�v���V�[�W��
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
// �t�@�C���^�u
//	WM_CREATE �N���b�N
//
static LRESULT yavFileTabCreateMessage( HWND hWnd, LPCREATESTRUCT cs )
{
	/*---- �Ǘ��f�[�^���� ----*/
	YavFileTabData* data = new YavFileTabData;
	ZeroMemory( data, sizeof( YavFileTabData ) );
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	/*---- �^�u�E�B���h�E���� ----*/
	DWORD tabStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | TCS_MULTILINE;
	if( data->tabButton != FALSE ) tabStyle |= TCS_BUTTONS;

	data->hFileTab = CreateWindowExW( 0, WC_TABCONTROL, L"YAViewer FileTab", tabStyle, 0, 0, 0, 0, hWnd, (HMENU)cs->lpCreateParams, cs->hInstance, NULL );
	setupFileTab( data->hFileTab );


	return 0;
}

//----------------------------------------------
// �t�@�C���^�u
//	WM_DESTROY �N���b�N
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
// �t�@�C���^�u
//	WM_SIZE �N���b�N
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
// �t�@�C���^�u
//	WM_NOTIFY �N���b�N
//
static LRESULT yavFileTabNotifyMessage( HWND hWnd, UINT id, NMHDR* pNotifyHead )
{
	/*---- �e�E�B���h�E�ɒʒm�R�[�h�𑗐M ----*/
	HWND hParent = ::GetParent( hWnd );
	if( hParent )
	{
		return ::SendMessageW( hParent, WM_NOTIFY, (WPARAM)id, (LPARAM)pNotifyHead );
	}

	return 0;
}



//----------------------------------------------
//
// �T�u�N���X���֌W
//
//----------------------------------------------
//----------------------------------------------
// �t�@�C���^�u�T�u�N���X�f�[�^�擾
//
static YavFileTabSubData* getFileTabSubData( HWND hWnd )
{
	return (YavFileTabSubData*)GetWindowLongPtrW( hWnd, GWLP_USERDATA );
}


//----------------------------------------------
// �t�@�C���^�u ������
//
static void setupFileTab( HWND hWnd )
{
	/*---- �f�[�^���� ----*/
	YavFileTabSubData* data = new YavFileTabSubData;
	ZeroMemory( data, sizeof( YavFileTabSubData ) );
	data->oldIndex = -1;
	SetWindowLongPtrW( hWnd, GWLP_USERDATA, (LONG_PTR)data );


	/*---- �A�C�R������ -----*/
	data->hTabClose			= LoadIcon( ::GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_TABCLOSE ) );
	data->hTabCloseSelect	= LoadIcon( ::GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_TABCLOSE_SEL ) );


	/*---- �T�u�N���X�� ----*/
	data->oldTabWndProc = (WNDPROC)GetWindowLongPtrW( hWnd, GWLP_WNDPROC );
	SetWindowLongPtrW( hWnd, GWLP_WNDPROC, (LONG_PTR)yavFileTabSubClassProc );


	/*---- �t�H���g�ݒ� ----*/
	SendMessageW( hWnd, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), 0 );
}


//----------------------------------------------
// �T�u�N���X���p�E�B���h�E�v���V�[�W��
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
				/*---- �ړ��J�n���� ----*/
				if( data->closeSkip == 0 )
				{
					int				nIndex;
					TCHITTESTINFO	info;

					data->closeSkip = -1;

					// �^�b�`�ʒu
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


				/*---- ����{�^������ ----*/
				if( data->closeSkip <= 0 )
				{
					return yavFileTabSubClassMouseMoveCrack( hWnd, LOWORD( lParam ), HIWORD( lParam ), (UINT)wParam );
				}
				else
				{
					int				nIndex;
					TCHITTESTINFO	info;

					// �^�b�`�ʒu
					GetCursorPos( &info.pt );
					ScreenToClient( hWnd, &info.pt );
					nIndex = TabCtrl_HitTest( hWnd, &info );
					if( nIndex >= 0 && nIndex != data->moveTagIndex )
					{
						// �f�[�^���ёւ�
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
// �T�u�N���X�� WM_MOUSEMOVE�n���h�����O
//
static LRESULT yavFileTabSubClassMouseMoveCrack( HWND hWnd, int x, int y, UINT keyFlags )
{
	YavFileTabSubData* data = getFileTabSubData( hWnd );


	/*---- �}�E�X�̃g���b�L���O���� ----*/
	if( data && !data->mouseTracking )
	{// �}�E�X�C�x���g���g���b�L���O����
        TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = hWnd;
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		TrackMouseEvent( &tme );

		data->mouseTracking = true;
	}


	/*---- �J�[�\���ʒu�擾 ----*/
	TCHITTESTINFO	info;
	info.pt.x = x; info.pt.y = y;
	int nIndex = TabCtrl_HitTest( hWnd, &info );
	bool hover = false;


	/*---- �{�^�����[�h�Ȃ�ĕ`�� ----*/
	if( GetFileTabButtonMode( ::GetParent( hWnd ) ) )
	{
		if( data->oldIndex != nIndex )
		{
			::InvalidateRect( hWnd, NULL, FALSE );
		}
	}


	/*---- ����A�C�R���\�� ----*/
	if( nIndex >= 0 )
	{
		HICON hIcon = data->hTabClose;
		HDC hDC = GetDC( hWnd );

		// ����A�C�R�����W
		RECT rect, itemRect;
		TabCtrl_GetItemRect( hWnd, nIndex, &itemRect );
		GetFileTabCloseRectToItemRect( &itemRect, &rect );

		// ��`���ɂ��邩����
		HBRUSH hBrush = NULL;
		HPEN hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_ACTIVEBORDER ) );
		POINT& pt = info.pt;
		if( rect.left <= pt.x && pt.x < rect.right && rect.top <= pt.y && pt.y < rect.bottom )
		{
			hBrush = ::CreateSolidBrush( RGB( 255, 64, 64 ) );
			hIcon = data->hTabCloseSelect;
			hover = true;
		}

		// ��������
		if( data->mmDrawCount == 1 )
		{
			// �w�i�`��
			if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );
			if( hPen )	 hPen = (HPEN)::SelectObject( hDC, hPen );
			::Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
			if( hPen )	 hPen = (HPEN)::SelectObject( hDC, hPen );
			if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );

			// �A�C�R���`��
			::DrawIconEx( hDC, rect.left, rect.top, hIcon, rect.right-rect.left, rect.bottom-rect.top, 0, NULL, DI_NORMAL );
		}

		// �u���V�p��
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


	/*---- ��Ԃ��i�[���Ă��� ----*/
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
// �T�u�N���X�� WM_PAINT�n���h�����O
//
static LRESULT yavFileTabSubClassPaintCrack( HWND hWnd )
{
	YavFileTabSubData* data = getFileTabSubData( hWnd );
	int nMaxIndex = TabCtrl_GetItemCount( hWnd );
	bool isButton = GetFileTabButtonMode( ::GetParent( hWnd ) );


	/*---- �`�揈�� ----*/
	PAINTSTRUCT paint;
	HDC hDC = BeginPaint( hWnd, &paint );


	/*---- �f�o�C�X�R���e�L�X�g��ݒ� ----*/
	HFONT hOldFont = (HFONT)::SelectObject( hDC, (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 ) );


	/*---- �w�i�h��Ԃ� ----*/
	FillRect( hDC, &paint.rcPaint, ::GetSysColorBrush( COLOR_MENU ) );


	/*---- �\���̈�h��Ԃ����� ----*/
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


	/*---- �`��͈͋�` ----*/
	HPEN hBorderPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_ACTIVEBORDER ) );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hBorderPen );
	if( !isButton )
	{
		::Rectangle( hDC, rect.left, rect.top-1, rect.right+1, rect.bottom-1 );
	}


	/*---- ���݂̃J�[�\���ʒu���擾 ----*/
	TCHITTESTINFO info;
	GetCursorPos( &info.pt );
	ScreenToClient( hWnd, &info.pt );
	int nHoverIndex = TabCtrl_HitTest( hWnd, &info );


	/*---- �A�C�e����S���\�� ----*/
	for( int i = 0; i < nMaxIndex; i++ )
	{
		/*---- �\���p�����[�^ ----*/
		YavFileTabDrawParam drawParam;
		ZeroMemory( &drawParam, sizeof( drawParam ) );
		drawParam.data = data;


		/*---- �A�C�e���̏����擾 ----*/
		wchar_t text[ MAX_PATH ];
		TCITEMW item;
		item.mask = TCIF_TEXT | TCIF_PARAM | TCIF_STATE;
		item.pszText = text;
		item.cchTextMax = MAX_PATH;
		item.dwStateMask = TCIS_BUTTONPRESSED | TCIS_HIGHLIGHTED;
		TabCtrl_GetItem( hWnd, i, &item );
		drawParam.pItem = &item;
		

		/*---- �g���X�e�[�^�X�K�p ----*/
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


		/*---- �p�����[�^�擾 ----*/
		if( item.lParam )
		{
			drawParam.param = (YavFileTabParam*)item.lParam;
		}


		/*---- �\����`�擾 ----*/
		RECT itemRect;
		TabCtrl_GetItemRect( hWnd, i, &itemRect );
		drawParam.pRect = &itemRect;


		/*---- �\�� ----*/
		yavFileTabItemDraw( hDC, i, &drawParam, isButton );
	}


	/*---- �f�o�C�X�R���e�L�X�g�𕜋A ----*/
	::SelectObject( hDC, hOldFont );
	::SelectObject( hDC, hOldPen );


	/*---- �����OS�֕Ԃ� ----*/
	::EndPaint( hWnd, &paint );
	return 0;
}

//----------------------------------------------
// �^�u�\���֐�
//
static void yavFileTabItemDraw( HDC hDC, int idx, YavFileTabDrawParam* drawParam, bool isButton )
{
	COLORREF oldTextColor = ::GetTextColor( hDC );
	HPEN hPen = NULL;


	/*---- �y���̐��� ----*/
	if( drawParam->state & YFT_STATE_SELECTED && drawParam->param->useTabColor )
	{
		hPen = ::CreatePen( PS_SOLID, 1, drawParam->param->tabColor );
	}


	/*---- �w�i�\�� ----*/
	if( drawParam->state & YFT_STATE_MOVING )
	{
		::FillRect( hDC, drawParam->pRect, ::GetSysColorBrush( COLOR_HOTLIGHT ) );

		// �^�u�F���當���F��ݒ�
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

		// �^�u�F���當���F��ݒ�
		COLORREF textColor = RGB( 0, 0, 0 ), c = drawParam->param->tabColor;
		if( ((GetRValue( c ) + GetGValue( c ) + GetBValue( c )) / 3) <= 162 )
		{
			textColor = RGB( 255, 255, 255 );
		}
		oldTextColor = ::SetTextColor( hDC, textColor );
	}


	/*---- �^�u�̈��؂�� ----*/
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



	/*---- �^�u�����\�� ----*/
	RECT textRect = *(drawParam->pRect);
	textRect.bottom -= 2;
	::SetBkMode( hDC, TRANSPARENT );
	::DrawTextW( hDC, drawParam->pItem->pszText, -1, &textRect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE );
	::SetTextColor( hDC, oldTextColor );


	/*---- ����A�C�R���\�� ----*/
	if( !drawParam->param->disableClose && (drawParam->state & YFT_STATE_HOVER) )
	{
		HICON hIcon = drawParam->data->hTabClose;

		// ����A�C�R�����W
		RECT rect;
		GetFileTabCloseRectToItemRect( drawParam->pRect, &rect );

		// ��`���ɂ��邩����
		HBRUSH hBrush = NULL;
		POINT pt = *(drawParam->hoverPoint);
		if( rect.left <= pt.x && pt.x < rect.right && rect.top <= pt.y && pt.y < rect.bottom )
		{
			hBrush = ::CreateSolidBrush( RGB( 255, 64, 64 ) );
			hIcon = drawParam->data->hTabCloseSelect;
		}

		// �w�i�`��
		if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );
		::Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
		if( hBrush ) hBrush = (HBRUSH)::SelectObject( hDC, hBrush );

		// �A�C�R���`��
		::DrawIconEx( hDC, rect.left, rect.top, hIcon, rect.right-rect.left, rect.bottom-rect.top, 0, NULL, DI_NORMAL );

		// �u���V�p��
		if( hBrush )
		{
			::DeleteObject( hBrush );
		}
	}



	/*---- �����F�߂� ----*/
	::SetTextColor( hDC, oldTextColor );
}

