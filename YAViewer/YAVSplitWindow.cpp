/*----------------------------------------------------------------------------------------
*
* YAViewer SplitWindow�֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVSplitWindow.h"
#include <Windowsx.h>


//----------------------------------------------
// �����E�B���h�E�Ǘ��f�[�^
//
struct RegSplitWindowData
{
	int			mode;				// �������[�h
	int			lock;				// �Œ�E�B���h�E���
	int			width;				// ���A�܂��͏㕔���̕�
	int			border;				// �{�[�_�[�T�C�Y
	BOOL		repaint;			// �X�V���ɍĕ`������N�G�X�g���邩
	BOOL		dbclkCnter;			// �_�u���N���b�N���ɃZ���^�����O���邩

	BOOL		movSplit;			// �����E�B���h�E����������
	RECT		selRect;			// �I��͈͋�`

	HWND		hWnd[ 2 ];			// �Ǘ��E�B���h�E 0:�� 1:�E
	HWND		hOldCaptureWnd;		// �ȑO�ɊǗ�����Ă����L���v�`���E�B���h�E

	RegSplitWindowData( void )
		: mode( 0 ), lock( 0 ), width( -1 ), border( 4 ), repaint( TRUE ), dbclkCnter( TRUE ), movSplit( FALSE ), hOldCaptureWnd( NULL )
	{ hWnd[ 0 ] = NULL; hWnd[ 1 ] = NULL; }
};


//----------------------------------------------
// �t�@�C���X�R�[�v�O���[�o���֐��錾
//
static RegSplitWindowData* getSplitWindowData( HWND hSplitWnd );
static void updateSplitWindowTarget( HWND hSplitWnd );
static void splitWindowSizeCrack( HWND hWnd, int type, WORD width, WORD height );


//----------------------------------------------
// �����E�B���h�E�N���X�o�^
//
bool RegisterSplitWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= sizeof( RegSplitWindowData* );
	wc.hInstance		= hInstance;
	wc.style			= CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= YAVSplitWindowProc;
	wc.lpszClassName	= L"YAViewer SplitWindowClass";
	wc.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);

	return (RegisterClassExW( &wc ) != NULL);
}


//----------------------------------------------
// �����E�B���h�E����
//
HWND CreateSplitWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle )
{
	DWORD style = WS_CHILD | WS_VISIBLE | addStyle;
	DWORD exStyle = 0;
	RegSplitWindowData* data = new RegSplitWindowData;
	HWND ret = CreateWindowExW( exStyle, L"YAViewer SplitWindowClass", L"", style, 0, 0, 0, 0, hParent, NULL, hInstance, data );
	if( !ret )
	{
		delete data;
	}

	return ret;
}


//----------------------------------------------
// �����E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK YAVSplitWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	default: return ::DefWindowProcW( hWnd, msg, wParam, lParam );

	case WM_CREATE:
		SetWindowLongPtrW( hWnd, 0, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams );
		break;
	case WM_DESTROY:
		{
			RegSplitWindowData* data = getSplitWindowData( hWnd );
			SetWindowLongPtrW( hWnd, 0, 0 );
			delete data;
		}
		break;
	case WM_SIZE:
		splitWindowSizeCrack( hWnd, (int)wParam, LOWORD( lParam ), HIWORD( lParam ) );
		break;

	case WM_LBUTTONDBLCLK:
		{
			RegSplitWindowData* data = getSplitWindowData( hWnd );
			if( data && data->dbclkCnter )
			{
				/*---- �_�u���N���b�N���͕����ʒu��^�񒆂� ----*/
				int width = data->width;
				RECT rect;
				GetClientRect( hWnd, &rect );
				switch( data->mode )
				{
				case SPL_VERTICAL:
					width = (rect.right - rect.left) / 2;
					break;
				case SPL_HORIZONTAL:
					width = (rect.bottom - rect.top) / 2;
					break;
				}
				if( width != data->width )
				{
					data->width = width;
					updateSplitWindowTarget( hWnd );
				}
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			RegSplitWindowData* data = getSplitWindowData( hWnd );
			if( data )
			{
				data->hOldCaptureWnd = SetCapture( hWnd );
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			RegSplitWindowData* data = getSplitWindowData( hWnd );
			if( data )
			{
				if( data->hOldCaptureWnd )
				{
					SetCapture( data->hOldCaptureWnd );
				}
				else
				{
					ReleaseCapture();
				}
				data->hOldCaptureWnd = NULL;

				// �͈͋�`������
				HDC hDC = ::GetDC( ::GetDesktopWindow() );
				::DrawFocusRect( hDC, &data->selRect );
				::ReleaseDC( hWnd, hDC );

				updateSplitWindowTarget( hWnd );
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			RegSplitWindowData* data = getSplitWindowData( hWnd );
			if( data )
			{
				/*---- �A�C�R���̒��ւ� ----*/
				switch( data->mode )
				{
				case SPL_VERTICAL:
					SetCursor( LoadCursor( NULL, IDC_SIZEWE ) );
					break;
				case SPL_HORIZONTAL:
					SetCursor( LoadCursor( NULL, IDC_SIZENS ) );
					break;
				}


				/*---- �����ʒu�ړ����� ----*/
				if( GetCapture() == hWnd )
				{
					bool update = false;
					RECT rect;
					GetClientRect( hWnd, &rect );
					int x = GET_X_LPARAM( lParam ), y = GET_Y_LPARAM( lParam );

					switch( data->mode )
					{
					case SPL_VERTICAL:
						if( rect.left > x )		x = rect.left + data->border / 2;
						if( rect.right < x )	x = rect.right - data->border / 2;
						if( data->lock == SPL_LOCK_LEFT )
						{
							update = (data->width != x);
							data->width = x;
						}
						else
						{
							update = (data->width != ((rect.right - rect.left) - x));
							data->width = (rect.right - rect.left) - x;
						}
						break;
					case SPL_HORIZONTAL:
						if( rect.top > y )		y = rect.top + data->border / 2;
						if( rect.bottom < y )	y = rect.bottom - data->border / 2;
						if( data->lock == SPL_LOCK_LEFT )
						{
							update = (data->width != y);
							data->width = y;
						}
						else
						{
							update = (data->width != ((rect.bottom - rect.top) - y));
							data->width = (rect.bottom - rect.top) - y;
						}
						break;
					}
					if( update )
					{
						RECT selRect;
						POINT pos;
						GetClientRect( hWnd, &selRect );

						// �X�N���[�����W�n�ɕϊ�
						pos.x = selRect.left; pos.y = selRect.top;
						ClientToScreen( hWnd, &pos );

						selRect.right	= pos.x + selRect.right - selRect.left;
						selRect.left	= pos.x;
						selRect.bottom	= pos.y + selRect.bottom - selRect.top;
						selRect.top		= pos.y;
						
						// �{�[�_���v�Z
						switch( data->mode )
						{
						case SPL_VERTICAL:
							selRect.left += data->width - (data->border/2);
							selRect.right = selRect.left + data->border;
							break;
						case SPL_HORIZONTAL:
							selRect.top		= (selRect.bottom - data->width) - (data->border/2);
							selRect.bottom	= selRect.top + data->border;
							break;
						}

						// �f�X�N�g�b�v�ɔ͈͋�`��\��
						HDC hDC = ::GetDC( ::GetDesktopWindow() );
						::DrawFocusRect( hDC, &data->selRect );
						::DrawFocusRect( hDC, &selRect );
						::ReleaseDC( hWnd, hDC );

						data->selRect = selRect;
					}
				}
			}
		}
		break;
	}

	return 0;
}


