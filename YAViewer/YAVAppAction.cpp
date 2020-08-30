/*----------------------------------------------------------------------------------------
*
* YAViewer アプリケーション動作関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVAppAction.h"
#include "YAVWindow.h"
#include "YAVView.h"
#include "YAVFileTree.h"
#include "YAVSendEditor.h"
#include "YAVShortcut.h"
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <string>


//-------------------------------------
// グローバル変数
//
static wchar_t			sg_AppDirectoryPath[ MAX_PATH ] = L"";
static const wchar_t*	sgc_DefaultSettingPath = L"YavSetting.ini";


//-------------------------------------
// グローバル関数
//
static bool insertFileTabItem( HWND hTabWnd, aafile::AAFile* file, const wchar_t* path, bool selCursor );
static aafile::AAFile* eraseFileTabItem( HWND hTabWnd, int idx );
static HWND getSendStringTarget( DWORD target );
static void setSettingFilePath( LPCWSTR path, LPWSTR filePath, size_t length );


//-------------------------------------
// 設定入出力関数
//
static void writeWindowInitData( RegYavAppData* appData, LPCWSTR settingfile );
static void writeSettingInitData( RegYavAppData* appData, LPCWSTR settingfile );
static void readWindowInitData( RegYavAppData* appData, LPCWSTR settingfile );
static void readSettingInitData( RegYavAppData* appData, LPCWSTR settingfile );


//-------------------------------------
// エディタに対するデータ送信処理
//
struct SendEditPasteParam
{
	DWORD	target;			// 送信対象の種別
	DWORD	option;			// 送信オプション
};
static bool SendEditPaste( HWND hWnd, LPARAM lParam )
{
	bool ret = false;


	/*---- ターゲットとオプションごとに処理を分ける ----*/
	SendEditPasteParam* sendParam = (SendEditPasteParam*)lParam;
	switch( sendParam->option )
	{
	case 0:// 通常貼り付け
		{
			BYTE list[ 2 ] = { VK_CONTROL, 'V' };
			SendKeyStroke( hWnd, list, 2 );
		}
		ret = true;
		break;

	case SST_TEXTBOX_FLAG:
		switch( sendParam->target )
		{
		case SST_FACE_EDIT:// (´д｀)Edit
			{
				BYTE list[ 2 ] = { VK_MENU, VK_SPACE };
				SendKeyStroke( hWnd, list, 2 );
			}
			ret = true;
			break;
		case SST_ORINRIN_EDITOR:// Orinrin Editor
			SetForegroundWindow( hWnd );
			SendMessageW( hWnd, WM_COMMAND, 0x00018014, 0 );
			ret = true;
			break;
		}
		break;
	}


	return ret;
}


//-------------------------------------
// ファイルタブにファイルを追加する
//
bool YavAddFile( RegYavAppData* appData, LPCWSTR filePath, bool setView )
{
	bool ret = false;

	if( appData )
	{
		int idx = YavGetFileTabIndex( appData, filePath );

		if( idx >= 0 )
		{
			/*---- 読み込み済みファイル選択 ----*/
			if( setView )
			{
				ret = YavSelectFile( appData, idx );
			}
		}
		else
		{
			/*---- 追加ファイル読み込み ----*/
			aafile::AAFile* file = aafile::AAFileReader::CreateAAFileFromFile( filePath );
			if( file )
			{
				/*---- ファイルタブにデータ追加 ----*/
				ret = insertFileTabItem( appData->hFileTab, file, filePath, setView );


				/*---- ビューにセットする場合は更新する ----*/
				if( ret && setView )
				{
					SetShortcutFile( appData->hAAShortcut, file );
					SetViewFile( appData->hFileView, file );
				}
			}
		}
	}

	return ret;
}


