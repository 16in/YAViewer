/*----------------------------------------------------------------------------------------
*
* YAViewer �A�v���P�[�V��������֌W
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
// �O���[�o���ϐ�
//
static wchar_t			sg_AppDirectoryPath[ MAX_PATH ] = L"";
static const wchar_t*	sgc_DefaultSettingPath = L"YavSetting.ini";


//-------------------------------------
// �O���[�o���֐�
//
static bool insertFileTabItem( HWND hTabWnd, aafile::AAFile* file, const wchar_t* path, bool selCursor );
static aafile::AAFile* eraseFileTabItem( HWND hTabWnd, int idx );
static HWND getSendStringTarget( DWORD target );
static void setSettingFilePath( LPCWSTR path, LPWSTR filePath, size_t length );


//-------------------------------------
// �ݒ���o�͊֐�
//
static void writeWindowInitData( RegYavAppData* appData, LPCWSTR settingfile );
static void writeSettingInitData( RegYavAppData* appData, LPCWSTR settingfile );
static void readWindowInitData( RegYavAppData* appData, LPCWSTR settingfile );
static void readSettingInitData( RegYavAppData* appData, LPCWSTR settingfile );


//-------------------------------------
// �G�f�B�^�ɑ΂���f�[�^���M����
//
struct SendEditPasteParam
{
	DWORD	target;			// ���M�Ώۂ̎��
	DWORD	option;			// ���M�I�v�V����
};
static bool SendEditPaste( HWND hWnd, LPARAM lParam )
{
	bool ret = false;


	/*---- �^�[�Q�b�g�ƃI�v�V�������Ƃɏ����𕪂��� ----*/
	SendEditPasteParam* sendParam = (SendEditPasteParam*)lParam;
	switch( sendParam->option )
	{
	case 0:// �ʏ�\��t��
		{
			BYTE list[ 2 ] = { VK_CONTROL, 'V' };
			SendKeyStroke( hWnd, list, 2 );
		}
		ret = true;
		break;

	case SST_TEXTBOX_FLAG:
		switch( sendParam->target )
		{
		case SST_FACE_EDIT:// (�L�t�M)Edit
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
// �t�@�C���^�u�Ƀt�@�C����ǉ�����
//
bool YavAddFile( RegYavAppData* appData, LPCWSTR filePath, bool setView )
{
	bool ret = false;

	if( appData )
	{
		int idx = YavGetFileTabIndex( appData, filePath );

		if( idx >= 0 )
		{
			/*---- �ǂݍ��ݍς݃t�@�C���I�� ----*/
			if( setView )
			{
				ret = YavSelectFile( appData, idx );
			}
		}
		else
		{
			/*---- �ǉ��t�@�C���ǂݍ��� ----*/
			aafile::AAFile* file = aafile::AAFileReader::CreateAAFileFromFile( filePath );
			if( file )
			{
				/*---- �t�@�C���^�u�Ƀf�[�^�ǉ� ----*/
				ret = insertFileTabItem( appData->hFileTab, file, filePath, setView );


				/*---- �r���[�ɃZ�b�g����ꍇ�͍X�V���� ----*/
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
// �t�@�C���^�u����t�@�C�����폜����
//
bool YavDelFile( RegYavAppData* appData, int idx )
{
	bool ret = false;

	if( appData )
	{
		/*---- �t�@�C���^�u�Ƀf�[�^�ǉ� ----*/
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
// �t�@�C���^�u����t�@�C������������
//
int YavGetFileTabIndex( RegYavAppData* appData, LPCWSTR filePath )
{
	int ret = -1;

	if( appData )
	{
		int count = TabCtrl_GetItemCount( appData->hFileTab );


		/*---- �A�C�e����擪���瑖�����A�`�F�b�N���Ă��� ----*/
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
// �t�@�C���^�u����t�@�C�����擾����
//
bool YavGetFile( RegYavAppData* appData, int idx, LPWSTR filePath, size_t length )
{
	bool ret = false;

	if( appData )
	{
		/*---- �w��̃C���f�b�N�X�̃A�C�e���擾 ----*/
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
// �t�@�C���^�u�o�^�ς݃t�@�C�������擾����
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
// �t�@�C���^�u����t�@�C����I������
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
// �t�@�C���^�u�J���[��ݒ�/�擾����
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
// �t�@�C���^�u��ۑ�����
//
bool YavSaveFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	bool ret = false;


	if( appData )
	{
		/*---- �o�͐�ݒ� ----*/
		wchar_t dir[ MAX_PATH ];
		wcscpy_s( dir, filePath );
		PathRemoveFileSpecW( dir );
		if( !::PathIsDirectoryW( dir ) )
		{
			::CreateDirectoryW( dir, NULL );
		}


		/*---- �����o���f�[�^���� ----*/
		std::wstring buf = L"";
		for( int i = 0; i < YavGetFileCount( appData ); i++ )
		{
			// �t�@�C���p�X
			wchar_t path[ MAX_PATH ];
			YavGetFile( appData, i, path, MAX_PATH );
			buf += L"File://";
			buf += path;
			buf += L"\r\n";

			// �^�u�F
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


		/*---- �����o�� ----*/
		HANDLE hFile = CreateFileW( filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( !hFile )
		{// �����o���p�t�@�C���̃I�[�v���Ɏ��s
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"�t�@�C���^�u���X�g�t�@�C���̐����Ɏ��s���܂����B", L"Error YAViewer FileTabList File Create.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{// �t�@�C���Ƀf�[�^�����o��
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
// �t�@�C���^�u�𕜌�����
//
bool YavLoadFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	bool ret = false;


	/*---- �ǂݏo�� ----*/
	HANDLE hFile = CreateFileW( filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		if( !errdlg )
		{
			MessageBoxW( GetParent( appData->hFileTab ), L"�t�@�C���^�u���X�g�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B", L"Error YAViewer FileTabList File Read.", MB_OK | MB_ICONERROR );
		}
	}
	else
	{
		DWORD size = GetFileSize( hFile, NULL ), readSize;
		wchar_t* fileData = new wchar_t[ size / sizeof( wchar_t ) ];
		ReadFile( hFile, fileData, size, &readSize, NULL );
		CloseHandle( hFile );

		
		/*---- �t�@�C���f�[�^�`�F�b�N ----*/
		if( fileData[ 0 ] != 0xfeff )
		{
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"�t�@�C���^�u���X�g�t�@�C���̕����R�[�h��UTF-16LE�ł���܂���B", L"Error YAViewer FileTabList File BOM Check.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{
			// �t�@�C�����폜
			while( YavDelFile( appData, 0 ) ){ Sleep( 1 ); }

			// ���s�R�[�h����ɕ�����𐶐�
			readSize /= sizeof( wchar_t );
			std::wstring buf = L"";
			for( size_t i = 1; i < readSize; i++ )
			{
				if( !(fileData[ i ] == L'\r' && fileData[ i + 1 ] == L'\n') )
				{// ���s�R�[�h�łȂ�
					buf += fileData[ i ];
				}
				else
				{// ���s�R�[�h
					// �t�@�C���^�u���X�g�ɒǉ�
					if( buf.size() > 0 )
					{
						if( _wcsnicmp( L"file://", buf.c_str(), 7 ) == 0 )
						{// �t�@�C���s��
							YavAddFile( appData, &(buf.c_str()[ 7 ]), false );
						}
						else if( _wcsnicmp( L"tabcolor://", buf.c_str(), 11 ) == 0 )
						{// �^�u�J���[
							COLORREF tc = wcstoul( &(buf.c_str()[11]), NULL, 16 );
							YavSelectTabColor( appData, -1, tc, false );
						}
						else
						{// �w��̌`���łȂ��ꍇ���t�@�C���s�Ɣ��f����
							YavAddFile( appData, buf.c_str(), false );
						}
					}

					// �o�b�t�@�ꎞ�N���A
					buf.clear();
					i++;
				}
			}

			ret = true;
		}


		/*---- �f�[�^�p�� ----*/
		delete[] fileData;
	}


	return ret;
}


//-------------------------------------
// �w��̕�������N���b�v�{�[�h�֑��M
//
bool YavSendClipBoardString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length )
{
	bool ret = false;

	
	/*---- �N���b�v�{�[�h�p������𐶐� ----*/
	HGLOBAL hString = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * (length + 1) );
	if( hString )
	{
		wchar_t* str = (wchar_t*)GlobalLock( hString );
		wcsncpy_s( str, length + 1, string, length );
		GlobalUnlock( hString );


		/*---- �N���b�v�{�[�h�ɕ����񑗐M ----*/
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
// �w��̕�������N���b�v�{�[�h�֑��M
//
bool YavSendClipBoardUnicodeString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length )
{
	std::wstring str;

	/*---- �Q�ƕ����ϊ� ----*/
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
// �t�@�C���c���[��ۑ�����
//
bool YavSaveFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg )
{
	bool ret = false;

	if( appData )
	{
		/*---- ���[�g�v�f�擾 ----*/
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


		/*---- �J���Ă���v�f�擾 ----*/
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


		/*---- �����o�� ----*/
		HANDLE hFile = CreateFileW( filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( !hFile )
		{// �����o���p�t�@�C���̃I�[�v���Ɏ��s
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"�t�@�C���c���[���X�g�t�@�C���̐����Ɏ��s���܂����B", L"Error YAViewer FileTreeList File Create.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{// �t�@�C���Ƀf�[�^�����o��
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
// �t�@�C���^�u�𕜌�����
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
		/*---- �ǂݏo�� ----*/
		HANDLE hFile = CreateFileW( filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile == INVALID_HANDLE_VALUE )
		{
			if( !errdlg )
			{
				MessageBoxW( GetParent( appData->hFileTab ), L"�t�@�C���c���[���X�g�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B", L"Error YAViewer FileTreeList File Read.", MB_OK | MB_ICONERROR );
			}
		}
		else
		{
			DWORD size = GetFileSize( hFile, NULL ), readSize;
			wchar_t* fileData = new wchar_t[ size / sizeof( wchar_t ) ];
			ReadFile( hFile, fileData, size, &readSize, NULL );
			CloseHandle( hFile );

			
			/*---- �t�@�C���f�[�^�`�F�b�N ----*/
			if( fileData[ 0 ] != 0xfeff )
			{
				if( !errdlg )
				{
					MessageBoxW( GetParent( appData->hFileTab ), L"�t�@�C���c���[���X�g�t�@�C���̕����R�[�h��UTF-16LE�ł���܂���B", L"Error YAViewer FileTreeList File BOM Check.", MB_OK | MB_ICONERROR );
				}
			}
			else
			{
				// �c���[����U�p��
				TreeDeleteAll( appData->hFileTree );

				// ���s�R�[�h���Ƃɕ�����𐶐�
				readSize /= sizeof( wchar_t );
				std::wstring buf = L"";
				for( size_t i = 1; i < readSize; i++ )
				{
					if( !(fileData[ i ] == L'\r' && fileData[ i + 1 ] == L'\n') )
					{// ���s�R�[�h�łȂ�
						buf += fileData[ i ];
					}
					else
					{// ���s�R�[�h
						// �t�@�C���^�u���X�g�ɒǉ�
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

						// �o�b�t�@�ꎞ�N���A
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
// �w��̕�������G�f�B�^�֑��M
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
		/*---- �Q�ƕ����ϊ� ----*/
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


	/*---- ���M��E�B���h�E�̎擾 ----*/
	HWND hTarget = getSendStringTarget( target );
	if( hTarget )
	{
		/*---- �N���b�v�{�[�h�p������𐶐� ----*/
		HGLOBAL hString = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * (length + 1) );
		if( hString )
		{
			wchar_t* str = (wchar_t*)GlobalLock( hString );
			wcsncpy_s( str, length + 1, string, length );
			GlobalUnlock( hString );
		}


		/*---- �N���b�v�{�[�h�Ƀf�[�^�𑗐M ----*/
		SendEditPasteParam param = { target & SST_EDITOR_MASK, target & SST_OPTION_MASK };
		ret = SendClipString( hTarget, hString, SendEditPaste, (LPARAM)&param );
	}

	return ret;
}


//-------------------------------------
// ���s�t�@�C���f�B���N�g���ݒ�/�擾
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
// �ݒ�t�@�C���ۑ�/�ǂݍ���
//
void YavSaveAppSetting( RegYavAppData* appData, LPCWSTR settingfile )
{
	if( appData )
	{
		/*---- �t�@�C���p�X���� ----*/
		wchar_t path[ MAX_PATH ];
		setSettingFilePath( settingfile, path, MAX_PATH );


		/*---- �E�B���h�E��񏑂��o�� ----*/
		writeWindowInitData( appData, path );


		/*---- �ݒ��񏑂��o�� ----*/
		writeSettingInitData( appData, path );
	}
}
void YavLoadAppSetting( RegYavAppData* appData, LPCWSTR settingfile )
{
	if( appData )
	{
		/*---- �����o�N���A ----*/
		ZeroMemory( appData, sizeof( RegYavAppData ) );


		/*---- �t�@�C���p�X���� ----*/
		wchar_t path[ MAX_PATH ];
		setSettingFilePath( settingfile, path, MAX_PATH );

		
		/*---- �E�B���h�E���擾 ----*/
		readWindowInitData( appData, path );


		/*---- �ݒ���擾 ----*/
		readSettingInitData( appData, path );
	}
}


//-------------------------------------
// �t�@�C���^�u�Ƀt�@�C����}��
//
static bool insertFileTabItem( HWND hTabWnd, aafile::AAFile* file, const wchar_t* path, bool selCursor )
{
	bool ret = false;

	if( hTabWnd && file && path )
	{
		/*---- �p�����[�^����� ----*/
		YavFileTabParam* param = new YavFileTabParam;
		ZeroMemory( param, sizeof( YavFileTabParam ) );
		param->file = file;
		wcscpy_s( param->filePath, path );
		param->flags = 0;


		/*---- �t�@�C���A�C�e������� ----*/
		TCITEMW	item;
		ZeroMemory( &item, sizeof( item ) );
		item.mask = (TCIF_TEXT | TCIF_PARAM);
		item.pszText = PathFindFileNameW( param->filePath );
		item.lParam = (LPARAM)param;


		/*---- �^�u�A�C�e�����I�[�ɒǉ� ----*/
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
// �t�@�C���^�u����t�@�C�����폜
//
static aafile::AAFile* eraseFileTabItem( HWND hTabWnd, int idx )
{
	aafile::AAFile* ret = NULL;

	if( hTabWnd )
	{
		/*---- �A�C�e�����擾���� ----*/
		TCITEMW	item;
		ZeroMemory( &item, sizeof( item ) );
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem( hTabWnd, idx, &item );


		/*---- �^�u�A�C�e�����폜 ----*/
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
// �����񑗐M��̃E�B���h�E�n���h�����擾
//
static HWND getSendStringTarget( DWORD target )
{
	HWND ret = NULL, hTopWnd = NULL;
	UINT wnd = target & SST_EDITOR_MASK;


	switch( wnd )
	{
	case SST_FACE_EDIT:
		if( (hTopWnd = GetWindowHandle( NULL, L"�i�L�t�M�jEdit", L"TEditFrame" )) != NULL )
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
// �ݒ�t�@�C���p�X�𐶐�
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
// �ݒ���o�͊֐�
//
//------------------------------------------------------------------------------------------------
BOOL WritePrivateProfileIntW( LPCWSTR lpAppName, LPCWSTR lpKeyName, int value, LPCWSTR lpFileName )
{
	wchar_t str[ 260 ];
	wsprintf( str, L"%d", value );
	return WritePrivateProfileStringW( lpAppName, lpKeyName, str, lpFileName );
}

//------------------------------------
// �E�B���h�E��������񏑂��o��
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
// �ݒ菉������񏑂��o��
//
static void writeSettingInitData( RegYavAppData* appData, LPCWSTR settingfile )
{
	// ���M��
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.clipUnicodeCopy", appData->clipUnicodeCopy, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.left", appData->sendTargetLeft, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.right", appData->sendTargetRight, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"send.target.center", appData->sendTargetCenter, settingfile );

	// �t�H���g�֌W
	WritePrivateProfileStringW( L"YAView.Setting", L"font.name", appData->fontName, settingfile );
	WritePrivateProfileIntW( L"YAView.Setting", L"font.size", appData->fontSize, settingfile );

	// ���������֌W
	WritePrivateProfileStringW( L"YAView.Setting", L"tab.initfile", appData->initTabPath, settingfile );
	WritePrivateProfileStringW( L"YAView.Setting", L"tree.initfile", appData->initTreePath, settingfile );

	// �L���Ȋg���q
	WritePrivateProfileIntW( L"YAView.Setting", L"tree.extension", appData->enableExtension, settingfile );
}

//------------------------------------
// �E�B���h�E���������ǂݏo��
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
	
	// �����I�ȃX�e�[�^�X�̏�������
	appData->tabButtonMode	= 1;
}

//------------------------------------
// �ݒ菉�������ǂݏo��
//
static void readSettingInitData( RegYavAppData* appData, LPCWSTR settingfile )
{
	// ���M��
	appData->clipUnicodeCopy	= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.clipUnicodeCopy", 0, settingfile );
	appData->sendTargetLeft		= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.left",	SST_FACE_EDIT, settingfile );
	appData->sendTargetCenter	= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.center",	SST_FACE_EDIT | SST_TEXTBOX_FLAG, settingfile );
	appData->sendTargetRight	= GetPrivateProfileIntW( L"YAView.Setting", L"send.target.right",	SST_CLIPBOARD, settingfile );

	// �t�H���g�֌W
	GetPrivateProfileStringW( L"YAView.Setting", L"font.name", L"�l�r �o�S�V�b�N", appData->fontName, sizeof(appData->fontName)/sizeof(wchar_t), settingfile );
	appData->fontSize = GetPrivateProfileIntW( L"YAView.Setting", L"font.size", 16, settingfile );

	// ���������֌W
	GetPrivateProfileStringW( L"YAView.Setting", L"tab.initfile", L"temporary\\YavTemporaryTab.ytd", appData->initTabPath, MAX_PATH, settingfile );
	GetPrivateProfileStringW( L"YAView.Setting", L"tree.initfile", L"temporary\\YavTemporaryTree.ytr", appData->initTreePath, MAX_PATH, settingfile );

	// �L���Ȋg���q
	appData->enableExtension = GetPrivateProfileIntW( L"YAView.Setting", L"tree.extension", TITEM_EXT_MLT|TITEM_EXT_AST|TITEM_EXT_ASD, settingfile );
}
