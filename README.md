# CubeMapTool
CubeMapTool for PBR

CubeMapTool.exe -irr PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp 128 128

CubeMapTool.exe -mip PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp

CubeMapTool.exe -mip Env.bmp <-hdr> <-bm>

CubeMapTool.exe -mip -sphere Env.bmp <-hdr> <-bm>

CubeMapTool.exe -lut LUT.bmp 256 256