//-------------------------------------
// ファイルタブからファイルを削除する
//
bool YavDelFile( RegYavAppData* appData, int idx )
{
	bool ret = false;

	if( appData )
	{
		/*---- ファイルタブにデータ追加 ----*/
		aafile::AAFile* file = eraseFileTabItem( appData->hFileTab, idx );
		if( file == GetViewFile( appData->hFileView ) )
		{
			SetShortcutFile( appData->hAAShortcut, NULL );
			SetViewFile( appData->hFileView, NULL );
		}
		if( file )
		{
			aafile::AAFileReader::ReleaseAAFile( file );
			ret = true;
		}
	}

	return ret;
}

//-------------------------------------
// ファイルタブからファイルを検索する
//
int YavGetFileTabIndex( RegYavAppData* appData, LPCWSTR filePath )
{
	int ret = -1;

	if( appData )
	{
		int count = TabCtrl_GetItemCount( appData->hFileTab );


		/*---- アイテムを先頭から走査し、チェックしていく ----*/
		for( int i = 0; i < count; i++ )
		{
			TC_ITEMW item = { 0 };
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem( appData->hFileTab, i, &item );
			if( item.lParam )
			{
				YavFileTabParam* tab = (YavFileTabParam*)item.lParam;
				if( wcscmp( filePath, tab->filePath ) == 0 )
				{
					ret = i;
					break;
				}
			}
		}
	}

	return ret;
}


//-------------------------------------
// ファイルタブからファイルを取得する
//
bool YavGetFile( RegYavAppData* appData, int idx, LPWSTR filePath, size_t length )
{
	bool ret = false;

	if( appData )
	{
		/*---- 指定のインデックスのアイテム取得 ----*/
		TC_ITEMW item = { 0 };
		item.mask = TCIF_PARAM;
		if( TabCtrl_GetItem( appData->hFileTab, idx, &item ) )
		{
			YavFileTabParam* tabParam = (YavFileTabParam*)item.lParam;
			if( tabParam )
			{
				wcsncpy_s( filePath, length, tabParam->filePath, length );
				ret = true;
			}
		}
	}


	return ret;
}


//-------------------------------------
// ファイルタブ登録済みファイル数を取得する
//
int YavGetFileCount( RegYavAppData* appData )
{
	int ret = 0;

	if( appData )
	{
		ret = TabCtrl_GetItemCount( appData->hFileTab );
	}

	return ret;
}


//-------------------------------------
// ファイルタブからファイルを選択する
//
bool YavSelectFile( RegYavAppData* appData, LPCWSTR filePath )
{
	bool ret = false;
	int idx = YavGetFileTabIndex( appData, filePath );

	if( idx >= 0 )
	{
		ret = YavSelectFile( appData, idx );
	}

	return ret;
}
bool YavSelectFile( RegYavAppData* appData, int idx )
{
	bool ret = false;

	if( appData )
	{
		ret = (TabCtrl_SetCurSel( appData->hFileTab, idx ) >= 0);
		if( ret )
		{
			TC_ITEMW item = { 0 };
			YavFileTabParam* tab;
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem( appData->hFileTab, idx, &item );
			tab = (YavFileTabParam*)item.lParam;

			if( tab )
			{
				SetShortcutFile( appData->hAAShortcut, tab->file );
				SetViewFile( appData->hFileView, tab->file );
			}

			ret = (tab != NULL);
		}
	}

	return ret;
}


//-------------------------------------
// ファイルタブカラーを設定/取得する
//
bool YavSelectTabColor( RegYavAppData* appData, int idx, COLORREF tabColor, bool disable )
{
	bool ret = false;

	if( appData )
	{
		if( idx < 0 )
		{
			idx = TabCtrl_GetItemCount( appData->hFileTab ) - 1;
		}

		TC_ITEMW item = { 0 };
		YavFileTabParam* tab;
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem( appData->hFileTab, idx, &item );
		tab = (YavFileTabParam*)item.lParam;

		if( tab )
		{
			tab->useTabColor = !disable;
			tab->tabColor = tabColor;
		}

		ret = (tab != NULL);
	}

	return ret;
}
bool YavGetTabColor( RegYavAppData* appData, int idx, COLORREF* tabColor )
{
	bool ret = false;

	if( appData )
	{
		TC_ITEMW item = { 0 };
		YavFileTabParam* tab;
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem( appData->hFileTab, idx, &item );
		tab = (YavFileTabParam*)item.lParam;

		if( tab )
		{
			ret = (tab->useTabColor != 0);
			if( tabColor ) *tabColor = tab->tabColor;
		}
	}

	return ret;
}


//-------------------------------------
// ファイルタブを保存する
//
bool YavSaveFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	bool ret = false;


	if( appData )
	{
		/*---- 出力先設定 ----*/
		wchar_t dir[ MAX_PATH ];
		wcscpy_s( dir, filePath );
		PathRemoveFileSpecW( dir );
		if( !::PathIsDirectoryW( dir ) )
		{
			::CreateDirectoryW( dir, NULL );
		}


		/*---- 書き出しデータ生成 ----*/
		std::wstring buf = L"";
		for( int i = 0; i < YavGetFileCount( appData ); i++ )
		{
			// ファイルパス
			wchar_t path[ MAX_PATH ];
			YavGetFile( appData, i, path, MAX_PATH );
			buf += L"File://";
			buf += path;
			buf += L"\r\n";

			// タブ色
			COLORREF color;
			if( YavGetTabColor( appData, i, &color ) )
			{
				wchar_t ct[ 7 ];
				swprintf( ct, 7, L"%06X", (color & 0x00ffffff) );
				buf += L"TabColor://";
				buf += ct;
				buf += L"\r\n";
			}
		}


		/*---- 書き出し ----*/
		HANDLE hFile = CreateFileW( filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( !hFile )
		{// 書き出し用ファイルのオープンに失敗
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"ファイルタブリストファイルの生成に失敗しました。", L"Error YAViewer FileTabList File Create.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{// ファイルにデータ書き出し
			DWORD writeSize;
			WORD BOM = L'\xfeff';
			WriteFile( hFile, &BOM, sizeof( BOM ), &writeSize, NULL );
			WriteFile( hFile, buf.c_str(), (DWORD)(sizeof( wchar_t ) * buf.size()), &writeSize, NULL );
			CloseHandle( hFile );

			ret = true;
		}
	}


	return ret;
}


//-------------------------------------
// ファイルタブを復元する
//
bool YavLoadFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	bool ret = false;


	/*---- 読み出し ----*/
	HANDLE hFile = CreateFileW( filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		if( !errdlg )
		{
			MessageBoxW( GetParent( appData->hFileTab ), L"ファイルタブリストファイルの読み込みに失敗しました。", L"Error YAViewer FileTabList File Read.", MB_OK | MB_ICONERROR );
		}
	}
	else
	{
		DWORD size = GetFileSize( hFile, NULL ), readSize;
		wchar_t* fileData = new wchar_t[ size / sizeof( wchar_t ) ];
		ReadFile( hFile, fileData, size, &readSize, NULL );
		CloseHandle( hFile );

		
		/*---- ファイルデータチェック ----*/
		if( fileData[ 0 ] != 0xfeff )
		{
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"ファイルタブリストファイルの文字コードがUTF-16LEでありません。", L"Error YAViewer FileTabList File BOM Check.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{
			// ファイルを削除
			while( YavDelFile( appData, 0 ) ){ Sleep( 1 ); }

			// 改行コードごろに文字列を生成
			readSize /= sizeof( wchar_t );
			std::wstring buf = L"";
			for( size_t i = 1; i < readSize; i++ )
			{
				if( !(fileData[ i ] == L'\r' && fileData[ i + 1 ] == L'\n') )
				{// 改行コードでない
					buf += fileData[ i ];
				}
				else
				{// 改行コード
					// ファイルタブリストに追加
					if( buf.size() > 0 )
					{
						if( _wcsnicmp( L"file://", buf.c_str(), 7 ) == 0 )
						{// ファイル行か
							YavAddFile( appData, &(buf.c_str()[ 7 ]), false );
						}
						else if( _wcsnicmp( L"tabcolor://", buf.c_str(), 11 ) == 0 )
						{// タブカラー
							COLORREF tc = wcstoul( &(buf.c_str()[11]), NULL, 16 );
							YavSelectTabColor( appData, -1, tc, false );
						}
						else
						{// 指定の形式でない場合もファイル行と判断する
							YavAddFile( appData, buf.c_str(), false );
						}
					}

					// バッファ一時クリア
					buf.clear();
					i++;
				}
			}

			ret = true;
		}


		/*---- データ廃棄 ----*/
		delete[] fileData;
	}


	return ret;
}


