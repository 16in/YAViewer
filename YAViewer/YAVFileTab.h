/*----------------------------------------------------------------------------------------
*
* YAViewer �t�@�C���^�u�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//------------------------------------------------------------------------------
//
//	�֐����^�錾
//
//------------------------------------------------------------------------------
//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterYAVFileTabClass( HINSTANCE hInstance );

//----------------------------------------------
// �t�@�C���^�u�E�B���h�E����
//
HWND CreateFileTabWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle, DWORD id );

//----------------------------------------------
// �t�@�C���^�u�E�B���h�E�擾
//	�q�E�B���h�E�̓o�^��
//
HWND GetFileTabWindow( HWND hWnd );

//----------------------------------------------
// �t�@�C���^�u - x�{�^����`�擾
//
bool GetFileTabCloseRect( HWND hWnd, int idx, RECT* dst );
bool GetFileTabCloseRectToItemRect( const RECT* tabItemRect, RECT* dst );

//----------------------------------------------
// �t�@�C���^�u - x�{�^���X�L�b�v����
//
bool FileTabCloseSkip( HWND hWnd, int* moveIndex );

//----------------------------------------------
// �t�@�C���^�u - �^�u�{�^�����[�h�ݒ�
//
void SetFileTabButtonMode( HWND hWnd, bool buttom );
bool GetFileTabButtonMode( HWND hWnd );
