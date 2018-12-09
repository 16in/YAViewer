/*----------------------------------------------------------------------------------------
*
* YAViewer View ������֌W
*
*----------------------------------------------------------------------------------------*/
#include "YAVViewDetail.h"
#include "refchar.h"
#include <string>
#include <Commctrl.h>


//----------------------------------------------
// �O���[�o���֐�
//
// �\���`�����Ƃ̎q�E�B���h�E�T�C�Y�ύX
static void setCategoryViewSize( HWND hViewWnd, int width, int height, int itemSize );
static void setNormalViewSize( HWND hViewWnd, int width, int height );

// �f�o�C�X�R���e�L�X�g�Ƀf�[�^����������
//	 �������f�o�C�X�R���e�L�X�g���Ȃ��ꍇ�͐���
static void paintPage( RegViewPageData* data, HWND hPageWnd, HFONT hFont );

// �������f�o�C�X�R���e�L�X�g�̐���
static void createPageDC( RegViewPageData* data, HWND hPageWnd, HFONT hFont );

// �ŃT�C�Y���v�Z
static void getPageSize( RegViewPageData* data );

// �擪�����̕������ƕ����R�[�h���擾
static int getCharWidth( RegViewPageData* data, const wchar_t* str, size_t length, unsigned long& step, wchar_t* ch = NULL );

// �Q�ƕ����񂩃`�F�b�N���A�Q�ƕ�����Ȃ�ΎQ�Ƃ���������
static bool checkRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount );


//----------------------------------------------
// �r���[�E�B���h�E�Ɏq�����邩
//
bool CheckViewWindowChild( HWND hViewWnd )
{
	return GetWindow( hViewWnd, GW_CHILD ) != NULL;
}


//----------------------------------------------
// �r���[�E�B���h�E�T�C�Y�ύX
//
void SetViewWindowSize( HWND hViewWnd, int width, int height )
{
	if( CheckViewWindowChild( hViewWnd ) )
	{
		RegViewWindowData* data = GetViewWindowData( hViewWnd );

		switch( data->mode )
		{
		case 0:
			setNormalViewSize( hViewWnd, width, height );
			break;
		case 1:
			setCategoryViewSize( hViewWnd, width, height, data->categorySize );
			break;
		}
	}
}


//----------------------------------------------
// �o�^�ς݂̕ł��f�o�C�X�R���e�L�X�g�ɏ�������
//
void AllPaintPage( HWND hViewWnd )
{
	if( CheckViewWindowChild( hViewWnd ) )
	{
		RegViewWindowData* data = GetViewWindowData( hViewWnd );
		for( size_t i = 0; i < data->file->pageCount; i++ )
		{
			RegViewPageData* page = GetViewPageData( data->pages[ i ] );
			if( page )
			{
				getPageSize( page );
				//paintPage( page, data->pages[ i ], data->hFont );
				//createPageDC( page, data->pages[ i ], data->hFont );
			}
		}
	}
}


//----------------------------------------------
// �w��̃y�[�W�f�[�^�������o��
//
void PaintPage( RegViewWindowData* viewData, HWND hPageWnd, RegViewPageData* pageData )
{
	if( viewData && pageData && !pageData->hDeviceContext )
	{
		paintPage( pageData, hPageWnd, viewData->hFont );
	}
}


//----------------------------------------------
// �Ńf�[�^���E�B���h�E�ɃR�s�[����
//
void CopyPageData( HDC hPageDC, RegViewWindowData* viewData, RegViewPageData* pageData )
{
#define ROP SRCCOPY		/* ���X�^���Z�� */

	SetStretchBltMode( hPageDC, HALFTONE );
	SetBrushOrgEx( hPageDC, 0, 0, NULL );


	/*---- �f�o�C�X�R���e�L�X�g�̃J���[���] ----*/
	if( !pageData->invert )
	{
		RECT rect = { 0, 0, pageData->width, pageData->height };
		InvertRect( pageData->hDeviceContext, &rect );
		pageData->invert = true;
	}

	
	/*---- �f�o�C�X�R���e�L�X�g�R�s�[ ----*/
	switch( viewData->mode )
	{
	case 0:
		BitBlt( hPageDC, 0, 0, pageData->width, pageData->height, pageData->hDeviceContext, 0, 0, ROP );
		break;
	case 1:
		if( pageData->width <= viewData->categorySize
			&& pageData->height <= viewData->categorySize )
		{// ���܂�̂ŃZ���^�����O���Ă��̂܂ܕ\��
			int x = (viewData->categorySize - pageData->width) / 2;
			int y = (viewData->categorySize - pageData->height) / 2;
			BitBlt( hPageDC, x, y, pageData->width, pageData->height, pageData->hDeviceContext, 0, 0, ROP );
		}
		else
		{// �k�����ĕ\��
			int x = 0, y = 0;
			int width = pageData->width, height = pageData->height;
			if( width >= height )
			{// �������L���ꍇ�͏c�ɃZ���^�����O
				int sh = (int)((float)height * ((float)viewData->categorySize / (float)width));
				y = (viewData->categorySize - sh) / 2;
				StretchBlt( hPageDC, x, y, viewData->categorySize, sh, pageData->hDeviceContext, 0, 0, width, height, ROP );
			}
			else
			{// �c�����L���ꍇ�͉��ɃZ���^�����O
				int sw = (int)((float)width * ((float)viewData->categorySize / (float)height));
				x = (viewData->categorySize - sw) / 2;
				StretchBlt( hPageDC, x, y, sw, viewData->categorySize, pageData->hDeviceContext, 0, 0, width, height, ROP );
			}
		}
		break;
	}
}


//----------------------------------------------
// �o�^�ς݂̕\���f�[�^���擾����
//
RegViewWindowData* GetViewWindowData( HWND hViewWnd )
{
	RegViewWindowData* ret = NULL;
	if( hViewWnd )
	{
		ret = (RegViewWindowData*)GetWindowLongPtrW( hViewWnd, 0 );

#ifdef _DEBUG
		// �f�[�^�̐������`�F�b�N
		if( ret && ret->file )
		{
			if( ret->file->type < 0 || ret->file->type >= aafile::AAFILETYPE_MAX )
			{
				MessageBoxW( hViewWnd, L"�t�@�C���^�C�v���s���ł��B", L"�G���[", MB_OK );
			}
		}
#endif
	}
	return ret;
}


//----------------------------------------------
// �o�^�ς݂̃y�[�W�f�[�^���擾����
//
RegViewPageData* GetViewPageData( HWND hPageWnd )
{
	RegViewPageData* ret = NULL;
	if( hPageWnd )
	{
		ret = (RegViewPageData*)GetWindowLongPtrW( hPageWnd, 0 );
	}
	return ret;
}


//----------------------------------------------
// �c�[���`�b�v�\����ݒ肷��
//
void SetTooltipEnablePage( HWND hViewWnd, bool enable )
{
	if( CheckViewWindowChild( hViewWnd ) )
	{
		RegViewWindowData* data = GetViewWindowData( hViewWnd );
		for( size_t i = 0; i < data->file->pageCount; i++ )
		{
			SendMessageW( GetViewPageData( data->pages[ i ] )->hToolHint, TTM_ACTIVATE, enable, 0 );
		}
	}
}


//----------------------------------------------
// �J�e�S���r���[�@�ŕ\��
//
static void setCategoryViewSize( HWND hViewWnd, int width, int height, int itemSize )
{
	/*---- �X�N���[���o�[�̏�Ԃ��擾 ----*/
	SCROLLINFO sc;
	ZeroMemory( &sc, sizeof( sc ) );
	sc.cbSize = sizeof( sc );
	sc.fMask = SIF_ALL;
	::GetScrollInfo( hViewWnd, SB_VERT, &sc );


	/*---- �E�B���h�E�T�C�Y��ݒ� ----*/
	// �q�E�B���h�E�̃T�C�Y��ݒ肵�Ă���
	int x = 0, y = -(sc.nPos * itemSize);
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	for( size_t i = 0; i < data->file->pageCount; i++ )
	{
		if( x + itemSize > width )
		{
			x = 0;
			y += itemSize;
		}
		MoveWindow( data->pages[ i ], x, y, itemSize, itemSize, TRUE );
		x += itemSize;
	}


	/*---- �X�N���[���o�[�̏����X�V ----*/
	if( !data )
	{
		sc.nMax = sc.nMin = 0;
		sc.nPage = sc.nPos = 0;
	}
	else
	{
		int wCount = (width > 0) ? width / itemSize : 1;
		sc.nMin = 0;
		sc.nMax = data->file->pageCount / wCount + 1;
		sc.nPage = (width > 0) ? height / itemSize : 1;
	}
	SetScrollInfo( hViewWnd, SB_VERT, &sc, TRUE );
}


