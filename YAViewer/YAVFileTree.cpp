/*----------------------------------------------------------------------------------------
*
* YAViewer Tree�֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVFileTree.h"
#include "YAVResource.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <process.h>


#define YAVTREE_CLASSNAME	L"YAVTREE_TREEWINDOWCLASS"


//----------------------------------------------
// �c���[�r���[�Ǘ����
//
struct RegTreeWindowData
{
	HWND		hTreeWindow;			// �c���[�r���[�{��

	bool		sendMessageRoot;		// ���[�g�Ƀ��b�Z�[�W�𑗂邩
	DWORD		extension;				// �L���Ȋg���q

	RegTreeWindowData( void )
		: hTreeWindow( NULL ), sendMessageRoot( false ), extension( TITEM_EXT_ALL )
	{
	}
};


//----------------------------------------------
// �c���[�A�C�e���Ǘ����
//
struct RegTreeItemData
{
	wchar_t			path[ MAX_PATH ];	// �t�@�C���p�X
	bool			isfile;				// �t�@�C���ł��邩
	bool			openSubDirectory;	// �T�u�f�B���N�g�����܂߂邩

	RegTreeItemData( void )
		: isfile( false ), openSubDirectory( false )
	{
		ZeroMemory( path, sizeof( path ) );
	}
};


//----------------------------------------------
// �^��`
//
typedef std::vector<std::wstring>	wstring_array;	// ������z��
typedef std::vector<LPWSTR>			TreeItemList;	// ���X�g�A�C�e��


//----------------------------------------------
// �O���[�o���֐��錾
//
static RegTreeWindowData* getTreeWindowData( HWND hTreeWnd );
static LRESULT sendTreeMessage( HWND hTreeWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static HTREEITEM insertFileTree( HWND hTreeWnd, HTREEITEM hRoot, LPCWSTR filePath, bool subDirectory, DWORD enableExtension );
static void selecteTreeItem( HWND hTreeWnd, HTREEITEM hSelItem );
static void expandTreeItem( HWND hTreeWnd, HTREEITEM hSelItem, DWORD extension, bool forceOpen = false );
static bool checkInsertablePath( LPCWSTR path, DWORD extFlag );
static bool openTreeItem( HWND hTreeWnd, HTREEITEM hItem, LPCWSTR dirPAth, bool subdir, DWORD extension );
static bool openTreeAllItem( HWND hTreeWnd, HTREEITEM hItem, DWORD extension );

// ���b�Z�[�W�N���b�J
static LRESULT treeCreateMessageCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct );
static LRESULT treeDestroyMessageCrack( HWND hWnd );
static LRESULT treeSizeMessageCrack( HWND hWnd, int width, int height );
static LRESULT treeDragAndDropCrack( HWND hWnd, HDROP hDrop );

// �R���e�L�X�g���j���[�\��
static void treeDirectoryMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt );
static void treeFileMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt );


//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterTreeWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= sizeof( RegTreeWindowData* );
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor			= LoadIconW( NULL, IDC_ARROW );
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= TreeWindowProc;
	wc.lpszClassName	= YAVTREE_CLASSNAME;

	return (RegisterClassExW( &wc ) != NULL);
}


//----------------------------------------------
// �r���[�E�B���h�E����
//
HWND CreateTreeWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle )
{
	HWND ret = NULL;
	DWORD style = WS_CHILD | WS_VISIBLE | addStyle;
	DWORD exStyle = 0;

	RegTreeWindowData* param = new RegTreeWindowData();
	ret = CreateWindowExW(
		exStyle, YAVTREE_CLASSNAME, L"YAViewer FileTree", style,
		0, 0, 0, 0, hParent, NULL, hInstance, param );
	if( !ret )
	{
		delete param;
	}

	return ret;
}


//----------------------------------------------
// �r���[�E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK TreeWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	default:		 return ::DefWindowProcW( hWnd, msg, wParam, lParam );
	case WM_CREATE:	 return treeCreateMessageCrack( hWnd, (LPCREATESTRUCT)lParam );
	case WM_DESTROY: return treeDestroyMessageCrack( hWnd );
	case WM_SIZE:	 return treeSizeMessageCrack( hWnd, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );

	// D&D������
	case WM_DROPFILES: return treeDragAndDropCrack( hWnd, (HDROP)wParam );

	// �c���[�r���[����̃��b�Z�[�W������
	case WM_NOTIFY:
		{
			RegTreeWindowData* data = getTreeWindowData( hWnd );
			switch( ((LPNMHDR)lParam)->code )
			{
			case TVN_SELCHANGED:
				selecteTreeItem( data->hTreeWindow, ((LPNMTREEVIEW)lParam)->itemNew.hItem );
				break;
			case TVN_ITEMEXPANDING:	
				if( !(((LPNMTREEVIEW)lParam)->itemNew.state & TVIS_EXPANDEDONCE) )
				{
					expandTreeItem( data->hTreeWindow, ((LPNMTREEVIEW)lParam)->itemNew.hItem, data->extension );
				}
				break;
			case TVN_DELETEITEM:
				{
					LPNMTREEVIEW treeView = (LPNMTREEVIEW)lParam;
					if( treeView->itemOld.lParam )
					{
						RegTreeItemData* itemData = (RegTreeItemData*)treeView->itemOld.lParam;
						delete itemData;
						treeView->itemOld.lParam = NULL;
					}
				}
				break;
			case NM_RCLICK:
				/*---- �R���e�L�X�g���j���[�Ăяo�� ----*/
				{
					POINT pt;
					GetCursorPos( &pt );

					TV_HITTESTINFO info = { pt, (TVHT_ONITEM | TVHT_ONITEMBUTTON) };
					ScreenToClient( data->hTreeWindow, &info.pt );
					if( TreeView_HitTest( data->hTreeWindow, &info ) != NULL )
					{
						TVITEM item = { 0 };
						item.mask = TVIF_HANDLE | TVIF_PARAM;
						item.hItem = info.hItem;
						if( TreeView_GetItem( data->hTreeWindow, &item ) )
						{
							RegTreeItemData* itemData = (RegTreeItemData*)item.lParam;
							if( itemData )
							{
								if( itemData->isfile )
								{
									treeFileMenu( hWnd, itemData, info.hItem, pt );
								}
								else
								{
									treeDirectoryMenu( hWnd, itemData, info.hItem, pt );
								}
							}
						}
					}
				}
				break;
			}
		}
		break;
	}
	return 0;
}