//-------------------------------------
// 指定の文字列をクリップボードへ送信
//
bool YavSendClipBoardString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length )
{
	bool ret = false;

	
	/*---- クリップボード用文字列を生成 ----*/
	HGLOBAL hString = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * (length + 1) );
	if( hString )
	{
		wchar_t* str = (wchar_t*)GlobalLock( hString );
		wcsncpy_s( str, length + 1, string, length );
		GlobalUnlock( hString );


		/*---- クリップボードに文字列送信 ----*/
		if( OpenClipboard( hWnd ) )
		{
			EmptyClipboard();
			SetClipboardData( CF_UNICODETEXT, hString );
			CloseClipboard();
			ret = true;
		}
	}


	return ret;
}


//-------------------------------------
// 指定の文字列をクリップボードへ送信
//
bool YavSendClipBoardUnicodeString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length )
{
	std::wstring str;

	/*---- 参照文字変換 ----*/
	for( size_t i = 0; i < length; )
	{
		unsigned long step, refCount;
		wchar_t refChar[ 2 ];
		if( !CheckRefChar( &string[ i ], (unsigned long)(length - i), refChar, step, refCount ) )
		{
			str += string[ i ];
			i++;
		}
		else
		{
			if( refCount == 1 )
			{
				str += refChar[ 0 ];
			}
			else
			{
				str += refChar[ 0 ];
				str += refChar[ 1 ];
			}
			i += step;
		}
	}
	string = str.c_str( );

	return YavSendClipBoardString( appData, hWnd, string, length );
}


//-------------------------------------
// ファイルツリーを保存する
//
bool YavSaveFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	bool ret = false;

	if( appData )
	{
		/*---- ルート要素取得 ----*/
		std::wstring buf = L"[root start]\r\n";
		DWORD count = GetTreeRootItem( appData->hFileTree, NULL, 0 );
		if( count > 0 )
		{
			LPWSTR* pathList = new LPWSTR[ count ];
			count = GetTreeRootItem( appData->hFileTree, pathList, count );
			for( DWORD i = 0; i < count; i++ )
			{
				buf += pathList[ i ];
				buf += L"\r\n";
			}
			delete[] pathList;
		}
		buf += L"[root end]\r\n[open start]\r\n";


		/*---- 開いている要素取得 ----*/
		count = GetTreeOpenItem( appData->hFileTree, NULL, 0 );
		if( count > 0 )
		{
			LPWSTR* pathList = new LPWSTR[ count ];
			count = GetTreeOpenItem( appData->hFileTree, pathList, count );
			for( DWORD i = 0; i < count; i++ )
			{
				buf += pathList[ i ];
				buf += L"\r\n";
			}
			delete[] pathList;
		}
		buf += L"[open end]\r\n";


		/*---- 書き出し ----*/
		HANDLE hFile = CreateFileW( filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( !hFile )
		{// 書き出し用ファイルのオープンに失敗
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"ファイルツリーリストファイルの生成に失敗しました。", L"Error YAViewer FileTreeList File Create.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{// ファイルにデータ書き出し
			DWORD writeSize;
			WORD BOM = L'\xfeff';
			WriteFile( hFile, &BOM, (DWORD)(sizeof( BOM )), &writeSize, NULL );
			WriteFile( hFile, buf.c_str(), (DWORD)(sizeof( wchar_t ) * buf.size()), &writeSize, NULL );
			CloseHandle( hFile );

			ret = true;
		}
	}

	return ret;
}


