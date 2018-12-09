/*----------------------------------------------------------------------------------------
*
* YAViewer ���M��I���֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVSendSelector.h"
#include "YAVAppAction.h"


#define YAVVELECTOR_CLASSNAME		L"YAVVIEW_SELECTORWINDOWCLASS"


//----------------------------------------------
// �Ǘ��\���̒�`
//
struct RegSelectorData
{
	HWND		hLeftComboBox;			// ���N���b�N�R���{�{�b�N�X
	HWND		hCenterComboBox;		// ���N���b�N�R���{�{�b�N�X
	HWND		hDblClickButton;		// �_�u���N���b�N�{�^��
	HWND		hLeftLabel;				// ���������x��
	HWND		hCenterLabel;			// ���������x��
};
struct SendSelectTarget
{
	DWORD		target;			// �^�[�Q�b�g
	LPCWSTR		name;			// ���O
};


//----------------------------------------------
// �O���[�o���ϐ���`
//
static const SendSelectTarget scgSelectTarget[] =
{
	{ SST_CLIPBOARD,						L"�N���b�v�{�[�h" },
	{ SST_FACE_EDIT,						L"(�L�t�M) Edit" },
	{ SST_FACE_EDIT | SST_TEXTBOX_FLAG,		L"(�L�t�M) ÷���ޯ��" },
	{ SST_ORINRIN_EDITOR,					L"Orinrin Editor" },
	{ SST_ORINRIN_EDITOR | SST_TEXTBOX_FLAG,L"Orinrin ڲ��ޯ��" },

	{ SST_UNKNOWN, NULL }
};


//----------------------------------------------
// �O���[�o���֐���`
//
static void setupDefaultComboItem( HWND hComb, DWORD target );

// �E�B���h�E���b�Z�[�W�N���b�J�[
static LRESULT yavCreateSelectorCracker( HWND hWnd, LPCREATESTRUCT cs );
static void yavDestroySelectorCracker( HWND hWnd );
static void yavSizeSelectorCracker( HWND hWnd, WORD mode, INT cx, INT cy );


//----------------------------------------------
// �E�B���h�E�N���X�o�^
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
// ���M��I���E�B���h�E����
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
// ���M��I���E�B���h�E�W���R�[���o�b�N
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
// ���M�� �ݒ�/�擾
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
// �I����� �ݒ�/�擾
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
// �R���{�{�b�N�X�̃A�C�e����ǉ�
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
// WM_CREATE �N���b�J�[
//
static LRESULT yavCreateSelectorCracker( HWND hWnd, LPCREATESTRUCT cs )
{
	/*---- �f�[�^���Z�b�g ----*/
	RegSelectorData* data = (RegSelectorData*)cs->lpCreateParams;
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	/*---- ���x���ǉ� ----*/
	data->hLeftLabel	= CreateWindowW( L"STATIC", L"��", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );
	data->hCenterLabel	= CreateWindowW( L"STATIC", L"��", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );


	/*---- �R���{�{�b�N�X�ǉ� ----*/
	DWORD style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST;
	data->hLeftComboBox		= CreateWindowW( L"COMBOBOX", L"", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );
	data->hCenterComboBox	= CreateWindowW( L"COMBOBOX", L"", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );

	style = WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX;
	data->hDblClickButton	= CreateWindowW( L"BUTTON", L"�_�u���N���b�N", style, 0, 0, 0, 0, hWnd, NULL, cs->hInstance, NULL );

	/*---- �A�C�e���ݒ� ----*/
	setupDefaultComboItem( data->hLeftComboBox, 0 );
	setupDefaultComboItem( data->hCenterComboBox, 0 );


	return 0;
}

//----------------------------------------------
// WM_DESTROY �N���b�J�[
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
// WM_SIZE �N���b�J�[
//
static void yavSizeSelectorCracker( HWND hWnd, WORD mode, INT cx, INT cy )
{
	RegSelectorData* data = (RegSelectorData*)GetWindowLongPtrW( hWnd, 0 );


	/*---- �\���ʒu�A�T�C�Y�ύX ----*/
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
	

	// �_�u���N���b�N
	MoveWindow( data->hDblClickButton, width * 2 + 8, 0, width - 8, cy - 4, TRUE );
}
