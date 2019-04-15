#include "pch.h"
#include "ShaderReader.h"


ShaderReader::ShaderReader()
{
}


ShaderReader::~ShaderReader()
{
}

Shader ShaderReader::LoadCompiledShader(std::string shader)
{
	Shader _shader;

	std::string fullPath = _getModulePath() + "Shaders\\" + shader;

	std::ifstream shaderFileStream(fullPath, std::ios::in | std::ios::binary | std::ios::ate);

	if (shaderFileStream.is_open())
	{
		_shader.bufferSize = (size_t)shaderFileStream.tellg();
		_shader.compiledShaderCode = (uint8_t*)malloc(_shader.bufferSize);
		shaderFileStream.seekg(0, std::ios::beg);
		shaderFileStream.read((char*)_shader.compiledShaderCode, _shader.bufferSize);
		shaderFileStream.close();
		_shader.name = shader.c_str();
	}

	return _shader;
}

std::string ShaderReader::_getModulePath()
{
	HMODULE hModule = GetModuleHandleA(NULL);
	CHAR path[256];
	size_t size = (size_t)GetModuleFileNameA(hModule, path, 256);

	std::string s_path(path);

	for (size_t i = size - 1; true; i--)
	{
		if (s_path[i] == '\\')
			return s_path;
		else
			s_path.pop_back();
	}
}
