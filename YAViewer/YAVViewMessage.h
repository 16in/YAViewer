/*----------------------------------------------------------------------------------------
*
* YAViewer Viewer 独自メッセージ関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


/**
* アイテム選択時
*
*	WPARAM 
*		下位ワード
*					0x0000 : 左クリック
*					0x0001 : 右クリック
*					0x0002 : 中クリック
*		上位ワード
*					0x0001 : SHIFTキー押下
*					0x0002 : CONTROLキー押下
*
*	LPARAM -		SelectItem*
*
*	return			常に0
*/
#define WM_VIEW_SELECTITEM		WM_APP
typedef struct _tagViewSelectItem
{
	const wchar_t*		itemString;			// アイテム文字列
	unsigned long		itemLength;			// アイテム文字数
} ViewSelectItem;
