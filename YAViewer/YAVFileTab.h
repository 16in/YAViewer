/*----------------------------------------------------------------------------------------
*
* YAViewer ファイルタブ関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//------------------------------------------------------------------------------
//
//	関数原型宣言
//
//------------------------------------------------------------------------------
//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterYAVFileTabClass( HINSTANCE hInstance );

//----------------------------------------------
// ファイルタブウィンドウ生成
//
HWND CreateFileTabWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle, DWORD id );

//----------------------------------------------
// ファイルタブウィンドウ取得
//	子ウィンドウの登録先
//
HWND GetFileTabWindow( HWND hWnd );

//----------------------------------------------
// ファイルタブ - xボタン矩形取得
//
bool GetFileTabCloseRect( HWND hWnd, int idx, RECT* dst );
bool GetFileTabCloseRectToItemRect( const RECT* tabItemRect, RECT* dst );

//----------------------------------------------
// ファイルタブ - xボタンスキップ判定
//
bool FileTabCloseSkip( HWND hWnd, int* moveIndex );

//----------------------------------------------
// ファイルタブ - タブボタンモード設定
//
void SetFileTabButtonMode( HWND hWnd, bool buttom );
bool GetFileTabButtonMode( HWND hWnd );
