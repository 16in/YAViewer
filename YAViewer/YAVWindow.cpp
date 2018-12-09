/*----------------------------------------------------------------------------------------
*
* YAViewer Window�֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVResource.h"
#include "YAVWindow.h"
#include "YAVSplitWindow.h"
#include "YAVView.h"
#include "YAVFileTree.h"
#include "YAVFileTab.h"
#include "YAVSendSelector.h"
#include "YAVShortcut.h"
#include "YAVAppAction.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <string>
#include <process.h>


#define YAV_APP_CAPTION		L"YAViewer"
#define YAV_VERSION			L"YAViewer ver0.01 ��"


//----------------------------------------------
// �t�@�C���X�R�[�v�O���[�o���֐��錾
//
// �A�v���P�[�V�����E�B���h�E ���b�Z�[�W�N���b�N
static LRESULT appWindowCreateCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct );
static LRESULT appWindowCommandCrack( HWND hWnd, WORD id, WORD notifyCode, HWND hControl );
static LRESULT appWindowSizeCrack( HWND hWnd, int type, WORD width, WORD height );
static LRESULT appWindowMoveCrack( HWND hWnd, WORD x, WORD y );
static LRESULT appWindowDestroyCrack( HWND hWnd );
static LRESULT appWindowNotifyCrack( HWND hWnd, UINT id, NMHDR* pNotifyHead );
static LRESULT appWindowContextMenu( HWND hWnd, HWND hCtrlWnd, int x, int y );

// �o�[�W�����_�C�A���O�\��
static void appShowVersionDialog( HWND hWnd );


//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterYAVWindowClass( HINSTANCE hInstance )
{
	/*---- �g�p����E�B���h�E�N���X��o�^ ----*/
	if( !RegisterViewWindowClass( hInstance ) )		return false;
	if( !RegisterTreeWindowClass( hInstance ) )		return false;
	if( !RegisterSplitWindowClass( hInstance ) )	return false;
	if( !RegisterSelectorWindowClass( hInstance ) )	return false;
	if( !RegisterYAVFileTabClass( hInstance ) )		return false;
	if( !RegisterYAVShortcutClass( hInstance ) )	return false;


	/*---- �A�v���P�[�V�����E�B���h�E�N���X�o�^ ----*/
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize		= sizeof( wc );
	wc.cbWndExtra	= sizeof( RegYavAppData* );
	wc.hInstance	= hInstance;
	wc.style		= CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc	= YAVAppWindowProc;
	wc.hIcon		= ::LoadIcon( hInstance, MAKEINTRESOURCEW( IDI_APP ) );
	wc.lpszClassName= L"YAViewer AppWindowClass";
	wc.lpszMenuName	= MAKEINTRESOURCEW( IDR_MAINMENU );

	return (RegisterClassExW( &wc ) != NULL);
}


