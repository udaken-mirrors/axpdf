/*
SUSIE32 '00AM' Plug-in for LZH(LH0のみ)
*/

#include <windows.h>
#include "spi00am.h"
#include "spi00am_ex.h"

//---------------------------------------------------------------------------
/* エントリポイント */
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
ヘッダを見て対応フォーマットか確認.
対応しているものならtrue,そうでなければfalseを返す.
filnameはファイル名(NULLが入っていることもある).
dataはファイルのヘッダで,サイズは HEADBUF_SIZE.
*/
/* 2KB以内で */
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
filnameはファイル名.
lenは読み込み開始オフセット(MacBin対応のため).
lphInfはファイル情報の入ったハンドルを受け取る変数へのポインタ.
アーカイブ情報を取得.エラーコードを返す.

※アーカイブ情報は独自にある程度キャッシュする。
GetArchiveInfo では *アーカイブファイル* の一致するものが返される。
GetFileInfo ではアーカイブ内の *ファイル名* の一致するものが返される。
GetFileEx で渡される fileInfo は *position* の一致するものが渡される。
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
			/* Method(7+'\0'以内での適当な文字列を返却) */
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
dest,ファイルの入ったLOCALメモリーハンドルを受け取る変数へのポインタ.
指定されたfileInfo構造体へのポインタ.
エラーコードを返す.
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

