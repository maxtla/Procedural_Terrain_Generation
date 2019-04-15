#pragma once

struct Shader
{
	uint8_t * compiledShaderCode = nullptr;
	size_t bufferSize = 0;
	const char * name = "";
};
//This class reads in the compiled shader objects and creates ID3D10Blob objects from the data which can be used to create Pipeline State Objects
class ShaderReader
{
public:
	ShaderReader();
	~ShaderReader();

	Shader LoadCompiledShader(std::string shader);
private:
	std::string _getModulePath();
};
