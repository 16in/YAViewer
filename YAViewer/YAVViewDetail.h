/*----------------------------------------------------------------------------------------
*
* YAViewer View ������֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <AAFileAccessor.h>


//----------------------------------------------
// �E�B���h�E�֘A�t���f�[�^
//
struct RegViewWindowData
{
	aafile::AAFile*		file;					// �\������t�@�C��
	int					mode;					// �\�����[�h
	int					categorySize;			// �J�e�S���\�����̃T�C�Y
	bool				sendRoot;				// ���b�Z�[�W�̑��M������[�g�E�B���h�E�ɂ��邩

	HWND*				pages;					// �y�[�W�E�B���h�E�f�[�^
	HFONT				hFont;					// �`��p�t�H���g
	int					lfHeight;				// �s�̍���
	int					charWidth[ 0x10000 ];	// �������e�[�u��

	bool				enableTooltip;			// �c�[���`�b�v�t���O

	RegViewWindowData( void )
		: file( NULL ), mode( 0 ), categorySize( 128 ),
		sendRoot( false ), pages( NULL ), hFont( NULL ), lfHeight( 12 ), enableTooltip( true )
	{ ZeroMemory( charWidth, sizeof( charWidth ) ); }

	virtual ~RegViewWindowData( void )
	{}
};
struct RegViewPageData
{
	aafile::AAFilePage*	page;					// �Ńf�[�^
	long				width, height;			// �ŃT�C�Y
	unsigned long		lfCount;				// ���s��
	int*				lfHeight;				// �s�̍����ւ̃|�C���^
	int*				charWidthTable;			// �������e�[�u��

	bool				mouseTracking;			// �}�E�X�̒ǐ�
	bool				hover;					// �}�E�X����ɂ��邩
	bool				invert;					// ���]�ς݂ł��邩

	// �������f�o�C�X�R���e�L�X�g
	HDC					hDeviceContext;			// �f�o�C�X�R���e�L�X�g�n���h��
	HBITMAP				hDeviceContextBMP;		// �f�o�C�X�R���e�L�X�g�r�b�g�}�b�v
	HBITMAP				hOldDeviceContextBMP;	// �ȑO�̃f�o�C�X�R���e�L�X�g�r�b�g�}�b�v
	
	// �c�[���q���g
	HWND				hToolHint;				// �c�[���q���g�R���g���[��
	wchar_t*			toolString;				// �c�[���\��������

	RegViewPageData( int* cwTable, int* lfh )
		: page( NULL ), width( 0 ), height( 0 ), lfCount( 0 ), lfHeight( lfh ), charWidthTable( cwTable ),
		mouseTracking( false ), hover( false ), invert( false ),
		hDeviceContext( NULL ), hDeviceContextBMP( NULL ), hOldDeviceContextBMP( NULL ), hToolHint( NULL ), toolString( NULL )
	{}

	virtual ~RegViewPageData( void )
	{}
};


//----------------------------------------------
// �r���[�E�B���h�E�Ɏq�����邩
//
bool CheckViewWindowChild( HWND hViewWnd );


//----------------------------------------------
// �r���[�E�B���h�E�T�C�Y�ύX
//
void SetViewWindowSize( HWND hViewWnd, int width, int height );


//----------------------------------------------
// �o�^�ς݂̕ł��f�o�C�X�R���e�L�X�g�ɏ�������
//
void AllPaintPage( HWND hViewWnd );


//----------------------------------------------
// �w��̃y�[�W�f�[�^�������o��
//
void PaintPage( RegViewWindowData* viewData, HWND hPageWnd, RegViewPageData* pageData );


//----------------------------------------------
// �Ńf�[�^���E�B���h�E�ɃR�s�[����
//
void CopyPageData( HDC hPageDC, RegViewWindowData* viewData, RegViewPageData* pageData );


//----------------------------------------------
// �o�^�ς݂̕\���f�[�^���擾����
//
RegViewWindowData* GetViewWindowData( HWND hViewWnd );


//----------------------------------------------
// �o�^�ς݂̃y�[�W�f�[�^���擾����
//
RegViewPageData* GetViewPageData( HWND hPageWnd );


//----------------------------------------------
// �c�[���`�b�v�\����ݒ肷��
//
void SetTooltipEnablePage( HWND hViewWnd, bool enable );