//----------------------------------------------
// �g�����b�Z�[�W���M��ݒ�
//
void SetTreeSendMessageRootWindow( HWND hTreeWnd, bool sendRoot )
{
	RegTreeWindowData* data = getTreeWindowData( hTreeWnd );
	if( data )
	{
		data->sendMessageRoot = sendRoot;
	}
}
bool GetTreeSendMessageRootWindow( HWND hTreeWnd )
{
	bool ret = false;
	RegTreeWindowData* data = getTreeWindowData( hTreeWnd );
	if( data )
	{
		ret = data->sendMessageRoot;
	}
	return ret;
}


//----------------------------------------------
// �Ǘ��t�@�C���p�X�̒ǉ�
//
bool TreeAddFilePath( HWND hTreeWnd, LPCWSTR filePath, bool subdir, bool caret )
{
	bool ret = false;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		if( filePath && PathFileExistsW( filePath ) )
		{
			HTREEITEM hItem = insertFileTree( data->hTreeWindow, NULL, filePath, subdir, data->extension );
			if( hItem && caret )
			{
				TreeView_Select( data->hTreeWindow, hItem, TVGN_CARET );
			}
			ret = (hItem != NULL);
		}
	}

	return ret;
}


//----------------------------------------------
// �Ǘ��t�@�C���p�X�̍폜
//
bool TreeDeleteFilePath( HWND hTreeWnd, LPCWSTR filePath )
{
	bool ret = false;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		/*---- ���[�g�A�C�e���擾 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = TreeView_GetRoot( data->hTreeWindow );
		item.mask = TVIF_HANDLE | TVIF_PARAM;


		/*---- �ǂݎ���Ԃ͌J��Ԃ� ----*/
		while( TreeView_GetItem( data->hTreeWindow, &item ) )
		{
			RegTreeItemData* pItem = (RegTreeItemData*)item.lParam;
			if( wcscmp( pItem->path, filePath ) == 0 )
			{
				TreeView_DeleteItem( data->hTreeWindow, item.hItem );
				ret = true;
				break;
			}
			item.hItem = TreeView_GetNextSibling( data->hTreeWindow, item.hItem );
		}
	}

	return ret;
}