//-------------------------------------
// ファイルタブを復元する
//
bool YavLoadFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	static const int MODE_IDOL = 0;
	static const int ROOT_APPEND = 1;
	static const int OPEN_NODE = 2;

	int mode = MODE_IDOL;
	bool ret = false;

	if( appData )
	{
		/*---- 読み出し ----*/
		HANDLE hFile = CreateFileW( filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile == INVALID_HANDLE_VALUE )
		{
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"ファイルツリーリストファイルの読み込みに失敗しました。", L"Error YAViewer FileTreeList File Read.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{
			DWORD size = GetFileSize( hFile, NULL ), readSize;
			wchar_t* fileData = new wchar_t[ size / sizeof( wchar_t ) ];
			ReadFile( hFile, fileData, size, &readSize, NULL );
			CloseHandle( hFile );

			
			/*---- ファイルデータチェック ----*/
			if( fileData[ 0 ] != 0xfeff )
			{
				if( !errdlg )
				{
					MessageBoxW( GetParent( appData->hFileTab ), L"ファイルツリーリストファイルの文字コードがUTF-16LEでありません。", L"Error YAViewer FileTreeList File BOM Check.", MB_OK | MB_ICONERROR );
				}
			}
			else
			{
				// ツリーを一旦廃棄
				TreeDeleteAll( appData->hFileTree );

				// 改行コードごとに文字列を生成
				readSize /= sizeof( wchar_t );
				std::wstring buf = L"";
				for( size_t i = 1; i < readSize; i++ )
				{
					if( !(fileData[ i ] == L'\r' && fileData[ i + 1 ] == L'\n') )
					{// 改行コードでない
						buf += fileData[ i ];
					}
					else
					{// 改行コード
						// ファイルタブリストに追加
						if( buf.size() > 0 )
						{
							if( buf == L"[root start]" )
							{
								mode = ROOT_APPEND;
							}
							else if( buf == L"[open start]" )
							{
								mode = OPEN_NODE;
							}
							else if( buf == L"[root end]" )
							{
								mode = MODE_IDOL;
							}
							else if( buf == L"[open end]" )
							{
								mode = MODE_IDOL;
							}
							else
							{
								switch( mode )
								{
								case ROOT_APPEND:
									TreeAddFilePath( appData->hFileTree, buf.c_str(), true );
									break;
								case OPEN_NODE:
									TreeOpenDirectory( appData->hFileTree, buf.c_str(), false );
									break;
								}
							}
						}

						// バッファ一時クリア
						buf.clear();
						i++;
					}
				}
			}
		}
	}

	return ret;
}


