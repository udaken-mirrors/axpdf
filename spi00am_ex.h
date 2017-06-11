/*
SUSIE32 '00AM' Plug-in for LZH(LH0�̂�)
*/

#ifndef spi00am_ex_h
#define spi00am_ex_h

#include <windows.h>
#include "spi00am.h"
#include "common.h"

/*-------------------------------------------------------------------------*/
// ����Plugin�̏��
/*-------------------------------------------------------------------------*/
static const char *pluginfo[] = {
	"00AM",					/* Plug-in API�o�[�W���� */
	"PDF to BMP with MuPDF(in SumatraPDF-2.2.1) Ver1.15 by mimizuno",	/* Plug-in���A�o�[�W�����y�� copyright */
	"*.PDF;*.XPS",				/* ��\�I�Ȋg���q ("*.JPG" "*.RGB;*.Q0" �Ȃ�) */
	"PDF",	/* �t�@�C���`���� */
};


// �w�b�_�`�F�b�N���ɕK�v�ȃT�C�Y.2KB�ȓ���
#define HEADBUF_SIZE (2048)

BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
BOOL IsSupportedEx(char *filename, char *data);
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf);
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo,
			SPI_PROGRESS lpPrgressCallback, long lData);

#endif
