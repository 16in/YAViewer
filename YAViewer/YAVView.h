/*----------------------------------------------------------------------------------------
*
* YAViewer View�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "AAFileAccessor.h"


//----------------------------------------------
// View�E�B���h�E����e�E�B���h�E�֑����郁�b�Z�[�W��
//	SetSendMessageRootWindow�ɐ^���w�肵���ꍇ
//	View�E�B���h�E�̃��[�g�E�B���h�E�ɑ΂��ă��b�Z�[�W�𑗂�܂��B
//
#include "YAVViewMessage.h"


//------------------------------------------------------------------------------
//
//	�}�N���A�\���̒�`
//
//------------------------------------------------------------------------------
//----------------------------------------------
// View�E�B���h�E�\���`���}�N��
//
#define VIEWMODE_NORMAL		0		/* �ʏ�\�� */
#define VIEWMODE_CATEGORY	1		/* �J�e�S���\�� */



//------------------------------------------------------------------------------
//
//	�֐����^�錾
//
//------------------------------------------------------------------------------
//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterViewWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// �r���[�E�B���h�E����
//
HWND CreateViewWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// �r���[�E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK ViewWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// �X�N���[���ʒu�ݒ�/�擾
//
void SetViewScrollPosition( HWND hViewWnd, int pos );
void SetViewScrollPositionByPage( HWND hViewWnd, int page );
int  GetViewScrollPosition( HWND hViewWnd );


//----------------------------------------------
// �\���t�H���g�ݒ�/�擾
//
void SetViewFont( HWND hViewWnd, HFONT hFont );
HFONT GetViewFont( HWND hViewWnd );


//----------------------------------------------
// �\�����[�h�ݒ�/�擾
//
void SetViewMode( HWND hViewWnd, int mode );
int GetViewMode( HWND hViewWnd );


//----------------------------------------------
// �c�[���q���g�\���ݒ�/�擾
//
void SetViewToolTip( HWND hViewWnd, bool enable );
bool GetViewToolTip( HWND hViewWnd );


//----------------------------------------------
// �J�e�S���\���T�C�Y�ݒ�/�擾
//
void SetViewCategorySize( HWND hViewWnd, int size );
int GetViewCategorySize( HWND hViewWnd );


//----------------------------------------------
// �\���t�@�C���ݒ�/�擾
//
void SetViewFile( HWND hViewWnd, aafile::AAFile* file );
aafile::AAFile* GetViewFile( HWND hViewWnd );


//----------------------------------------------
// �g�����b�Z�[�W���M��ݒ�/�擾
//
void SetViewSendMessageRootWindow( HWND hViewWnd, bool sendRoot );
bool GetViewSendMessageRootWindow( HWND hViewWnd );


//----------------------------------------------
// �Q�ƕ��������
//
bool CheckRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount );
