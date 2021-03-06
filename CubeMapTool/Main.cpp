#include "stdafx.h"


static void splitfilename(const char *name, char *fname, char *ext)
{
	const char *p = NULL;
	const char *c = NULL;
	const char *base = name;

	for (p = base; *p; p++) {
		if (*p == '/' || *p == '\\') {
			do {
				p++;
			} while (*p == '/' || *p == '\\');

			base = p;
		}
	}

	size_t len = strlen(base);
	for (p = base + len; p != base && *p != '.'; p--);
	if (p == base && *p != '.') p = base + len;

	if (fname) {
		for (c = base; c < p; c++) {
			fname[c - base] = *c;
		}

		fname[c - base] = 0;
	}

	if (ext) {
		for (c = p; c < base + len; c++) {
			ext[c - p] = *c;
		}

		ext[c - p] = 0;
	}
}

static int GetStrIndex(int argc, char** argv, const char* str)
{
	for (size_t i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], str) == 0)
			return i;
	}
	return -1;
}

#pragma region IrradianceMap

static void SaveIrradianceMap(CUBEMAP *pIrradianceMap, int width, int height)
{
	gli::texture_cube textureCube(gli::texture::format_type::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(width, height), 1);
	{
		for (int face = 0; face < 6; face++) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					COLORREF color = IMAGE_GetPixelColor(&pIrradianceMap->faces[face], x, y);
					textureCube.store(gli::extent2d(x, y), face, 0, color);
				}
			}
		}
	}
	gli::save_dds(textureCube, "IrradianceMap.dds");

	IMAGE imgPreview;
	IMAGE_ZeroImage(&imgPreview);
	{
		char szDebugFileName[] = "Preview_Irradiance.bmp";
		PreviewMap(pIrradianceMap, &imgPreview);
		IMAGE_SaveBmp(szDebugFileName, &imgPreview);
	}
	IMAGE_FreeImage(&imgPreview);
}

static void GenerateEnvIrradianceMap(int argc, char** argv)
{
	if (argc != 5) {
		printf("Generate irradiance map fail!\n");
		printf("eg: CubeMapTool.exe -irr Env.bmp 128 128\n");
		return;
	}

	int width, height;
	char szEnvMapFileName[_MAX_PATH];

	strcpy(szEnvMapFileName, argv[2]);
	width = atoi(argv[3]);
	height = atoi(argv[4]);

	IMAGE envmap;
	IMAGE_ZeroImage(&envmap);
	IMAGE_LoadImage(szEnvMapFileName, &envmap);

	CUBEMAP irradiancemap;
	CubeMapInit(&irradiancemap);
	CubeMapAlloc(&irradiancemap, width, height, 24);

	{
		GenerateEnvIrradianceMap(&envmap, &irradiancemap, 1024);
		SaveIrradianceMap(&irradiancemap, width, height);
	}

	CubeMapFree(&irradiancemap);
	IMAGE_FreeImage(&envmap);

	return;
}

static void GenerateCubeIrradianceMap(int argc, char** argv)
{
	if (argc != 10) {
		printf("Generate irradiance map fail!\n");
		printf("eg: CubeMapTool.exe -irr PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp 128 128\n");
		return;
	}

	int width, height;
	char szCubeMapFileNames[6][_MAX_PATH];

	strcpy(szCubeMapFileNames[0], argv[2]);
	strcpy(szCubeMapFileNames[1], argv[3]);
	strcpy(szCubeMapFileNames[2], argv[4]);
	strcpy(szCubeMapFileNames[3], argv[5]);
	strcpy(szCubeMapFileNames[4], argv[6]);
	strcpy(szCubeMapFileNames[5], argv[7]);
	width = atoi(argv[8]);
	height = atoi(argv[9]);

	CUBEMAP cubemap;
	CubeMapInit(&cubemap);
	CubeMapLoad(&cubemap, szCubeMapFileNames);

	CUBEMAP irradiancemap;
	CubeMapInit(&irradiancemap);
	CubeMapAlloc(&irradiancemap, width, height, 24);

	{
		GenerateCubeIrradianceMap(&cubemap, &irradiancemap, 1024);
		SaveIrradianceMap(&irradiancemap, width, height);
	}

	CubeMapFree(&irradiancemap);
	CubeMapFree(&cubemap);
}

#pragma endregion

#pragma region Mipmap

