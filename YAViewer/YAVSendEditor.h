/*----------------------------------------------------------------------------------------
*
* YAViewer データ送信関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//----------------------------------------------
// 型定義
//
// SendClipStringの実動作部分割り込み
typedef bool (*SendClipStringProc)( HWND, LPARAM );


//----------------------------------------------
// キーストロークを送る
//
void SendKeyStroke( HWND hWnd, BYTE* vkList, int count );


//----------------------------------------------
// ウィンドウを検索する
//
HWND GetWindowHandle( HWND hParent, LPCWSTR text, LPCWSTR clsName );


//----------------------------------------------
// クリップボードを経由してデータを送信する
//
bool SendClipString( HWND hWnd, HANDLE hString, SendClipStringProc proc, LPARAM lParam );