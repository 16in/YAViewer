/*----------------------------------------------------------------------------------------
*
* YAViewer �f�[�^���M�֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVSendEditor.h"


//----------------------------------------------
// �\���̒�`
//
// enumWindowProc��LPARAM
struct enumWindowProcParam
{
	LPCWSTR		text;
	LPCWSTR		clsName;
	HWND		hResultWnd;
};


//----------------------------------------------
// �O���[�o���֐�
//
static BOOL CALLBACK enumWindowProc( HWND hWnd, LPARAM lParam );
static HWND getChildWindow( HWND hWnd, LPCWSTR text, LPCWSTR clsName );


//----------------------------------------------
// �L�[�X�g���[�N�𑗂�
//
void SendKeyStroke( HWND hWnd, BYTE* vkList, int count )
{
	if( hWnd )
	{
		SetForegroundWindow( hWnd );
	}
	if( vkList )
	{
		for( int i = 0; i < count; i++ )
		{
			Sleep( 5 );
			keybd_event( vkList[ i ], 0, 0, 0 );
		}
		for( int i = count - 1; i >= 0; i-- )
		{
			Sleep( 5 );
			keybd_event( vkList[ i ], 0, KEYEVENTF_KEYUP, 0 );
		}
	}
}


//----------------------------------------------
// �E�B���h�E����������
//
HWND GetWindowHandle( HWND hParent, LPCWSTR text, LPCWSTR clsName )
{
	HWND ret = NULL;

	if( !hParent )
	{
		/*---- �g�b�v���x���E�B���h�E�̌��� ----*/
		enumWindowProcParam param;
		ZeroMemory( &param, sizeof( param ) );
		param.text = text; param.clsName = clsName;
		EnumWindows( enumWindowProc, (LPARAM)&param );
		ret = param.hResultWnd;
	}
	else
	{
		/*---- �q�E�B���h�E�̌��� ----*/
		ret = getChildWindow( hParent, text, clsName );
	}


	return ret;
}


//----------------------------------------------
// �N���b�v�{�[�h���o�R���ăf�[�^�𑗐M����
//
bool SendClipString( HWND hWnd, HANDLE hString, SendClipStringProc proc, LPARAM lParam )
{
	bool ret = false;

	if( hWnd && hString && proc )
	{
		/*---- �L���b�V�����E���ăN���b�v�{�[�h�̃f�[�^������������ ----*/
		HANDLE hChash = NULL;
		if( OpenClipboard( hWnd ) )
		{
			// �L���b�V�����m�ۂ���
			HANDLE hOldData = GetClipboardData( CF_UNICODETEXT );
			if( hOldData )
			{
				wchar_t* oldStr = (wchar_t*)GlobalLock( hOldData );
				size_t len = wcslen( oldStr ) + 1;

				hChash = GlobalAlloc( GHND | GMEM_SHARE, sizeof( wchar_t ) * len );
				wchar_t* chash = (wchar_t*)GlobalLock( hChash );
				wcscpy_s( chash, len, oldStr );
				GlobalUnlock( hChash );
				GlobalUnlock( hOldData );
			}

			// �N���b�v�{�[�h�̃f�[�^������������
			EmptyClipboard();
			SetClipboardData( CF_UNICODETEXT, hString );
			CloseClipboard();
		}


		/*---- ���荞�݂����s ----*/
		ret = proc( hWnd, lParam );


		/*---- �L���b�V�����N���b�v�{�[�h�֖߂� ----*/
		if( hChash && OpenClipboard( hWnd ) )
		{
			// �N���b�v�{�[�h���L���b�V���̒l�ɖ߂�
			EmptyClipboard();
			SetClipboardData( CF_UNICODETEXT, hChash );
			CloseClipboard();
		}
	}

	return ret;
}


// �����p
//----------------------------------------------
// �g�b�v���x���E�B���h�E����������
//
static BOOL CALLBACK enumWindowProc( HWND hWnd, LPARAM lParam )
{
	BOOL ret = TRUE;
	enumWindowProcParam* param = (enumWindowProcParam*)lParam;


	/*---- �o�b�t�@���m�� ----*/
	size_t txtLength = param->text ? wcslen( param->text ) + 1 : 1;
	size_t clsLength = param->clsName ? wcslen( param->clsName ) + 1 : 1;
	wchar_t* winText = new wchar_t[ txtLength ];
	wchar_t* clsName = new wchar_t[ clsLength ];
	
	
	/*---- �E�B���h�E�̏��𓾂� ----*/
	GetWindowTextW( hWnd, winText, txtLength );
	GetClassNameW( hWnd, clsName, clsLength );


	/*---- ���� ----*/
	if( param->text == NULL || wcscmp( param->text, winText ) == 0 )
	{
		if( param->clsName == NULL || wcscmp( param->clsName, clsName ) == 0 )
		{
			param->hResultWnd = hWnd;
			ret = FALSE;
		}
	}


	/*---- �o�b�t�@���J�� ----*/
	delete[] clsName;
	delete[] winText;


	return ret;
}


//----------------------------------------------
// �q�E�B���h�E�����Ȃ��猟�����Ă���
//
static HWND getChildWindow( HWND hWnd, LPCWSTR text, LPCWSTR clsName )
{
	HWND ret = NULL;


	/*---- �o�b�t�@���m�� ----*/
	size_t txtLength = text ? wcslen( text ) + 1 : 1;
	size_t clsLength = clsName ? wcslen( clsName ) + 1 : 1;
	wchar_t* winText = new wchar_t[ txtLength ];
	wchar_t* winclsName = new wchar_t[ clsLength ];
	
	
	/*---- �E�B���h�E�̏��𓾂� ----*/
	GetWindowTextW( hWnd, winText, txtLength );
	GetClassNameW( hWnd, winclsName, clsLength );


	/*---- ���� ----*/
	if( text == NULL || wcscmp( text, winText ) == 0 )
	{
		if( clsName == NULL || wcscmp( clsName, winclsName ) == 0 )
		{
			ret = hWnd;
		}
	}


	/*---- �o�b�t�@���J�� ----*/
	delete[] winclsName;
	delete[] winText;


	/*---- �ċA�I�ɂ������Ă����� ----*/
	if( !ret )
	{
		// �q�E�B���h�E����
		HWND nextWindow = GetWindow( hWnd, GW_CHILD );
		if( nextWindow )
		{
			ret = getChildWindow( nextWindow, text, clsName );
		}

		// �Z��E�B���h�E��
		if( !ret )
		{
			nextWindow = GetWindow( hWnd, GW_HWNDNEXT );
			if( nextWindow )
			{
				ret = getChildWindow( nextWindow, text, clsName );
			}
		}
	}


	return ret;
}