//----------------------------------------------
// �Ǘ��t�@�C���p�X�̍폜
//
void TreeDeleteAll( HWND hTreeWnd )
{
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		TreeView_DeleteAllItems( data->hTreeWindow );
	}
}


//----------------------------------------------
// �w��p�X�̃f�B���N�g�����J��
//
static bool openTreeItem( HWND hTreeWnd, HTREEITEM hItem, LPCWSTR dirPath, bool subdir, DWORD extension )
{
	bool ret = false;

	if( hItem )
	{
		/*---- �A�C�e������ ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = hItem;
		item.mask = TVIF_HANDLE | TVIF_PARAM;
		while( TreeView_GetItem( hTreeWnd, &item ) )
		{
			RegTreeItemData* pItem = (RegTreeItemData*)item.lParam;

			// �w��f�B���N�g������v������J��
			if( !pItem->isfile && wcsncmp( pItem->path, dirPath, wcslen( pItem->path ) ) == 0 )
			{
				if( wcscmp( pItem->path, dirPath ) == 0 )
				{
					TreeView_Expand( hTreeWnd, item.hItem, TVE_EXPAND );
					if( subdir )
					{
						openTreeAllItem( hTreeWnd, item.hItem, extension );
					}
					ret = true;
					break;
				}
				else
				{
					if( openTreeItem( hTreeWnd, TreeView_GetChild( hTreeWnd, item.hItem ), dirPath, subdir, extension ) )
					{
						ret = true;
						break;
					}
				}
			}
			item.hItem = TreeView_GetNextSibling( hTreeWnd, item.hItem );
		}
	}

	return ret;
}
bool TreeOpenDirectory( HWND hTreeWnd, LPCWSTR dirPath, bool subdir )
{
	bool ret = false;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		ret = openTreeItem( data->hTreeWindow, TreeView_GetRoot( data->hTreeWindow ), dirPath, subdir, data->extension );
	}

	return ret;
}


//----------------------------------------------
// �w��A�C�e���ȉ��̃f�B���N�g�������ׂĊJ��
//
static bool openTreeAllItem( HWND hTreeWnd, HTREEITEM hItem, DWORD extension )
{
	bool ret = false;


	/*---- �����̔z�������ׂč폜���Ă��� ----*/
	HTREEITEM hChild = TreeView_GetChild( hTreeWnd, hItem );
	while( hChild )
	{
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = hChild;
		item.mask = TVIF_HANDLE | TVIF_PARAM;
		TreeView_GetItem( hTreeWnd, &item );
		if( item.lParam )
		{
			RegTreeItemData* itemData = (RegTreeItemData*)item.lParam;
			if( !itemData->isfile )
			{
				openTreeItem( hTreeWnd, item.hItem, itemData->path, true, extension );
			}
		}
		hChild = TreeView_GetNextSibling( hTreeWnd, hChild );
	}


	return ret;
}


//----------------------------------------------
// ���[�g�A�C�e���ꗗ�擾
//
DWORD GetTreeRootItem( HWND hTreeWnd, LPWSTR* paths, DWORD count )
{
	DWORD ret = 0;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		/*---- ���[�g�A�C�e���擾 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = TreeView_GetRoot( data->hTreeWindow );
		item.mask = TVIF_HANDLE | TVIF_PARAM;


		/*---- �ǂݎ���Ԃ͌J��Ԃ� ----*/
		while( TreeView_GetItem( data->hTreeWindow, &item ) )
		{
			RegTreeItemData* pItem = (RegTreeItemData*)item.lParam;
			if( ret < count )
			{
				paths[ ret ] = pItem->path;
			}
			ret++;

			item.hItem = TreeView_GetNextSibling( data->hTreeWindow, item.hItem );
		}
	}

	return ret;
}


