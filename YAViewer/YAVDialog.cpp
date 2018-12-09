/*----------------------------------------------------------------------------------------
*
* YAViewer ダイアログ関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVDialog.h"
#include "YAVAppAction.h"
#include <string>


//----------------------------------------------
// AA関係ファイルを開くダイアログ
//
bool OpenAAFileDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- フィルタ生成 ----*/
	LPCWSTR filter =
		L"AA関連ファイル(*.txt;*.mlt;*.ast;*.asd)\0" L"*.txt;*.mlt;*.ast;*.asd\0"
		L"テキストファイル(*.txt)\0"	L"*.txt\0"
		L"MLTファイル(*.mlt)\0"			L"*.mlt\0"
		L"ASTファイル(*.ast)\0"			L"*.ast\0"
		L"ASDファイル(*.asd)\0"			L"*.asd\0"
		L"全てのファイル(*.*)\0"		L"*.*\0\0";
	

	/*---- ダイアログを開く ----*/
	static wchar_t filePath[ MAX_PATH ], fileName[ MAX_PATH ];
	static OPENFILENAMEW of;
	ZeroMemory( &of, sizeof( of ) );
	of.lStructSize		= sizeof( of );
	of.lpstrFilter		= filter;
	of.hwndOwner		= hOwner;
	of.Flags			= OFN_FILEMUSTEXIST;
	of.lpstrFile		= filePath;
	of.lpstrFileTitle	= fileName;
	of.nMaxFile			= sizeof( filePath );
	of.nMaxFileTitle	= sizeof( fileName );
	of.lpstrTitle		= L"ファイルを開く";
	of.lpstrDefExt		= L"mlt";

	if( GetOpenFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// タブを保存ダイアログ
//
bool SaveFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- フィルタ生成 ----*/
	LPCWSTR filter = L"タブ復元ファイル(*.ytd)\0" L"*.ytd\0\0";
	

	/*---- ダイアログを開く ----*/
	static wchar_t filePath[ MAX_PATH ], fileName[ MAX_PATH ];
	static OPENFILENAMEW of;
	ZeroMemory( &of, sizeof( of ) );
	of.lStructSize		= sizeof( of );
	of.lpstrFilter		= filter;
	of.hwndOwner		= hOwner;
	of.Flags			= OFN_FILEMUSTEXIST;
	of.lpstrFile		= filePath;
	of.lpstrFileTitle	= fileName;
	of.nMaxFile			= sizeof( filePath );
	of.nMaxFileTitle	= sizeof( fileName );
	of.lpstrInitialDir	= YavGetAppDirectory();
	of.lpstrTitle		= L"タブ復元データを保存";
	of.lpstrDefExt		= L"ytd";

	if( GetSaveFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// タブを復元ダイアログ
//
bool OpenFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- フィルタ生成 ----*/
	LPCWSTR filter = L"タブ復元ファイル(*.ytd)\0" L"*.ytd\0\0";
	

	/*---- ダイアログを開く ----*/
	static wchar_t filePath[ MAX_PATH ], fileName[ MAX_PATH ];
	static OPENFILENAMEW of;
	ZeroMemory( &of, sizeof( of ) );
	of.lStructSize		= sizeof( of );
	of.lpstrFilter		= filter;
	of.hwndOwner		= hOwner;
	of.Flags			= OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	of.lpstrFile		= filePath;
	of.lpstrFileTitle	= fileName;
	of.nMaxFile			= sizeof( filePath );
	of.nMaxFileTitle	= sizeof( fileName );
	of.lpstrInitialDir	= YavGetAppDirectory();
	of.lpstrTitle		= L"タブ復元データを開く";
	of.lpstrDefExt		= L"ytd";

	if( GetOpenFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// ツリーを保存ダイアログ
//
bool SaveFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- フィルタ生成 ----*/
	LPCWSTR filter = L"ツリー復元ファイル(*.ytr)\0" L"*.ytr\0\0";
	

	/*---- ダイアログを開く ----*/
	static wchar_t filePath[ MAX_PATH ], fileName[ MAX_PATH ];
	static OPENFILENAMEW of;
	ZeroMemory( &of, sizeof( of ) );
	of.lStructSize		= sizeof( of );
	of.lpstrFilter		= filter;
	of.hwndOwner		= hOwner;
	of.Flags			= OFN_FILEMUSTEXIST;
	of.lpstrFile		= filePath;
	of.lpstrFileTitle	= fileName;
	of.nMaxFile			= sizeof( filePath );
	of.nMaxFileTitle	= sizeof( fileName );
	of.lpstrInitialDir	= YavGetAppDirectory();
	of.lpstrTitle		= L"ツリー復元データを保存";
	of.lpstrDefExt		= L"ytr";

	if( GetSaveFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// ツリーを復元ダイアログ
//
bool OpenFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- フィルタ生成 ----*/
	LPCWSTR filter = L"ツリー復元ファイル(*.ytr)\0" L"*.ytr\0\0";
	

	/*---- ダイアログを開く ----*/
	static wchar_t filePath[ MAX_PATH ], fileName[ MAX_PATH ];
	static OPENFILENAMEW of;
	ZeroMemory( &of, sizeof( of ) );
	of.lStructSize		= sizeof( of );
	of.lpstrFilter		= filter;
	of.hwndOwner		= hOwner;
	of.Flags			= OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	of.lpstrFile		= filePath;
	of.lpstrFileTitle	= fileName;
	of.nMaxFile			= sizeof( filePath );
	of.nMaxFileTitle	= sizeof( fileName );
	of.lpstrInitialDir	= YavGetAppDirectory();
	of.lpstrTitle		= L"ツリー復元データを開く";
	of.lpstrDefExt		= L"ytr";

	if( GetOpenFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// タブカラーダイアログ
//
bool SelectTabColorDialog( HWND hOwner, COLORREF* color )
{
	static COLORREF custColors[ 16 ];


	/*---- 引数チェック ----*/
	if( !color ) return false;


	/*---- ダイアログ設定 ----*/
	CHOOSECOLORW ccs;
	ZeroMemory( &ccs, sizeof( ccs ) );

	ccs.lStructSize = sizeof( ccs );
	ccs.hwndOwner	= hOwner;
	ccs.lpCustColors= custColors;
	ccs.rgbResult	= *color;
	ccs.Flags		= CC_FULLOPEN | CC_RGBINIT;


	/*---- ダイアログを開く ----*/
	if( ChooseColorW( &ccs ) == FALSE ) return false;
	*color = ccs.rgbResult;


	return true;
}



