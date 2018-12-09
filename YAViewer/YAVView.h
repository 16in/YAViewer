/*----------------------------------------------------------------------------------------
*
* YAViewer View関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "AAFileAccessor.h"


//----------------------------------------------
// Viewウィンドウから親ウィンドウへ送られるメッセージ類
//	SetSendMessageRootWindowに真を指定した場合
//	Viewウィンドウのルートウィンドウに対してメッセージを送ります。
//
#include "YAVViewMessage.h"


//------------------------------------------------------------------------------
//
//	マクロ、構造体定義
//
//------------------------------------------------------------------------------
//----------------------------------------------
// Viewウィンドウ表示形式マクロ
//
#define VIEWMODE_NORMAL		0		/* 通常表示 */
#define VIEWMODE_CATEGORY	1		/* カテゴリ表示 */



//------------------------------------------------------------------------------
//
//	関数原型宣言
//
//------------------------------------------------------------------------------
//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterViewWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// ビューウィンドウ生成
//
HWND CreateViewWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// ビューウィンドウ標準コールバック
//
LRESULT CALLBACK ViewWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// スクロール位置設定/取得
//
void SetViewScrollPosition( HWND hViewWnd, int pos );
void SetViewScrollPositionByPage( HWND hViewWnd, int page );
int  GetViewScrollPosition( HWND hViewWnd );


//----------------------------------------------
// 表示フォント設定/取得
//
void SetViewFont( HWND hViewWnd, HFONT hFont );
HFONT GetViewFont( HWND hViewWnd );


//----------------------------------------------
// 表示モード設定/取得
//
void SetViewMode( HWND hViewWnd, int mode );
int GetViewMode( HWND hViewWnd );


//----------------------------------------------
// ツールヒント表示設定/取得
//
void SetViewToolTip( HWND hViewWnd, bool enable );
bool GetViewToolTip( HWND hViewWnd );


//----------------------------------------------
// カテゴリ表示サイズ設定/取得
//
void SetViewCategorySize( HWND hViewWnd, int size );
int GetViewCategorySize( HWND hViewWnd );


//----------------------------------------------
// 表示ファイル設定/取得
//
void SetViewFile( HWND hViewWnd, aafile::AAFile* file );
aafile::AAFile* GetViewFile( HWND hViewWnd );


//----------------------------------------------
// 拡張メッセージ送信先設定/取得
//
void SetViewSendMessageRootWindow( HWND hViewWnd, bool sendRoot );
bool GetViewSendMessageRootWindow( HWND hViewWnd );


//----------------------------------------------
// 参照文字列解決
//
bool CheckRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount );
