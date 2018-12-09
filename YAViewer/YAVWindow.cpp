/*----------------------------------------------------------------------------------------
*
* YAViewer Window関係
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
#define YAV_VERSION			L"YAViewer ver0.01 β"


//----------------------------------------------
// ファイルスコープグローバル関数宣言
//
// アプリケーションウィンドウ メッセージクラック
static LRESULT appWindowCreateCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct );
static LRESULT appWindowCommandCrack( HWND hWnd, WORD id, WORD notifyCode, HWND hControl );
static LRESULT appWindowSizeCrack( HWND hWnd, int type, WORD width, WORD height );
static LRESULT appWindowMoveCrack( HWND hWnd, WORD x, WORD y );
static LRESULT appWindowDestroyCrack( HWND hWnd );
static LRESULT appWindowNotifyCrack( HWND hWnd, UINT id, NMHDR* pNotifyHead );
static LRESULT appWindowContextMenu( HWND hWnd, HWND hCtrlWnd, int x, int y );

// バージョンダイアログ表示
static void appShowVersionDialog( HWND hWnd );


//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterYAVWindowClass( HINSTANCE hInstance )
{
	/*---- 使用するウィンドウクラスを登録 ----*/
	if( !RegisterViewWindowClass( hInstance ) )		return false;
	if( !RegisterTreeWindowClass( hInstance ) )		return false;
	if( !RegisterSplitWindowClass( hInstance ) )	return false;
	if( !RegisterSelectorWindowClass( hInstance ) )	return false;
	if( !RegisterYAVFileTabClass( hInstance ) )		return false;
	if( !RegisterYAVShortcutClass( hInstance ) )	return false;


	/*---- アプリケーションウィンドウクラス登録 ----*/
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
// ウィンドウ生成
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
// アプリケーションウィンドウ標準コールバック
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
	// 拡張ウィンドウメッセージ
	//
	case YAV_WM_ADDFILE:// ファイルをタブに追加する
		if( YavAddFile( GetYAVAppData( hWnd ), ((YavWmAddFile*)lParam)->filePath, (wParam != FALSE) ) )
		{
			// 描画領域を更新
			RECT rect; GetClientRect( hWnd, &rect );
			appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			return TRUE;
		}
		break;

	case YAV_WM_DELFILE:// ファイルをタブから削除
		if( YavDelFile( GetYAVAppData( hWnd ), (int)wParam ) )
		{
			// 描画領域を更新
			RECT rect; GetClientRect( hWnd, &rect );
			appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			return TRUE;
		}
		break;

	case YAV_WM_SELECTITEM:// アイテム選択処理
		{
			ViewSelectItem* item = (ViewSelectItem*)lParam;
			DWORD target = SST_CLIPBOARD;
			switch( wParam )
			{
			case 0x0000:// 左クリック
				target = GetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetLeftClick );
				break;
			case 0x0002:// 中クリック
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

	case YAV_WM_TREE_SELECTITEM:// ツリーアイテム選択
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

	case YAV_WM_SAVEFILETAB:// ファイルタブリスト保存
		{
			/*---- ファイル一覧を生成 ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			YavSaveFileTab( data, (LPCWSTR)lParam );
		}
		break;

	case YAV_WM_LOADFILETAB:// ファイルタブリスト復元
		{
			/*---- ファイル一覧を生成 ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			if( YavLoadFileTab( data, (LPCWSTR)lParam ) )
			{
				YavSelectFile( data, 0 );

				/*---- 描画領域更新 ----*/
				RECT rect; GetClientRect( hWnd, &rect );
				appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			}
		}
		break;

	case YAV_WM_SAVEFILETREE:// ファイルツリー保存
		{
			/*---- ファイル一覧を生成 ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			YavSaveFileTree( data, (LPCWSTR)lParam );
		}
		break;

	case YAV_WM_LOADFILETREE:// ファイルツリー復元
		{
			/*---- ファイル一覧を生成 ----*/
			RegYavAppData* data = GetYAVAppData( hWnd );
			if( YavLoadFileTree( data, (LPCWSTR)lParam ) )
			{
				/*---- 描画領域更新 ----*/
				RECT rect; GetClientRect( hWnd, &rect );
				appWindowSizeCrack( hWnd, 0, (WORD)(rect.right-rect.left), (WORD)(rect.bottom-rect.top) );
			}
		}
		break;

	case YAV_WM_PAGEJUMP:// ファイルの指定位置へジャンプ
		{
			RegYavAppData* data = GetYAVAppData( hWnd );
			SetViewScrollPositionByPage( data->hFileView, (int)lParam );
		}
		break;
	}

	return 0;
}


