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

	SHADER_ARG_INT_ARRAY,
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

enum shaderFlags_t {
	SHADER_NONE = 0,
	SHADER_VERTEX = 0b1,
	SHADER_FRAGMENT = 0b10,
	SHADER_COMPUTE = 0b100,
};

class Shader {
public:
	const char* paths[3];
	u32 id;
	ShaderArg* args;
	u8 flags;
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
void ShaderSetInt( Renderer* renderer, Shader* shader, const char* name, int value );

void ShaderSetMat4Array( class Renderer* renderer, Shader* shader, const char* name, Mat4* array, i32 count );
void ShaderSetIntArray(class Renderer* renderer, Shader* shader, const char* name, Mat4* array, i32 count);