//----------------------------------------------
// �\������Ă���A�C�e���̈ꗗ�擾
//
static void getTreeItem( HWND hTreeWnd, HTREEITEM root, TreeItemList& list )
{
	if( root != NULL )
	{
		/*---- �A�C�e���擾 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = root;
		item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
		item.stateMask = TVIS_EXPANDED;

		/*---- �ǂݎ���Ԃ͌J��Ԃ� ----*/
		while( TreeView_GetItem( hTreeWnd, &item ) )
		{
			if( item.state & TVIS_EXPANDED )
			{
				RegTreeItemData* pItem = (RegTreeItemData*)item.lParam;
				list.push_back( pItem->path );
				getTreeItem( hTreeWnd, TreeView_GetChild( hTreeWnd, item.hItem ), list );
			}
			item.hItem = TreeView_GetNextSibling( hTreeWnd, item.hItem );
		}
	}
}
DWORD GetTreeOpenItem( HWND hTreeWnd, LPWSTR* paths, DWORD count )
{
	DWORD ret = 0;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		TreeItemList list;
		getTreeItem( data->hTreeWindow, TreeView_GetRoot( data->hTreeWindow ), list );

		if( paths )
		{
			for( DWORD i = 0; i < count; i++ )
			{
				paths[ i ] = list[ i ];
			}
		}

		ret = (DWORD)list.size();
	}

	return ret;
}


//----------------------------------------------
// �g���q�t���O�ݒ�
//
void SetEnableTreeItemExtension( HWND hTreeWnd, DWORD enableExtension )
{
	RegTreeWindowData* data = getTreeWindowData( hTreeWnd );
	if( data )
	{
		data->extension = enableExtension;
	}
}
DWORD GetEnableTreeItemExtension( HWND hTreeWnd )
{
	RegTreeWindowData* data = getTreeWindowData( hTreeWnd );
	if( !data ) return 0;

	return data->extension;
}


//----------------------------------------------
// �E�B���h�E�֘A�f�[�^���擾
//
static RegTreeWindowData* getTreeWindowData( HWND hTreeWnd )
{
	RegTreeWindowData* ret = NULL;

	if( hTreeWnd )
	{
		ret = (RegTreeWindowData*)GetWindowLongPtrW( hTreeWnd, 0 );
	}

	return ret;
}


//----------------------------------------------
// �ݒ�ɉ����ă��b�Z�[�W�𑗐M����
//
static LRESULT sendTreeMessage( HWND hTreeWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND hWnd = NULL;
	
	RegTreeWindowData* data = getTreeWindowData( hTreeWnd );
	if( !data->sendMessageRoot ) hWnd = GetParent( hTreeWnd );
	else						 hWnd = GetAncestor( hTreeWnd, GA_ROOT );

	return SendMessageW( hWnd, msg, wParam, lParam );
}


//----------------------------------------------
// �����؂��\�z����
//
static HTREEITEM insertFileTree( HWND hTreeWnd, HTREEITEM hRoot, LPCWSTR filePath, bool subDirectory, DWORD enableExtension )
{
	HTREEITEM ret = NULL;
	if( checkInsertablePath( filePath, enableExtension ) )
	{
		wchar_t path[ MAX_PATH ];
		wcscpy_s( path, filePath );


		/*---- �}����񏉊��� ----*/
		TVINSERTSTRUCTW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hParent = hRoot;
		item.hInsertAfter = TVI_LAST;
		item.item.mask = TVIF_TEXT | TVIF_PARAM;


		/*---- �p�X���������ꍇ�͒ǉ��f�[�^�𐶐����� ----*/
		RegTreeItemData* itParam = new RegTreeItemData();
		wcscpy_s( itParam->path, path );
		itParam->isfile = !PathIsDirectoryW( filePath );
		itParam->openSubDirectory = !itParam->isfile ? subDirectory : false;


		/*---- �t�@�C�����֒u������ ----*/
		PathStripPathW( path );
		if( !itParam->isfile )
		{
			size_t len = wcslen( path );
			if( path[ len ] == L'\\' || path[ len ] == L'/' )
			{
				path[ len ] = L'\0';
			}
			item.item.mask |= TVIF_CHILDREN;
			item.item.cChildren = 1;
		}


		/*---- �p�X���c���[�̃��[�g�֒ǉ����� ----*/
		item.item.pszText = path;
		item.item.lParam = (LPARAM)itParam;
		if( (ret = TreeView_InsertItem( hTreeWnd, &item )) == NULL )
		{
			MessageBox( hTreeWnd, path, L"�I�[�v�����s", MB_OK );
		}
	}

	return ret;
}


