/*
SUSIE32 '00AM' Plug-in for LZH(LH0のみ)
*/

#ifndef spi00am_ex_h
#define spi00am_ex_h

#include <windows.h>
#include "spi00am.h"
#include "common.h"

/*-------------------------------------------------------------------------*/
// このPluginの情報
/*-------------------------------------------------------------------------*/
static const char *pluginfo[] = {
	"00AM",					/* Plug-in APIバージョン */
	"PDF to BMP with MuPDF(in SumatraPDF-2.2.1) Ver1.15 by mimizuno",	/* Plug-in名、バージョン及び copyright */
	"*.PDF;*.XPS",				/* 代表的な拡張子 ("*.JPG" "*.RGB;*.Q0" など) */
	"PDF",	/* ファイル形式名 */
};


// ヘッダチェック等に必要なサイズ.2KB以内で
#define HEADBUF_SIZE (2048)

BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
BOOL IsSupportedEx(char *filename, char *data);
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf);
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo,
			SPI_PROGRESS lpPrgressCallback, long lData);

#endif