//----------------------------------------------
// �����E�B���h�E���[�h�ݒ�/�擾
//
int SetSplitWindowMode( HWND hSplitWnd, int mode )
{
	int ret = -1;
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );

	if( data && (ret = data->mode) != mode )
	{
		data->mode = mode;
		updateSplitWindowTarget( hSplitWnd );
	}

	return ret;
}
int GetSplitWindowMode( HWND hSplitWnd )
{
	int ret = -1;
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );

	if( data )
	{
		ret = data->mode;
	}

	return ret;
}


//----------------------------------------------
// �����̈��؂�T�C�Y�ݒ�/�擾
//
int SetSplitBorderWidth( HWND hSplitWnd, int width )
{
	int ret = 0;
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );

	if( data && (ret = data->border) != width )
	{
		data->border = width;
		
		updateSplitWindowTarget( hSplitWnd );
	}

	return ret;
}
int GetSplitBorderWidth( HWND hSplitWnd )
{
	int ret = 0;
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );

	if( data )
	{
		ret = data->border;
	}

	return ret;
}


//----------------------------------------------
// �����T�C�Y�ݒ�/�擾
//
int SetSplitWidth( HWND hSplitWnd, int width, bool update )
{
	int ret = 0;
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );

	if( data && (ret = data->width) != width )
	{
		data->width = width;
		
		if( update ) updateSplitWindowTarget( hSplitWnd );
	}

	return ret;
}
int GetSplitWidth( HWND hSplitWnd )
{
	int ret = 0;
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );

	if( data )
	{
		ret = data->width;
	}

	return ret;
}

