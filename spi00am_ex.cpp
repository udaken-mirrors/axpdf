/*
SUSIE32 '00AM' Plug-in for LZH(LH0�̂�)
*/

#include <windows.h>
#include "spi00am.h"
#include "spi00am_ex.h"

//---------------------------------------------------------------------------
/* �G���g���|�C���g */
BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	g_hModule = hModule;
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			llog("DLL_PROCESS_ATTACH");
			loadConfig();
			break;
		case DLL_THREAD_ATTACH:
			llog("DLL_THREAD_ATTACH");
			break;
		case DLL_THREAD_DETACH:
			llog("DLL_THREAD_DETACH");
			break;
		case DLL_PROCESS_DETACH:
			llog("DLL_PROCESS_DETACH");
			break;
	}
	return TRUE;
}

//---------------------------------------------------------------------------
/*
�w�b�_�����đΉ��t�H�[�}�b�g���m�F.
�Ή����Ă�����̂Ȃ�true,�����łȂ����false��Ԃ�.
filname�̓t�@�C����(NULL�������Ă��邱�Ƃ�����).
data�̓t�@�C���̃w�b�_��,�T�C�Y�� HEADBUF_SIZE.
*/
/* 2KB�ȓ��� */
BOOL IsSupportedEx(char *filename, char *data)
{
	llog("IsSupportedEx");
	llog(filename);
	if (memcmp(data, "%PDF-", 5) == 0) return TRUE;
	fz_document *doc;
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	int ret = openDocument(ctx, &doc, filename);
	if (ret != SPI_ALL_RIGHT) { return FALSE; }
	fz_close_document(doc);
	fz_free_context(ctx);
	return TRUE;
}
//---------------------------------------------------------------------------
/*
filname�̓t�@�C����.
len�͓ǂݍ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���).
lphInf�̓t�@�C�����̓������n���h�����󂯎��ϐ��ւ̃|�C���^.
�A�[�J�C�u�����擾.�G���[�R�[�h��Ԃ�.

���A�[�J�C�u���͓Ǝ��ɂ�����x�L���b�V������B
GetArchiveInfo �ł� *�A�[�J�C�u�t�@�C��* �̈�v������̂��Ԃ����B
GetFileInfo �ł̓A�[�J�C�u���� *�t�@�C����* �̈�v������̂��Ԃ����B
GetFileEx �œn����� fileInfo �� *position* �̈�v������̂��n�����B
*/
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf)
{
	llog("GetArchiveInfoEx");
	fz_document *doc;
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	fz_page *page;

	int fcount = 0;
	unsigned long filesize;
	int ret;

	fz_try(ctx) {
		ret = openDocument(ctx, &doc, filename);
		if (ret != SPI_ALL_RIGHT) {return ret;}
		fcount = fz_count_pages(doc);
		*lphInf = LocalAlloc(LPTR, sizeof(fileInfo)*(fcount+1));
		if (*lphInf == NULL) {
			fz_close_document(doc);
			return SPI_NO_MEMORY;
		}

		fileInfo *pinfo = (fileInfo *)*lphInf;
		for (int i=0; i < fcount; i++) {
			/* Method(7+'\0'�ȓ��ł̓K���ȕ������ԋp) */
			lstrcpy((char*)pinfo->method, "MuPDF");
			fz_try(ctx) {
				page = fz_load_page(doc, i);
			} fz_catch(ctx) {
				fz_close_document(doc);
				return SPI_OUT_OF_ORDER;
			}
			ret = GetBitmapToMemory(ctx, NULL, &filesize, doc, page, g_resolution);
			if (ret != 0) { fz_close_document(doc); return ret;}
			fz_free_page(doc, page);
			pinfo->position = i;
			pinfo->compsize = filesize;
			pinfo->filesize = filesize;
			pinfo->timestamp = i;
			sprintf((char*)pinfo->filename, "%03d.bmp", i);
			pinfo->crc = 0;
			pinfo++;
		}
		pinfo->method[0] = '\0';

		fz_close_document(doc);
	} fz_catch(ctx) {
		fz_close_document(doc);
		return SPI_FILE_READ_ERROR;
	}
	fz_free_context(ctx);

	llog("GetArchiveInfoEx,SPI_ALL_RIGHT");
	return SPI_ALL_RIGHT;
}
//---------------------------------------------------------------------------
/*
dest,�t�@�C���̓�����LOCAL�������[�n���h�����󂯎��ϐ��ւ̃|�C���^.
�w�肳�ꂽfileInfo�\���̂ւ̃|�C���^.
�G���[�R�[�h��Ԃ�.
*/
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo,
			SPI_PROGRESS lpPrgressCallback, long lData)
{
	llog("GetFileEx");
	fz_document *doc;
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	fz_page *page;

	unsigned long filesize;
	int ret;

	fz_try(ctx) {
		ret = openDocument(ctx, &doc, filename);
		if (ret != SPI_ALL_RIGHT) {return ret;}
		llog("GetFileEx loadPage");
		fz_try(ctx) {
			page = fz_load_page(doc, pinfo->position);
		} fz_catch(ctx) {
			fz_close_document(doc);
			return SPI_OUT_OF_ORDER;
		}
		ret = GetBitmapToMemory(ctx, dest, &filesize, doc, page, g_resolution);
		llog("GetFileEx loadPageEnd");
		if (ret) {
			llog("GetFileEx loadPage Error");
		}

		fz_close_document(doc);
	} fz_catch(ctx) {
		fz_close_document(doc);
		return SPI_FILE_READ_ERROR;
	}
	fz_free_context(ctx);

	return ret;
}

