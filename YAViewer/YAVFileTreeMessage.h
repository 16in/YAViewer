/*----------------------------------------------------------------------------------------
*
* YAViewer FileTree �Ǝ����b�Z�[�W�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


/**
* �A�C�e���I����
*
*	WPARAM -		0 : �t�@�C��
*					1 : �f�B���N�g��
*
*	LPARAM -		TreeSelectItem*
*
*	return			���0
*/
#define WM_TREE_SELECTITEM		(WM_APP + 0x1000)
typedef struct _tagTreeSelectItem
{
	const wchar_t*		filePath;		// �I���t�@�C���p�X
} TreeSelectItem;
