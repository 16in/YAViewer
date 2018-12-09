/*----------------------------------------------------------------------------------------
*
* YAViewer �_�C�A���O�֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVDialog.h"
#include "YAVAppAction.h"
#include <string>


//----------------------------------------------
// AA�֌W�t�@�C�����J���_�C�A���O
//
bool OpenAAFileDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- �t�B���^���� ----*/
	LPCWSTR filter =
		L"AA�֘A�t�@�C��(*.txt;*.mlt;*.ast;*.asd)\0" L"*.txt;*.mlt;*.ast;*.asd\0"
		L"�e�L�X�g�t�@�C��(*.txt)\0"	L"*.txt\0"
		L"MLT�t�@�C��(*.mlt)\0"			L"*.mlt\0"
		L"AST�t�@�C��(*.ast)\0"			L"*.ast\0"
		L"ASD�t�@�C��(*.asd)\0"			L"*.asd\0"
		L"�S�Ẵt�@�C��(*.*)\0"		L"*.*\0\0";
	

	/*---- �_�C�A���O���J�� ----*/
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
	of.lpstrTitle		= L"�t�@�C�����J��";
	of.lpstrDefExt		= L"mlt";

	if( GetOpenFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// �^�u��ۑ��_�C�A���O
//
bool SaveFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- �t�B���^���� ----*/
	LPCWSTR filter = L"�^�u�����t�@�C��(*.ytd)\0" L"*.ytd\0\0";
	

	/*---- �_�C�A���O���J�� ----*/
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
	of.lpstrTitle		= L"�^�u�����f�[�^��ۑ�";
	of.lpstrDefExt		= L"ytd";

	if( GetSaveFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// �^�u�𕜌��_�C�A���O
//
bool OpenFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- �t�B���^���� ----*/
	LPCWSTR filter = L"�^�u�����t�@�C��(*.ytd)\0" L"*.ytd\0\0";
	

	/*---- �_�C�A���O���J�� ----*/
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
	of.lpstrTitle		= L"�^�u�����f�[�^���J��";
	of.lpstrDefExt		= L"ytd";

	if( GetOpenFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// �c���[��ۑ��_�C�A���O
//
bool SaveFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- �t�B���^���� ----*/
	LPCWSTR filter = L"�c���[�����t�@�C��(*.ytr)\0" L"*.ytr\0\0";
	

	/*---- �_�C�A���O���J�� ----*/
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
	of.lpstrTitle		= L"�c���[�����f�[�^��ۑ�";
	of.lpstrDefExt		= L"ytr";

	if( GetSaveFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// �c���[�𕜌��_�C�A���O
//
bool OpenFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength )
{
	bool ret = false;


	/*---- �t�B���^���� ----*/
	LPCWSTR filter = L"�c���[�����t�@�C��(*.ytr)\0" L"*.ytr\0\0";
	

	/*---- �_�C�A���O���J�� ----*/
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
	of.lpstrTitle		= L"�c���[�����f�[�^���J��";
	of.lpstrDefExt		= L"ytr";

	if( GetOpenFileNameW( &of ) )
	{
		wcsncpy_s( path, pathLength, filePath, pathLength );
		ret = true;
	}


	return ret;
}


//----------------------------------------------
// �^�u�J���[�_�C�A���O
//
bool SelectTabColorDialog( HWND hOwner, COLORREF* color )
{
	static COLORREF custColors[ 16 ];


	/*---- �����`�F�b�N ----*/
	if( !color ) return false;


	/*---- �_�C�A���O�ݒ� ----*/
	CHOOSECOLORW ccs;
	ZeroMemory( &ccs, sizeof( ccs ) );

	ccs.lStructSize = sizeof( ccs );
	ccs.hwndOwner	= hOwner;
	ccs.lpCustColors= custColors;
	ccs.rgbResult	= *color;
	ccs.Flags		= CC_FULLOPEN | CC_RGBINIT;


	/*---- �_�C�A���O���J�� ----*/
	if( ChooseColorW( &ccs ) == FALSE ) return false;
	*color = ccs.rgbResult;


	return true;
}



