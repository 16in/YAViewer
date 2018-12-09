/*----------------------------------------------------------------------------------------
*
* YAViewer �Ǝ����b�Z�[�W�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include "YAVViewMessage.h"
#include "YAVFileTreeMessage.h"


/**
* �A�C�e���I����
*	�ڍׂ�YAVViewMessage.h���Q��
*/
#define YAV_WM_SELECTITEM		WM_VIEW_SELECTITEM

/**
* �c���[�A�C�e���I����
*	�ڍׂ�YAVFileTreeMessage.h���Q��
*/
#define YAV_WM_TREE_SELECTITEM		WM_TREE_SELECTITEM

/**
* �t�@�C���ǉ�
*	�w��̃t�@�C�����t�@�C���^�u�ɒǉ����܂��B
*
*	WPARAM -	0 : �t�@�C���r���[�ɓ]�����Ȃ�
*				1 : �t�@�C���r���[�ɓ]������
*
*	LPARAM -	YavWmAddFile*
*
*	return		BOOL		�ǂݍ��݂ɐ���������^�A���s������U
*/
#define YAV_WM_ADDFILE			(WM_APP + 1)
typedef struct _tagYavWmAddFile
{
	const wchar_t*		filePath;		// �ǉ�����t�@�C���ւ̃p�X
} YavWmAddFile;

/**
* �t�@�C���^�u����폜
*	�w��̃t�@�C�����t�@�C���^�u����폜���܂��B
*
*	WPARAM -	int			�폜����t�@�C���^�u�̃C���f�b�N�X
*							�������ݒ肳�ꂽ�ꍇ�͂��ׂĔp��
*
*	return		BOOL		�ǂݍ��݂ɐ���������^�A���s������U
*/
#define YAV_WM_DELFILE			(WM_APP + 2)

/**
* �^�u��ۑ�
*	�t�@�C���^�u���w��̃t�@�C���ɕۑ����܂��B
*
*	LPARAM -	LPCWSTR		�ۑ���
*
*	return		BOOL		�ۑ��ɐ���������^�A���s������U
*/
#define YAV_WM_SAVEFILETAB		(WM_APP + 3)

/**
* �^�u�𕜌�
*	�t�@�C���^�u���w��̃t�@�C���ɕ������܂��B
*
*	LPARAM -	LPCWSTR		������
*
*	return		BOOL		�����ɐ���������^�A���s������U
*/
#define YAV_WM_LOADFILETAB		(WM_APP + 4)

/**
* �c���[��ۑ�
*	�t�@�C���c���[���w��̃t�@�C���ɕۑ����܂��B
*
*	LPARAM -	LPCWSTR		�ۑ���
*
*	return		BOOL		�ۑ��ɐ���������^�A���s������U
*/
#define YAV_WM_SAVEFILETREE		(WM_APP + 5)

/**
* �c���[�𕜌�
*	�t�@�C���c���[���w��̃t�@�C���ɕ������܂��B
*
*	LPARAM -	LPCWSTR		������
*
*	return		BOOL		�����ɐ���������^�A���s������U
*/
#define YAV_WM_LOADFILETREE		(WM_APP + 6)

/**
* �y�[�W�W�����v���s
*	���݊J���Ă���t�@�C���̎w��y�[�W�փW�����v���܂��B
*
*	LPARAM -	page		�y�[�W�ԍ�
*
*	return		BOOL		�����ɐ���������^�A���s������U
*/
#define YAV_WM_PAGEJUMP			(WM_APP + 7)