//-------------------------------------
// 指定の文字列をエディタへ送信
//
bool YavSendEditorString( RegYavAppData* appData, DWORD target, LPCWSTR string, size_t length )
{
	bool ret = false;
	std::wstring str;


	if( (target & SST_EDITOR_MASK) != SST_ORINRIN_EDITOR )
	{
		str = string;
	}
	else
	{
		/*---- 参照文字変換 ----*/
		for( size_t i = 0; i < length; )
		{
			unsigned long step, refCount;
			wchar_t refChar[ 2 ];
			if( !CheckRefChar( &string[ i ], (unsigned long)(length-i), refChar, step, refCount ) )
			{
				str += string[ i ];
				i++;
			}
			else
			{				
				if( refCount == 1 )
				{
					str += refChar[ 0 ];
				}
				else
				{
					str += refChar[ 0 ];
					str += refChar[ 1 ];
				}
				i += step;
			}
		}
	}
	string = str.c_str();


	/*---- 送信先ウィンドウの取得 ----*/
	HWND hTarget = getSendStringTarget( target );
	if( hTarget )
	{
		/*---- クリップボード用文字列を生成 ----*/
		HGLOBAL hString = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * (length + 1) );
		if( hString )
		{
			wchar_t* str = (wchar_t*)GlobalLock( hString );
			wcsncpy_s( str, length + 1, string, length );
			GlobalUnlock( hString );
		}


		/*---- クリップボードにデータを送信 ----*/
		SendEditPasteParam param = { target & SST_EDITOR_MASK, target & SST_OPTION_MASK };
		ret = SendClipString( hTarget, hString, SendEditPaste, (LPARAM)&param );
	}

	return ret;
}


//-------------------------------------
// 実行ファイルディレクトリ設定/取得
//
void YavSetAppDirectory( LPCWSTR appDirectory )
{
	if( appDirectory )
	{
		wcscpy_s( sg_AppDirectoryPath, appDirectory );
	}
}
LPCWSTR YavGetAppDirectory( void )
{
	return sg_AppDirectoryPath;
}


//-------------------------------------
// 設定ファイル保存/読み込み
//
void YavSaveAppSetting( RegYavAppData* appData, LPCWSTR settingfile )
{
	if( appData )
	{
		/*---- ファイルパス生成 ----*/
		wchar_t path[ MAX_PATH ];
		setSettingFilePath( settingfile, path, MAX_PATH );


		/*---- ウィンドウ情報書き出し ----*/
		writeWindowInitData( appData, path );


		/*---- 設定情報書き出し ----*/
		writeSettingInitData( appData, path );
	}
}
void YavLoadAppSetting( RegYavAppData* appData, LPCWSTR settingfile )
{
	if( appData )
	{
		/*---- メンバクリア ----*/
		ZeroMemory( appData, sizeof( RegYavAppData ) );


		/*---- ファイルパス生成 ----*/
		wchar_t path[ MAX_PATH ];
		setSettingFilePath( settingfile, path, MAX_PATH );

		
		/*---- ウィンドウ情報取得 ----*/
		readWindowInitData( appData, path );


		/*---- 設定情報取得 ----*/
		readSettingInitData( appData, path );
	}
}


//-------------------------------------
// ファイルタブにファイルを挿入
//
static bool insertFileTabItem( HWND hTabWnd, aafile::AAFile* file, const wchar_t* path, bool selCursor )
{
	bool ret = false;

	if( hTabWnd && file && path )
	{
		/*---- パラメータを作る ----*/
		YavFileTabParam* param = new YavFileTabParam;
		ZeroMemory( param, sizeof( YavFileTabParam ) );
		param->file = file;
		wcscpy_s( param->filePath, path );
		param->flags = 0;


		/*---- ファイルアイテムを作る ----*/
		TCITEMW	item;
		ZeroMemory( &item, sizeof( item ) );
		item.mask = (TCIF_TEXT | TCIF_PARAM);
		item.pszText = PathFindFileNameW( param->filePath );
		item.lParam = (LPARAM)param;


		/*---- タブアイテムを終端に追加 ----*/
		int insertPos = TabCtrl_GetItemCount( hTabWnd );
		TabCtrl_InsertItem( hTabWnd, insertPos, &item );
		if( selCursor )
		{
			TabCtrl_SetCurSel( hTabWnd, insertPos );
		}

		ret = true;
	}

	return ret;
}


