/*----------------------------------------------------------------------------------------
*
* YAViewer Tree�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "YAVFileTreeMessage.h"


//------------------------------------------------------------------------------
//
//	�萔��`
//
//------------------------------------------------------------------------------
//----------------------------------------------
// �g���q�t���O
//
enum TreeItemExtension
{
	TITEM_EXT_TXT	= 0x01,		// .txt
	TITEM_EXT_MLT	= 0x02,		// .mlt
	TITEM_EXT_AST	= 0x04,		// .ast
	TITEM_EXT_ASD	= 0x08,		// .asd

	TITEM_EXT_TXT_SHIFT = 0,
	TITEM_EXT_MLT_SHIFT,
	TITEM_EXT_AST_SHIFT,
	TITEM_EXT_ASD_SHIFT,

	TITEM_EXT_MAX_SHIFT,

	TITEM_EXT_ALL	= 0xffffffff
};


//------------------------------------------------------------------------------
//
//	�֐����^�錾
//
//------------------------------------------------------------------------------
//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterTreeWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// �r���[�E�B���h�E����
//
HWND CreateTreeWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// �r���[�E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK TreeWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// �g�����b�Z�[�W���M��ݒ�/�擾
//
void SetTreeSendMessageRootWindow( HWND hTreeWnd, bool sendRoot );
bool GetTreeSendMessageRootWindow( HWND hTreeWnd );


//----------------------------------------------
// �Ǘ��t�@�C���p�X�̒ǉ�
//
bool TreeAddFilePath( HWND hTreeWnd, LPCWSTR filePath, bool subdir, bool caret = false );


//----------------------------------------------
// �Ǘ��t�@�C���p�X�̍폜
//
bool TreeDeleteFilePath( HWND hTreeWnd, LPCWSTR filePath );


//----------------------------------------------
// �Ǘ��t�@�C���p�X�̍폜
//
void TreeDeleteAll( HWND hTreeWnd );


//----------------------------------------------
// �w��p�X�̃f�B���N�g�����J��
//
bool TreeOpenDirectory( HWND hTreeWnd, LPCWSTR dirPath, bool subdir );


//----------------------------------------------
// ���[�g�A�C�e���ꗗ�擾
//
DWORD GetTreeRootItem( HWND hTreeWnd, LPWSTR* paths, DWORD count );


//----------------------------------------------
// �\������Ă���A�C�e���̈ꗗ�擾
//
DWORD GetTreeOpenItem( HWND hTreeWnd, LPWSTR* paths, DWORD count );


//----------------------------------------------
// �g���q�t���O�ݒ�
//
void SetEnableTreeItemExtension( HWND hTreeWnd, DWORD enableExtension );
DWORD GetEnableTreeItemExtension( HWND hTreeWnd );
