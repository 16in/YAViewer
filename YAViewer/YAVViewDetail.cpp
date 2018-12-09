/*----------------------------------------------------------------------------------------
*
* YAViewer View 実動作関係
*
*----------------------------------------------------------------------------------------*/
#include "YAVViewDetail.h"
#include "refchar.h"
#include <string>
#include <Commctrl.h>


//----------------------------------------------
// グローバル関数
//
// 表示形式ごとの子ウィンドウサイズ変更
static void setCategoryViewSize( HWND hViewWnd, int width, int height, int itemSize );
static void setNormalViewSize( HWND hViewWnd, int width, int height );

// デバイスコンテキストにデータを書き込む
//	 メモリデバイスコンテキストがない場合は生成
static void paintPage( RegViewPageData* data, HWND hPageWnd, HFONT hFont );

// メモリデバイスコンテキストの生成
static void createPageDC( RegViewPageData* data, HWND hPageWnd, HFONT hFont );

// 頁サイズを計算
static void getPageSize( RegViewPageData* data );

// 先頭文字の文字幅と文字コードを取得
static int getCharWidth( RegViewPageData* data, const wchar_t* str, size_t length, unsigned long& step, wchar_t* ch = NULL );

// 参照文字列かチェックし、参照文字列ならば参照を解決する
static bool checkRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount );


//----------------------------------------------
// ビューウィンドウに子があるか
//
bool CheckViewWindowChild( HWND hViewWnd )
{
	return GetWindow( hViewWnd, GW_CHILD ) != NULL;
}


//----------------------------------------------
// ビューウィンドウサイズ変更
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
// 登録済みの頁をデバイスコンテキストに書き込む
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
// 指定のページデータを書き出す
//
void PaintPage( RegViewWindowData* viewData, HWND hPageWnd, RegViewPageData* pageData )
{
	if( viewData && pageData && !pageData->hDeviceContext )
	{
		paintPage( pageData, hPageWnd, viewData->hFont );
	}
}


//----------------------------------------------
// 頁データをウィンドウにコピーする
//
void CopyPageData( HDC hPageDC, RegViewWindowData* viewData, RegViewPageData* pageData )
{
#define ROP SRCCOPY		/* ラスタ演算式 */

	SetStretchBltMode( hPageDC, HALFTONE );
	SetBrushOrgEx( hPageDC, 0, 0, NULL );


	/*---- デバイスコンテキストのカラー反転 ----*/
	if( !pageData->invert )
	{
		RECT rect = { 0, 0, pageData->width, pageData->height };
		InvertRect( pageData->hDeviceContext, &rect );
		pageData->invert = true;
	}

	
	/*---- デバイスコンテキストコピー ----*/
	switch( viewData->mode )
	{
	case 0:
		BitBlt( hPageDC, 0, 0, pageData->width, pageData->height, pageData->hDeviceContext, 0, 0, ROP );
		break;
	case 1:
		if( pageData->width <= viewData->categorySize
			&& pageData->height <= viewData->categorySize )
		{// 収まるのでセンタリングしてそのまま表示
			int x = (viewData->categorySize - pageData->width) / 2;
			int y = (viewData->categorySize - pageData->height) / 2;
			BitBlt( hPageDC, x, y, pageData->width, pageData->height, pageData->hDeviceContext, 0, 0, ROP );
		}
		else
		{// 縮小して表示
			int x = 0, y = 0;
			int width = pageData->width, height = pageData->height;
			if( width >= height )
			{// 横幅が広い場合は縦にセンタリング
				int sh = (int)((float)height * ((float)viewData->categorySize / (float)width));
				y = (viewData->categorySize - sh) / 2;
				StretchBlt( hPageDC, x, y, viewData->categorySize, sh, pageData->hDeviceContext, 0, 0, width, height, ROP );
			}
			else
			{// 縦幅が広い場合は横にセンタリング
				int sw = (int)((float)width * ((float)viewData->categorySize / (float)height));
				x = (viewData->categorySize - sw) / 2;
				StretchBlt( hPageDC, x, y, sw, viewData->categorySize, pageData->hDeviceContext, 0, 0, width, height, ROP );
			}
		}
		break;
	}
}


