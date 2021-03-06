#include "stdafx.h"


void CubeMapInit(CUBEMAP *pCubeMap)
{
	for (int index = 0; index < 6; index++) {
		IMAGE_ZeroImage(&pCubeMap->faces[index]);
	}
}

void CubeMapFree(CUBEMAP *pCubeMap)
{
	for (int index = 0; index < 6; index++) {
		IMAGE_FreeImage(&pCubeMap->faces[index]);
	}
}

BOOL CubeMapAlloc(CUBEMAP *pCubeMap, int width, int height, int bitcount)
{
	int rcode = NO_ERROR;

	for (int index = 0; index < 6; index++) {
		rcode = IMAGE_AllocImage(&pCubeMap->faces[index], width, height, bitcount);
		if (rcode != NO_ERROR) goto ERR;
	}

	goto RET;
ERR:
	CubeMapFree(pCubeMap);
	return FALSE;
RET:
	return TRUE;
}

BOOL CubeMapLoad(CUBEMAP *pCubeMap, char szFileNames[6][_MAX_PATH])
{
	int rcode = NO_ERROR;

	for (int index = 0; index < 6; index++) {
		rcode = IMAGE_LoadImage(szFileNames[index], &pCubeMap->faces[index]);
		if (rcode != NO_ERROR) goto ERR;
	}

	goto RET;
ERR:
	CubeMapFree(pCubeMap);
	return FALSE;
RET:
	return TRUE;
}

unsigned long CubeMapGetPixelColor(CUBEMAP *pCubeMap, glm::vec3 &texcoord)
{
	/*
	https://en.wikipedia.org/wiki/Cube_mapping

	major axis
	direction     target                             sc     tc    ma
	----------    -------------------------------    ---    ---   ---
	+rx           TEXTURE_CUBE_MAP_POSITIVE_X_EXT    -rz    -ry   rx
	-rx           TEXTURE_CUBE_MAP_NEGATIVE_X_EXT    +rz    -ry   rx
	+ry           TEXTURE_CUBE_MAP_POSITIVE_Y_EXT    +rx    +rz   ry
	-ry           TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT    +rx    -rz   ry
	+rz           TEXTURE_CUBE_MAP_POSITIVE_Z_EXT    +rx    -ry   rz
	-rz           TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT    -rx    -ry   rz
	*/

	const float rx = texcoord.x;
	const float ry = texcoord.y;
	const float rz = texcoord.z;
	const float arx = fabs(rx);
	const float ary = fabs(ry);
	const float arz = fabs(rz);

	int face;
	int x, y;
	float sc, tc, ma;

	if (arx >= ary && arx >= arz) {
		if (rx >= 0.0f) {
			face = TEXTURE_CUBE_MAP_POSITIVE_X;
			sc = -rz;
			tc = -ry;
			ma = arx;
		}
		else {
			face = TEXTURE_CUBE_MAP_NEGATIVE_X;
			sc = rz;
			tc = -ry;
			ma = arx;
		}
	}
	else if (ary >= arx && ary >= arz) {
		if (ry >= 0.0f) {
			face = TEXTURE_CUBE_MAP_POSITIVE_Y;
			sc = rx;
			tc = rz;
			ma = ary;
		}
		else {
			face = TEXTURE_CUBE_MAP_NEGATIVE_Y;
			sc = rx;
			tc = -rz;
			ma = ary;
		}
	}
	else {
		if (rz >= 0.0f) {
			face = TEXTURE_CUBE_MAP_POSITIVE_Z;
			sc = rx;
			tc = -ry;
			ma = arz;
		}
		else {
			face = TEXTURE_CUBE_MAP_NEGATIVE_Z;
			sc = -rx;
			tc = -ry;
			ma = arz;
		}
	}

	x = (int)(((sc / ma + 1.0f) * 0.5f) * CUBEMAP_WIDTH(pCubeMap));
	y = (int)(((tc / ma + 1.0f) * 0.5f) * CUBEMAP_HEIGHT(pCubeMap));

	x = max(x, 0);
	x = min(x, CUBEMAP_WIDTH(pCubeMap) - 1);

	y = max(y, 0);
	y = min(y, CUBEMAP_HEIGHT(pCubeMap) - 1);

	return IMAGE_GetPixelColor(&pCubeMap->faces[face], x, y);
}

