/***********************************************************************
*
* AA�֘A�t�@�C���A�N�Z�X�p��`��
*
************************************************************************/
#pragma once


namespace aafile {


/**
* �t�@�C���`���񋓑�
*/
enum AAFileType
{
	AFT_TXT		= 0x00,			//!< �����ȃe�L�X�g�t�@�C��(.txt)
	AFT_MLT,					//!< MLT�t�@�C��(.mlt)
	AFT_AST,					//!< AST�t�@�C��(.ast)
	AFT_ASD,					//!< ASD�t�@�C��(.asd)
	AFT_AADATA,					//!< AAData�t�@�C��(.txt)
	AFT_AALIST,					//!< AAList�t�@�C��(.txt)

	AAFILETYPE_MAX
};

/**
* �y�[�W�f�[�^�\����
*/
struct AAFilePage
{
	const wchar_t*		name;			//!< �y�[�W��
	const wchar_t*		value;			//!< �y�[�W���e
	unsigned long		nameLength;		//!< �y�[�W��������
	unsigned long		valueLength;	//!< �y�[�W���e������
};

/**
* �t�@�C���S�̂̍\����
*/
struct AAFile
{
	AAFileType			type;			//!< �t�@�C���^�C�v
	AAFilePage*			pageList;		//!< �y�[�W�f�[�^�̈ꗗ
	unsigned long		pageCount;		//!< �y�[�W��

	const wchar_t*		fileData;		//!< �t�@�C���ǂݍ��݂��琶�������ꍇ�̃t�@�C�����e
										///  �����񂩂琶�������ꍇ��NULL
};


}
