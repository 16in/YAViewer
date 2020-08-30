/*----------------------------------------------------------------------------------------
*
* YAViewer Window�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "YAVDialog.h"
#include "YAViewerMessage.h"
#include "YAVAppAction.h"


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E�o�^�f�[�^
//
typedef struct _tagRegYavAppData
{
	//-- �E�B���h�E�֌W
	HWND			hFileTab;					// �t�@�C���^�u
	HWND			hCtrlWnd;					// �R���g���[���E�B���h�E
	HWND			hSelector;					// ���M��ݒ�E�B���h�E
	HWND			hFileTree;					// �t�@�C���c���[
	HWND			hFileView;					// �t�@�C���r���[
	HWND			hTreeShort;					// �c���[�ƃV���[�g�J�b�g
	HWND			hAAShortcut;				// AA�V���[�g�J�b�g�E�B���h�E

	//-- �t�H���g�֌W
	HFONT			hAAFont;					// AA�p�t�H���g
	wchar_t			fontName[ 32 ];				// �t�H���g��
	long			fontSize;					// �t�H���g�T�C�Y

	//-- ���M��ݒ�
	DWORD			clipUnicodeCopy;			// �N���b�v�{�[�h�Ƀ��j�R�[�h�ϊ����đ��M���邩
	DWORD			sendTargetLeft;				// ���N���b�N���M��
	DWORD			sendTargetRight;			// �E�N���b�N���M��
	DWORD			sendTargetCenter;			// ���N���b�N���M��

	//-- �c���[�ݒ�
	DWORD			enableExtension;			// �L���Ȋg���q

	//-- �E�B���h�E���
	long			x, y;						// ���W
	long			width, height;				// ���ƍ���
	long			splitPos;					// �����E�B���h�E�ʒu
	long			reverseSplit;				// �����E�B���h�E����ւ�
	long			viewMode;					// �\�����[�h
	long			toolHint;					// �c�[���q���g�\��
	long			categorySize;				// �J�e�S���\���T�C�Y
	long			tabButtonMode;				// �^�O�{�^�����[�h
	long			dblclickMode;				// �_�u���N���b�N�I�����[�h
	long			shortcutSize;				// �V���[�g�J�b�g�E�B���h�E�T�C�Y

	//-- ���������
	wchar_t			initTabPath[ MAX_PATH ];	// �^�u�������t�@�C���w��
	wchar_t			initTreePath[ MAX_PATH ];	// �c���[�������t�@�C���w��
} RegYavAppData;


//----------------------------------------------
// �E�B���h�E�N���X�o�^
//
bool RegisterYAVWindowClass( HINSTANCE hInstance );


//----------------------------------------------
// �E�B���h�E����
//
HWND CreateYAVWindow( HINSTANCE hInstance, RegYavAppData* appData );


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E�W���R�[���o�b�N
//
LRESULT CALLBACK YAVAppWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//----------------------------------------------
// �A�v���P�[�V�����E�B���h�E�o�^�f�[�^�擾
//
RegYavAppData* GetYAVAppData( HWND hWnd );
