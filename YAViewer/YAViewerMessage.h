/*----------------------------------------------------------------------------------------
*
* YAViewer 独自メッセージ関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "YAVViewMessage.h"
#include "YAVFileTreeMessage.h"


/**
* アイテム選択時
*	詳細はYAVViewMessage.hを参照
*/
#define YAV_WM_SELECTITEM		WM_VIEW_SELECTITEM

/**
* ツリーアイテム選択時
*	詳細はYAVFileTreeMessage.hを参照
*/
#define YAV_WM_TREE_SELECTITEM		WM_TREE_SELECTITEM

/**
* ファイル追加
*	指定のファイルをファイルタブに追加します。
*
*	WPARAM -	0 : ファイルビューに転送しない
*				1 : ファイルビューに転送する
*
*	LPARAM -	YavWmAddFile*
*
*	return		BOOL		読み込みに成功したら真、失敗したら偽
*/
#define YAV_WM_ADDFILE			(WM_APP + 1)
typedef struct _tagYavWmAddFile
{
	const wchar_t*		filePath;		// 追加するファイルへのパス
} YavWmAddFile;

/**
* ファイルタブから削除
*	指定のファイルをファイルタブから削除します。
*
*	WPARAM -	int			削除するファイルタブのインデックス
*							負数が設定された場合はすべて廃棄
*
*	return		BOOL		読み込みに成功したら真、失敗したら偽
*/
#define YAV_WM_DELFILE			(WM_APP + 2)

/**
* タブを保存
*	ファイルタブを指定のファイルに保存します。
*
*	LPARAM -	LPCWSTR		保存先
*
*	return		BOOL		保存に成功したら真、失敗したら偽
*/
#define YAV_WM_SAVEFILETAB		(WM_APP + 3)

/**
* タブを復元
*	ファイルタブを指定のファイルに復元します。
*
*	LPARAM -	LPCWSTR		復元先
*
*	return		BOOL		復元に成功したら真、失敗したら偽
*/
#define YAV_WM_LOADFILETAB		(WM_APP + 4)

/**
* ツリーを保存
*	ファイルツリーを指定のファイルに保存します。
*
*	LPARAM -	LPCWSTR		保存先
*
*	return		BOOL		保存に成功したら真、失敗したら偽
*/
#define YAV_WM_SAVEFILETREE		(WM_APP + 5)

/**
* ツリーを復元
*	ファイルツリーを指定のファイルに復元します。
*
*	LPARAM -	LPCWSTR		復元先
*
*	return		BOOL		復元に成功したら真、失敗したら偽
*/
#define YAV_WM_LOADFILETREE		(WM_APP + 6)

/**
* ページジャンプ実行
*	現在開いているファイルの指定ページへジャンプします。
*
*	LPARAM -	page		ページ番号
*
*	return		BOOL		復元に成功したら真、失敗したら偽
*/
#define YAV_WM_PAGEJUMP			(WM_APP + 7)