// �ȈՃA�N�Z�X�p
int SetSplitLeftWidth( HWND hSplitWnd, int width )	{ return SetSplitWidth( hSplitWnd, width ); }
int SetSplitTopWidth( HWND hSplitWnd, int width )	{ return SetSplitWidth( hSplitWnd, width ); }
int GetSplitLeftWidth( HWND hSplitWnd )	{ return GetSplitWidth( hSplitWnd ); }
int GetSplitTopWidth( HWND hSplitWnd )	{ return GetSplitWidth( hSplitWnd ); }



//----------------------------------------------
// �����̈���_�u���N���b�N�ŃZ���^�����O���邩�ݒ�/�擾
//
BOOL SetSplitDbClickCentering( HWND hSplitWnd, BOOL dbclkCnter )
{
	BOOL ret = FALSE;

	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		ret = data->dbclkCnter;
		data->dbclkCnter = dbclkCnter;
	}

	return ret;
}
BOOL GetSplitDbClickCentering( HWND hSplitWnd )
{
	BOOL ret = FALSE;

	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		ret = data->dbclkCnter;
	}

	return ret;
}


//----------------------------------------------
// �����ΏۃE�B���h�E�ݒ�/�擾
//
void SetSplitTargetWindow( HWND hSplitWnd, HWND left, HWND right )
{
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		bool update = (data->hWnd[ 0 ] != left || data->hWnd[ 1 ] != right);

		data->hWnd[ 0 ] = left;
		data->hWnd[ 1 ] = right;

		if( update ) updateSplitWindowTarget( hSplitWnd );
	}
}
void GetSplitTargetWindow( HWND hSplitWnd, HWND* left, HWND* right )
{
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		if( left )  *left  = data->hWnd[ 0 ];
		if( right ) *right = data->hWnd[ 1 ];
	}
}


//----------------------------------------------
// �����ΏۃE�B���h�E�Œ�����ݒ�/�擾
//
void SetSplitWindowLock( HWND hSplitWnd, int lock )
{
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		data->lock = lock;
	}
}
int GetSplitWindowLock( HWND hSplitWnd )
{
	int ret = 0;

	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		ret = data->lock;
	}

	return ret;
}


