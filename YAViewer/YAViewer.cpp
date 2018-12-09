/*----------------------------------------------------------------------------------------
*
* �A�X�L�[�A�[�g�t�@�C���r���[�A YAViewer
*
*----------------------------------------------------------------------------------------*/
#include <Windows.h>
#include <Shlwapi.h>
#include <commctrl.h>
#include "YAVWindow.h"
#include "YAVSendSelector.h"


#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "Shlwapi.lib" )


//----------------------------------------------
// �G���g���|�C���g
//
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR cmdline, int nShowCmd )
{
	RegYavAppData	appData;


	/*---- �R�����R���g���[�������� ----*/
	InitCommonControls();


	/*---- �A�v���P�[�V�����f�B���N�g���̃p�X��ݒ� ----*/
	wchar_t path[ MAX_PATH ];
	GetModuleFileNameW( NULL, path, MAX_PATH );
	PathRemoveFileSpecW( path );
	YavSetAppDirectory( path );


	/*---- ��������Ԃ�ǂݍ��� ----*/
	YavLoadAppSetting( &appData, NULL );


	/*---- �E�B���h�E�N���X�o�^ ----*/
	if( !RegisterYAVWindowClass( hInstance ) )
	{
		MessageBoxW( NULL, L"�E�B���h�E�N���X�̓o�^�Ɏ��s���܂����B", L"Error Register WindowClass", MB_OK | MB_ICONERROR );
		return 0;
	}


	/*---- �A�v���P�[�V�����E�B���h�E���� ----*/
	HWND hAppWnd = CreateYAVWindow( hInstance, &appData );
	if( !hAppWnd )
	{
		MessageBoxW( NULL, L"�E�B���h�E�̐����Ɏ��s���܂����B", L"Error CreateWindow", MB_OK | MB_ICONERROR );
		return 0;
	}
	ShowWindow( hAppWnd, nShowCmd );
	UpdateWindow( hAppWnd );


	/*---- ���b�Z�[�W���[�v ----*/
	MSG msg;
	while( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		::TranslateMessage( &msg );
		::DispatchMessageW( &msg );
	}


	/*---- �I���R�[�h���� ----*/
	if( msg.message == WM_QUIT )
	{
		YavSaveAppSetting( &appData );
		return (int)msg.wParam;
	}


	return 0;
}
