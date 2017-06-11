/*
SUSIE32 '00AM' Plug-in for LZH(LH0�̂�)
*/

#include <windows.h>
#include "spi00am.h"
#include "spi00am_ex.h"
#include "infcache.h"
#include "common.h"

//�O���[�o���ϐ�
InfoCache infocache; //�A�[�J�C�u���L���b�V���N���X

//---------------------------------------------------------------------------
/* �G���g���|�C���g */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	llog("DllMain");
	switch (ul_reason_for_call) {
		case DLL_PROCESS_DETACH:
			infocache.Clear();
			break;
	}
	return SpiEntryPoint(hModule, ul_reason_for_call, lpReserved);
}

/***************************************************************************
 * SPI�֐�
 ***************************************************************************/
//---------------------------------------------------------------------------
int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	llog("GetPluginInfo");
	if (infono < 0 || infono >= (sizeof(pluginfo) / sizeof(char *))) 
		return 0;

	lstrcpyn(buf, pluginfo[infono], buflen);

	return lstrlen(buf);
}
//---------------------------------------------------------------------------
int __stdcall IsSupported(LPSTR filename, DWORD dw)
{
	char *data;
	char buff[HEADBUF_SIZE];

	char buf[128];
	BYTE *d =(BYTE*)dw;
	if ((dw & 0xFFFF0000) == 0) {
	/* dw�̓t�@�C���n���h�� */
		llog("FileHandle");
		DWORD ReadBytes;
		if (!ReadFile((HANDLE)dw, buff, HEADBUF_SIZE, &ReadBytes, NULL)) {
			return 0;
		}
		data = buff;
	} else {
	/* dw�̓o�b�t�@�ւ̃|�C���^ */
		data = (char *)dw;
		llog("Buffer");
	}

	/* �t�H�[�}�b�g�m�F */
	if (IsSupportedEx(filename, data)) return 1;

	return 0;
}
//---------------------------------------------------------------------------
//�A�[�J�C�u�����L���b�V������
int GetArchiveInfoCache(char *filename, long len, HLOCAL *phinfo, fileInfo *pinfo)
{
	int ret = infocache.Dupli(filename, phinfo, pinfo);
	if (ret != SPI_NO_FUNCTION) return ret;

	//�L���b�V���ɖ���
	HLOCAL hinfo;
	HANDLE hf;
	char headbuf[HEADBUF_SIZE];
	DWORD ReadBytes;

	hf = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf == INVALID_HANDLE_VALUE) return SPI_FILE_READ_ERROR;
	if (SetFilePointer(hf, len, NULL, FILE_BEGIN) != (DWORD)len) {
		CloseHandle(hf);
		return SPI_FILE_READ_ERROR;
	}
	if (!ReadFile(hf, headbuf, HEADBUF_SIZE, &ReadBytes, NULL)) {
		CloseHandle(hf);
		return SPI_FILE_READ_ERROR;
	}
	CloseHandle(hf);
	if (ReadBytes != HEADBUF_SIZE) return SPI_NOT_SUPPORT;
	if (!IsSupportedEx(filename, headbuf)) return SPI_NOT_SUPPORT;
	
	ret = GetArchiveInfoEx(filename, len, &hinfo);
	if (ret != SPI_ALL_RIGHT) return ret;

	//�L���b�V��
	infocache.Add(filename, &hinfo);

if (phinfo != NULL) {
	UINT size = LocalSize(hinfo);
	/* �o�͗p�̃������̊��蓖�� */
	*phinfo = LocalAlloc(LMEM_FIXED, size);
	if (*phinfo == NULL) {
		return SPI_NO_MEMORY;
	}

	memcpy(*phinfo, (void*)hinfo, size);
} else {
	fileInfo *ptmp = (fileInfo *)hinfo;
	if (pinfo->filename[0] != '\0') {
		for (;;) {
			if (ptmp->method[0] == '\0') return SPI_NO_FUNCTION;
			if (lstrcmpi(ptmp->filename, pinfo->filename) == 0) break;
			ptmp++;
		}
	} else {
		for (;;) {
			if (ptmp->method[0] == '\0') return SPI_NO_FUNCTION;
			if (ptmp->position == pinfo->position) break;
			ptmp++;
		}
	}
	*pinfo = *ptmp;
}
	return SPI_ALL_RIGHT;
}
//---------------------------------------------------------------------------
int __stdcall GetArchiveInfo(LPSTR buf, long len, unsigned int flag, HLOCAL *lphInf)
{
	llog("GetArchiveInfo");
	//���������͂ɂ͑Ή����Ȃ�
	if ((flag & 7) != 0) return SPI_NO_FUNCTION;

	*lphInf = NULL;
	return GetArchiveInfoCache(buf, len, lphInf, NULL);
}
//---------------------------------------------------------------------------
int __stdcall GetFileInfo
(LPSTR buf, long len, LPSTR filename, unsigned int flag, struct fileInfo *lpInfo)
{
	//���������͂ɂ͑Ή����Ȃ�
	if ((flag & 7) != 0) return SPI_NO_FUNCTION;

	lstrcpy(lpInfo->filename, filename);

	return GetArchiveInfoCache(buf, len, NULL, lpInfo);
}
//---------------------------------------------------------------------------
int __stdcall GetFile(LPSTR src, long len,
			   LPSTR dest, unsigned int flag,
			   SPI_PROGRESS lpPrgressCallback, long lData)
{
	//�t�@�C���ւ̏o�͂͑Ή����Ă��Ȃ�
	if ((flag & 0x700) == 0) return SPI_NO_FUNCTION;
	//���������͂ɂ͑Ή����Ȃ�
	if ((flag & 7) != 0) return SPI_NO_FUNCTION;

	fileInfo info;
	info.filename[0] = '\0';
	info.position = len;
	int ret = GetArchiveInfoCache(src, 0, NULL, &info);
	if (ret != SPI_ALL_RIGHT) return ret;

	return GetFileEx(src, (HLOCAL *)dest, &info, lpPrgressCallback, lData);
}
//---------------------------------------------------------------------------

