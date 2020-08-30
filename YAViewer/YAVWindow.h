/*----------------------------------------------------------------------------------------
*
* YAViewer Window関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "YAVDialog.h"
#include "YAViewerMessage.h"
#include "YAVAppAction.h"


//----------------------------------------------
// アプリケーションウィンドウ登録データ
//
typedef struct _tagRegYavAppData
{
	//-- ウィンドウ関係
	HWND			hFileTab;					// ファイルタブ
	HWND			hCtrlWnd;					// コントロールウィンドウ
	HWND			hSelector;					// 送信先設定ウィンドウ
	HWND			hFileTree;					// ファイルツリー
	HWND			hFileView;					// ファイルビュー
	HWND			hTreeShort;					// ツリーとショートカット
	HWND			hAAShortcut;				// AAショートカットウィンドウ

	//-- フォント関係
	HFONT			hAAFont;					// AA用フォント
	wchar_t			fontName[ 32 ];				// フォント名
	long			fontSize;					// フォントサイズ

	//-- 送信先設定
	DWORD			clipUnicodeCopy;			// クリップボードにユニコード変換して送信するか
	DWORD			sendTargetLeft;				// 左クリック送信先
	DWORD			sendTargetRight;			// 右クリック送信先
	DWORD			sendTargetCenter;			// 中クリック送信先

	//-- ツリー設定
	DWORD			enableExtension;			// 有効な拡張子

	//-- ウィンドウ状態
	long			x, y;						// 座標
	long			width, height;				// 幅と高さ
	long			splitPos;					// 分割ウィンドウ位置
	long			reverseSplit;				// 分割ウィンドウ入れ替え
	long			viewMode;					// 表示モード
	long			toolHint;					// ツールヒント表示
	long			categorySize;				// カテゴリ表示サイズ
	long			tabButtonMode;				// タグボタンモード
	long			dblclickMode;				// ダブルクリック選択モード
	long			shortcutSize;				// ショートカットウィンドウサイズ

	//-- 初期化情報
	wchar_t			initTabPath[ MAX_PATH ];	// タブ初期化ファイル指定
	wchar_t			initTreePath[ MAX_PATH ];	// ツリー初期化ファイル指定
} RegYavAppData;


//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterYAVWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// ウィンドウ生成
//
HWND CreateYAVWindow( HINSTANCE hInstance, RegYavAppData* appData );


//----------------------------------------------
// アプリケーションウィンドウ標準コールバック
//
LRESULT CALLBACK YAVAppWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// アプリケーションウィンドウ登録データ取得
//
RegYavAppData* GetYAVAppData( HWND hWnd );