void PreviewMap(CUBEMAP *pCubeMap, IMAGE *imgPreview)
{
	//     +Y
	// -X  +Z  +X  -Z
	//     -Y

	int rcode = NO_ERROR;

	rcode = IMAGE_AllocImage(imgPreview, CUBEMAP_WIDTH(pCubeMap) * 4, CUBEMAP_HEIGHT(pCubeMap) * 3, CUBEMAP_BITCOUNT(pCubeMap));
	if (rcode != NO_ERROR) goto RET;

	IMAGE_SetImageArea(imgPreview, 
		CUBEMAP_WIDTH(pCubeMap), 0,
		CUBEMAP_WIDTH(pCubeMap) * 2 - 1, CUBEMAP_HEIGHT(pCubeMap) - 1);
	rcode = IMAGE_CopyImageArea(&pCubeMap->faces[TEXTURE_CUBE_MAP_POSITIVE_Y], imgPreview);

	IMAGE_SetImageArea(imgPreview, 
		0, CUBEMAP_HEIGHT(pCubeMap),
		CUBEMAP_WIDTH(pCubeMap) - 1, CUBEMAP_HEIGHT(pCubeMap) * 2 - 1);
	rcode = IMAGE_CopyImageArea(&pCubeMap->faces[TEXTURE_CUBE_MAP_NEGATIVE_X], imgPreview);
	if (rcode != NO_ERROR) goto RET;

	IMAGE_SetImageArea(imgPreview,
		CUBEMAP_WIDTH(pCubeMap), CUBEMAP_HEIGHT(pCubeMap),
		CUBEMAP_WIDTH(pCubeMap) * 2 - 1, CUBEMAP_HEIGHT(pCubeMap) * 2 - 1);
	rcode = IMAGE_CopyImageArea(&pCubeMap->faces[TEXTURE_CUBE_MAP_POSITIVE_Z], imgPreview);
	if (rcode != NO_ERROR) goto RET;

	IMAGE_SetImageArea(imgPreview,
		CUBEMAP_WIDTH(pCubeMap) * 2, CUBEMAP_HEIGHT(pCubeMap),
		CUBEMAP_WIDTH(pCubeMap) * 3 - 1, CUBEMAP_HEIGHT(pCubeMap) * 2 - 1);
	rcode = IMAGE_CopyImageArea(&pCubeMap->faces[TEXTURE_CUBE_MAP_POSITIVE_X], imgPreview);
	if (rcode != NO_ERROR) goto RET;

	IMAGE_SetImageArea(imgPreview,
		CUBEMAP_WIDTH(pCubeMap) * 3, CUBEMAP_HEIGHT(pCubeMap),
		CUBEMAP_WIDTH(pCubeMap) * 4 - 1, CUBEMAP_HEIGHT(pCubeMap) * 2 - 1);
	rcode = IMAGE_CopyImageArea(&pCubeMap->faces[TEXTURE_CUBE_MAP_NEGATIVE_Z], imgPreview);
	if (rcode != NO_ERROR) goto RET;

	IMAGE_SetImageArea(imgPreview,
		CUBEMAP_WIDTH(pCubeMap), CUBEMAP_HEIGHT(pCubeMap) * 2,
		CUBEMAP_WIDTH(pCubeMap) * 2 - 1, CUBEMAP_HEIGHT(pCubeMap) * 3 - 1);
	rcode = IMAGE_CopyImageArea(&pCubeMap->faces[TEXTURE_CUBE_MAP_NEGATIVE_Y], imgPreview);
	if (rcode != NO_ERROR) goto RET;

RET:
	IMAGE_SetImageArea(imgPreview, 0, 0, IMAGE_WIDTH(imgPreview) - 1, IMAGE_HEIGHT(imgPreview) - 1);
	return;
}
