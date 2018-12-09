/*----------------------------------------------------------------------------------------
*
* YAViewer SplitWindow関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//----------------------------------------------
// 分割モード定数
//
#define SPL_VERTICAL	0		/* 垂直分割 */
#define SPL_HORIZONTAL	1		/* 水平分割 */


//----------------------------------------------
// 固定モード定数
//
#define SPL_LOCK_LEFT	0		/* 左側固定 */
#define SPL_LOCK_TOP	0		/* 上側固定 */
#define SPL_LOCK_RIGHT	1		/* 右側固定 */
#define SPL_LOCK_BOTTOM	1		/* 下側固定 */


//----------------------------------------------
// 分割ウィンドウクラス登録
//
bool RegisterSplitWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// 分割ウィンドウ生成
//
HWND CreateSplitWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// 分割ウィンドウ標準コールバック
//
LRESULT CALLBACK YAVSplitWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// 分割ウィンドウモード設定/取得
//
int SetSplitWindowMode( HWND hSplitWnd, int mode );
int GetSplitWindowMode( HWND hSplitWnd );


//----------------------------------------------
// 分割領域区切りサイズ設定/取得
//
int SetSplitBorderWidth( HWND hSplitWnd, int width );
int GetSplitBorderWidth( HWND hSplitWnd );


//----------------------------------------------
// 分割サイズ設定/取得
//
int SetSplitWidth( HWND hSplitWnd, int width, bool update = true );
int GetSplitWidth( HWND hSplitWnd );
int SetSplitLeftWidth( HWND hSplitWnd, int width );
int GetSplitLeftWidth( HWND hSplitWnd );
int SetSplitTopWidth( HWND hSplitWnd, int width );
int GetSplitTopWidth( HWND hSplitWnd );


//----------------------------------------------
// 分割領域をダブルクリックでセンタリングするか設定/取得
//
BOOL SetSplitDbClickCentering( HWND hSplitWnd, BOOL dbclkCnter );
BOOL GetSplitDbClickCentering( HWND hSplitWnd );


//----------------------------------------------
// 分割対象ウィンドウ設定/取得
//
void SetSplitTargetWindow( HWND hSplitWnd, HWND left, HWND right );
void GetSplitTargetWindow( HWND hSplitWnd, HWND* left, HWND* right );


//----------------------------------------------
// 分割対象ウィンドウ固定方向設定/取得
//
void SetSplitWindowLock( HWND hSplitWnd, int lock );
int GetSplitWindowLock( HWND hSplitWnd );


//----------------------------------------------
// 分割対象ウィンドウ入れ替え
//
void SwapSplitTargetWindow( HWND hSplitWnd );
