// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

#include "glew.h"
#include "glut.h"
#include "gli/gli.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

#include "IMAGE.H"



#define PI 3.1415926535897932384626433832795f


#define TEXTURE_CUBE_MAP_POSITIVE_X 0
#define TEXTURE_CUBE_MAP_NEGATIVE_X 1
#define TEXTURE_CUBE_MAP_POSITIVE_Y 2
#define TEXTURE_CUBE_MAP_NEGATIVE_Y 3
#define TEXTURE_CUBE_MAP_POSITIVE_Z 4
#define TEXTURE_CUBE_MAP_NEGATIVE_Z 5

#define CUBEMAP_WIDTH(cubemap) IMAGE_WIDTH(&(cubemap)->faces[0])
#define CUBEMAP_HEIGHT(cubemap) IMAGE_HEIGHT(&(cubemap)->faces[0])
#define CUBEMAP_BITCOUNT(cubemap) IMAGE_BITCOUNT(&(cubemap)->faces[0])

struct CUBEMAP {
	IMAGE faces[6];
};

void CubeMapInit(CUBEMAP *pCubeMap);
void CubeMapFree(CUBEMAP *pCubeMap);
BOOL CubeMapAlloc(CUBEMAP *pCubeMap, int width, int height, int bitcount);
BOOL CubeMapLoad(CUBEMAP *pCubeMap, char szFileNames[6][_MAX_PATH]);
unsigned long CubeMapGetPixelColor(CUBEMAP *pCubeMap, glm::vec3 &texcoord);
void PreviewMap(CUBEMAP *pCubeMap, IMAGE *imgPreview);


extern GLuint rbo;
extern GLuint fbo;
extern GLuint fboTexture;
extern GLuint fboTextureWidth;
extern GLuint fboTextureHeight;
BOOL CreateFBO(int width, int height);
void DestroyFBO(void);


extern GLuint program;
extern GLuint attribLocationPosition;
extern GLuint attribLocationTexcoord;
extern GLuint uniformLocationTexture;
extern GLuint uniformLocationTexcoordMatrix;
extern GLuint uniformLocationModelViewProjectionMatrix;
extern GLuint uniformLocationSHRed;
extern GLuint uniformLocationSHGrn;
extern GLuint uniformLocationSHBlu;
extern GLuint uniformLocationSamples;
extern GLuint uniformLocationRoughness;
extern GLuint uniformLocationEnvmap;
extern GLuint uniformLocationCubemap;
extern GLuint uniformLocationHDRmap;
extern GLuint uniformLocationIsBM;


BOOL CreateProgram(const char *szShaderVertexCode, const char *szShaderFragmentCode);
void DestroyProgram(void);


struct vertex {
	float position[3];
	float texcoord[2];
};

extern GLuint ibo;
extern GLuint vbo;
BOOL CreateVBO(const vertex *vertices, int numVertices, const unsigned short *indices, int numIndices);
void DestroyVBO(void);


GLuint CreateTexture2D(IMAGE *pImage);
GLuint CreateTextureCube(CUBEMAP *pCubeMap);
void DestroyTexture(GLuint texture);


BOOL GenerateEnvIrradianceMap(IMAGE *pEnvMap, CUBEMAP *pIrrMap, int samples);
BOOL GenerateCubeIrradianceMap(CUBEMAP *pCubeMap, CUBEMAP *pIrrMap, int samples);

BOOL GenerateEnvMipmaps(IMAGE *pEnvMap, IMAGE pMipmaps[], int mipLevels, int samples, bool isHDR, bool isBM);
BOOL GenerateSphereEnvMipmaps(IMAGE *pEnvMap, IMAGE pMipmaps[], int mipLevels, int samples, bool isHDR, bool isBM);
BOOL GenerateCubeMipmaps(CUBEMAP *pCubeMap, CUBEMAP pMipmaps[], int mipLevels, int samples);

BOOL GenerateLUT(IMAGE *pImage, int samples);
