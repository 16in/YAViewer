/***********************************************************************
*
* AA関連ファイルアクセス用定義類
*
************************************************************************/
#pragma once


namespace aafile {


/**
* ファイル形式列挙体
*/
enum AAFileType
{
	AFT_TXT		= 0x00,			//!< 純粋なテキストファイル(.txt)
	AFT_MLT,					//!< MLTファイル(.mlt)
	AFT_AST,					//!< ASTファイル(.ast)
	AFT_ASD,					//!< ASDファイル(.asd)
	AFT_AADATA,					//!< AADataファイル(.txt)
	AFT_AALIST,					//!< AAListファイル(.txt)

	AAFILETYPE_MAX
};

/**
* ページデータ構造体
*/
struct AAFilePage
{
	const wchar_t*		name;			//!< ページ名
	const wchar_t*		value;			//!< ページ内容
	unsigned long		nameLength;		//!< ページ名文字列長
	unsigned long		valueLength;	//!< ページ内容文字列長
};

/**
* ファイル全体の構造体
*/
struct AAFile
{
	AAFileType			type;			//!< ファイルタイプ
	AAFilePage*			pageList;		//!< ページデータの一覧
	unsigned long		pageCount;		//!< ページ数

	const wchar_t*		fileData;		//!< ファイル読み込みから生成した場合のファイル内容
										///  文字列から生成した場合はNULL
};


}