//----------------------------------------------
// 登録済みの表示データを取得する
//
RegViewWindowData* GetViewWindowData( HWND hViewWnd )
{
	RegViewWindowData* ret = NULL;
	if( hViewWnd )
	{
		ret = (RegViewWindowData*)GetWindowLongPtrW( hViewWnd, 0 );

#ifdef _DEBUG
		// データの整合性チェック
		if( ret && ret->file )
		{
			if( ret->file->type < 0 || ret->file->type >= aafile::AAFILETYPE_MAX )
			{
				MessageBoxW( hViewWnd, L"ファイルタイプが不正です。", L"エラー", MB_OK );
			}
		}
#endif
	}
	return ret;
}


//----------------------------------------------
// 登録済みのページデータを取得する
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
// ツールチップ表示を設定する
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
// カテゴリビュー　頁表示
//
static void setCategoryViewSize( HWND hViewWnd, int width, int height, int itemSize )
{
	/*---- スクロールバーの状態を取得 ----*/
	SCROLLINFO sc;
	ZeroMemory( &sc, sizeof( sc ) );
	sc.cbSize = sizeof( sc );
	sc.fMask = SIF_ALL;
	::GetScrollInfo( hViewWnd, SB_VERT, &sc );


	/*---- ウィンドウサイズを設定 ----*/
	// 子ウィンドウのサイズを設定していく
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


	/*---- スクロールバーの情報を更新 ----*/
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
// ノーマルビュー　頁表示
//
static void setNormalViewSize( HWND hViewWnd, int width, int height )
{
	/*---- スクロールバーの状態を取得 ----*/
	SCROLLINFO sc;
	ZeroMemory( &sc, sizeof( sc ) );
	sc.cbSize = sizeof( sc );
	sc.fMask = SIF_ALL;
	::GetScrollInfo( hViewWnd, SB_VERT, &sc );


	/*---- ウィンドウサイズを設定 ----*/
	// 子ウィンドウのサイズを設定していく
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

		// 上方向の設定
		y = 0;
		for( size_t i = sc.nPos; i > 0; i-- )
		{
			RegViewPageData* pageData = GetViewPageData( data->pages[ i - 1 ] );
			y -= pageData->height;
			MoveWindow( data->pages[ i - 1 ], 0, y-1, width, pageData->height+1, TRUE );
		}
	}


	/*---- スクロールバーの情報を更新 ----*/
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
// デバイスコンテキストにAAを書き込む
//
static void paintPage( RegViewPageData* data, HWND hPageWnd, HFONT hFont )
{
	/*---- デバイスコンテキストがない場合は生成 ----*/
	if( !data->hDeviceContext )
	{
		createPageDC( data, hPageWnd, hFont );
	}


	/*---- デバイスコンテキストの設定 ----*/
	HFONT hOldFont = (HFONT)SelectObject( data->hDeviceContext, hFont );
	int oldMode = SetBkMode( data->hDeviceContext, TRANSPARENT );
	int oldTextColor = SetTextColor( data->hDeviceContext, RGB( 0xff, 0xff, 0xff ) );


	/*---- 一文字ずつ出力 ----*/
	std::wstring lfString = L"";
	int y = 0;
	for( unsigned long pos = 0, step = 1; pos < data->page->valueLength; pos += step )
	{
		bool lf = false;
		step = 1;

		// 改行チェック
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
			/*---- 表示文字列取得 ----*/
			wchar_t ch[ 4 ] = { 0 };
			getCharWidth( data, &data->page->value[ pos ], data->page->valueLength, step, ch );
			lfString += ch;
		}
		else
		{
			/*-- ライン出力 --*/
			TextOutW( data->hDeviceContext, 0, y, lfString.c_str(), lfString.size() );
			lfString.clear();
			y += *data->lfHeight;
		}
	}
	TextOutW( data->hDeviceContext, 0, y, lfString.c_str(), lfString.size() );


	/*---- デバイスコンテキストの設定 ----*/
	SelectObject( data->hDeviceContext, hOldFont );
	SetBkMode( data->hDeviceContext, oldMode );
	SetTextColor( data->hDeviceContext, oldTextColor );
}


