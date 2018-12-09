/*----------------------------------------------------------------------------------------
*
* YAViewer ���M��I���֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//------------------------------------------------------------------------------
//
//	�萔��`
//
//------------------------------------------------------------------------------
// ���M���`
//
enum SendSelector
{
	SendTargetLeftClick = 0,			// ���N���b�N
	SendTargetCenterClick,				// ���N���b�N
	SendTargetMax						// �ő�l
};


//------------------------------------------------------------------------------
//
//	�֐����^�錾
//
//------------------------------------------------------------------------------
//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterSelectorWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// ���M��I���E�B���h�E����
//
HWND CreateSelectorWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// ���M��I���E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK SelectorWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// ���M�� �ݒ�/�擾
//
void SetSelectSendSelector( HWND hWnd, SendSelector st, DWORD target );
DWORD GetSelectSendSelector( HWND hWnd, SendSelector st );


//----------------------------------------------
// �I����� �ݒ�/�擾
//
void SetSelectDoubleClick( HWND hWnd, bool dblclick );
bool GetSelectDoubleClick( HWND hWnd );