static void SaveEnvMipmaps(IMAGE *pMipmaps, int mipLevels, int widthMipLevels, int heightMipLevels)
{
	gli::texture2d texture(gli::texture::format_type::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(1 << widthMipLevels, 1 << heightMipLevels));
	{
		for (int mip = 0; mip < mipLevels; mip++) {
			for (int y = 0; y < (1 << (heightMipLevels - mip)); y++) {
				for (int x = 0; x < (1 << (widthMipLevels - mip)); x++) {
					COLORREF color = IMAGE_GetPixelColor(&pMipmaps[mip], x, y);
					texture.store(gli::extent2d(x, y), mip, color);
				}
			}
		}
	}
	gli::save_dds(texture, "Mipmaps.dds");

	for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
		char szDebugFileName[_MAX_PATH];
		sprintf(szDebugFileName, "Preview_Mipmap%d.bmp", mipLevel);
		IMAGE_SaveBmp(szDebugFileName, &pMipmaps[mipLevel]);
	}
}

static void SaveCubeMipmaps(CUBEMAP *pMipmaps, int mipLevels)
{
	gli::texture_cube textureCube(gli::texture::format_type::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(1 << mipLevels, 1 << mipLevels));
	{
		for (int mip = 0; mip < mipLevels; mip++) {
			for (int face = 0; face < 6; face++) {
				for (int y = 0; y < (1 << (mipLevels - mip)); y++) {
					for (int x = 0; x < (1 << (mipLevels - mip)); x++) {
						COLORREF color = IMAGE_GetPixelColor(&pMipmaps[mip].faces[face], x, y);
						textureCube.store(gli::extent2d(x, y), face, mip, color);
					}
				}
			}
		}
	}
	gli::save_dds(textureCube, "Mipmaps.dds");

	for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
		IMAGE imgPreview;
		IMAGE_ZeroImage(&imgPreview);
		{
			char szDebugFileName[_MAX_PATH];
			sprintf(szDebugFileName, "Preview_Mipmap%d.bmp", mipLevel);
			PreviewMap(&pMipmaps[mipLevel], &imgPreview);
			IMAGE_SaveBmp(szDebugFileName, &imgPreview);
		}
		IMAGE_FreeImage(&imgPreview);
	}
}

static void GenerateEnvMipmaps(int argc, char** argv)
{
	if (argc < 3) {
		printf("Generate mipmaps fail!\n");
		printf("eg: CubeMapTool.exe -mip Env.bmp <-hdr> <-bm>\n");
		return;
	}
	bool isHDR = GetStrIndex(argc, argv, "-hdr");
	bool isBM = GetStrIndex(argc, argv, "-bm") > 0;	//Only Blue Changle Is HDR
	char szEnvMapFileName[_MAX_PATH];
	strcpy(szEnvMapFileName, argv[2]);

	IMAGE envmap;
	IMAGE_ZeroImage(&envmap);
	IMAGE_LoadImage(szEnvMapFileName, &envmap);

	const int MAX_LEVEL = 16;
	IMAGE mipmaps[MAX_LEVEL];
	memset(mipmaps, 0, sizeof(mipmaps));

	{
		int mipLevels = 0;
		int widthMipLevels = 0;
		int heightMipLevels = 0;
		while ((1 << widthMipLevels) < IMAGE_WIDTH(&envmap)) widthMipLevels++;
		while ((1 << heightMipLevels) < IMAGE_HEIGHT(&envmap)) heightMipLevels++;
		mipLevels = min(widthMipLevels, heightMipLevels);

		for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			int width = 1 << (widthMipLevels - mipLevel);
			int height = 1 << (heightMipLevels - mipLevel);
			IMAGE_AllocImage(&mipmaps[mipLevel], width, height, 32);
		}

		GenerateEnvMipmaps(&envmap, mipmaps, mipLevels, 1024, isHDR, isBM);
		SaveEnvMipmaps(mipmaps, mipLevels, widthMipLevels, heightMipLevels);
	}

	for (int index = 0; index < MAX_LEVEL; index++) {
		IMAGE_FreeImage(&mipmaps[index]);
	}

	IMAGE_FreeImage(&envmap);
}

