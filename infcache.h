//fileInfo�L���b�V���N���X

#ifndef infcache_h
#define infcache_h

#include <windows.h>
#include "critsect.h"
#include "spi00am.h"

typedef struct ArcInfo
{
	HLOCAL hinfo;			// fileInfo[]
	char path[MAX_PATH];	// �t�@�C���p�X
} ArcInfo;

#define INFOCACHE_MAX 0x10
class InfoCache
{
public:
	InfoCache();
	~InfoCache();
	void Clear(void); //�L���b�V���N���A
	void Add(char *filepath, HLOCAL *ph); //�L���b�V���ɒǉ��BINFOCACHE_MAX �𒴂���ƌÂ��̂͏����B
	//�L���b�V���ɂ���΃A�[�J�C�u�����R�s�[�B
	int Dupli(char *filepath, HLOCAL *ph, fileInfo *pinfo);
private:
	CriticalSection cs;
	ArcInfo arcinfo[INFOCACHE_MAX];
	int nowno;
	bool GetCache(char *filepath, HLOCAL *ph);
};

#endif