//-------------------------------------
// ファイルタブからファイルを削除
//
static aafile::AAFile* eraseFileTabItem( HWND hTabWnd, int idx )
{
	aafile::AAFile* ret = NULL;

	if( hTabWnd )
	{
		/*---- アイテムを取得する ----*/
		TCITEMW	item;
		ZeroMemory( &item, sizeof( item ) );
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem( hTabWnd, idx, &item );


		/*---- タブアイテムを削除 ----*/
		YavFileTabParam* param = (YavFileTabParam*)item.lParam;
		if( TabCtrl_DeleteItem( hTabWnd, idx ) )
		{
			ret = param->file;
			delete param;
		}
	}

	return ret;
}


//------------------------------------
// 文字列送信先のウィンドウハンドルを取得
//
static HWND getSendStringTarget( DWORD target )
{
	HWND ret = NULL, hTopWnd = NULL;
	UINT wnd = target & SST_EDITOR_MASK;


	switch( wnd )
	{
	case SST_FACE_EDIT:
		if( (hTopWnd = GetWindowHandle( NULL, L"（´д｀）Edit", L"TEditFrame" )) != NULL )
		{
			ret = GetWindowHandle( GetWindow( hTopWnd, GW_CHILD ), NULL, L"TChonEdit" );
		}
		break;
	case SST_ORINRIN_EDITOR:
		if( (hTopWnd = GetWindowHandle( NULL, L"OrinrinEditor", L"ORINRINEDITOR" )) != NULL )
		{
			ret = GetWindowHandle( GetWindow( hTopWnd, GW_CHILD ), NULL, L"EDIT_VIEW" );
		}
		break;
	case SST_SAKURA_EDITOR:
		if( (hTopWnd = GetWindowHandle( NULL, L"OrinrinEditor", L"ORINRINEDITOR" )) != NULL )
		{
			ret = GetWindowHandle( GetWindow( hTopWnd, GW_CHILD ), NULL, L"EDIT_VIEW" );
		}
		break;
	}


	return ret;
}


//------------------------------------
// 設定ファイルパスを生成
//
static void setSettingFilePath( LPCWSTR path, LPWSTR filePath, size_t length )
{
	if( path == NULL )
	{
		swprintf_s( filePath, length, L"%s\\%s", sg_AppDirectoryPath, sgc_DefaultSettingPath );
	}
	else
	{
		wcsncpy_s( filePath, length, path, length );
		GetFullPathNameW( filePath, (DWORD)length, filePath, NULL );
	}
}



//------------------------------------------------------------------------------------------------
//
// 設定入出力関数
//
//------------------------------------------------------------------------------------------------
BOOL WritePrivateProfileIntW( LPCWSTR lpAppName, LPCWSTR lpKeyName, int value, LPCWSTR lpFileName )
{
	wchar_t str[ 260 ];
	wsprintf( str, L"%d", value );
	return WritePrivateProfileStringW( lpAppName, lpKeyName, str, lpFileName );
}

//------------------------------------
// ウィンドウ初期化情報書き出し
//
static void writeWindowInitData( RegYavAppData* appData, LPCWSTR settingfile )
{
	WritePrivateProfileIntW( L"YAView.Window", L"x",			appData->x, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"y",			appData->y, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"width",		appData->width, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"height",		appData->height, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"splitPos",		appData->splitPos, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"reverseSplit",	appData->reverseSplit, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"viewMode",		appData->viewMode, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"toolHint",		appData->toolHint, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"tabButtonMode",appData->tabButtonMode, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"categorySize",	appData->categorySize, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"doubleClick",	appData->dblclickMode, settingfile );
	WritePrivateProfileIntW( L"YAView.Window", L"shortcutSize",	appData->shortcutSize, settingfile );
}