static void GenerateSphereEnvMipmaps(int argc, char** argv)
{
	if (argc < 4) {
		printf("Generate mipmaps fail!\n");
		printf("eg: CubeMapTool.exe -mip -sphere Env.bmp <-hdr> <-bm>\n");
		return;
	}
	bool isHDR = GetStrIndex(argc, argv, "-hdr") > 0;
	bool isBM = GetStrIndex(argc, argv, "-bm") > 0;	//Only Blue Changle Is HDR
	char szEnvMapFileName[_MAX_PATH];
	strcpy(szEnvMapFileName, argv[3]);

	IMAGE envmap;
	IMAGE_ZeroImage(&envmap);
	IMAGE_LoadImage(szEnvMapFileName, &envmap);

	const int MAX_LEVEL = 16;
	IMAGE mipmaps[MAX_LEVEL];
	memset(mipmaps, 0, sizeof(mipmaps));

	{
		int mipLevels = 0;
		int widthMipLevels = 0;
		int heightMipLevels = 0;
		while ((1 << widthMipLevels) < IMAGE_WIDTH(&envmap)) widthMipLevels++;
		while ((1 << heightMipLevels) < IMAGE_HEIGHT(&envmap)) heightMipLevels++;
		mipLevels = max(widthMipLevels, heightMipLevels);

		for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			int width = 1 << (widthMipLevels - mipLevel);
			int height = 1 << (heightMipLevels - mipLevel);
			int size = max(height, width);
			IMAGE_AllocImage(&mipmaps[mipLevel], size, size, 32);
		}

		GenerateSphereEnvMipmaps(&envmap, mipmaps, mipLevels, 1024, isHDR, isBM);
		SaveEnvMipmaps(mipmaps, mipLevels, mipLevels, mipLevels);
	}

	for (int index = 0; index < MAX_LEVEL; index++) {
		IMAGE_FreeImage(&mipmaps[index]);
	}

	IMAGE_FreeImage(&envmap);
}

static void GenerateCubeMipmaps(int argc, char** argv)
{
	if (argc != 8) {
		printf("Generate mipmaps fail!\n");
		printf("eg: CubeMapTool.exe -mip PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp\n");
		return;
	}

	char szCubeMapFileNames[6][_MAX_PATH];
	strcpy(szCubeMapFileNames[0], argv[2]);
	strcpy(szCubeMapFileNames[1], argv[3]);
	strcpy(szCubeMapFileNames[2], argv[4]);
	strcpy(szCubeMapFileNames[3], argv[5]);
	strcpy(szCubeMapFileNames[4], argv[6]);
	strcpy(szCubeMapFileNames[5], argv[7]);

	CUBEMAP cubemap;
	CubeMapInit(&cubemap);
	CubeMapLoad(&cubemap, szCubeMapFileNames);

	const int MAX_LEVEL = 16;
	CUBEMAP mipmaps[MAX_LEVEL];
	memset(mipmaps, 0, sizeof(mipmaps));

	{
		int mipLevels = 0;
		while ((1 << mipLevels) < max(CUBEMAP_WIDTH(&cubemap), CUBEMAP_HEIGHT(&cubemap))) mipLevels++;

		for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			int size = 1 << (mipLevels - mipLevel);
			CubeMapAlloc(&mipmaps[mipLevel], size, size, 32);
		}

		GenerateCubeMipmaps(&cubemap, mipmaps, mipLevels, 1024);
		SaveCubeMipmaps(mipmaps, mipLevels);
	}

	for (int mipLevel = 0; mipLevel < MAX_LEVEL; mipLevel++) {
		CubeMapFree(&mipmaps[mipLevel]);
	}

	CubeMapFree(&cubemap);
}

#pragma endregion

#pragma region LUT

void GenerateLUT(char szFileName[_MAX_PATH], int width, int height)
{
	IMAGE image;
	IMAGE_ZeroImage(&image);
	{
		IMAGE_AllocImage(&image, width, height, 24);
		GenerateLUT(&image, 1024);
		IMAGE_SaveBmp(szFileName, &image);
	}
	IMAGE_FreeImage(&image);
}

void GenerateLUT(int argc, char** argv)
{
	int width, height;
	char szLUTFileName[_MAX_PATH];

	if (argc != 5) {
		goto ERR;
	}

	strcpy(szLUTFileName, argv[2]);

	width = atoi(argv[3]);
	height = atoi(argv[4]);

	GenerateLUT(szLUTFileName, width, height);

	goto RET;
ERR:
	printf("Generate lut fail!\n");
	printf("eg: CubeMapTool.exe -lut LUT.bmp 256 256\n");
RET:
	return;
}

#pragma endregion

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(0, 0);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("");
	glewInit();

	if (argc > 1) {
		char *opt = argv[1];

		if (stricmp(opt, "-irr") == 0) {
			if (argc == 5) {
				GenerateEnvIrradianceMap(argc, argv);
			}

			if (argc == 10) {
				GenerateCubeIrradianceMap(argc, argv);
			}

			return 0;
		}

		if (stricmp(opt, "-mip") == 0) {

			if (GetStrIndex(argc, argv, "-sphere") > 0) {
				GenerateSphereEnvMipmaps(argc, argv);
			}

			if (argc == 3 || argc == 4) {
				GenerateEnvMipmaps(argc, argv);
			}


			if (argc == 8) {
				GenerateCubeMipmaps(argc, argv);
			}

			return 0;
		}

		if (stricmp(opt, "-lut") == 0) {
			GenerateLUT(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-mipFishEye") == 0) {
			GenerateLUT(argc, argv);
			return 0;
		}
	}

	printf("Error: Params not match!\n");
	return 0;
}
