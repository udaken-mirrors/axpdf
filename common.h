#ifndef LOAD_MY_COMMON
#define LOAD_MY_COMMON

#include <windows.h>
#include <tchar.h>
extern "C" {
#include "fitz.h"
}
#include "spi00am.h"


#define PASS_LENGTH (256)

void loadConfig();
int getPasswordSuggest(int num, char *buf, int len);
int GetBitmapToMemory(fz_context *ctx, HLOCAL *hLocal, unsigned long *buf_size, fz_document *doc, fz_page *page, int resolution);
int openDocument(fz_context *ctx, fz_document **doc, char *filename);
void GetUtf8FromSjis(HLOCAL *dst, char *src);

extern int g_resolution;
extern char iniFile[];
extern HANDLE g_hModule;

#if 0
#define MY_COMMON_DEBUG
void llog(char *s);
#else
#define llog(s) (void)0
#endif

#endif