//----------------------------------------------
// �c���[��̃A�C�e�����I�����ꂽ
//
static void selecteTreeItem( HWND hTreeWnd, HTREEITEM hSelItem )
{
	/*---- �I���A�C�e���擾 ----*/
	TVITEMW item;
	ZeroMemory( &item, sizeof( item ) );
	item.hItem = hSelItem;
	item.mask = TVIF_PARAM;
	TreeView_GetItem( hTreeWnd, &item );

	RegTreeItemData* pItem = (RegTreeItemData*)item.lParam;
	if( pItem )
	{
		TreeSelectItem selItem;
		ZeroMemory( &selItem, sizeof( selItem ) );
		selItem.filePath = pItem->path;

		sendTreeMessage( GetParent( hTreeWnd ), WM_TREE_SELECTITEM, !pItem->isfile, (LPARAM)&selItem );
	}
}


//----------------------------------------------
// �c���[��̃A�C�e���̊J�����s
//
static void expandTreeItem( HWND hTreeWnd, HTREEITEM hSelItem, DWORD extension, bool forceOpen )
{
	TVITEMW item;
	ZeroMemory( &item, sizeof( item ) );
	item.hItem = hSelItem;
	item.mask = TVIF_HANDLE | TVIF_PARAM;
	if( TreeView_GetItem( hTreeWnd, &item ) )
	{
		RegTreeItemData* data = (RegTreeItemData*)item.lParam;
		
		::OutputDebugStringW( data->path );
		::OutputDebugStringW( L"\n");

		if( data && !data->isfile )
		{
			/*---- �����p�f�[�^ ----*/
			wstring_array dirs, files;
			wchar_t seachPath[ MAX_PATH ];
			swprintf_s( seachPath, L"%s\\*.*", data->path );


			/*---- �f�B���N�g�������� ----*/
			WIN32_FIND_DATAW findData;
			HANDLE hFind = FindFirstFileW( seachPath, &findData );
			if( hFind )
			{
				do
				{
					/*---- �}���`�F�b�N ----*/
					if( wcscmp( findData.cFileName, L"." ) && wcscmp( findData.cFileName, L".." ) )
					{
						std::wstring full = data->path;
						full += L'\\';
						full += findData.cFileName;
						if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
						{// �f�B���N�g��
							dirs.push_back( full );
						}
						else
						{// �t�@�C��
							files.push_back( full );
						}
					}
				} while( FindNextFileW( hFind, &findData ) );
				FindClose( hFind );
			}


			/*---- �f�[�^��ǉ����Ă��� ----*/
			if( data->openSubDirectory || forceOpen )
			{
				wstring_array::iterator it = dirs.begin();
				for( ; it != dirs.end(); it++ )
				{
					insertFileTree( hTreeWnd, hSelItem, it->c_str(), data->openSubDirectory || forceOpen, extension );
				}
			}
			wstring_array::iterator it = files.begin();
			for( ; it != files.end(); it++ )
			{
				insertFileTree( hTreeWnd, hSelItem, it->c_str(), data->openSubDirectory || forceOpen, extension );
			}
		}
	}
}