//------------------------------------
// 設定初期化情報書き出し
//
static void writeSettingInitData( RegYavAppData* appData, LPCWSTR settingfile )
{
	// 送信先
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.clipUnicodeCopy", appData->clipUnicodeCopy, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.left", appData->sendTargetLeft, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.right", appData->sendTargetRight, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.center", appData->sendTargetCenter, settingfile );

	// フォント関係
	WritePrivateProfileStringW( L"YAView.Setting", L"font.name", appData->fontName, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"font.size", appData->fontSize, settingfile );

	// 初期化情報関係
	WritePrivateProfileStringW( L"YAView.Setting", L"tab.initfile", appData->initTabPath, settingfile );
	WritePrivateProfileStringW( L"YAView.Setting", L"tree.initfile", appData->initTreePath, settingfile );

	// 有効な拡張子
	WritePrivateProfileIntW( L"YAView.Setting", L"tree.extension", appData->enableExtension, settingfile );
}

//------------------------------------
// ウィンドウ初期化情報読み出し
//
static void readWindowInitData( RegYavAppData* appData, LPCWSTR settingfile )
{
	appData->x				= GetPrivateProfileIntW( L"YAView.Window", L"x",			CW_USEDEFAULT, settingfile );
	appData->y				= GetPrivateProfileIntW( L"YAView.Window", L"y",			CW_USEDEFAULT, settingfile );
	appData->width			= GetPrivateProfileIntW( L"YAView.Window", L"width",		CW_USEDEFAULT, settingfile );
	appData->height			= GetPrivateProfileIntW( L"YAView.Window", L"height",		CW_USEDEFAULT, settingfile );
	appData->splitPos		= GetPrivateProfileIntW( L"YAView.Window", L"splitPos",		-1, settingfile );
	appData->reverseSplit	= GetPrivateProfileIntW( L"YAView.Window", L"reverseSplit",	false, settingfile );
	appData->viewMode		= GetPrivateProfileIntW( L"YAView.Window", L"viewMode",		0, settingfile );
	appData->toolHint		= GetPrivateProfileIntW( L"YAView.Window", L"toolHint",		1, settingfile );
	appData->tabButtonMode	= GetPrivateProfileIntW( L"YAView.Window", L"tabButtonMode",1, settingfile );
	appData->categorySize	= GetPrivateProfileIntW( L"YAView.Window", L"categorySize",	128, settingfile );
	appData->dblclickMode	= GetPrivateProfileIntW( L"YAView.Window", L"doubleClick",	0, settingfile );
	appData->shortcutSize	= GetPrivateProfileIntW( L"YAView.Window", L"shortcutSize",	-1, settingfile );
	
	// 強制的なステータスの書き換え
	appData->tabButtonMode	= 1;
}

//------------------------------------
// 設定初期化情報読み出し
//
static void readSettingInitData( RegYavAppData* appData, LPCWSTR settingfile )
{
	// 送信先
	appData->clipUnicodeCopy	= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.clipUnicodeCopy", 0, settingfile );
	appData->sendTargetLeft		= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.left",	SST_FACE_EDIT, settingfile );
	appData->sendTargetCenter	= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.center",	SST_FACE_EDIT | SST_TEXTBOX_FLAG, settingfile );
	appData->sendTargetRight	= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.right",	SST_CLIPBOARD, settingfile );

	// フォント関係
	GetPrivateProfileStringW( L"YAView.Setting", L"font.name", L"ＭＳ Ｐゴシック", appData->fontName, sizeof(appData->fontName)/sizeof(wchar_t), settingfile );
	appData->fontSize = GetPrivateProfileIntW( L"YAView.Setting", L"font.size", 16, settingfile );

	// 初期化情報関係
	GetPrivateProfileStringW( L"YAView.Setting", L"tab.initfile", L"temporary\\YavTemporaryTab.ytd", appData->initTabPath, MAX_PATH, settingfile );
	GetPrivateProfileStringW( L"YAView.Setting", L"tree.initfile", L"temporary\\YavTemporaryTree.ytr", appData->initTreePath, MAX_PATH, settingfile );

	// 有効な拡張子
	appData->enableExtension = GetPrivateProfileIntW( L"YAView.Setting", L"tree.extension", TITEM_EXT_MLT|TITEM_EXT_AST|TITEM_EXT_ASD, settingfile );
}
