/*----------------------------------------------------------------------------------------
*
* YAViewer �f�[�^���M�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//----------------------------------------------
// �^��`
//
// SendClipString�̎����암�����荞��
typedef bool (*SendClipStringProc)( HWND, LPARAM );


//----------------------------------------------
// �L�[�X�g���[�N�𑗂�
//
void SendKeyStroke( HWND hWnd, BYTE* vkList, int count );


//----------------------------------------------
// �E�B���h�E����������
//
HWND GetWindowHandle( HWND hParent, LPCWSTR text, LPCWSTR clsName );


//----------------------------------------------
// �N���b�v�{�[�h���o�R���ăf�[�^�𑗐M����
//
bool SendClipString( HWND hWnd, HANDLE hString, SendClipStringProc proc, LPARAM lParam );