//----------------------------------------------
// アプリケーションウィンドウ登録データ取得
//
RegYavAppData* GetYAVAppData( HWND hWnd )
{
	if( !hWnd ) return NULL;
	return (RegYavAppData*)GetWindowLongPtrW( hWnd, 0 );
}


//----------------------------------------------
// アプリケーションウィンドウ
//	WM_CREATEの処理
//
static LRESULT appWindowCreateCrack( HWND hWnd, LPCREATESTRUCT pCreateStruct )
{
	/*---- 登録データ設定 ----*/
	LPCREATESTRUCT cs = pCreateStruct;
	RegYavAppData* appData = (RegYavAppData*)cs->lpCreateParams;
	SetWindowLongPtrW( hWnd, 0, (LONG_PTR)appData );


	/*---- フォントを生成 ----*/
	LOGFONTW logFont;
	ZeroMemory( &logFont, sizeof( logFont ) );
	wcscpy_s( logFont.lfFaceName, appData->fontName/*L"ＭＳ Ｐゴシック"*/ );
	logFont.lfHeight = -appData->fontSize;
	appData->hAAFont = ::CreateFontIndirectW( &logFont );


	/*---- タブウィンドウ生成 ----*/
	appData->hFileTab = CreateFileTabWindow( cs->hInstance, hWnd, 0, IDC_FILETAB );


	/*---- コントロールウィンドウ生成 ----*/
	appData->hCtrlWnd = CreateSplitWindow( cs->hInstance, GetFileTabWindow( appData->hFileTab ), 20 );
	SetSplitWindowLock( appData->hCtrlWnd, SPL_LOCK_LEFT );


	/*---- ショートカット/ツリーウィンドウ生成 ----*/
	appData->hTreeShort = CreateSplitWindow( cs->hInstance, appData->hCtrlWnd, 0 );
	if( appData->hTreeShort )
	{
		SetSplitWindowMode( appData->hTreeShort, SPL_HORIZONTAL );
		SetSplitWindowLock( appData->hTreeShort, SPL_LOCK_BOTTOM );
	}


	/*---- ファイルツリー生成 ----*/
	if( appData->hTreeShort )
	{
		appData->hFileTree = CreateTreeWindow( cs->hInstance, appData->hTreeShort, WS_BORDER );
	}
	else
	{
		appData->hFileTree = CreateTreeWindow( cs->hInstance, appData->hCtrlWnd, WS_BORDER );
	}


	/*---- ショートカットウィンドウ生成 ----*/
	if( appData->hTreeShort )
	{
		appData->hAAShortcut = CreateShortcutWindow( cs->hInstance, appData->hTreeShort, WS_BORDER );
		SetShortcutSendNotifyWindow( appData->hAAShortcut, hWnd );
	}


	/*---- ファイルビュー生成 ----*/
	appData->hFileView = CreateViewWindow( cs->hInstance, appData->hCtrlWnd, WS_BORDER );
	SetViewFont( appData->hFileView, appData->hAAFont );

	// アプリケーションウィンドウへメッセージを送る
	SetViewSendMessageRootWindow( appData->hFileView, true );
	SetTreeSendMessageRootWindow( appData->hFileTree, true );


	/*---- コントロールウィンドウにターゲット設定 ----*/
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


	/*---- 送信先選択ウィンドウ生成 ----*/
	appData->hSelector = CreateSelectorWindow( cs->hInstance, hWnd, 0 );


	/*---- ファイル表示モード設定 ----*/
	SetViewMode( appData->hFileView, appData->viewMode );


	/*---- ツールヒントの有効無効 ----*/
	if( !appData->toolHint )
	{// デフォがtrueのはずなので、合わせて変更する
		::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_VIEW_TOOLTIP, 0 ), 0 );
	}


	/*---- タブボタンモードの有効無効 ----*/
	if( appData->tabButtonMode )
	{// デフォがfalseのはずなので、合わせて変更する
		::SendMessageW( hWnd, WM_COMMAND, MAKEWPARAM( IDM_TAB_BUTTON, 0 ), 0 );
	}


	/*---- ダブルクリックモードの有効無効 ----*/
	if( appData->dblclickMode )
	{
		SetSelectDoubleClick( appData->hSelector, true );
	}


	/*---- 分割ウィンドウサイズ設定 ----*/
	if( appData->splitPos < 0 )
	{
		if( cs->cx < 0 )appData->splitPos = 300;
		else			appData->splitPos = cs->cx / 2;
	}
	SetSplitWidth( appData->hCtrlWnd, appData->splitPos );


	/*---- 拡張子選択 ----*/
	SetEnableTreeItemExtension( appData->hFileTree, appData->enableExtension );
	{
		// メニュー更新
		HMENU hMenu = GetMenu( hWnd );
		MENUITEMINFO info;
		info.cbSize = sizeof( info );
		info.fMask = MIIM_STATE;
		for( WORD i = 0; i < TITEM_EXT_MAX_SHIFT; i++ )
		{
			if( (appData->enableExtension & (1 << i)) )
			{// チェック ON
				info.fState = MFS_CHECKED;
			}
			else
			{// チェック OFF
				info.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfoW( hMenu, IDM_ENABLE_TXT + i, FALSE, &info );
		}
	}


	/*---- カテゴリ表示サイズ設定 ----*/
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


	/*---- ファイルタブ初期化 ----*/
	std::wstring initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += appData->initTabPath;
	if( ::PathFileExistsW( initPath.c_str() ) )
	{
		YavLoadFileTab( appData, initPath.c_str(), true );
		YavSelectFile( appData, 0 );
	}


	/*---- ファイルツリー初期化 ----*/
	initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += appData->initTreePath;
	if( ::PathFileExistsW( initPath.c_str() ) )
	{
		YavLoadFileTree( appData, initPath.c_str(), true );
	}


	/*---- 送信先を初期化 ----*/
	SetSelectSendSelector( appData->hSelector, SendTargetLeftClick, appData->sendTargetLeft );
	SetSelectSendSelector( appData->hSelector, SendTargetCenterClick, appData->sendTargetCenter );


	return 0;
}


