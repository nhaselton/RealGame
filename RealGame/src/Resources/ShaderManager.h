#pragma once
#include "def.h"
#include "Shader.h"

struct ShaderInfo {
	class Shader shader;
	ShaderInfo* next;
};

class ShaderManager {
public:
	PoolArena shaderAlloc; //ShaderInfo
	PoolArena argAlloc; //ShaderArg

	//List of shaders
	ShaderInfo* head;
};

extern ShaderManager shaderManager;

void CreateShaderManager( ShaderManager* manager, u32 numShaders, void* shaderMemory, 
	u32 numShaderArgs, void* shaderArgMemory );
class Shader* ShaderManagerCreateShader( ShaderManager* manager, const char* vertexPath, const char* fragPath );
class Shader* ShaderManagerCreateComputeShader( ShaderManager* manager, const char* path );
void ShaderAddArg( ShaderManager* manager, Shader* shader, shaderArg_t type, const char* name );