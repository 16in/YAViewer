/*----------------------------------------------------------------------------------------
*
* YAViewer Tree関係
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
// ツリービュー管理情報
//
struct RegTreeWindowData
{
	HWND		hTreeWindow;			// ツリービュー本体

	bool		sendMessageRoot;		// ルートにメッセージを送るか
	DWORD		extension;				// 有効な拡張子

	RegTreeWindowData( void )
		: hTreeWindow( NULL ), sendMessageRoot( false ), extension( TITEM_EXT_ALL )
	{
	}
};


//----------------------------------------------
// ツリーアイテム管理情報
//
struct RegTreeItemData
{
	wchar_t			path[ MAX_PATH ];	// ファイルパス
	bool			isfile;				// ファイルであるか
	bool			openSubDirectory;	// サブディレクトリも含めるか

	RegTreeItemData( void )
		: isfile( false ), openSubDirectory( false )
	{
		ZeroMemory( path, sizeof( path ) );
	}
};


//----------------------------------------------
// 型定義
//
typedef std::vector<std::wstring>	wstring_array;	// 文字列配列
typedef std::vector<LPWSTR>			TreeItemList;	// リストアイテム


//----------------------------------------------
// グローバル関数宣言
//
static RegTreeWindowData* getTreeWindowData( HWND hTreeWnd );
static LRESULT sendTreeMessage( HWND hTreeWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static HTREEITEM insertFileTree( HWND hTreeWnd, HTREEITEM hRoot, LPCWSTR filePath, bool subDirectory, DWORD enableExtension );
static void selecteTreeItem( HWND hTreeWnd, HTREEITEM hSelItem );
static void expandTreeItem( HWND hTreeWnd, HTREEITEM hSelItem, DWORD extension, bool forceOpen = false );
static bool checkInsertablePath( LPCWSTR path, DWORD extFlag );
static bool openTreeItem( HWND hTreeWnd, HTREEITEM hItem, LPCWSTR dirPAth, bool subdir, DWORD extension );
static bool openTreeAllItem( HWND hTreeWnd, HTREEITEM hItem, DWORD extension );

// メッセージクラッカ
static LRESULT treeCreateMessageCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct );
static LRESULT treeDestroyMessageCrack( HWND hWnd );
static LRESULT treeSizeMessageCrack( HWND hWnd, int width, int height );
static LRESULT treeDragAndDropCrack( HWND hWnd, HDROP hDrop );

// コンテキストメニュー表示
static void treeDirectoryMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt );
static void treeFileMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt );


//----------------------------------------------
// ウィンドウクラス登録
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
// ビューウィンドウ生成
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
// ビューウィンドウ標準コールバック
//
LRESULT CALLBACK TreeWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	default:		 return ::DefWindowProcW( hWnd, msg, wParam, lParam );
	case WM_CREATE:	 return treeCreateMessageCrack( hWnd, (LPCREATESTRUCT)lParam );
	case WM_DESTROY: return treeDestroyMessageCrack( hWnd );
	case WM_SIZE:	 return treeSizeMessageCrack( hWnd, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );

	// D&Dを処理
	case WM_DROPFILES: return treeDragAndDropCrack( hWnd, (HDROP)wParam );

	// ツリービューからのメッセージを処理
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
				/*---- コンテキストメニュー呼び出し ----*/
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
// 拡張メッセージ送信先設定
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
// 管理ファイルパスの追加
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
// 管理ファイルパスの削除
//
bool TreeDeleteFilePath( HWND hTreeWnd, LPCWSTR filePath )
{
	bool ret = false;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		/*---- ルートアイテム取得 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = TreeView_GetRoot( data->hTreeWindow );
		item.mask = TVIF_HANDLE | TVIF_PARAM;


		/*---- 読み取れる間は繰り返す ----*/
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
// 管理ファイルパスの削除
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
// 指定パスのディレクトリを開く
//
static bool openTreeItem( HWND hTreeWnd, HTREEITEM hItem, LPCWSTR dirPath, bool subdir, DWORD extension )
{
	bool ret = false;

	if( hItem )
	{
		/*---- アイテム判定 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = hItem;
		item.mask = TVIF_HANDLE | TVIF_PARAM;
		while( TreeView_GetItem( hTreeWnd, &item ) )
		{
			RegTreeItemData* pItem = (RegTreeItemData*)item.lParam;

			// 指定ディレクトリが一致したら開く
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
// 指定アイテム以下のディレクトリをすべて開く
//
static bool openTreeAllItem( HWND hTreeWnd, HTREEITEM hItem, DWORD extension )
{
	bool ret = false;


	/*---- 自分の配下をすべて削除しておく ----*/
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
// ルートアイテム一覧取得
//
DWORD GetTreeRootItem( HWND hTreeWnd, LPWSTR* paths, DWORD count )
{
	DWORD ret = 0;
	RegTreeWindowData* data = NULL;

	if( hTreeWnd && (data = getTreeWindowData( hTreeWnd )) != NULL )
	{
		/*---- ルートアイテム取得 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = TreeView_GetRoot( data->hTreeWindow );
		item.mask = TVIF_HANDLE | TVIF_PARAM;


		/*---- 読み取れる間は繰り返す ----*/
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
// 表示されているアイテムの一覧取得
//
static void getTreeItem( HWND hTreeWnd, HTREEITEM root, TreeItemList& list )
{
	if( root != NULL )
	{
		/*---- アイテム取得 ----*/
		TVITEMW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hItem = root;
		item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
		item.stateMask = TVIS_EXPANDED;

		/*---- 読み取れる間は繰り返す ----*/
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
// 拡張子フラグ設定
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
// ウィンドウ関連データを取得
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
// 設定に応じてメッセージを送信する
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
// 部分木を構築する
//
static HTREEITEM insertFileTree( HWND hTreeWnd, HTREEITEM hRoot, LPCWSTR filePath, bool subDirectory, DWORD enableExtension )
{
	HTREEITEM ret = NULL;
	if( checkInsertablePath( filePath, enableExtension ) )
	{
		wchar_t path[ MAX_PATH ];
		wcscpy_s( path, filePath );


		/*---- 挿入情報初期化 ----*/
		TVINSERTSTRUCTW item;
		ZeroMemory( &item, sizeof( item ) );
		item.hParent = hRoot;
		item.hInsertAfter = TVI_LAST;
		item.item.mask = TVIF_TEXT | TVIF_PARAM;


		/*---- パスが正しい場合は追加データを生成する ----*/
		RegTreeItemData* itParam = new RegTreeItemData();
		wcscpy_s( itParam->path, path );
		itParam->isfile = !PathIsDirectoryW( filePath );
		itParam->openSubDirectory = !itParam->isfile ? subDirectory : false;


		/*---- ファイル名へ置き換え ----*/
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


		/*---- パスをツリーのルートへ追加する ----*/
		item.item.pszText = path;
		item.item.lParam = (LPARAM)itParam;
		if( (ret = TreeView_InsertItem( hTreeWnd, &item )) == NULL )
		{
			MessageBox( hTreeWnd, path, L"オープン失敗", MB_OK );
		}
	}

	return ret;
}


//----------------------------------------------
// ツリー上のアイテムが選択された
//
static void selecteTreeItem( HWND hTreeWnd, HTREEITEM hSelItem )
{
	/*---- 選択アイテム取得 ----*/
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
// ツリー上のアイテムの開閉を実行
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
			/*---- 走査用データ ----*/
			wstring_array dirs, files;
			wchar_t seachPath[ MAX_PATH ];
			swprintf_s( seachPath, L"%s\\*.*", data->path );


			/*---- ディレクトリ内走査 ----*/
			WIN32_FIND_DATAW findData;
			HANDLE hFind = FindFirstFileW( seachPath, &findData );
			if( hFind )
			{
				do
				{
					/*---- 挿入チェック ----*/
					if( wcscmp( findData.cFileName, L"." ) && wcscmp( findData.cFileName, L".." ) )
					{
						std::wstring full = data->path;
						full += L'\\';
						full += findData.cFileName;
						if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
						{// ディレクトリ
							dirs.push_back( full );
						}
						else
						{// ファイル
							files.push_back( full );
						}
					}
				} while( FindNextFileW( hFind, &findData ) );
				FindClose( hFind );
			}


			/*---- データを追加していく ----*/
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
// 挿入可能なパスか判定
//
static bool checkInsertablePath( LPCWSTR path, DWORD extFlag )
{
	/*-- ディレクトリなら常に挿入可能 --*/
	if( PathIsDirectoryW( path ) ) return true;

	/*-- 拡張子判定 --*/
	wchar_t* ext = PathFindExtensionW( path );
	if( (extFlag & TITEM_EXT_TXT) && _wcsicmp( ext, L".txt" ) == 0 ) return true; 
	if( (extFlag & TITEM_EXT_MLT) && _wcsicmp( ext, L".mlt" ) == 0 ) return true; 
	if( (extFlag & TITEM_EXT_AST) && _wcsicmp( ext, L".ast" ) == 0 ) return true; 
	if( (extFlag & TITEM_EXT_ASD) && _wcsicmp( ext, L".asd" ) == 0 ) return true; 

	return false;
}


//----------------------------------------------
// WM_CREATEの処理
//
static LRESULT treeCreateMessageCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct )
{
	/*---- 管理データ登録 ----*/
	LPCREATESTRUCT cs = pCreateStruct;
	RegTreeWindowData* data = (RegTreeWindowData*)cs->lpCreateParams;
	if( data == NULL ) return -1;
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)data );


	/*---- ツリーウィンドウ生成 ----*/
	DWORD treeStyle = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	data->hTreeWindow =	CreateWindowExW( 0, WC_TREEVIEW, L"", treeStyle, 0, 0, cs->cx, cs->cy, hWnd, NULL, cs->hInstance, NULL );


	/*---- D&D受付 ----*/
	DragAcceptFiles( hWnd, TRUE );

	
	return 0;
}


//----------------------------------------------
// WM_DESTROYの処理
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
// WM_SIZEの処理
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
// WM_DROPFILESの処理
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
// ディレクトリコンテキストメニュー表示
//
static void treeDirectoryMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt )
{
	RegTreeWindowData* data = NULL;

	if( hWnd && (data = getTreeWindowData( hWnd )) != NULL )
	{
		/*---- アイテムの状態を取得しておく ----*/
		TVITEM tvItem = { 0 };
		tvItem.mask = TVIF_HANDLE | TVIF_STATE;
		tvItem.hItem = hItem;
		tvItem.stateMask = TVIS_EXPANDED;
		TreeView_GetItem( data->hTreeWindow, &tvItem );


		/*---- メニューを処理する ----*/
		HMENU hBaseMenu = LoadMenuW( NULL, MAKEINTRESOURCEW( IDR_FILETREEDIR_MENU ) );
		HMENU hPopup = GetSubMenu( hBaseMenu, 0 );

		// 開くメニューの状態設定
		if( tvItem.state & TVIS_EXPANDED )
		{
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_TYPE;

				wchar_t str[ ] = L"フォルダを閉じる";
				mItemInfo.fType = MFT_STRING;
				mItemInfo.dwTypeData = str;
				mItemInfo.cch = (UINT)wcslen( str );
				SetMenuItemInfoW( hPopup, IDM_TREE_OPENTREE, 0, &mItemInfo );
			}
		}

		// ルート削除メニューの追加判定
		if( TreeView_GetParent( data->hTreeWindow, hItem ) == NULL )
		{
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_TYPE | MIIM_ID;
				mItemInfo.wID = IDM_TREE_DELETEROOT;

				wchar_t str[] = L"ディレクトリをツリーから削除する";
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

		// メニューアクション
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
// ファイルコンテキストメニュー表示
//
static void treeFileMenu( HWND hWnd, RegTreeItemData* item, HTREEITEM hItem, POINT pt )
{
	RegTreeWindowData* data = NULL;

	if( hWnd && (data = getTreeWindowData( hWnd )) != NULL )
	{
		HMENU hBaseMenu = LoadMenuW( NULL, MAKEINTRESOURCEW( IDR_FILETREEFILE_MENU ) );
		HMENU hPopup = GetSubMenu( hBaseMenu, 0 );

		// ルート削除メニューの追加判定
		if( TreeView_GetParent( data->hTreeWindow, hItem ) == NULL )
		{
			{
				MENUITEMINFOW mItemInfo = { 0 };
				mItemInfo.cbSize = sizeof( mItemInfo );
				mItemInfo.fMask = MIIM_TYPE | MIIM_ID;
				mItemInfo.wID = IDM_TREE_DELETEROOT;

				wchar_t str[] = L"ファイルをツリーから削除する";
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

		// メニューアクション
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
