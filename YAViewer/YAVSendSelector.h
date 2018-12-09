/*----------------------------------------------------------------------------------------
*
* YAViewer 送信先選択関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//------------------------------------------------------------------------------
//
//	定数定義
//
//------------------------------------------------------------------------------
// 送信先定義
//
enum SendSelector
{
	SendTargetLeftClick = 0,			// 左クリック
	SendTargetCenterClick,				// 中クリック
	SendTargetMax						// 最大値
};


//------------------------------------------------------------------------------
//
//	関数原型宣言
//
//------------------------------------------------------------------------------
//----------------------------------------------
// ウィンドウクラス登録
//
bool RegisterSelectorWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// 送信先選択ウィンドウ生成
//
HWND CreateSelectorWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// 送信先選択ウィンドウ標準コールバック
//
LRESULT CALLBACK SelectorWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// 送信先 設定/取得
//
void SetSelectSendSelector( HWND hWnd, SendSelector st, DWORD target );
DWORD GetSelectSendSelector( HWND hWnd, SendSelector st );


//----------------------------------------------
// 選択方式 設定/取得
//
void SetSelectDoubleClick( HWND hWnd, bool dblclick );
bool GetSelectDoubleClick( HWND hWnd );