//----------------------------------------------
// �}���\�ȃp�X������
//
static bool checkInsertablePath( LPCWSTR path, DWORD extFlag )
{
	/*-- �f�B���N�g���Ȃ��ɑ}���\ --*/
	if( PathIsDirectoryW( path ) ) return true;

	/*-- �g���q���� --*/
	wchar_t* ext = PathFindExtensionW( path );
	if( (extFlag & TITEM_EXT_TXT) && _wcsicmp( ext, L".txt" ) == 0 ) return true; 
	if( (extFlag & TITEM_EXT_MLT) && _wcsicmp( ext, L".mlt" ) == 0 ) return true; 
	if( (extFlag & TITEM_EXT_AST) && _wcsicmp( ext, L".ast" ) == 0 ) return true; 
	if( (extFlag & TITEM_EXT_ASD) && _wcsicmp( ext, L".asd" ) == 0 ) return true; 

	return false;
}


//----------------------------------------------
// WM_CREATE�̏���
//
static LRESULT treeCreateMessageCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct )
{
	/*---- �Ǘ��f�[�^�o�^ ----*/
	LPCREATESTRUCT cs = pCreateStruct;
	RegTreeWindowData* data = (RegTreeWindowData*)cs->lpCreateParams;
	if( data == NULL ) return -1;
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	/*---- �c���[�E�B���h�E���� ----*/
	DWORD treeStyle = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	data->hTreeWindow =	CreateWindowExW( 0, WC_TREEVIEW, L"", treeStyle, 0, 0, cs->cx, cs->cy, hWnd, NULL, cs->hInstance, NULL );


	/*---- D&D��t ----*/
	DragAcceptFiles( hWnd, TRUE );

	
	return 0;
}


//----------------------------------------------
// WM_DESTROY�̏���
//
static LRESULT treeDestroyMessageCrack( HWND hWnd )
{
	RegTreeWindowData* data = getTreeWindowData( hWnd );
	if( data )
	{
		SetWindowLongPtrW( hWnd, 0, 0 );
		delete data;
	}

	return 0;
}

//----------------------------------------------
// WM_SIZE�̏���
//
static LRESULT treeSizeMessageCrack( HWND hWnd, int width, int height )
{
	RegTreeWindowData* data = getTreeWindowData( hWnd );
	if( data )
	{
		MoveWindow( data->hTreeWindow, 0, 0, width, height, TRUE );
	}

	return 0;
}

//----------------------------------------------
// WM_DROPFILES�̏���
//
static LRESULT treeDragAndDropCrack( HWND hWnd, HDROP hDrop )
{
	UINT count = DragQueryFileW( hDrop, -1, NULL, 0 );
	if( count > 0 )
	{
		for( UINT i = 0; i < count; i++ )
		{
			wchar_t path[ MAX_PATH ];
			DragQueryFileW( hDrop, i, path, MAX_PATH );

			TreeAddFilePath( hWnd, path, true );
		}
	}
	DragFinish( hDrop );

	return 0;
}


