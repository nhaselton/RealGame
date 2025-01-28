#pragma once
#include "def.h"

enum shaderArg_t {
	SHADER_ARG_FLOAT,
	SHADER_ARG_VEC2,
	SHADER_ARG_VEC3,
	SHADER_ARG_MAT3,
	SHADER_ARG_MAT4,
	SHADER_ARG_QUAT,
	SHADER_ARG_INT,

	SHADER_ARG_MAT4_ARRAY,
};

struct ShaderArg {
	char name[MAX_NAME_LENGTH]; //todo Hash?
	char value[MAX_SHADER_ARG_SIZE];
	
	shaderArg_t type;
	i32 uniformLoc;
	u32 shaderID;
	i32 arraySize;//-1 = no array

	//LL to all args in shader
	ShaderArg* next;
};

class Shader {
public:
	u32 id;
	ShaderArg* args;
	bool updateMVP; //Should the MVP be updated each frame
};

//Sets and uploads them to GPU
//Note: when setting Arrays, 
void ShaderSetMat4(  class Renderer* renderer, Shader* shader, const char* name, Mat4 mat );
void ShaderSetMat3(  class Renderer* renderer, Shader* shader, const char* name, Mat3 mat );
void ShaderSetVec4(  class Renderer* renderer, Shader* shader, const char* name, Vec4 vec );
void ShaderSetVec3(  class Renderer* renderer, Shader* shader, const char* name, Vec3 vec );
void ShaderSetVec2(  class Renderer* renderer, Shader* shader, const char* name, Vec2 vec );
void ShaderSetFloat( class Renderer* renderer, Shader* shader, const char* name, float value );
void ShaderSetInt(   class Renderer* renderer, Shader* shader, const char* name, i32 value );

void ShaderSetMat4Array( class Renderer* renderer, Shader* shader, const char* name, Mat4* array, i32 count );
