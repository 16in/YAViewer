/*----------------------------------------------------------------------------------------
*
* YAViewer �_�C�A���O�֌W
*
*----------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>


//----------------------------------------------
// AA�֌W�t�@�C�����J���_�C�A���O
//
bool OpenAAFileDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// �^�u��ۑ��_�C�A���O
//
bool SaveFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// �^�u�𕜌��_�C�A���O
//
bool OpenFileTabDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// �c���[��ۑ��_�C�A���O
//
bool SaveFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// �c���[�𕜌��_�C�A���O
//
bool OpenFileTreeDialog( HWND hOwner, wchar_t* path, size_t pathLength );


//----------------------------------------------
// �^�u�J���[�_�C�A���O
//
bool SelectTabColorDialog( HWND hOwner, COLORREF* color );
