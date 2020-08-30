/*----------------------------------------------------------------------------------------
*
* YAViewer �A�v���P�[�V��������֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <AAFileAccessor.h>


//-------------------------------------
// ���O��`�������
//
typedef struct _tagRegYavAppData RegYavAppData;


//-------------------------------------
// �t�@�C���^�u���\����
//
struct YavFileTabParam
{
	wchar_t				filePath[ MAX_PATH ];	// �t�@�C���p�X
	aafile::AAFile*		file;					// AA�t�@�C��
	int					scrollPosition;			// �X�N���[���ʒu

	union
	{
		DWORD			flags;					// �t���O
		struct {
			int			useTabColor : 1;		// �^�u�z�F�g�p�t���O
			int			disableClose: 1;		// ����s�t���O
			int : 30;
		};
	};
	COLORREF			tabColor;				// �^�u�z�F
};


//-------------------------------------
// �����񑗐M��t���O
//
enum SendStringTarget
{
	// �ΏۃG�f�B�^��
	SST_CLIPBOARD = 0x00,				// �N���b�v�{�[�h
	SST_FACE_EDIT,						// (�L�t�M)
	SST_ORINRIN_EDITOR,					// Orinrin

	SST_SAKURA_EDITOR,					// �T�N���G�f�B�^

	SST_MAX,							// �ΏۃG�f�B�^�ő�

	// ���M�I�v�V����
	SST_TEXTBOX_FLAG = 0x00010000,		// �e�L�X�g�{�b�N�X���M�t���O

	// �}�X�N
	SST_EDITOR_MASK = 0x0000ffff,
	SST_OPTION_MASK = 0xffff0000,

	// 32bit�g���t���O
	SST_UNKNOWN = 0xffffffff
};


//-------------------------------------
// �t�@�C���^�u�Ƀt�@�C����ǉ�����
//
bool YavAddFile( RegYavAppData* appData, LPCWSTR filePath, bool setView );


//-------------------------------------
// �t�@�C���^�u����t�@�C�����폜����
//
bool YavDelFile( RegYavAppData* appData, int idx );


//-------------------------------------
// �t�@�C���^�u����t�@�C������������
//
int YavGetFileTabIndex( RegYavAppData* appData, LPCWSTR filePath );


//-------------------------------------
// �t�@�C���^�u����t�@�C�����擾����
//
bool YavGetFile( RegYavAppData* appData, int idx, LPWSTR filePath, size_t length );


//-------------------------------------
// �t�@�C���^�u�o�^�ς݃t�@�C�������擾����
//
int YavGetFileCount( RegYavAppData* appData );


//-------------------------------------
// �t�@�C���^�u����t�@�C����I������
//
bool YavSelectFile( RegYavAppData* appData, LPCWSTR filePath );
bool YavSelectFile( RegYavAppData* appData, int idx );


//-------------------------------------
// �t�@�C���^�u�J���[��ݒ�/�擾����
//
bool YavSelectTabColor( RegYavAppData* appData, int idx, COLORREF tabColor, bool disable = false );
bool YavGetTabColor( RegYavAppData* appData, int idx, COLORREF* tabColor );


//-------------------------------------
// �t�@�C���^�u��ۑ�����
//
bool YavSaveFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// �t�@�C���^�u�𕜌�����
//
bool YavLoadFileTab( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// �t�@�C���c���[��ۑ�����
//
bool YavSaveFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// �t�@�C���^�u�𕜌�����
//
bool YavLoadFileTree( RegYavAppData* appData, LPCWSTR filePath, bool errdlg = false );


//-------------------------------------
// �w��̕�������N���b�v�{�[�h�֑��M
//
bool YavSendClipBoardString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length );


//-------------------------------------
// �w��̕���������j�R�[�h�ɕϊ����ăN���b�v�{�[�h�֑��M
//
bool YavSendClipBoardUnicodeString( RegYavAppData* appData, HWND hWnd, LPCWSTR string, size_t length );


//-------------------------------------
// �w��̕�������G�f�B�^�֑��M
//
bool YavSendEditorString( RegYavAppData* appData, DWORD target, LPCWSTR string, size_t length );


//-------------------------------------
// ���s�t�@�C���f�B���N�g���ݒ�/�擾
//
void YavSetAppDirectory( LPCWSTR appDirectory );
LPCWSTR YavGetAppDirectory( void );


//-------------------------------------
// �ݒ�t�@�C���ۑ�/�ǂݍ���
//
void YavSaveAppSetting( RegYavAppData* appData, LPCWSTR settingfile = NULL );
void YavLoadAppSetting( RegYavAppData* appData, LPCWSTR settingfile = NULL );
