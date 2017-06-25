#include "common.h"

int g_resolution = 144;//dpi
char iniFile[MAX_PATH];
HANDLE g_hModule;
int g_isUseTemp = 0;
void loadConfig()
{
	llog("loadConfig");
	GetModuleFileName((HMODULE)g_hModule, iniFile, MAX_PATH);
	strcpy(iniFile + strlen(iniFile) - 4, ".ini");
	g_resolution = GetPrivateProfileInt("Setting", "DPI", g_resolution, iniFile);
}
int getPasswordSuggest(int num, char *buf, int len)
{
	if (num < 0) num = -num;
	char key[64];
	sprintf(key, "PASS%d", num);
	GetPrivateProfileString("Password", key, "", buf, len, iniFile);
	return num + 1;
}

//hLocalにNULLを渡すとbuf_sizeにファイルサイズが返ってくる
int GetBitmapToMemory(fz_context *ctx, HLOCAL *hLocal, unsigned long *buf_size, fz_document *doc, fz_page *page, int resolution) {
	fz_device *dev;
	fz_colorspace *colorspace = fz_device_bgr(ctx);

	float zoom = (float)resolution / 72;
	fz_matrix ctm;
	fz_irect bbox;
	fz_pixmap *pix;
	fz_glyph_cache_s *glyphcache;
	fz_rect rect;

	ctm = *fz_scale(&ctm, zoom, zoom);
	rect = *fz_bound_page(ctx, page, &rect);
	bbox = *fz_round_rect(&bbox, fz_transform_rect(&rect, &ctm));

	unsigned long bitLength;
	const int bytePerPixel = 4;
	int width = bbox.x1 - bbox.x0,
		height = bbox.y1 - bbox.y0;
	if ((width * bytePerPixel) % 4 == 0) {
		bitLength = width * bytePerPixel;
	}
	else {
		bitLength = width * bytePerPixel + (4 - (width * bytePerPixel) % 4);
	}
	*buf_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bitLength*height;
	if (hLocal == NULL) {	//filesizeの取得
		return 0;
	}

	pix = fz_new_pixmap_with_bbox(ctx, colorspace, &bbox, TRUE);
	fz_clear_pixmap_with_value(ctx, pix, 255);

	dev = fz_new_draw_device(ctx, NULL, pix);
	fz_run_page(ctx, page, dev, &ctm, NULL);
	fz_drop_device(ctx, dev);

	BITMAPFILEHEADER fileHeader = {0};
	BITMAPINFOHEADER infoHeader = {0};
	fileHeader.bfType = 'M' * 256 + 'B';
	fileHeader.bfSize = *buf_size;
	fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(infoHeader);
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width;
	infoHeader.biHeight = -height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = bytePerPixel * 8;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = fileHeader.bfSize;
	infoHeader.biXPelsPerMeter = infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;

	*hLocal = LocalAlloc(LMEM_FIXED, *buf_size);
	if (*hLocal == NULL) {
		return SPI_NO_MEMORY;
	}

	unsigned char *buf = (unsigned char *)*hLocal;
	memcpy(buf, &fileHeader, sizeof(fileHeader));
	buf += sizeof(fileHeader);
	memcpy(buf, &infoHeader, sizeof(infoHeader));
	buf += sizeof(infoHeader);

	unsigned char *samples = fz_pixmap_samples(ctx, pix);
#if 0
	for (int i = 0; i < height; i++) {
		unsigned char *pd = buf + (height - i - 1)*bitLength;
		for (int j = 0; j < width; j++) {
			*(pd + 0) = *(samples + 2);
			*(pd + 1) = *(samples + 1);
			*(pd + 2) = *(samples + 0);
			pd += 3;
			samples += 3;
		}
	}
#else
	memcpy(buf, samples, fz_pixmap_stride(ctx, pix) * height);
#endif
	fz_drop_pixmap(ctx, pix);
	return 0;
}

int openDocument(fz_context *ctx, fz_document **doc, char *filename) {
	ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	HLOCAL utf8name;
	fz_try(ctx) {
		fz_register_document_handlers(ctx);
		fz_try(ctx) {
			GetUtf8FromSjis(&utf8name, filename);
			*doc = fz_open_document(ctx, (char*)utf8name);
		} fz_catch(ctx) {
			LocalFree(utf8name);
			return SPI_FILE_READ_ERROR;
		}
		LocalFree(utf8name);
		if (fz_needs_password(ctx, *doc)) {
			int num = 0;
			char pass[PASS_LENGTH];
			while (1) {
				num = getPasswordSuggest(num, pass, PASS_LENGTH);
				if (pass[0] == '\0') break;
				if (fz_authenticate_password(ctx, *doc, pass)) {
					llog("Password OK");
					break;
				}
			}
			if (pass[0] == '\0') {
				llog("Password not found");
				fz_drop_document(ctx, *doc);
				return SPI_FILE_READ_ERROR;
			}
		}
	} fz_catch(ctx) {
		fz_drop_document(ctx, *doc);
		return SPI_FILE_READ_ERROR;
	}
	return SPI_ALL_RIGHT;
}

//	SJISの文字列を渡すとUTF-8に変換して返す。
//	第一引数に返ってくるが、返ってきた変数はLocalFreeをすること
//	HLOCALは(char*)でキャストして使う。
//	ゼロクリアしてないので稀にバグるかも。
void GetUtf8FromSjis(HLOCAL *dst, char *src) {
	int size;
	HLOCAL hLocal;
	size = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, NULL, 0);
	hLocal = LocalAlloc(LMEM_FIXED, size * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, (LPWSTR)hLocal, size);
	int size8;
	size8 = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)hLocal, -1, NULL, 0, NULL, NULL);
	*dst = LocalAlloc(LMEM_FIXED, size8 * 2);
	ZeroMemory(*dst, size8 * 2);
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)hLocal, -1, (LPSTR)*dst, size8, NULL, NULL);
	LocalFree(hLocal);
}

#ifdef MY_COMMON_DEBUG
void llog(char *s) { FILE *fp = fopen("r:/output.log", "a"); fprintf(fp, "%s\n", s); fclose(fp); }
#endif
