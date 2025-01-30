#include "ShaderManager.h"
#include "Shader.h"
#include "Renderer\Renderer.h"
#include <glad\glad.h>

//From Shader.cpp, this is the only file that should call it
bool CreateShader( Shader* shader, const char* vertexPath, const char* fragPath );
bool CreateComputeShader( Shader* shader, const char* path );

void CreateShaderManager( ShaderManager* manager, 
	u32 numShaders, void* shaderMemory,
	u32 numShaderArgs, void* shaderArgMemory ) {
	CreatePoolArena( &manager->shaderAlloc, sizeof( ShaderInfo ), numShaders, shaderMemory, &globalArena, "Shader Pool");
	CreatePoolArena( &manager->argAlloc, sizeof( ShaderArg ), numShaderArgs, shaderArgMemory, &globalArena, "ShaderArg Pool" );
}

Shader* ShaderManagerCreateShader( ShaderManager* manager, const char* vertexPath, const char* fragPath ) {
	ShaderInfo* info = (ShaderInfo*) PoolArenaAllocate( &manager->shaderAlloc );
	
	//Make sure theres room
	if ( !info ) {
		LOG_ASSERT( LGS_RENDERER, "Can not allocate new shader %s\n", vertexPath );
		return 0;
	}

	//Make sure it actually Creates a shader
	if ( !CreateShader( &info->shader, vertexPath, fragPath ) ) {
		PoolArenaFree( &manager->shaderAlloc, info );
		return 0;
	}

	//Add shader to list
	info->next = manager->head;
	manager->head = info;
	return &info->shader;
}

Shader* ShaderManagerCreateComputeShader( ShaderManager* manager, const char* path ) {
	ShaderInfo* info = ( ShaderInfo* ) PoolArenaAllocate( &manager->shaderAlloc );

	//Make sure theres room
	if ( !info ) {
		LOG_ASSERT( LGS_RENDERER, "Can not allocate new shader %s\n", path );
		return 0;
	}

	//Make sure it actually Creates a shader
	if ( !CreateComputeShader( &info->shader, path ) ) {
		PoolArenaFree( &manager->shaderAlloc, info );
		return 0;
	}

	//Add shader to list
	info->next = manager->head;
	manager->head = info;
	return &info->shader;
}

void ShaderAddArg( ShaderManager* manager, Shader* shader, shaderArg_t type, const char* name  ) {
	ShaderArg* arg = ( ShaderArg* ) PoolArenaAllocate( &manager->argAlloc );

	if ( !arg ) {
		LOG_ASSERT( LGS_RENDERER, "Can not allocate shader arg" );
		return;
	}
	
	strcpy_s( arg->name, MAX_NAME_LENGTH, name );
	
	arg->shaderID = shader->id;
	arg->uniformLoc = glGetUniformLocation( arg->shaderID, arg->name );
	arg->type = type;

	//Make sure the uniform actually exists
	if ( arg->uniformLoc == -1 ) {
		LOG_WARNING( LGS_RENDERER, "Could not find shaderArg %s\n", name );
		PoolArenaFree( &manager->argAlloc, arg );
		return;
	}

	//Add to list
	arg->next = shader->args;
	shader->args = arg;
}

void ReloadShaders() {
	for ( ShaderInfo* info = shaderManager.head; info != 0; info = info->next) {
		info->shader.args = 0;
		glDeleteProgram( info->shader.id );
	}

	PoolArenaFreeAll( &shaderManager.argAlloc );
	PoolArenaFreeAll( &shaderManager.shaderAlloc );
	shaderManager.head = 0;
	RenderCreateShaders(&renderer);
}
