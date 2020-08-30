/*----------------------------------------------------------------------------------------
*
* YAViewer アプリケーション動作関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <AAFileAccessor.h>


//-------------------------------------
// 事前定義するもの
//
typedef struct _tagRegYavAppData RegYavAppData;


//-------------------------------------
// ファイルタブ情報構造体
//
struct YavFileTabParam
{
	wchar_t				filePath[ MAX_PATH ];	// ファイルパス
	aafile::AAFile*		file;					// AAファイル
	int					scrollPosition;			// スクロール位置

	union
	{
		DWORD			flags;					// フラグ
		struct {
			int			useTabColor : 1;		// タブ配色使用フラグ
			int			disableClose: 1;		// 閉じる不可フラグ
			int : 30;
		};
	};
	COLORREF			tabColor;				// タブ配色
};


//-------------------------------------
// 文字列送信先フラグ
//
enum SendStringTarget
{
	// 対象エディタ列挙
	SST_CLIPBOARD = 0x00,				// クリップボード
	SST_FACE_EDIT,						// (´д｀)
	SST_ORINRIN_EDITOR,					// Orinrin

	SST_SAKURA_EDITOR,					// サクラエディタ

	SST_MAX,							// 対象エディタ最大

	// 送信オプション
	SST_TEXTBOX_FLAG = 0x00010000,		// テキストボックス送信フラグ

	// マスク
	SST_EDITOR_MASK = 0x0000ffff,
	SST_OPTION_MASK = 0xffff0000,

	// 32bit拡張フラグ
	SST_UNKNOWN = 0xffffffff
};


//-------------------------------------
// ファイルタブにファイルを追加する
//
bool YavAddFile( RegYavAppData* appData, LPCWSTR filePath, bool setView );


//-------------------------------------
// ファイルタブからファイルを削除する
//
bool YavDelFile( RegYavAppData* appData, int idx );


//-------------------------------------
// ファイルタブからファイルを検索する
//
int YavGetFileTabIndex( RegYavAppData* appData, LPCWSTR filePath );


//-------------------------------------
// ファイルタブからファイルを取得する
//
bool YavGetFile( RegYavAppData* appData, int idx, LPWSTR filePath, size_t length );


//-------------------------------------
// ファイルタブ登録済みファイル数を取得する
//
int YavGetFileCount( RegYavAppData* appData );


//-------------------------------------
// ファイルタブからファイルを選択する
//
bool YavSelectFile( RegYavAppData* appData, LPCWSTR filePath );
bool YavSelectFile( RegYavAppData* appData, int idx );


//-------------------------------------
// ファイルタブカラーを設定/取得する
//
bool YavSelectTabColor( RegYavAppData* appData, int idx, COLORREF tabColor, bool disable = false );
bool YavGetTabColor( RegYavAppData* appData, int idx, COLORREF* tabColor );


//-------------------------------------
// ファイルタブを保存する
//
bool YavSaveFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// ファイルタブを復元する
//
bool YavLoadFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// ファイルツリーを保存する
//
bool YavSaveFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// ファイルタブを復元する
//
bool YavLoadFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// 指定の文字列をクリップボードへ送信
//
bool YavSendClipBoardString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length );


//-------------------------------------
// 指定の文字列をユニコードに変換してクリップボードへ送信
//
bool YavSendClipBoardUnicodeString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length );


//-------------------------------------
// 指定の文字列をエディタへ送信
//
bool YavSendEditorString( RegYavAppData* appData, DWORD target, LPCWSTR string, size_t length );


//-------------------------------------
// 実行ファイルディレクトリ設定/取得
//
void YavSetAppDirectory( LPCWSTR appDirectory );
LPCWSTR YavGetAppDirectory( void );


//-------------------------------------
// 設定ファイル保存/読み込み
//
void YavSaveAppSetting( RegYavAppData* appData, LPCWSTR settingfile = NULL );
void YavLoadAppSetting( RegYavAppData* appData, LPCWSTR settingfile = NULL );