//----------------------------------------------
// �E�B���h�E����
//
HWND CreateYAVWindow( HINSTANCE hInstance, RegYavAppData* appData )
{
	HWND ret = NULL;
	DWORD exStyle = 0;
	DWORD style = WS_OVERLAPPEDWINDOW;

	ret = CreateWindowExW( exStyle, L"YAViewer AppWindowClass", YAV_APP_CAPTION, style,
		appData->x, appData->y, appData->width, appData->height, NULL, NULL, hInstance, appData );

	return ret;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK YAVAppWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	default:
		return DefWindowProcW( hWnd, msg, wParam, lParam );

	case WM_CREATE:			return appWindowCreateCrack( hWnd, (LPCREATESTRUCT)lParam );
	case WM_DESTROY:		return appWindowDestroyCrack( hWnd );
	case WM_COMMAND:		return appWindowCommandCrack( hWnd, LOWORD( wParam ), HIWORD( wParam ), (HWND)lParam );
	case WM_SIZE:			return appWindowSizeCrack( hWnd, (int)wParam, LOWORD( lParam ), HIWORD( lParam ) );
	case WM_MOVE:			return appWindowMoveCrack( hWnd, LOWORD( lParam ), HIWORD( lParam ) );
	case WM_NOTIFY:			return appWindowNotifyCrack( hWnd, (UINT)wParam, (NMHDR*)lParam );
	case WM_CONTEXTMENU:	return appWindowContextMenu( hWnd, (HWND)wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );


	//-------------------------------------------------------
	// �g���E�B���h�E���b�Z�[�W
	//
	case YAV_WM_ADDFILE:// �t�@�C�����^�u�ɒǉ�����
		if( YavAddFile( GetYAVAppData( hWnd ), ((YavWmAddFile*)lParam)->filePath, (wParam != FALSE) ) )
		{
			// �`��̈���X�V
			RECT rect; GetClientRect( hWnd, &rect );
			appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			return TRUE;
		}
		break;

	case YAV_WM_DELFILE:// �t�@�C�����^�u����폜
		if( YavDelFile( GetYAVAppData( hWnd ), (int)wParam ) )
		{
			// �`��̈���X�V
			RECT rect; GetClientRect( hWnd, &rect );
			appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			return TRUE;
		}
		break;

	case YAV_WM_SELECTITEM:// �A�C�e���I������
		{
			ViewSelectItem* item = (ViewSelectItem*)lParam;
			DWORD target = SST_CLIPBOARD;
			switch( wParam )
			{
			case 0x0000:// ���N���b�N
				target = GetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetLeftClick );
				break;
			case 0x0002:// ���N���b�N
				target = GetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetCenterClick );
				break;
			}
			if( target == SST_CLIPBOARD )
			{
				YavSendClipBoardString( GetYAVAppData( hWnd ), hWnd, item->itemString, item->itemLength );
			}
			else
			{
				YavSendEditorString( GetYAVAppData( hWnd ), target, item->itemString, item->itemLength );
			}
		}
		break;

	case YAV_WM_TREE_SELECTITEM:// �c���[�A�C�e���I��
		{
			TreeSelectItem* item = (TreeSelectItem*)lParam;
			if( !wParam )
			{
				YavWmAddFile addParam;
				ZeroMemory( &addParam, sizeof( addParam ) );
				
				addParam.filePath = item->filePath;
				SendMessageW( hWnd, YAV_WM_ADDFILE, 1, (LPARAM)&addParam );
			}
		}
		break;

	case YAV_WM_SAVEFILETAB:// �t�@�C���^�u���X�g�ۑ�
		{
			/*---- �t�@�C���ꗗ�𐶐� ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			YavSaveFileTab( data, (LPCWSTR)lParam );
		}
		break;

	case YAV_WM_LOADFILETAB:// �t�@�C���^�u���X�g����
		{
			/*---- �t�@�C���ꗗ�𐶐� ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			if( YavLoadFileTab( data, (LPCWSTR)lParam ) )
			{
				YavSelectFile( data, 0 );

				/*---- �`��̈�X�V ----*/
				RECT rect; GetClientRect( hWnd, &rect );
				appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			}
		}
		break;

	case YAV_WM_SAVEFILETREE:// �t�@�C���c���[�ۑ�
		{
			/*---- �t�@�C���ꗗ�𐶐� ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			YavSaveFileTree( data, (LPCWSTR)lParam );
		}
		break;

	case YAV_WM_LOADFILETREE:// �t�@�C���c���[����
		{
			/*---- �t�@�C���ꗗ�𐶐� ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			if( YavLoadFileTree( data, (LPCWSTR)lParam ) )
			{
				/*---- �`��̈�X�V ----*/
				RECT rect; GetClientRect( hWnd, &rect );
				appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			}
		}
		break;

	case YAV_WM_PAGEJUMP:// �t�@�C���̎w��ʒu�փW�����v
		{
			RegYavAppData* data = GetYAVAppData( hWnd );
			SetViewScrollPositionByPage( data->hFileView, (int)lParam );
		}
		break;
	}

	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E�o�^�f�[�^�擾
//
RegYavAppData* GetYAVAppData( HWND hWnd )
{
	if( !hWnd ) return NULL;
	return (RegYavAppData*)GetWindowLongPtrW( hWnd, 0 );
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_CREATE�̏���
//
static LRESULT appWindowCreateCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct )
{
	/*---- �o�^�f�[�^�ݒ� ----*/
	LPCREATESTRUCT cs = pCreateStruct;
	RegYavAppData* appData = (RegYavAppData*)cs->lpCreateParams;
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)appData );


	/*---- �t�H���g�𐶐� ----*/
	LOGFONTW logFont;
	ZeroMemory( &logFont, sizeof( logFont ) );
	wcscpy_s( logFont.lfFaceName, appData->fontName/*L"�l�r �o�S�V�b�N"*/ );
	logFont.lfHeight = -appData->fontSize;
	appData->hAAFont = ::CreateFontIndirectW( &logFont );


	/*---- �^�u�E�B���h�E���� ----*/
	appData->hFileTab = CreateFileTabWindow( cs->hInstance, hWnd, 0, IDC_FILETAB );


	/*---- �R���g���[���E�B���h�E���� ----*/
	appData->hCtrlWnd = CreateSplitWindow( cs->hInstance, GetFileTabWindow( appData->hFileTab ), 20 );
	SetSplitWindowLock( appData->hCtrlWnd, SPL_LOCK_LEFT );


	/*---- �V���[�g�J�b�g/�c���[�E�B���h�E���� ----*/
	appData->hTreeShort = CreateSplitWindow( cs->hInstance, appData->hCtrlWnd, 0 );
	if( appData->hTreeShort )
	{
		SetSplitWindowMode( appData->hTreeShort, SPL_HORIZONTAL );
		SetSplitWindowLock( appData->hTreeShort, SPL_LOCK_BOTTOM );
	}


	/*---- �t�@�C���c���[���� ----*/
	if( appData->hTreeShort )
	{
		appData->hFileTree = CreateTreeWindow( cs->hInstance, appData->hTreeShort, WS_BORDER );
	}
	else
	{
		appData->hFileTree = CreateTreeWindow( cs->hInstance, appData->hCtrlWnd, WS_BORDER );
	}


	/*---- �V���[�g�J�b�g�E�B���h�E���� ----*/
	if( appData->hTreeShort )
	{
		appData->hAAShortcut = CreateShortcutWindow( cs->hInstance, appData->hTreeShort, WS_BORDER );
		SetShortcutSendNotifyWindow( appData->hAAShortcut, hWnd );
	}


	/*---- �t�@�C���r���[���� ----*/
	appData->hFileView = CreateViewWindow( cs->hInstance, appData->hCtrlWnd, WS_BORDER );
	SetViewFont( appData->hFileView, appData->hAAFont );

	// �A�v���P�[�V�����E�B���h�E�փ��b�Z�[�W�𑗂�
	SetViewSendMessageRootWindow( appData->hFileView, true );
	SetTreeSendMessageRootWindow( appData->hFileTree, true );


	/*---- �R���g���[���E�B���h�E�Ƀ^�[�Q�b�g�ݒ� ----*/
	if( appData->hTreeShort )
	{
		SetSplitTargetWindow( appData->hTreeShort, appData->hAAShortcut, appData->hFileTree );
		SetSplitWidth( appData->hTreeShort, appData->shortcutSize );
		if( !appData->reverseSplit )
		{
			SetSplitTargetWindow( appData->hCtrlWnd, appData->hTreeShort, appData->hFileView );
		}
		else
		{
			SetSplitTargetWindow( appData->hCtrlWnd, appData->hFileView, appData->hTreeShort );
		}
	}
	else
	{
		if( !appData->reverseSplit )
		{
			SetSplitTargetWindow( appData->hCtrlWnd, appData->hFileTree, appData->hFileView );
		}
		else
		{
			SetSplitTargetWindow( appData->hCtrlWnd, appData->hFileView, appData->hFileTree );
		}
	}


	/*---- ���M��I���E�B���h�E���� ----*/
	appData->hSelector = CreateSelectorWindow( cs->hInstance, hWnd, 0 );


	/*---- �t�@�C���\�����[�h�ݒ� ----*/
	SetViewMode( appData->hFileView, appData->viewMode );


	/*---- �c�[���q���g�̗L������ ----*/
	if( !appData->toolHint )
	{// �f�t�H��true�̂͂��Ȃ̂ŁA���킹�ĕύX����
		::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_VIEW_TOOLTIP, 0 ), 0 );
	}


	/*---- �^�u�{�^�����[�h�̗L������ ----*/
	if( appData->tabButtonMode )
	{// �f�t�H��false�̂͂��Ȃ̂ŁA���킹�ĕύX����
		::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_TAB_BUTTON, 0 ), 0 );
	}


	/*---- �_�u���N���b�N���[�h�̗L������ ----*/
	if( appData->dblclickMode )
	{
		SetSelectDoubleClick( appData->hSelector, true );
	}


	/*---- �����E�B���h�E�T�C�Y�ݒ� ----*/
	if( appData->splitPos < 0 )
	{
		if( cs->cx < 0 )appData->splitPos = 300;
		else			appData->splitPos = cs->cx / 2;
	}
	SetSplitWidth( appData->hCtrlWnd, appData->splitPos );


	/*---- �g���q�I�� ----*/
	SetEnableTreeItemExtension( appData->hFileTree, appData->enableExtension );
	{
		// ���j���[�X�V
		HMENU hMenu = GetMenu( hWnd );
		MENUITEMINFO info;
		info.cbSize = sizeof( info );
		info.fMask = MIIM_STATE;
		for( WORD i = 0; i < TITEM_EXT_MAX_SHIFT; i++ )
		{
			if( (appData->enableExtension & (1 << i)) )
			{// �`�F�b�N ON
				info.fState = MFS_CHECKED;
			}
			else
			{// �`�F�b�N OFF
				info.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfoW( hMenu, IDM_ENABLE_TXT + i, FALSE, &info );
		}
	}


	/*---- �J�e�S���\���T�C�Y�ݒ� ----*/
	switch( appData->categorySize )
	{
	case 32:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_32, 0 ), 0 ); break;
	case 64:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_64, 0 ), 0 ); break;
	case 96:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_96, 0 ), 0 ); break;
	case 128:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_128, 0 ), 0 ); break;
	case 160:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_160, 0 ), 0 ); break;
	case 192:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_192, 0 ), 0 ); break;
	case 224:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_224, 0 ), 0 ); break;
	case 256:	::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_CATEGORY_256, 0 ), 0 ); break;
	default:
		SetViewCategorySize( appData->hFileView, appData->categorySize );
		break;
	}


	/*---- �t�@�C���^�u������ ----*/
	std::wstring initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += appData->initTabPath;
	if( ::PathFileExistsW( initPath.c_str() ) )
	{
		YavLoadFileTab( appData, initPath.c_str(), true );
		YavSelectFile( appData, 0 );
	}


	/*---- �t�@�C���c���[������ ----*/
	initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += appData->initTreePath;
	if( ::PathFileExistsW( initPath.c_str() ) )
	{
		YavLoadFileTree( appData, initPath.c_str(), true );
	}


	/*---- ���M��������� ----*/
	SetSelectSendSelector( appData->hSelector, SendTargetLeftClick, appData->sendTargetLeft );
	SetSelectSendSelector( appData->hSelector, SendTargetCenterClick, appData->sendTargetCenter );


	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_COMMAND�̏���
//
static LRESULT appWindowCommandCrack( HWND hWnd, WORD id, WORD notifyCode, HWND hControl )
{
	switch( id )
	{
	case IDM_EXIT:// �I�����b�Z�[�W
		DestroyWindow( hWnd );
		break;
	case IDM_FILEOPEN:// �t�@�C�����J��
		{
			wchar_t path[ MAX_PATH ];
			if( OpenAAFileDialog( hWnd, path, MAX_PATH ) )
			{
				YavWmAddFile addFile = { 0 };
				addFile.filePath = path;

				SendMessageW( hWnd, YAV_WM_ADDFILE, 1, (LPARAM)&addFile );
			}
		}
		break;
	case IDM_TABSAVE:// �^�u��ۑ�
		{
			wchar_t path[ MAX_PATH ];
			if( SaveFileTabDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_SAVEFILETAB, 0, (LPARAM)path );
			}
		}
		break;
	case IDM_TABOPEN:// �^�u�𕜌�
		{
			wchar_t path[ MAX_PATH ];
			if( OpenFileTabDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_LOADFILETAB, 0, (LPARAM)path );
			}
		}
		break;
	case IDM_TREESAVE:// �c���[��ۑ�
		{
			wchar_t path[ MAX_PATH ];
			if( SaveFileTreeDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_SAVEFILETREE, 0, (LPARAM)path );
			}
		}
		break;
	case IDM_TREEOPEN:// �c���[�𕜌�
		{
			wchar_t path[ MAX_PATH ];
			if( OpenFileTreeDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_LOADFILETREE, 0, (LPARAM)path );
			}
		}
		break;

	case IDM_MOST_TOP:// �őO�ʕ\��
		{
			DWORD exStyle = (DWORD)GetWindowLongPtrW( hWnd, GWL_EXSTYLE );
			if( exStyle & WS_EX_TOPMOST )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED );
				CheckMenuItem( GetMenu( hWnd ), IDM_MOST_TOP, MF_BYCOMMAND | MF_UNCHECKED );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED );
				CheckMenuItem( GetMenu( hWnd ), IDM_MOST_TOP, MF_BYCOMMAND | MF_CHECKED );
			}
		}
		break;

	case IDM_VIEW_TOOLTIP:// �c�[���`�b�v�\��
		{
			bool enable = !GetViewToolTip( GetYAVAppData( hWnd )->hFileView );
			SetViewToolTip( GetYAVAppData( hWnd )->hFileView, enable );
			if( enable )
			{
				CheckMenuItem( GetMenu( hWnd ), IDM_VIEW_TOOLTIP, MF_BYCOMMAND | MF_UNCHECKED );
			}
			else
			{
				CheckMenuItem( GetMenu( hWnd ), IDM_VIEW_TOOLTIP, MF_BYCOMMAND | MF_CHECKED );
			}
		}
		break;

	case IDM_TAB_BUTTON:// �^�u���{�^���\���ɂ���
		{
			bool enable = !GetFileTabButtonMode( GetYAVAppData( hWnd )->hFileTab );
			SetFileTabButtonMode( GetYAVAppData( hWnd )->hFileTab, enable );
			
			RECT rect;
			GetClientRect( hWnd, &rect );
			appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			if( enable )
			{
				CheckMenuItem( GetMenu( hWnd ), IDM_TAB_BUTTON, MF_BYCOMMAND | MF_CHECKED );
			}
			else
			{
				CheckMenuItem( GetMenu( hWnd ), IDM_TAB_BUTTON, MF_BYCOMMAND | MF_UNCHECKED );
			}
		}
		break;
	case IDM_NORMAL_VIEW:// �m�[�}���\���ɐ؂�ւ�
		SetViewMode( GetYAVAppData( hWnd )->hFileView, 0 );
		break;
	case IDM_CATEGORY_VIEW:// �J�e�S���\���ɐ؂�ւ�
		SetViewMode( GetYAVAppData( hWnd )->hFileView, 1 );
		break;
	case IDM_EDITOR_VIEW:// �G�f�B�^�\���ɐ؂�ւ�
		SetViewMode( GetYAVAppData( hWnd )->hFileView, 2 );
		break;

	// ���M��ݒ�
	case IDM_SEND_FACEEDIT:
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetLeftClick, SST_FACE_EDIT );
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetCenterClick, SST_FACE_EDIT | SST_TEXTBOX_FLAG );
		break;
	case IDM_SEND_ORINEDIT:
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetLeftClick, SST_ORINRIN_EDITOR );
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetCenterClick, SST_ORINRIN_EDITOR | SST_TEXTBOX_FLAG );
		break;

	// �o�[�W�������
	case IDM_VERSION:
		appShowVersionDialog( hWnd );
		break;

	// �����E�B���h�E����ւ�
	case IDM_SPLIT_SWAP:
		SwapSplitTargetWindow( GetYAVAppData( hWnd )->hCtrlWnd );
		break;

	// �J�e�S���\���T�C�Y�ύX
	case IDM_CATEGORY_32:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 32 );	break;
	case IDM_CATEGORY_64:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 64 );	break;
	case IDM_CATEGORY_96:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 96 );	break;
	case IDM_CATEGORY_128:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 128 );	break;
	case IDM_CATEGORY_160:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 160 );	break;
	case IDM_CATEGORY_192:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 192 );	break;
	case IDM_CATEGORY_224:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 224 );	break;
	case IDM_CATEGORY_256:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 256 );	break;

	// �g���q�I������
	case IDM_ENABLE_MLT: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_MLT ); break;
	case IDM_ENABLE_AST: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_AST ); break;
	case IDM_ENABLE_ASD: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_ASD ); break;
	case IDM_ENABLE_TXT: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_TXT ); break;
	}


	// �J�e�S�����W�I�}�[�N�ύX
	if( IDM_CATEGORY_32 <= id && id <= IDM_CATEGORY_256 )
	{
		HMENU hMenu = GetMenu( hWnd );
		MENUITEMINFO info;

		info.cbSize = sizeof( info );
		info.fMask = MIIM_STATE;

		for( WORD i = IDM_CATEGORY_32; i <= IDM_CATEGORY_256; i++ )
		{
			if( i == id )
			{// �`�F�b�N ON
				info.fState = MFS_CHECKED;
			}
			else
			{// �`�F�b�N OFF
				info.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfoW( hMenu, i, FALSE, &info );
		}
	}

	// �g���q�I��ύX
	if( IDM_ENABLE_TXT <= id && id <= IDM_ENABLE_ASD )
	{
		// ���j���[�X�V
		DWORD ext = GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree );
		HMENU hMenu = GetMenu( hWnd );
		MENUITEMINFO info;
		info.cbSize = sizeof( info );
		info.fMask = MIIM_STATE;
		for( WORD i = 0; i < TITEM_EXT_MAX_SHIFT; i++ )
		{
			if( (ext & (1 << i)) )
			{// �`�F�b�N ON
				info.fState = MFS_CHECKED;
			}
			else
			{// �`�F�b�N OFF
				info.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfoW( hMenu, IDM_ENABLE_TXT + i, FALSE, &info );
		}

		// �c���[�r���[�X�V
		std::wstring initPath = YavGetAppDirectory();
		initPath += L'\\';
		initPath += GetYAVAppData( hWnd )->initTreePath;
		YavSaveFileTree( GetYAVAppData( hWnd ), initPath.c_str() );
		YavLoadFileTree( GetYAVAppData( hWnd ), initPath.c_str() );
	}


	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_SIZE�̏���
//
static LRESULT appWindowSizeCrack( HWND hWnd, int type, WORD width, WORD height )
{
	/*---- �A�v���f�[�^�擾 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- �f�[�^�R�s�[ -----*/
	if( type == SIZE_RESTORED )
	{
		RECT winRect;
		GetWindowRect( hWnd, &winRect );
		data->width = winRect.right - winRect.left;
		data->height = winRect.bottom - winRect.top;
	}

	
	/*---- �t�@�C���^�u�T�C�Y�X�V ----*/
	int SelectorSize = 32;
	RECT tabRect;
	MoveWindow( data->hFileTab, 0, 0, width, height - SelectorSize, TRUE );
	GetClientRect( data->hFileTab, &tabRect );
	TabCtrl_AdjustRect( data->hFileTab, FALSE, &tabRect );
	

	/*---- �R���g���[���E�B���h�E�T�C�Y�X�V ----*/
	MoveWindow( data->hCtrlWnd, tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top, TRUE );


	/*---- �Z���N�^�X�V ----*/
	MoveWindow( data->hSelector, 0, height - SelectorSize, width, SelectorSize, TRUE );


	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_MOVE�̏���
//
static LRESULT appWindowMoveCrack( HWND hWnd, WORD x, WORD y )
{
	/*---- �A�v���P�[�V�����f�[�^�擾 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- �E�B���h�E�ʒu���擾 ----*/
	RECT winRect;
	GetWindowRect( hWnd, &winRect );
	if( winRect.left > -data->width )
	{
		data->x = winRect.left;
		data->y = winRect.top;
	}


	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_DESTROY�̏���
//
static LRESULT appWindowDestroyCrack( HWND hWnd )
{
	/*---- �A�v���P�[�V�����f�[�^�擾 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- �\�����[�h�擾 ----*/
	data->viewMode = GetViewMode( data->hFileView );
	data->toolHint = GetViewToolTip( data->hFileView );
	data->tabButtonMode = GetFileTabButtonMode( data->hFileTab );


	/*---- �J�e�S���T�C�Y�擾 ----*/
	data->categorySize = GetViewCategorySize( data->hFileView );


	/*---- �����ʒu�擾 ----*/
	data->splitPos = GetSplitWidth( data->hCtrlWnd );


	/*---- ������Ԏ擾 ----*/
	HWND right = NULL;
	GetSplitTargetWindow( data->hCtrlWnd, NULL, &right );
	data->reverseSplit = (right != data->hFileView);
	
	if( data->hTreeShort )
	{
		data->shortcutSize = GetSplitWidth( data->hTreeShort );
	}


	/*---- �^�u�ۑ� ----*/
	std::wstring initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += data->initTabPath;
	YavSaveFileTab( data, initPath.c_str(), true );
	
	
	/*---- �c���[�ۑ� ----*/
	initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += data->initTreePath;
	YavSaveFileTree( data, initPath.c_str(), true );


	/*---- �c���[�g���q�I����ԕۑ� ----*/
	data->enableExtension = GetEnableTreeItemExtension( data->hFileTree );


	/*---- ���M��ۑ� ----*/
	data->sendTargetLeft	= GetSelectSendSelector( data->hSelector, SendTargetLeftClick );
	data->sendTargetCenter	= GetSelectSendSelector( data->hSelector, SendTargetCenterClick );
	data->dblclickMode		= GetSelectDoubleClick( data->hSelector ) ? 1 : 0;


	/*---- �A�v���P�[�V�����f�[�^�p�� ----*/
	SetWindowLongPtrW( hWnd, 0, 0 );

	PostQuitMessage( 0 );

	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_NOTIFY�̏���
//
static LRESULT appWindowNotifyCrack( HWND hWnd, UINT id, NMHDR* pNotifyHead )
{
	/*---- �A�v���P�[�V�����f�[�^�擾 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- �R�[�h�l���Ƃɕ��� ----*/
	switch( pNotifyHead->code )
	{
	case NM_CLICK:
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- ����{�^������ ----*/
			int				nIndex, mvIndex;
			TCHITTESTINFO	info;
			RECT			rect;
			bool			close = false;

			// �^�b�`�ʒu
			GetCursorPos( &info.pt );
			ScreenToClient( data->hFileTab, &info.pt );
			nIndex = TabCtrl_HitTest( data->hFileTab, &info );
			if( GetFileTabCloseRect( data->hFileTab, nIndex, &rect ) )
			{
				if( rect.left <= info.pt.x && info.pt.x <= rect.right
					&& rect.top <= info.pt.y && info.pt.y <= rect.bottom )
				{
					close = true;
				}
			}
			if( FileTabCloseSkip( data->hFileTab, &mvIndex ) )
			{
				/*---- �A�C�e���I�� ----*/
				TabCtrl_SetCurSel( data->hFileTab, mvIndex );

				TCITEMW item;
				item.mask = TCIF_PARAM;
				if( TabCtrl_GetItem( data->hFileTab, TabCtrl_GetCurSel( data->hFileTab ), &item ) )
				{
					if( item.lParam )
					{
						YavFileTabParam* param = (YavFileTabParam*)item.lParam;
						param->scrollPosition = GetViewScrollPosition( data->hFileView );
						SetShortcutFile( data->hAAShortcut, param->file );
						SetViewFile( data->hFileView, param->file );
						SetViewScrollPosition( data->hFileView, param->scrollPosition );
					}
				}
			}
			else
			{
				if( !close )
				{
				}
				else
				{
					/*---- �t�@�C������ ----*/
					SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)nIndex, NULL );
					

					/*---- �A�C�e���I�� ----*/
					TCITEMW item;
					item.mask = TCIF_PARAM;
					if( TabCtrl_GetItem( data->hFileTab, TabCtrl_GetCurSel( data->hFileTab ), &item ) )
					{
						if( item.lParam )
						{
							YavFileTabParam* param = (YavFileTabParam*)item.lParam;
							SetShortcutFile( data->hAAShortcut, param->file );
							SetViewFile( data->hFileView, param->file );
							SetViewScrollPosition( data->hFileView, param->scrollPosition );
						}
					}
				}
			}

			// �K�����������N���A����悤�ɁB
			FileTabCloseSkip( data->hFileTab, NULL );
		}
		break;

	case TCN_SELCHANGING:// �t�@�C���^�u�I��
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- ���Ȃ��ꍇ�̓X�N���[���f�[�^���i�[���Ă��� ----*/
			TCITEMW item;
			item.mask = TCIF_PARAM;
			if( TabCtrl_GetItem( data->hFileTab, TabCtrl_GetCurSel( data->hFileTab ), &item ) )
			{
				if( item.lParam )
				{
					YavFileTabParam* param = (YavFileTabParam*)item.lParam;
					param->scrollPosition = GetViewScrollPosition( data->hFileView );
				}
			}
		}
		return 0;

	case TCN_SELCHANGE:// �t�@�C���^�u�I��
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- �A�C�e���I�� ----*/
			TCITEMW item;
			item.mask = TCIF_PARAM;
			int idx = TabCtrl_GetCurSel( data->hFileTab );
			if( TabCtrl_GetItem( data->hFileTab, idx, &item ) && item.lParam )
			{
				YavFileTabParam* param = (YavFileTabParam*)item.lParam;
				SetShortcutFile( data->hAAShortcut, param->file );
				SetViewFile( data->hFileView, param->file );
				SetViewScrollPosition( data->hFileView, param->scrollPosition );
			}
		}
		break;
	case NM_RCLICK:// �E�N���b�N
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- �E�N���b�N���ꂽ�^�u�̑I�� ----*/
			int           nIndex, nMaxIndex;
			TCHITTESTINFO info;

			GetCursorPos( &info.pt );
			ScreenToClient( data->hFileTab, &info.pt );
			
			nIndex = TabCtrl_HitTest( data->hFileTab, &info );
			nMaxIndex = TabCtrl_GetItemCount( data->hFileTab );
			if( nIndex != -1 )
			{
				/*---- �R���e�L�X�g���j���[�Ăяo�� ----*/
				HMENU hBaseMenu = LoadMenuW( NULL, MAKEINTRESOURCEW( IDR_FILETABMENU ) );
				ClientToScreen( data->hFileTab, &info.pt );
				UINT id = TrackPopupMenu( GetSubMenu( hBaseMenu, 0 ), TPM_LEFTALIGN | TPM_RETURNCMD, info.pt.x, info.pt.y, 0, hWnd, NULL );
				DestroyMenu( hBaseMenu );


				/*---- �t�@�C���^�u�R���e�L�X�g��ID������ ----*/
				switch( id )
				{
				case IDM_CLOSEFILE:	// ����
					SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)nIndex, NULL );
					if( nIndex > 0 )
					{
						TabCtrl_SetCurSel( data->hFileTab, nIndex - 1 );
					}
					else
					{
						TabCtrl_SetCurSel( data->hFileTab, 0 );
					}
					break;
				case IDM_ALL_CLOSE:// ����ȊO�����ׂĕ���
					for( int i = 0; i < nMaxIndex; i++ )
					{
						SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)0, NULL );
					}
					TabCtrl_SetCurSel( data->hFileTab, 0 );
					break;
				case IDM_CLOSE_OTHER:// ����ȊO�����ׂĕ���
					for( int i = nIndex + 1; i < nMaxIndex; i++ )
					{
						SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)nIndex + 1, NULL );
					}
					for( int i = 0; i < nIndex; i++ )
					{
						SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)0, NULL );
					}
					TabCtrl_SetCurSel( data->hFileTab, 0 );
					break;

				case IDM_TAB_COLOR:
					{
						TCITEMW item;
						item.mask = TCIF_PARAM;
						TabCtrl_GetItem( data->hFileTab, nIndex, &item );
						if( item.lParam )
						{
							YavFileTabParam* param = (YavFileTabParam*)item.lParam;
							if( SelectTabColorDialog( hWnd, &param->tabColor ) )
							{
								param->useTabColor = true;
							}
							InvalidateRect( GetFileTabWindow( data->hFileTab ), NULL, TRUE );
						}
					}
					break;
				case IDM_TAB_CLEAR_COLOR:
					{
						TCITEMW item;
						item.mask = TCIF_PARAM;
						TabCtrl_GetItem( data->hFileTab, nIndex, &item );
						if( item.lParam )
						{
							YavFileTabParam* param = (YavFileTabParam*)item.lParam;
							param->useTabColor = false;
							InvalidateRect( GetFileTabWindow( data->hFileTab ), NULL, TRUE );
						}
					}
					break;

				case IDM_TAB_OPEN_DIRECTORY:// �t�@�C���̃t�H���_���J��
					{
						TCITEMW item;
						item.mask = TCIF_PARAM;
						TabCtrl_GetItem( data->hFileTab, nIndex, &item );
						if( item.lParam )
						{
							YavFileTabParam* param = (YavFileTabParam*)item.lParam;
							wchar_t path[ MAX_PATH ];
							wcscpy_s( path, param->filePath );
							::PathRemoveFileSpecW( path );
							::_wspawnlp( _P_NOWAIT, L"explorer", L"explorer", path, NULL );
						}
					}
					break;
				}


				/*---- �r���[�̕\����ԕύX ----*/
				{
					TCITEMW item;
					item.mask = TCIF_PARAM;
					int idx = TabCtrl_GetCurSel( data->hFileTab );
					TabCtrl_GetItem( data->hFileTab, idx, &item );

					if( item.lParam )
					{
						YavFileTabParam* param = (YavFileTabParam*)item.lParam;
						SetShortcutFile( data->hAAShortcut, param->file );
						SetViewFile( data->hFileView, param->file );
					}
				}


				/*---- �̈���ĕ`�� ----*/
				RECT rect;
				GetClientRect( hWnd, &rect );
				InvalidateRect( hWnd, NULL, TRUE );
				appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			}
		}
		break;
	}

	return 0;
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
//	WM_CONTEXTMENU�̏���
//
static LRESULT appWindowContextMenu( HWND hWnd, HWND hCtrlWnd, int x, int y )
{
	return DefWindowProcW( hWnd, WM_CONTEXTMENU, (WPARAM)hCtrlWnd, MAKELONG( x, y ) );
}


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E
// �o�[�W�����_�C�A���O�\��
//
static void appShowVersionDialog( HWND hWnd )
{
	MessageBoxW( hWnd, YAV_VERSION, L"�o�[�W�������", MB_OK | MB_ICONINFORMATION );
}
