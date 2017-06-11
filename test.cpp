#include <windows.h>
#include <stdio.h>
#include "common.h"

int main(int argc, char *argv[])
{
	fz_document *doc;
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	fz_page *page;
	int pagenum = 0;
	int getnum = 0;
	int g_resolution = 96;//dpi
	HLOCAL hLocal;
	unsigned long filesize;
	char *filename = NULL;

	if (argc > 1) {
		filename = argv[1];
	} else {
		printf("Usage: %s input.pdf\n", argv[0]);
		return 0;
	}
	if (argc > 2) {
		pagenum = argv[2][0]-'0';
	}
	if (argc > 3) {
		getnum = argv[3][0] - '0';
	}

	if (ctx == NULL) {
		return 0;
	}

	loadConfig();

	fz_try(ctx) {
		int ret;
		ret = openDocument(ctx, &doc, filename);
		if (ret != SPI_ALL_RIGHT) {return ret;}
		/*
		HLOCAL filename2;
		GetUtf8FromSjis(&filename2, filename);
		ret = openDocument(ctx, &doc, (char*)filename2);
		if (ret != SPI_ALL_RIGHT) {
			return 0;
		}
		LocalFree(filename2);
		*/
		page = fz_load_page(doc, pagenum);
		GetBitmapToMemory(ctx, &hLocal, &filesize, doc, page, g_resolution);
		printf("filesize:%d\n", filesize);
		unsigned char *p = (unsigned char *)hLocal;
		FILE *fp = fopen("output.bmp", "wb");
		for (int i=0;i<filesize;i++) {
			putc(p[i], fp);
		}
		fclose(fp);
		LocalFree(hLocal);
		fz_free_page(doc, page);
		fz_close_document(doc);
	} fz_catch(ctx) {
		return SPI_FILE_READ_ERROR;
	}
	printf("End\n");
	fz_free_context(ctx);

	return 0;
}