//----------------------------------------------
// メモリデバイスコンテキストの生成
//
static void createPageDC( RegViewPageData* data, HWND hPageWnd, HFONT hFont )
{
	/*---- サイズを計算 ----*/
	getPageSize( data );


	/*---- ビットマップインフォを作る ----*/
	BITMAPINFO bmpInfo;
	ZeroMemory( &bmpInfo, sizeof( ::BITMAPINFO ) );
	bmpInfo.bmiHeader.biSize = sizeof( ::BITMAPINFO );
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biWidth = data->width;
	bmpInfo.bmiHeader.biHeight = -data->height;


	/*---- ビットマップを生成する ----*/
	HDC hdc = GetDC( hPageWnd );
	void* bmpBuf;
	data->hDeviceContextBMP = CreateDIBSection( hdc, &bmpInfo, DIB_RGB_COLORS, &bmpBuf, NULL, 0 );
	data->hDeviceContext = CreateCompatibleDC( hdc );
	data->hOldDeviceContextBMP = (HBITMAP)SelectObject( data->hDeviceContext, data->hDeviceContextBMP );
	ReleaseDC( hPageWnd, hdc );
}


//----------------------------------------------
// 頁サイズを計算
//
static void getPageSize( RegViewPageData* data )
{
	/*---- 状態を初期化 ----*/
	data->lfCount = 1;
	data->height = data->width = 0;


	/*---- 文字幅を計算 ----*/
	int width = 0;
	for( unsigned long pos = 0, step = 1; pos < data->page->valueLength; pos += step )
	{
		bool lf = false;
		step = 1;

		// 改行チェック
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


	/*---- 高さを計算 ----*/
	data->height = data->lfCount * (*data->lfHeight);
}


//----------------------------------------------
// 先頭文字の文字幅と文字コードを取得
//
static int getCharWidth( RegViewPageData* data, const wchar_t* str, size_t length, unsigned long& step, wchar_t* ch )
{
	int ret = 0;
	wchar_t refChar[ 2 ];
	unsigned long refCount;

	if( !checkRefChar( str, length, refChar, step, refCount ) )
	{// 参照文字解決しないもの
		step = 1;
		ret = data->charWidthTable[ str[ 0 ] ];
		if( ch ) *ch = str[ 0 ];
	}
	else
	{// 参照文字解決済みのもの
		if( refCount == 1 )
		{// 非サロゲート文字
			ret = data->charWidthTable[ refChar[ 0 ] ];
			if( ch ) *ch = refChar[ 0 ];
		}
		else
		{// サロゲート文字…だけど、どうしよう
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
// 参照文字列かチェックし、参照文字列ならば参照を解決する
//
bool CheckRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount )
{
	return checkRefChar( str, length, refChar, step, refCount );
}
static bool checkRefChar( const wchar_t* str, unsigned long length, wchar_t refChar[2], unsigned long& step, unsigned long& refCount )
{
	bool ret = false;

	if( str[ 0 ] == L'&' && length >= 2 )
	{// 参照文字列
		if( str[ 1 ] != L'#' )
		{// 実体参照
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
		{// 数値参照
			wchar_t*		endPtr;
			unsigned long	code;

			if( towlower( str[ 2 ] ) != L'x' )
			{// 10進数参照
				code = wcstol( &str[ 2 ], &endPtr, 10 );
			}
			else
			{// 16進数参照
				code = wcstol( &str[ 3 ], &endPtr, 16 );
			}

			if( endPtr[ 0 ] == L';' )
			{// 参照文字列であった
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
