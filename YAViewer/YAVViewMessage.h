/*----------------------------------------------------------------------------------------
*
* YAViewer Viewer �Ǝ����b�Z�[�W�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


/**
* �A�C�e���I����
*
*	WPARAM 
*		���ʃ��[�h
*					0x0000 : ���N���b�N
*					0x0001 : �E�N���b�N
*					0x0002 : ���N���b�N
*		��ʃ��[�h
*					0x0001 : SHIFT�L�[����
*					0x0002 : CONTROL�L�[����
*
*	LPARAM -		SelectItem*
*
*	return			���0
*/
#define WM_VIEW_SELECTITEM		WM_APP
typedef struct _tagViewSelectItem
{
	const wchar_t*		itemString;			// �A�C�e��������
	unsigned long		itemLength;			// �A�C�e��������
} ViewSelectItem;
