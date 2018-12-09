/*----------------------------------------------------------------------------------------
*
* YAViewer FileTree 独自メッセージ関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


/**
* アイテム選択時
*
*	WPARAM -		0 : ファイル
*					1 : ディレクトリ
*
*	LPARAM -		TreeSelectItem*
*
*	return			常に0
*/
#define WM_TREE_SELECTITEM		(WM_APP + 0x1000)
typedef struct _tagTreeSelectItem
{
	const wchar_t*		filePath;		// 選択ファイルパス
} TreeSelectItem;