//----------------------------------------------
// �f�B���N�g���R���e�L�X�g���j���[�\��
//
static void treeDirectoryMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt )
{
	RegTreeWindowData* data = NULL;

	if( hWnd && (data = getTreeWindowData( hWnd )) != NULL )
	{
		/*---- �A�C�e���̏�Ԃ��擾���Ă��� ----*/
		TVITEM tvItem = { 0 };
		tvItem.mask = TVIF_HANDLE | TVIF_STATE;
		tvItem.hItem = hItem;
		tvItem.stateMask = TVIS_EXPANDED;
		TreeView_GetItem( data->hTreeWindow, &tvItem );


		/*---- ���j���[���������� ----*/
		HMENU hBaseMenu = LoadMenuW( NULL, MAKEINTRESOURCEW( IDR_FILETREEDIR_MENU ) );
		HMENU hPopup = GetSubMenu( hBaseMenu, 0 );

		// �J�����j���[�̏�Ԑݒ�
		if( tvItem.state & TVIS_EXPANDED )
		{
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_TYPE;

				wchar_t str[ ] = L"�t�H���_�����";
				mItemInfo.fType = MFT_STRING;
				mItemInfo.dwTypeData = str;
				mItemInfo.cch = (UINT)wcslen( str );
				SetMenuItemInfoW( hPopup, IDM_TREE_OPENTREE, 0, &mItemInfo );
			}
		}

		// ���[�g�폜���j���[�̒ǉ�����
		if( TreeView_GetParent( data->hTreeWindow, hItem ) == NULL )
		{
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_TYPE | MIIM_ID;
				mItemInfo.wID = IDM_TREE_DELETEROOT;

				wchar_t str[] = L"�f�B���N�g�����c���[����폜����";
				mItemInfo.fType = MFT_STRING;
				mItemInfo.dwTypeData = str;
				mItemInfo.cch = (UINT)wcslen( str );
				::InsertMenuItemW( hPopup, 0, TRUE, &mItemInfo );
			}
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_FTYPE;
				mItemInfo.fType = MFT_SEPARATOR;
				::InsertMenuItemW( hPopup, 1, TRUE, &mItemInfo );
			}
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_STATE;
				mItemInfo.fState= MFS_DISABLED;
				::SetMenuItemInfoW( hPopup, IDM_TREE_ROOTITEM, 0, &mItemInfo );
			}
		}

		UINT id = TrackPopupMenu( hPopup, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL );
		DestroyMenu( hBaseMenu );

		// ���j���[�A�N�V����
		switch( id )
		{
		case IDM_TREE_OPENTREE:
			TreeView_Expand( data->hTreeWindow, hItem, TVE_TOGGLE );
			break;
		case IDM_TREE_OPENALLDIRECTORY_TREE:
			openTreeAllItem( data->hTreeWindow, hItem, data->extension );
			break;
		case IDM_TREE_DELETEROOT:
			TreeDeleteFilePath( hWnd, item->path );
			break;
		case IDM_TREE_OPEN_DIRECTORY:
			::_wspawnlp( _P_NOWAIT, L"explorer", L"explorer", item->path, NULL );
			break;

		case IDM_TREE_ROOTITEM:
			TreeAddFilePath( hWnd, item->path, item->openSubDirectory, true );
			break;
		}
	}
}

//----------------------------------------------
// �t�@�C���R���e�L�X�g���j���[�\��
//
static void treeFileMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt )
{
	RegTreeWindowData* data = NULL;

	if( hWnd && (data = getTreeWindowData( hWnd )) != NULL )
	{
		HMENU hBaseMenu = LoadMenuW( NULL, MAKEINTRESOURCEW( IDR_FILETREEFILE_MENU ) );
		HMENU hPopup = GetSubMenu( hBaseMenu, 0 );

		// ���[�g�폜���j���[�̒ǉ�����
		if( TreeView_GetParent( data->hTreeWindow, hItem ) == NULL )
		{
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_TYPE | MIIM_ID;
				mItemInfo.wID = IDM_TREE_DELETEROOT;

				wchar_t str[] = L"�t�@�C�����c���[����폜����";
				mItemInfo.fType = MFT_STRING;
				mItemInfo.dwTypeData = str;
				mItemInfo.cch = (UINT)wcslen( str );
				::InsertMenuItemW( hPopup, 0, TRUE, &mItemInfo );
			}
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_FTYPE;
				mItemInfo.fType = MFT_SEPARATOR;
				::InsertMenuItemW( hPopup, 1, TRUE, &mItemInfo );
			}
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_STATE;
				mItemInfo.fState= MFS_DISABLED;
				::SetMenuItemInfoW( hPopup, IDM_TREE_ROOTITEM, 0, &mItemInfo );
			}
		}

		UINT id = TrackPopupMenu( hPopup, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL );
		DestroyMenu( hBaseMenu );

		// ���j���[�A�N�V����
		switch( id )
		{
		case IDM_TREE_OPENFILE:
			selecteTreeItem( data->hTreeWindow, hItem );
			break;
		case IDM_TREE_OPEN_DIRECTORY:
			{
				wchar_t path[ MAX_PATH ];
				wcscpy_s( path, item->path );
				::PathRemoveFileSpecW( path );
				::_wspawnlp( _P_NOWAIT, L"explorer", L"explorer", path, NULL );
			}
			break;
		case IDM_TREE_DELETEROOT:
			TreeDeleteFilePath( hWnd, item->path );
			break;

		case IDM_TREE_ROOTITEM:
			TreeAddFilePath( hWnd, item->path, item->openSubDirectory, true );
			break;
		}
	}
}
