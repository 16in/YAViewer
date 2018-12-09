/*----------------------------------------------------------------------------------------
*
* YAViewer Tree関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "YAVFileTreeMessage.h"


//------------------------------------------------------------------------------
//
//	定数定義
//
//------------------------------------------------------------------------------
//----------------------------------------------
// 拡張子フラグ
//
enum TreeItemExtension
{
	TITEM_EXT_TXT	= 0x01,		// .txt
	TITEM_EXT_MLT	= 0x02,		// .mlt
	TITEM_EXT_AST	= 0x04,		// .ast
	TITEM_EXT_ASD	= 0x08,		// .asd

	TITEM_EXT_TXT_SHIFT = 0,
	TITEM_EXT_MLT_SHIFT,
	TITEM_EXT_AST_SHIFT,
	TITEM_EXT_ASD_SHIFT,

	TITEM_EXT_MAX_SHIFT,

	TITEM_EXT_ALL	= 0xffffffff
};


//------------------------------------------------------------------------------
//
//	関数原型宣言
//
//------------------------------------------------------------------------------
//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterTreeWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// ビューウィンドウ生成
//
HWND CreateTreeWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// ビューウィンドウ標準コールバック
//
LRESULT CALLBACK TreeWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// 拡張メッセージ送信先設定/取得
//
void SetTreeSendMessageRootWindow( HWND hTreeWnd, bool sendRoot );
bool GetTreeSendMessageRootWindow( HWND hTreeWnd );


//----------------------------------------------
// 管理ファイルパスの追加
//
bool TreeAddFilePath( HWND hTreeWnd, LPCWSTR filePath, bool subdir, bool caret = false );


//----------------------------------------------
// 管理ファイルパスの削除
//
bool TreeDeleteFilePath( HWND hTreeWnd, LPCWSTR filePath );


//----------------------------------------------
// 管理ファイルパスの削除
//
void TreeDeleteAll( HWND hTreeWnd );


//----------------------------------------------
// 指定パスのディレクトリを開く
//
bool TreeOpenDirectory( HWND hTreeWnd, LPCWSTR dirPath, bool subdir );


//----------------------------------------------
// ルートアイテム一覧取得
//
DWORD GetTreeRootItem( HWND hTreeWnd, LPWSTR* paths, DWORD count );


//----------------------------------------------
// 表示されているアイテムの一覧取得
//
DWORD GetTreeOpenItem( HWND hTreeWnd, LPWSTR* paths, DWORD count );


//----------------------------------------------
// 拡張子フラグ設定
//
void SetEnableTreeItemExtension( HWND hTreeWnd, DWORD enableExtension );
DWORD GetEnableTreeItemExtension( HWND hTreeWnd );