//----------------------------------------------
// �m�[�}���r���[�@�ŕ\��
//
static void setNormalViewSize( HWND hViewWnd, int width, int height )
{
	/*---- �X�N���[���o�[�̏�Ԃ��擾 ----*/
	SCROLLINFO sc;
	ZeroMemory( &sc, sizeof( sc ) );
	sc.cbSize = sizeof( sc );
	sc.fMask = SIF_ALL;
	::GetScrollInfo( hViewWnd, SB_VERT, &sc );


	/*---- �E�B���h�E�T�C�Y��ݒ� ----*/
	// �q�E�B���h�E�̃T�C�Y��ݒ肵�Ă���
	int y = 0;
	RegViewWindowData* data = GetViewWindowData( hViewWnd );
	if( data )
	{
		for( size_t i = sc.nPos; i < data->file->pageCount; i++ )
		{
			RegViewPageData* pageData = GetViewPageData( data->pages[ i ] );
			MoveWindow( data->pages[ i ], 0, y-1, width, pageData->height+1, TRUE );
			y += pageData->height;
		}

		// ������̐ݒ�
		y = 0;
		for( size_t i = sc.nPos; i > 0; i-- )
		{
			RegViewPageData* pageData = GetViewPageData( data->pages[ i - 1 ] );
			y -= pageData->height;
			MoveWindow( data->pages[ i - 1 ], 0, y-1, width, pageData->height+1, TRUE );
		}
	}


	/*---- �X�N���[���o�[�̏����X�V ----*/
	if( !data )
	{
		sc.nMax = sc.nMin = 0;
		sc.nPage = sc.nPos = 0;
	}
	else
	{
		sc.nMin = 0; sc.nMax = data->file->pageCount;
		sc.nPage = 1;
	}
	SetScrollInfo( hViewWnd, SB_VERT, &sc, TRUE );
}


//----------------------------------------------
// �f�o�C�X�R���e�L�X�g��AA����������
//
static void paintPage( RegViewPageData* data, HWND hPageWnd, HFONT hFont )
{
	/*---- �f�o�C�X�R���e�L�X�g���Ȃ��ꍇ�͐��� ----*/
	if( !data->hDeviceContext )
	{
		createPageDC( data, hPageWnd, hFont );
	}


	/*---- �f�o�C�X�R���e�L�X�g�̐ݒ� ----*/
	HFONT hOldFont = (HFONT)SelectObject( data->hDeviceContext, hFont );
	int oldMode = SetBkMode( data->hDeviceContext, TRANSPARENT );
	int oldTextColor = SetTextColor( data->hDeviceContext, RGB( 0xff, 0xff, 0xff ) );


	/*---- �ꕶ�����o�� ----*/
	std::wstring lfString = L"";
	int y = 0;
	for( unsigned long pos = 0, step = 1; pos < data->page->valueLength; pos += step )
	{
		bool lf = false;
		step = 1;

		// ���s�`�F�b�N
		if( data->page->value[ pos ] == L'\r' )
		{
			lf = true;
			if( data->page->value[ pos + 1 ] == L'\n' ) step = 2;
		}
		else if( data->page->value[ pos ] == L'\n' )
		{
			lf = true;
		}

		if( !lf )
		{
			/*---- �\��������擾 ----*/
			wchar_t ch[ 4 ] = { 0 };
			getCharWidth( data, &data->page->value[ pos ], data->page->valueLength, step, ch );
			lfString += ch;
		}
		else
		{
			/*-- ���C���o�� --*/
			TextOutW( data->hDeviceContext, 0, y, lfString.c_str(), lfString.size() );
			lfString.clear();
			y += *data->lfHeight;
		}
	}
	TextOutW( data->hDeviceContext, 0, y, lfString.c_str(), lfString.size() );


	/*---- �f�o�C�X�R���e�L�X�g�̐ݒ� ----*/
	SelectObject( data->hDeviceContext, hOldFont );
	SetBkMode( data->hDeviceContext, oldMode );
	SetTextColor( data->hDeviceContext, oldTextColor );
}


//----------------------------------------------
// �������f�o�C�X�R���e�L�X�g�̐���
//
static void createPageDC( RegViewPageData* data, HWND hPageWnd, HFONT hFont )
{
	/*---- �T�C�Y���v�Z ----*/
	getPageSize( data );


	/*---- �r�b�g�}�b�v�C���t�H����� ----*/
	BITMAPINFO bmpInfo;
	ZeroMemory( &bmpInfo, sizeof( ::BITMAPINFO ) );
	bmpInfo.bmiHeader.biSize = sizeof( ::BITMAPINFO );
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biWidth = data->width;
	bmpInfo.bmiHeader.biHeight = -data->height;


	/*---- �r�b�g�}�b�v�𐶐����� ----*/
	HDC hdc = GetDC( hPageWnd );
	void* bmpBuf;
	data->hDeviceContextBMP = CreateDIBSection( hdc, &bmpInfo, DIB_RGB_COLORS, &bmpBuf, NULL, 0 );
	data->hDeviceContext = CreateCompatibleDC( hdc );
	data->hOldDeviceContextBMP = (HBITMAP)SelectObject( data->hDeviceContext, data->hDeviceContextBMP );
	ReleaseDC( hPageWnd, hdc );
}


