/*----------------------------------------------------------------------------------------
*
* YAViewer SplitWindow�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//----------------------------------------------
// �������[�h�萔
//
#define SPL_VERTICAL	0		/* �������� */
#define SPL_HORIZONTAL	1		/* �������� */


//----------------------------------------------
// �Œ胂�[�h�萔
//
#define SPL_LOCK_LEFT	0		/* �����Œ� */
#define SPL_LOCK_TOP	0		/* �㑤�Œ� */
#define SPL_LOCK_RIGHT	1		/* �E���Œ� */
#define SPL_LOCK_BOTTOM	1		/* �����Œ� */


//----------------------------------------------
// �����E�B���h�E�N���X�o�^
//
bool RegisterSplitWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// �����E�B���h�E����
//
HWND CreateSplitWindow( HINSTANCE hInstance, HWND hParent, DWORD addStyle );


//----------------------------------------------
// �����E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK YAVSplitWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// �����E�B���h�E���[�h�ݒ�/�擾
//
int SetSplitWindowMode( HWND hSplitWnd, int mode );
int GetSplitWindowMode( HWND hSplitWnd );


//----------------------------------------------
// �����̈��؂�T�C�Y�ݒ�/�擾
//
int SetSplitBorderWidth( HWND hSplitWnd, int width );
int GetSplitBorderWidth( HWND hSplitWnd );


//----------------------------------------------
// �����T�C�Y�ݒ�/�擾
//
int SetSplitWidth( HWND hSplitWnd, int width, bool update = true );
int GetSplitWidth( HWND hSplitWnd );
int SetSplitLeftWidth( HWND hSplitWnd, int width );
int GetSplitLeftWidth( HWND hSplitWnd );
int SetSplitTopWidth( HWND hSplitWnd, int width );
int GetSplitTopWidth( HWND hSplitWnd );


//----------------------------------------------
// �����̈���_�u���N���b�N�ŃZ���^�����O���邩�ݒ�/�擾
//
BOOL SetSplitDbClickCentering( HWND hSplitWnd, BOOL dbclkCnter );
BOOL GetSplitDbClickCentering( HWND hSplitWnd );


//----------------------------------------------
// �����ΏۃE�B���h�E�ݒ�/�擾
//
void SetSplitTargetWindow( HWND hSplitWnd, HWND left, HWND right );
void GetSplitTargetWindow( HWND hSplitWnd, HWND* left, HWND* right );


//----------------------------------------------
// �����ΏۃE�B���h�E�Œ�����ݒ�/�擾
//
void SetSplitWindowLock( HWND hSplitWnd, int lock );
int GetSplitWindowLock( HWND hSplitWnd );


//----------------------------------------------
// �����ΏۃE�B���h�E����ւ�
//
void SwapSplitTargetWindow( HWND hSplitWnd );
