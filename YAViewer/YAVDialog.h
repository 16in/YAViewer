/*----------------------------------------------------------------------------------------
*
* YAViewer ダイアログ関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//----------------------------------------------
// AA関係ファイルを開くダイアログ
//
bool OpenAAFileDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// タブを保存ダイアログ
//
bool SaveFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// タブを復元ダイアログ
//
bool OpenFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// ツリーを保存ダイアログ
//
bool SaveFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// ツリーを復元ダイアログ
//
bool OpenFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// タブカラーダイアログ
//
bool SelectTabColorDialog( HWND hOwner, COLORREF* color );