//----------------------------------------------
// アプリケーションウィンドウ
//	WM_COMMANDの処理
//
static LRESULT appWindowCommandCrack( HWND hWnd, WORD id, WORD notifyCode, HWND hControl )
{
	switch( id )
	{
	case IDM_EXIT:// 終了メッセージ
		DestroyWindow( hWnd );
		break;
	case IDM_FILEOPEN:// ファイルを開く
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
	case IDM_TABSAVE:// タブを保存
		{
			wchar_t path[ MAX_PATH ];
			if( SaveFileTabDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_SAVEFILETAB, 0, (LPARAM)path );
			}
		}
		break;
	case IDM_TABOPEN:// タブを復元
		{
			wchar_t path[ MAX_PATH ];
			if( OpenFileTabDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_LOADFILETAB, 0, (LPARAM)path );
			}
		}
		break;
	case IDM_TREESAVE:// ツリーを保存
		{
			wchar_t path[ MAX_PATH ];
			if( SaveFileTreeDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_SAVEFILETREE, 0, (LPARAM)path );
			}
		}
		break;
	case IDM_TREEOPEN:// ツリーを復元
		{
			wchar_t path[ MAX_PATH ];
			if( OpenFileTreeDialog( hWnd, path, MAX_PATH ) )
			{
				SendMessageW( hWnd, YAV_WM_LOADFILETREE, 0, (LPARAM)path );
			}
		}
		break;

	case IDM_MOST_TOP:// 最前面表示
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

	case IDM_VIEW_TOOLTIP:// ツールチップ表示
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

	case IDM_TAB_BUTTON:// タブをボタン表示にする
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
	case IDM_NORMAL_VIEW:// ノーマル表示に切り替え
		SetViewMode( GetYAVAppData( hWnd )->hFileView, 0 );
		break;
	case IDM_CATEGORY_VIEW:// カテゴリ表示に切り替え
		SetViewMode( GetYAVAppData( hWnd )->hFileView, 1 );
		break;
	case IDM_EDITOR_VIEW:// エディタ表示に切り替え
		SetViewMode( GetYAVAppData( hWnd )->hFileView, 2 );
		break;

	// 送信先設定
	case IDM_SEND_FACEEDIT:
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetLeftClick, SST_FACE_EDIT );
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetCenterClick, SST_FACE_EDIT | SST_TEXTBOX_FLAG );
		break;
	case IDM_SEND_ORINEDIT:
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetLeftClick, SST_ORINRIN_EDITOR );
		SetSelectSendSelector( GetYAVAppData( hWnd )->hSelector, SendTargetCenterClick, SST_ORINRIN_EDITOR | SST_TEXTBOX_FLAG );
		break;

	// バージョン情報
	case IDM_VERSION:
		appShowVersionDialog( hWnd );
		break;

	// 分割ウィンドウ入れ替え
	case IDM_SPLIT_SWAP:
		SwapSplitTargetWindow( GetYAVAppData( hWnd )->hCtrlWnd );
		break;

	// カテゴリ表示サイズ変更
	case IDM_CATEGORY_32:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 32 );	break;
	case IDM_CATEGORY_64:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 64 );	break;
	case IDM_CATEGORY_96:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 96 );	break;
	case IDM_CATEGORY_128:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 128 );	break;
	case IDM_CATEGORY_160:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 160 );	break;
	case IDM_CATEGORY_192:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 192 );	break;
	case IDM_CATEGORY_224:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 224 );	break;
	case IDM_CATEGORY_256:	SetViewCategorySize( GetYAVAppData( hWnd )->hFileView, 256 );	break;

	// 拡張子選択処理
	case IDM_ENABLE_MLT: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_MLT ); break;
	case IDM_ENABLE_AST: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_AST ); break;
	case IDM_ENABLE_ASD: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_ASD ); break;
	case IDM_ENABLE_TXT: SetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree, GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree ) ^ TITEM_EXT_TXT ); break;
	}


	// カテゴリラジオマーク変更
	if( IDM_CATEGORY_32 <= id && id <= IDM_CATEGORY_256 )
	{
		HMENU hMenu = GetMenu( hWnd );
		MENUITEMINFO info;

		info.cbSize = sizeof( info );
		info.fMask = MIIM_STATE;

		for( WORD i = IDM_CATEGORY_32; i <= IDM_CATEGORY_256; i++ )
		{
			if( i == id )
			{// チェック ON
				info.fState = MFS_CHECKED;
			}
			else
			{// チェック OFF
				info.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfoW( hMenu, i, FALSE, &info );
		}
	}

	// 拡張子選択変更
	if( IDM_ENABLE_TXT <= id && id <= IDM_ENABLE_ASD )
	{
		// メニュー更新
		DWORD ext = GetEnableTreeItemExtension( GetYAVAppData( hWnd )->hFileTree );
		HMENU hMenu = GetMenu( hWnd );
		MENUITEMINFO info;
		info.cbSize = sizeof( info );
		info.fMask = MIIM_STATE;
		for( WORD i = 0; i < TITEM_EXT_MAX_SHIFT; i++ )
		{
			if( (ext & (1 << i)) )
			{// チェック ON
				info.fState = MFS_CHECKED;
			}
			else
			{// チェック OFF
				info.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfoW( hMenu, IDM_ENABLE_TXT + i, FALSE, &info );
		}

		// ツリービュー更新
		std::wstring initPath = YavGetAppDirectory();
		initPath += L'\\';
		initPath += GetYAVAppData( hWnd )->initTreePath;
		YavSaveFileTree( GetYAVAppData( hWnd ), initPath.c_str() );
		YavLoadFileTree( GetYAVAppData( hWnd ), initPath.c_str() );
	}


	return 0;
}


//----------------------------------------------
// アプリケーションウィンドウ
//	WM_SIZEの処理
//
static LRESULT appWindowSizeCrack( HWND hWnd, int type, WORD width, WORD height )
{
	/*---- アプリデータ取得 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- データコピー -----*/
	if( type == SIZE_RESTORED )
	{
		RECT winRect;
		GetWindowRect( hWnd, &winRect );
		data->width = winRect.right - winRect.left;
		data->height = winRect.bottom - winRect.top;
	}

	
	/*---- ファイルタブサイズ更新 ----*/
	int SelectorSize = 32;
	RECT tabRect;
	MoveWindow( data->hFileTab, 0, 0, width, height - SelectorSize, TRUE );
	GetClientRect( data->hFileTab, &tabRect );
	TabCtrl_AdjustRect( data->hFileTab, FALSE, &tabRect );
	

	/*---- コントロールウィンドウサイズ更新 ----*/
	MoveWindow( data->hCtrlWnd, tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top, TRUE );


	/*---- セレクタ更新 ----*/
	MoveWindow( data->hSelector, 0, height - SelectorSize, width, SelectorSize, TRUE );


	return 0;
}


//----------------------------------------------
// アプリケーションウィンドウ
//	WM_MOVEの処理
//
static LRESULT appWindowMoveCrack( HWND hWnd, WORD x, WORD y )
{
	/*---- アプリケーションデータ取得 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- ウィンドウ位置を取得 ----*/
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
// アプリケーションウィンドウ
//	WM_DESTROYの処理
//
static LRESULT appWindowDestroyCrack( HWND hWnd )
{
	/*---- アプリケーションデータ取得 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- 表示モード取得 ----*/
	data->viewMode = GetViewMode( data->hFileView );
	data->toolHint = GetViewToolTip( data->hFileView );
	data->tabButtonMode = GetFileTabButtonMode( data->hFileTab );


	/*---- カテゴリサイズ取得 ----*/
	data->categorySize = GetViewCategorySize( data->hFileView );


	/*---- 分割位置取得 ----*/
	data->splitPos = GetSplitWidth( data->hCtrlWnd );


	/*---- 分割状態取得 ----*/
	HWND right = NULL;
	GetSplitTargetWindow( data->hCtrlWnd, NULL, &right );
	data->reverseSplit = (right != data->hFileView);
	
	if( data->hTreeShort )
	{
		data->shortcutSize = GetSplitWidth( data->hTreeShort );
	}


	/*---- タブ保存 ----*/
	std::wstring initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += data->initTabPath;
	YavSaveFileTab( data, initPath.c_str(), true );
	
	
	/*---- ツリー保存 ----*/
	initPath = YavGetAppDirectory();
	initPath += L'\\';
	initPath += data->initTreePath;
	YavSaveFileTree( data, initPath.c_str(), true );


	/*---- ツリー拡張子選択状態保存 ----*/
	data->enableExtension = GetEnableTreeItemExtension( data->hFileTree );


	/*---- 送信先保存 ----*/
	data->sendTargetLeft	= GetSelectSendSelector( data->hSelector, SendTargetLeftClick );
	data->sendTargetCenter	= GetSelectSendSelector( data->hSelector, SendTargetCenterClick );
	data->dblclickMode		= GetSelectDoubleClick( data->hSelector ) ? 1 : 0;


	/*---- アプリケーションデータ廃棄 ----*/
	SetWindowLongPtrW( hWnd, 0, 0 );

	PostQuitMessage( 0 );

	return 0;
}


//----------------------------------------------
// アプリケーションウィンドウ
//	WM_NOTIFYの処理
//
static LRESULT appWindowNotifyCrack( HWND hWnd, UINT id, NMHDR* pNotifyHead )
{
	/*---- アプリケーションデータ取得 ----*/
	RegYavAppData* data = GetYAVAppData( hWnd );


	/*---- コード値ごとに分岐 ----*/
	switch( pNotifyHead->code )
	{
	case NM_CLICK:
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- 閉じるボタン判定 ----*/
			int				nIndex, mvIndex;
			TCHITTESTINFO	info;
			RECT			rect;
			bool			close = false;

			// タッチ位置
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
				/*---- アイテム選択 ----*/
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
					/*---- ファイル閉じる ----*/
					SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)nIndex, NULL );
					

					/*---- アイテム選択 ----*/
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

			// 必ずあたいをクリアするように。
			FileTabCloseSkip( data->hFileTab, NULL );
		}
		break;

	case TCN_SELCHANGING:// ファイルタブ選択中
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- 閉じない場合はスクロールデータを格納しておく ----*/
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

	case TCN_SELCHANGE:// ファイルタブ選択
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- アイテム選択 ----*/
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
	case NM_RCLICK:// 右クリック
		if( pNotifyHead->idFrom == IDC_FILETAB )
		{
			/*---- 右クリックされたタブの選択 ----*/
			int           nIndex, nMaxIndex;
			TCHITTESTINFO info;

			GetCursorPos( &info.pt );
			ScreenToClient( data->hFileTab, &info.pt );
			
			nIndex = TabCtrl_HitTest( data->hFileTab, &info );
			nMaxIndex = TabCtrl_GetItemCount( data->hFileTab );
			if( nIndex != -1 )
			{
				/*---- コンテキストメニュー呼び出し ----*/
				HMENU hBaseMenu = LoadMenuW( NULL, MAKEINTRESOURCEW( IDR_FILETABMENU ) );
				ClientToScreen( data->hFileTab, &info.pt );
				UINT id = TrackPopupMenu( GetSubMenu( hBaseMenu, 0 ), TPM_LEFTALIGN | TPM_RETURNCMD, info.pt.x, info.pt.y, 0, hWnd, NULL );
				DestroyMenu( hBaseMenu );


				/*---- ファイルタブコンテキストのIDを処理 ----*/
				switch( id )
				{
				case IDM_CLOSEFILE:	// 閉じる
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
				case IDM_ALL_CLOSE:// これ以外をすべて閉じる
					for( int i = 0; i < nMaxIndex; i++ )
					{
						SendMessageW( hWnd, YAV_WM_DELFILE, (WPARAM)0, NULL );
					}
					TabCtrl_SetCurSel( data->hFileTab, 0 );
					break;
				case IDM_CLOSE_OTHER:// これ以外をすべて閉じる
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

				case IDM_TAB_OPEN_DIRECTORY:// ファイルのフォルダを開く
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


				/*---- ビューの表示状態変更 ----*/
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


				/*---- 領域を再描画 ----*/
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
// アプリケーションウィンドウ
//	WM_CONTEXTMENUの処理
//
static LRESULT appWindowContextMenu( HWND hWnd, HWND hCtrlWnd, int x, int y )
{
	return DefWindowProcW( hWnd, WM_CONTEXTMENU, (WPARAM)hCtrlWnd, MAKELONG( x, y ) );
}


//----------------------------------------------
// アプリケーションウィンドウ
// バージョンダイアログ表示
//
static void appShowVersionDialog( HWND hWnd )
{
	MessageBoxW( hWnd, YAV_VERSION, L"バージョン情報", MB_OK | MB_ICONINFORMATION );
}
