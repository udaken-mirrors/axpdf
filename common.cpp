#include "common.h"

int g_resolution = 144;//dpi
char iniFile[MAX_PATH];
HANDLE g_hModule;
int g_isUseTemp = 0;
void loadConfig()
{
	llog("loadConfig");
	GetModuleFileName((HMODULE)g_hModule, iniFile, MAX_PATH);
	strcpy(iniFile+strlen(iniFile)-4, ".ini");
	g_resolution = GetPrivateProfileInt("Setting", "DPI", g_resolution, iniFile);
}
int getPasswordSuggest(int num, char *buf, int len)
{
	if (num < 0) num = -num;
	char key[64];
	sprintf(key, "PASS%d", num);
	GetPrivateProfileString("Password", key, "", buf, len, iniFile);
	return num+1;
}

//hLocal��NULL��n����buf_size�Ƀt�@�C���T�C�Y���Ԃ��Ă���
int GetBitmapToMemory(fz_context *ctx, HLOCAL *hLocal, unsigned long *buf_size, fz_document *doc, fz_page *page, int resolution) {
	fz_display_list *list;
	fz_device *dev;
	fz_colorspace *colorspace = fz_device_rgb;

	float zoom = (float)resolution/72;
	fz_matrix ctm;
	fz_bbox bbox;
	fz_pixmap *pix;
	fz_glyph_cache_s *glyphcache;
	fz_rect rect;

	ctm = fz_scale(zoom, zoom);
	rect = fz_bound_page(doc, page);
	bbox = fz_round_rect(fz_transform_rect(ctm, rect));
	
	unsigned long bitLength;
	int width = bbox.x1-bbox.x0,
		height = bbox.y1-bbox.y0;
	if ((width*3) % 4 == 0) {
		bitLength = width*3;
	} else {
		bitLength = width*3 + (4-(width*3)%4);
	}
	*buf_size = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bitLength*height;
	if (hLocal == NULL) {	//filesize�̎擾
		return 0;
	}

	pix = fz_new_pixmap_with_bbox(ctx, colorspace, bbox);
	fz_clear_pixmap_with_value(ctx, pix, 255);

	dev = fz_new_draw_device(ctx, pix);
	fz_run_page(doc, page, dev, ctm, NULL);
	fz_free_device(dev);

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));
	memset(&infoHeader, 0, sizeof(infoHeader));
	fileHeader.bfType = 'M'*256+'B';
	fileHeader.bfSize = *buf_size;
	fileHeader.bfOffBits = sizeof(fileHeader)+sizeof(infoHeader);
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	infoHeader.biSize = 40;
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
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

	unsigned char *ps = fz_pixmap_samples(ctx, pix);
	for (int i=0;i<height;i++) {
		unsigned char *pd = buf + (height-i-1)*bitLength;
		for (int j=0;j<width;j++) {
			*(pd+0) = *(ps+2);
			*(pd+1) = *(ps+1);
			*(pd+2) = *(ps+0);
			pd+=3;
			ps+=4;
		}
	}

	fz_drop_pixmap(ctx, pix);
	return 0;
}

int openDocument(fz_context *ctx, fz_document **doc, char *filename) {
	ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	HLOCAL utf8name;
	fz_try(ctx) {
		fz_try(ctx) {
			GetUtf8FromSjis(&utf8name, filename);
			*doc = fz_open_document(ctx, (char*)utf8name);
		} fz_catch(ctx) {
			LocalFree(utf8name);
			return SPI_FILE_READ_ERROR;
		}
		LocalFree(utf8name);
		if (fz_needs_password(*doc)) {
			int num=0;
			char pass[PASS_LENGTH];
			while (1) {
				num = getPasswordSuggest(num, pass, PASS_LENGTH);
				if (pass[0] == '\0') break;
				if (fz_authenticate_password(*doc, pass)) {
					llog("Password OK");
					break;
				}
			}
			if (pass[0] == '\0') {
				llog("Password not found");
				fz_close_document(*doc);
				return SPI_FILE_READ_ERROR;
			}
		}
	} fz_catch(ctx) {
		fz_close_document(*doc);
		return SPI_FILE_READ_ERROR;
	}
	return SPI_ALL_RIGHT;
}

//	SJIS�̕������n����UTF-8�ɕϊ����ĕԂ��B
//	�������ɕԂ��Ă��邪�A�Ԃ��Ă����ϐ���LocalFree�����邱��
//	HLOCAL��(char*)�ŃL���X�g���Ďg���B
//	�[���N���A���ĂȂ��̂ŋH�Ƀo�O�邩���B
void GetUtf8FromSjis(HLOCAL *dst, char *src) {
	int size;
	HLOCAL hLocal;
	size = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, NULL, 0);
	hLocal = LocalAlloc(LMEM_FIXED, size*2+2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, (LPWSTR)hLocal, size);
	int size8;
	size8 = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)hLocal, -1, NULL, 0, NULL, NULL);
	*dst = LocalAlloc(LMEM_FIXED, size8*2);
	ZeroMemory(*dst, size8*2);
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)hLocal, -1, (LPSTR)*dst, size8, NULL, NULL);
	LocalFree(hLocal);
}

#ifdef MY_COMMON_DEBUG
void llog(char *s) { FILE *fp=fopen("r:/output.log", "a");fprintf(fp,"%s\n",s);fclose(fp);}
#endif
