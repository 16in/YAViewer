/*----------------------------------------------------------------------
*
* AA�֘A�t�@�C�����[�_����Q�Ƃ��郍�[�J���֐�
*
*	Copyright (c) 2012 _16in/��7N5y1wtOn2
*
*----------------------------------------------------------------------*/
#pragma once

#include <AAFileAccessor.h>
#include "types.h"


namespace aafile {


/**
* MLT�t�@�C���𕪊�
*	@param[in]	pageArray			�y�[�W�f�[�^�i�[��
*	@param[in]	str					�����Ώە�����
*	@param[in]	length				������
*/
static void SplitMLT( AAFilePageArray& pageArray, const wchar_t* str, unsigned long length );

/**
* AST�t�@�C���𕪊�
*	@param[in]	pageArray			�y�[�W�f�[�^�i�[��
*	@param[in]	str					�����Ώە�����
*	@param[in]	length				������
*/
static void SplitAST( AAFilePageArray& pageArray, const wchar_t* str, unsigned long length );

/**
* ASD�t�@�C���𕪊�
*	@param[in]	pageArray			�y�[�W�f�[�^�i�[��
*	@param[in]	str					�����Ώە�����
*	@param[in]	length				������
*/
static void SplitASD( AAFilePageArray& pageArray, wchar_t* str, unsigned long length );

/**
* AAData�t�@�C���𕪊�
*	@param[in]	pageArray			�y�[�W�f�[�^�i�[��
*	@param[in]	str					�����Ώە�����
*	@param[in]	length				������
*/
static void SplitAADATA( AAFilePageArray& pageArray, const wchar_t* str, unsigned long length );

/**
* AAList�t�@�C���𕪊�
*	@param[in]	pageArray			�y�[�W�f�[�^�i�[��
*	@param[in]	str					�����Ώە�����
*	@param[in]	length				������
*/
static void SplitAALIST( AAFilePageArray& pageArray, const wchar_t* str, unsigned long length );

/**
* AAList/AAData�t�@�C���𕪊�
*	@param[in]	pageArray			�y�[�W�f�[�^�i�[��
*	@param[in]	str					�����Ώە�����
*	@param[in]	length				������
*	@param[in]	start				�擪����������
*	@param[in]	end					�I�[����������
*/
static void SplitAAListData( AAFilePageArray& pageArray, const wchar_t* str, unsigned long length, const wchar_t* start, const wchar_t* end );


}


/*----------------------------------------------------------------------
* License
*
*	AAFileAccessor
*
*	Copyright (c) 2012 _16in/��7N5y1wtOn2
*
*	Permission is hereby granted, free of charge, to any person obtaining
*	a copy of this software and associated documentation files (the "Software"),
*	to deal in the Software without restriction,
*	including without limitation the rights to use, copy, modify, merge,
*	publish, distribute, sublicense, and/or sell copies of the Software,
*	and to permit persons to whom the Software is furnished to do so,
*	subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be
*	included in all copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
*	THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
*	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*	THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*----------------------------------------------------------------------*/