//----------------------------------------------
// �ŃT�C�Y���v�Z
//
static void getPageSize( RegViewPageData* data )
{
	/*---- ��Ԃ������� ----*/
	data->lfCount = 1;
	data->height = data->width = 0;


	/*---- ���������v�Z ----*/
	int width = 0;
	for( unsigned long pos = 0, step = 1; pos < data->page->valueLength; pos += step )
	{
		bool lf = false;
		step = 1;

		// ���s�`�F�b�N
		if( data->page->value[ pos ] == L'\r' )
		{
			lf = true;
			if( data->page->value[ pos + 1 ] == L'\n' ) step = 2;;
		}
		else if( data->page->value[ pos ] == L'\n' )
		{
			lf = true;
		}

		if( !lf )
		{
			width += getCharWidth( data, &data->page->value[ pos ], data->page->valueLength, step );
		}
		else
		{
			if( data->width < width ) data->width = width;
			width = 0;
			data->lfCount++;
		}
	}
	if( data->width < width ) data->width = width;


	/*---- �������v�Z ----*/
	data->height = data->lfCount * (*data->lfHeight);
}


//----------------------------------------------
// �擪�����̕������ƕ����R�[�h���擾
//
static int getCharWidth( RegViewPageData* data, const wchar_t* str, size_t length, unsigned long& step, wchar_t* ch )
{
	int ret = 0;
	wchar_t refChar[ 2 ];
	unsigned long refCount;

	if( !checkRefChar( str, length, refChar, step, refCount ) )
	{// �Q�ƕ����������Ȃ�����
		step = 1;
		ret = data->charWidthTable[ str[ 0 ] ];
		if( ch ) *ch = str[ 0 ];
	}
	else
	{// �Q�ƕ��������ς݂̂���
		if( refCount == 1 )
		{// ��T���Q�[�g����
			ret = data->charWidthTable[ refChar[ 0 ] ];
			if( ch ) *ch = refChar[ 0 ];
		}
		else
		{// �T���Q�[�g�����c�����ǁA�ǂ����悤
			if( ch )
			{
				ch[ 0 ] = refChar[ 0 ];
				ch[ 1 ] = refChar[ 1 ];
			}
		}
	}

	return ret;
}


//----------------------------------------------------------
// �Q�ƕ����񂩃`�F�b�N���A�Q�ƕ�����Ȃ�ΎQ�Ƃ���������
//
bool CheckRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount )
{
	return checkRefChar( str, length, refChar, step, refCount );
}
static bool checkRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount )
{
	bool ret = false;

	if( str[ 0 ] == L'&' && length >= 2 )
	{// �Q�ƕ�����
		if( str[ 1 ] != L'#' )
		{// ���̎Q��
			for( size_t i = 0; sg_RefCharList[ i ].refString != NULL; i++ )
			{
				size_t n = wcslen( sg_RefCharList[ i ].refString );
				if( length >= n )
				{
					if( _wcsnicmp( str, sg_RefCharList[ i ].refString, n ) == 0 )
					{
						refChar[ 0 ] = sg_RefCharList[ i ].refChar;
						step = n;
						refCount = 1;
						ret = true;
						break;
					}
				}
			}
		}
		else
		{// ���l�Q��
			wchar_t*		endPtr;
			unsigned long	code;

			if( towlower( str[ 2 ] ) != L'x' )
			{// 10�i���Q��
				code = wcstol( &str[ 2 ], &endPtr, 10 );
			}
			else
			{// 16�i���Q��
				code = wcstol( &str[ 3 ], &endPtr, 16 );
			}

			if( endPtr[ 0 ] == L';' )
			{// �Q�ƕ�����ł�����
				if( code <= 0xffff )
				{
					refChar[ 0 ] = (wchar_t)code;
					refCount = 1;
				}
				else
				{
					unsigned long X = code - 0x10000;
					refChar[ 0 ] = (wchar_t)((X / 0x400) + 0xD800);
					refChar[ 1 ] = (wchar_t)((X % 0x400) + 0xDC00);
					refCount = 2;
				}
				step = endPtr - str + 1;
				ret = true;
			}
		}
	}

	return ret;
}
