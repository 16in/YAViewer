//----------------------------------------------------------------------
//
// YAViewer リソース定数定義
//
//----------------------------------------------------------------------
#pragma once


//-------------------------------------
// コントロール識別ID
//
#define IDC_FILETAB			0x0001			/* ファイルタブ */


//------------------------------
// アイコンリソース
//
#define IDI_APP				0x1000			/* アプリケーションアイコン */
#define IDI_TABCLOSE		0x1001			/* タブクローズアイコン */
#define IDI_TABCLOSE_SEL	0x1002			/* タブクローズアイコン 選択中 */


//-------------------------------------
// メインメニューリソース
//
#define IDR_MAINMENU		0x1000			/* メインメニュー */

// ファイル
#define IDM_EXIT			0x1001			/* ファイル - 終了 */
#define IDM_FILEOPEN		0x1002			/* ファイル - ファイルを開く */
#define IDM_TABSAVE			0x1003			/* ファイル - タブを保存 */
#define IDM_TABOPEN			0x1004			/* ファイル - タブを復元 */
#define IDM_TREESAVE		0x1005			/* ファイル - ツリーを保存 */
#define IDM_TREEOPEN		0x1006			/* ファイル - ツリーを復元 */

// 表示
#define IDM_MOST_TOP		0x2000			/* 表示 - 最前面 */
#define IDM_NORMAL_VIEW		0x2001			/* 表示 - 通常表示 */
#define IDM_CATEGORY_VIEW	0x2002			/* 表示 - カテゴリ表示 */
#define IDM_EDITOR_VIEW		0x2003			/* 表示 - エディタ表示 */
#define IDM_SPLIT_SWAP		0x2004			/* 表示 - 分割表示入れ替え */
#define IDM_VIEW_TOOLTIP	0x2005			/* 表示 - ツールチップ表示 */
#define IDM_TAB_BUTTON		0x2006			/* 表示 - タブボタン表示 */
#define IDM_LOCK_TREE		0x2010			/* 表示 - ツリーロック */
#define IDM_LOCK_ASCII		0x2011			/* 表示 - ＡＡロック */

// 表示 - カテゴリ表示サイズ
#define IDM_CATEGORY_32		0x2100			/* カテゴリ表示サイズ - 32 */
#define IDM_CATEGORY_64		0x2101			/* カテゴリ表示サイズ - 64 */
#define IDM_CATEGORY_96		0x2102			/* カテゴリ表示サイズ - 96 */
#define IDM_CATEGORY_128	0x2103			/* カテゴリ表示サイズ - 128 */
#define IDM_CATEGORY_160	0x2104			/* カテゴリ表示サイズ - 160 */
#define IDM_CATEGORY_192	0x2105			/* カテゴリ表示サイズ - 192 */
#define IDM_CATEGORY_224	0x2106			/* カテゴリ表示サイズ - 224 */
#define IDM_CATEGORY_256	0x2107			/* カテゴリ表示サイズ - 256 */
#define IDM_CATEGORY_384	0x2108			/* カテゴリ表示サイズ - 384 */
#define IDM_CATEGORY_512	0x2109			/* カテゴリ表示サイズ - 512 */
#define IDM_CATEGORY_768	0x210a			/* カテゴリ表示サイズ - 768 */
#define IDM_CATEGORY_1024	0x210b			/* カテゴリ表示サイズ - 1024 */

// 表示 - 拡張子選択
#define IDM_ENABLE_TXT		0x2180			/* 拡張子選択 - .txt */
#define IDM_ENABLE_MLT		0x2181			/* 拡張子選択 - .mlt */
#define IDM_ENABLE_AST		0x2182			/* 拡張子選択 - .ast */
#define IDM_ENABLE_ASD		0x2183			/* 拡張子選択 - .asd */

// 設定
#define IDM_CLIP_UNICODE	0x4100			/* クリップボードユニコード送信 */

// 送信先
#define IDM_SEND_FACEEDIT	0x4000			/* 送信先 - (´д｀)Edit */
#define IDM_SEND_ORINEDIT	0x4001			/* 送信先 - OrinrinEditor */

// ヘルプ
#define IDM_VERSION			0x7800			/* ヘルプ - バージョン */


//------------------------------
// ファイルタブコンテキストメニューリソース
//
#define IDR_FILETABMENU			0x1100

#define IDM_CLOSEFILE			0x8000			/* タブ - 閉じる */
#define IDM_ALL_CLOSE			0x8001			/* タブ - すべて閉じる */
#define IDM_CLOSE_OTHER			0x8002			/* タブ - このファイル以外すべて閉じる */
#define IDM_TAB_COLOR			0x8100			/* タブ - タブ色を設定 */
#define IDM_TAB_CLEAR_COLOR		0x8101			/* タブ - タブ色をクリア */
#define IDM_TAB_OPEN_DIRECTORY	0x8200			/* タブ - ファイルのあるフォルダを開く */


//------------------------------
// ファイルツリーコンテキストメニューリソース
//
#define IDR_FILETREEDIR_MENU	0x1200
#define IDR_FILETREEFILE_MENU	0x1201

#define IDM_TREE_OPENTREE		0x9000			/* ツリー - ディレクトリを展開する */
#define IDM_TREE_OPENALLDIRECTORY_TREE 0x9010	/* ツリー - 指定ディレクトリ以下をすべて展開する */
#define IDM_TREE_CLOSEALLDIRECTORY_TREE 0x9020	/* ツリー - 指定ディレクトリ以下をすべて閉じる */
#define	IDM_TREE_OPENFILE		0x9400			/* ツリー - ファイルを開く */
#define IDM_TREE_OPEN_DIRECTORY	0x9800			/* ツリー - エクスプローラで開く */
#define IDM_TREE_DELETEROOT		0x9801			/* ツリー - ディレクトリを削除する */
#define IDM_TREE_ROOTITEM		0x9802			/* ツリー - ルートアイテム化 */