//----------------------------------------------
// �����ΏۃE�B���h�E����ւ�
//
void SwapSplitTargetWindow( HWND hSplitWnd )
{
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		HWND hWnd = data->hWnd[ 0 ];
		data->hWnd[ 0 ] = data->hWnd[ 1 ];
		data->hWnd[ 1 ] = hWnd;

		RECT rect;
		GetClientRect( hSplitWnd, &rect );
		switch( data->mode )
		{
		case SPL_VERTICAL:
			data->width = (rect.right - rect.left) - data->width;
			break;
		case SPL_HORIZONTAL:
			data->width = (rect.bottom - rect.top) - data->width;
			break;
		}

		updateSplitWindowTarget( hSplitWnd );
	}
}



//----------------------------------------------
// �����E�B���h�E�f�[�^�̎擾
//
static RegSplitWindowData* getSplitWindowData( HWND hSplitWnd )
{
	RegSplitWindowData* ret = NULL;

	if( hSplitWnd ) ret = (RegSplitWindowData*)GetWindowLongPtrW( hSplitWnd, 0 );

	return ret;
}


//----------------------------------------------
// �����E�B���h�E�̐ݒ�𔽉f
//
static void updateSplitWindowTarget( HWND hSplitWnd )
{
	RegSplitWindowData* data = getSplitWindowData( hSplitWnd );
	if( data )
	{
		/*---- �N���C�A���g�T�C�Y�𓾂� ----*/
		RECT rect;
		GetClientRect( hSplitWnd, &rect );
		int width  = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		float bWidth = (float)data->border / 2.0f;
		int lWidth = data->width;


		/*---- ���[�h���ƂɃ^�[�Q�b�g���X�V���� ----*/
		POINT pos[ 2 ] = { { rect.left, rect.top }, { rect.left, rect.top } };
		SIZE size[ 2 ] = { { width, height }, { width, height } };
		switch( data->mode )
		{
		case SPL_VERTICAL:
			if( data->lock != SPL_LOCK_LEFT )
			{
				lWidth = width - lWidth;
			}
			if( lWidth <= 0 )		lWidth = 0;
			if( lWidth >= width )	lWidth = width;
			pos[ 1 ].x = (LONG)(rect.left + lWidth + bWidth);
			size[ 0 ].cx = (LONG)(rect.left + lWidth - bWidth);
			size[ 1 ].cx -= pos[ 1 ].x;
			break;
		case SPL_HORIZONTAL:
			if( data->lock != SPL_LOCK_TOP )
			{
				lWidth = height - lWidth;
			}
			if( lWidth <= 0 )		lWidth = 0;
			if( lWidth >= height )	lWidth = height;
			pos[ 1 ].y = (LONG)(rect.top + lWidth + bWidth);
			size[ 0 ].cy = (LONG)(rect.top + lWidth - bWidth);
			size[ 1 ].cy -= pos[ 1 ].y;
			break;
		}


		/*---- �E�B���h�E���X�V ----*/
		for( int i = 0; i < 2; i++ )
		{
			if( data->hWnd[ i ] )
			{
				MoveWindow( data->hWnd[ i ], pos[ i ].x, pos[ i ].y, size[ i ].cx, size[ i ].cy, data->repaint );
				::InvalidateRect( data->hWnd[ i ], NULL, TRUE );
			}
		}
	}
}


//----------------------------------------------
// �����E�B���h�E�T�C�Y���b�Z�[�W�N���b�J
//
static void splitWindowSizeCrack( HWND hWnd, int type, WORD width, WORD height )
{
	RegSplitWindowData* data = getSplitWindowData( hWnd );
	if( data && width > 0 && height > 0 )
	{
		// ���߂ėL���Ȓl���������ꍇ�͒��S�ŕ����Ƃ���
		if( data->width < 0 )
		{
			switch( data->mode )
			{
			case SPL_VERTICAL:
				data->width = width / 2;
				break;
			case SPL_HORIZONTAL:
				data->width = height / 2;
				break;
			}
		}
	}

	updateSplitWindowTarget( hWnd );
}
