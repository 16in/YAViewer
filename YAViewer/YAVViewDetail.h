/*----------------------------------------------------------------------------------------
*
* YAViewer View 実動作関係
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <AAFileAccessor.h>


//----------------------------------------------
// ウィンドウ関連付けデータ
//
struct RegViewWindowData
{
	aafile::AAFile*		file;					// 表示するファイル
	int					mode;					// 表示モード
	int					categorySize;			// カテゴリ表示時のサイズ
	bool				sendRoot;				// メッセージの送信先をルートウィンドウにするか

	HWND*				pages;					// ページウィンドウデータ
	HFONT				hFont;					// 描画用フォント
	int					lfHeight;				// 行の高さ
	int					charWidth[ 0x10000 ];	// 文字幅テーブル

	bool				enableTooltip;			// ツールチップフラグ

	RegViewWindowData( void )
		: file( NULL ), mode( 0 ), categorySize( 128 ),
		sendRoot( false ), pages( NULL ), hFont( NULL ), lfHeight( 12 ), enableTooltip( true )
	{ ZeroMemory( charWidth, sizeof( charWidth ) ); }

	virtual ~RegViewWindowData( void )
	{}
};
struct RegViewPageData
{
	aafile::AAFilePage*	page;					// 頁データ
	long				width, height;			// 頁サイズ
	unsigned long		lfCount;				// 改行数
	int*				lfHeight;				// 行の高さへのポインタ
	int*				charWidthTable;			// 文字幅テーブル

	bool				mouseTracking;			// マウスの追跡
	bool				hover;					// マウスが上にあるか
	bool				invert;					// 反転済みであるか

	// メモリデバイスコンテキスト
	HDC					hDeviceContext;			// デバイスコンテキストハンドル
	HBITMAP				hDeviceContextBMP;		// デバイスコンテキストビットマップ
	HBITMAP				hOldDeviceContextBMP;	// 以前のデバイスコンテキストビットマップ
	
	// ツールヒント
	HWND				hToolHint;				// ツールヒントコントロール
	wchar_t*			toolString;				// ツール表示文字列

	RegViewPageData( int* cwTable, int* lfh )
		: page( NULL ), width( 0 ), height( 0 ), lfCount( 0 ), lfHeight( lfh ), charWidthTable( cwTable ),
		mouseTracking( false ), hover( false ), invert( false ),
		hDeviceContext( NULL ), hDeviceContextBMP( NULL ), hOldDeviceContextBMP( NULL ), hToolHint( NULL ), toolString( NULL )
	{}

	virtual ~RegViewPageData( void )
	{}
};


//----------------------------------------------
// ビューウィンドウに子があるか
//
bool CheckViewWindowChild( HWND hViewWnd );


//----------------------------------------------
// ビューウィンドウサイズ変更
//
void SetViewWindowSize( HWND hViewWnd, int width, int height );


//----------------------------------------------
// 登録済みの頁をデバイスコンテキストに書き込む
//
void AllPaintPage( HWND hViewWnd );


//----------------------------------------------
// 指定のページデータを書き出す
//
void PaintPage( RegViewWindowData* viewData, HWND hPageWnd, RegViewPageData* pageData );


//----------------------------------------------
// 頁データをウィンドウにコピーする
//
void CopyPageData( HDC hPageDC, RegViewWindowData* viewData, RegViewPageData* pageData );


//----------------------------------------------
// 登録済みの表示データを取得する
//
RegViewWindowData* GetViewWindowData( HWND hViewWnd );


//----------------------------------------------
// 登録済みのページデータを取得する
//
RegViewPageData* GetViewPageData( HWND hPageWnd );


//----------------------------------------------
// ツールチップ表示を設定する
//
void SetTooltipEnablePage( HWND hViewWnd, bool enable );
