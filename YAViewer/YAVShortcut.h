/*----------------------------------------------------------------------------------------
*
* YAViewer �V���[�g�J�b�g�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <AAFileAccessor.h>


//------------------------------------------------------------------------------
//
//	�֐����^�錾
//
//------------------------------------------------------------------------------
//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterYAVShortcutClass( HINSTANCE hInstance );

//----------------------------------------------
// �V���[�g�J�b�g�E�B���h�E����
//
HWND CreateShortcutWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );

//----------------------------------------------
// ���b�Z�[�W�̑��M��
//
void SetShortcutSendNotifyWindow( HWND hShortcut, HWND hTarget );

//----------------------------------------------
// �V���[�g�J�b�g����
//
void SetShortcutFile( HWND hShortcut, aafile::AAFile* file );
