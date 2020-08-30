//----------------------------------------------------------------------
//
// YAViewer ���\�[�X�萔��`
//
//----------------------------------------------------------------------
#pragma once


//-------------------------------------
// �R���g���[������ID
//
#define IDC_FILETAB			0x0001			/* �t�@�C���^�u */


//------------------------------
// �A�C�R�����\�[�X
//
#define IDI_APP				0x1000			/* �A�v���P�[�V�����A�C�R�� */
#define IDI_TABCLOSE		0x1001			/* �^�u�N���[�Y�A�C�R�� */
#define IDI_TABCLOSE_SEL	0x1002			/* �^�u�N���[�Y�A�C�R�� �I�� */


//-------------------------------------
// ���C�����j���[���\�[�X
//
#define IDR_MAINMENU		0x1000			/* ���C�����j���[ */

// �t�@�C��
#define IDM_EXIT			0x1001			/* �t�@�C�� - �I�� */
#define IDM_FILEOPEN		0x1002			/* �t�@�C�� - �t�@�C�����J�� */
#define IDM_TABSAVE			0x1003			/* �t�@�C�� - �^�u��ۑ� */
#define IDM_TABOPEN			0x1004			/* �t�@�C�� - �^�u�𕜌� */
#define IDM_TREESAVE		0x1005			/* �t�@�C�� - �c���[��ۑ� */
#define IDM_TREEOPEN		0x1006			/* �t�@�C�� - �c���[�𕜌� */

// �\��
#define IDM_MOST_TOP		0x2000			/* �\�� - �őO�� */
#define IDM_NORMAL_VIEW		0x2001			/* �\�� - �ʏ�\�� */
#define IDM_CATEGORY_VIEW	0x2002			/* �\�� - �J�e�S���\�� */
#define IDM_EDITOR_VIEW		0x2003			/* �\�� - �G�f�B�^�\�� */
#define IDM_SPLIT_SWAP		0x2004			/* �\�� - �����\������ւ� */
#define IDM_VIEW_TOOLTIP	0x2005			/* �\�� - �c�[���`�b�v�\�� */
#define IDM_TAB_BUTTON		0x2006			/* �\�� - �^�u�{�^���\�� */
#define IDM_LOCK_TREE		0x2010			/* �\�� - �c���[���b�N */
#define IDM_LOCK_ASCII		0x2011			/* �\�� - �`�`���b�N */

// �\�� - �J�e�S���\���T�C�Y
#define IDM_CATEGORY_32		0x2100			/* �J�e�S���\���T�C�Y - 32 */
#define IDM_CATEGORY_64		0x2101			/* �J�e�S���\���T�C�Y - 64 */
#define IDM_CATEGORY_96		0x2102			/* �J�e�S���\���T�C�Y - 96 */
#define IDM_CATEGORY_128	0x2103			/* �J�e�S���\���T�C�Y - 128 */
#define IDM_CATEGORY_160	0x2104			/* �J�e�S���\���T�C�Y - 160 */
#define IDM_CATEGORY_192	0x2105			/* �J�e�S���\���T�C�Y - 192 */
#define IDM_CATEGORY_224	0x2106			/* �J�e�S���\���T�C�Y - 224 */
#define IDM_CATEGORY_256	0x2107			/* �J�e�S���\���T�C�Y - 256 */
#define IDM_CATEGORY_384	0x2108			/* �J�e�S���\���T�C�Y - 384 */
#define IDM_CATEGORY_512	0x2109			/* �J�e�S���\���T�C�Y - 512 */
#define IDM_CATEGORY_768	0x210a			/* �J�e�S���\���T�C�Y - 768 */
#define IDM_CATEGORY_1024	0x210b			/* �J�e�S���\���T�C�Y - 1024 */

// �\�� - �g���q�I��
#define IDM_ENABLE_TXT		0x2180			/* �g���q�I�� - .txt */
#define IDM_ENABLE_MLT		0x2181			/* �g���q�I�� - .mlt */
#define IDM_ENABLE_AST		0x2182			/* �g���q�I�� - .ast */
#define IDM_ENABLE_ASD		0x2183			/* �g���q�I�� - .asd */

// �ݒ�
#define IDM_CLIP_UNICODE	0x4100			/* �N���b�v�{�[�h���j�R�[�h���M */

// ���M��
#define IDM_SEND_FACEEDIT	0x4000			/* ���M�� - (�L�t�M)Edit */
#define IDM_SEND_ORINEDIT	0x4001			/* ���M�� - OrinrinEditor */

// �w���v
#define IDM_VERSION			0x7800			/* �w���v - �o�[�W���� */


//------------------------------
// �t�@�C���^�u�R���e�L�X�g���j���[���\�[�X
//
#define IDR_FILETABMENU			0x1100

#define IDM_CLOSEFILE			0x8000			/* �^�u - ���� */
#define IDM_ALL_CLOSE			0x8001			/* �^�u - ���ׂĕ��� */
#define IDM_CLOSE_OTHER			0x8002			/* �^�u - ���̃t�@�C���ȊO���ׂĕ��� */
#define IDM_TAB_COLOR			0x8100			/* �^�u - �^�u�F��ݒ� */
#define IDM_TAB_CLEAR_COLOR		0x8101			/* �^�u - �^�u�F���N���A */
#define IDM_TAB_OPEN_DIRECTORY	0x8200			/* �^�u - �t�@�C���̂���t�H���_���J�� */


//------------------------------
// �t�@�C���c���[�R���e�L�X�g���j���[���\�[�X
//
#define IDR_FILETREEDIR_MENU	0x1200
#define IDR_FILETREEFILE_MENU	0x1201

#define IDM_TREE_OPENTREE		0x9000			/* �c���[ - �f�B���N�g����W�J���� */
#define IDM_TREE_OPENALLDIRECTORY_TREE 0x9010	/* �c���[ - �w��f�B���N�g���ȉ������ׂēW�J���� */
#define IDM_TREE_CLOSEALLDIRECTORY_TREE 0x9020	/* �c���[ - �w��f�B���N�g���ȉ������ׂĕ��� */
#define	IDM_TREE_OPENFILE		0x9400			/* �c���[ - �t�@�C�����J�� */
#define IDM_TREE_OPEN_DIRECTORY	0x9800			/* �c���[ - �G�N�X�v���[���ŊJ�� */
#define IDM_TREE_DELETEROOT		0x9801			/* �c���[ - �f�B���N�g�����폜���� */
#define IDM_TREE_ROOTITEM		0x9802			/* �c���[ - ���[�g�A�C�e���� */
