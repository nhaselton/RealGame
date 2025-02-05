#include "Shader.h"
#include <glad\glad.h>
#include "Renderer\Renderer.h" 
void ShaderCheckCompileErrors( GLuint shader, const char* type ) {
	GLint success;
	GLchar infoLog[1024];
	if ( strcmp( type, "PROGRAM" ) ) { //Shader
		glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
		if ( !success ) {
			glGetShaderInfoLog( shader, 1024, NULL, infoLog );
			LOG_ERROR( LGS_RENDERER, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n---------------------------------------------------------------------------------\n",
				type, infoLog );
			
		}
	}
	else {	//Program
		glGetProgramiv( shader, GL_LINK_STATUS, &success );
		if ( !success ) {
			glGetProgramInfoLog( shader, 1024, NULL, infoLog );
			LOG_ERROR( LGS_RENDERER, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n---------------------------------------------------------------------------------\n",
				type, infoLog );
		}
	}
}

bool CreateComputeShader( Shader* shader, const char* path ) {
	memset( shader, 0, sizeof( *shader ) );
	TEMP_ARENA_SET;

	NFile file;
	CreateNFile( &file, path, "rb" );

	if ( !NFileValid( &file ) ) {
		LOG_ASSERT( LGS_RENDERER, "Could not load shader %s\n" );
		return false;
	}

	char* buffer = ( char* ) TEMP_ALLOC( file.length + 1 );
	NFileRead( &file, buffer, file.length );
	buffer[file.length] = '\0';

	NFileClose(&file);

	u32 cProg;
	cProg = glCreateShader( GL_COMPUTE_SHADER );
	glShaderSource( cProg, 1, &buffer, NULL );
	glCompileShader( cProg );
	ShaderCheckCompileErrors( cProg, "Compute" );

	shader->id = glCreateProgram();
	glAttachShader( shader->id, cProg );
	glLinkProgram( shader->id );
	ShaderCheckCompileErrors( cProg, "PROGRAM" );

	shader->flags = SHADER_COMPUTE;
	shader->paths[2] = path;

	glDeleteShader( cProg );
	return true;
}

//Return if shader was crated successfully
bool CreateShader( Shader* shader, const char* vertPath, const char* fragPath) {
	memset( shader, 0, sizeof( *shader ) );

	TEMP_ARENA_SET

	NFile vertFile;
	NFile fragFile;

	CreateNFile( &vertFile, vertPath, "rb" );
	CreateNFile( &fragFile, fragPath, "rb" );

	bool validFiles = true;

	if ( !NFileValid( &vertFile ) ) {
		validFiles = false;
		LOG_ERROR( LGS_IO, "Could not open Vertex file %s\n", vertPath );
	}

	if ( !NFileValid( &fragFile ) ) {
		validFiles = false;
		LOG_ERROR( LGS_IO, "Could not open Frag file %s\n", fragPath );
	}

	//Make sure no temp stack allocations are done here
	if ( !validFiles ) {
		return false;
	}

	//Read in files
	char* vertBuffer = (char*) StackArenaAllocate( &tempArena, vertFile.length + 1 );
	NFileRead( &vertFile, vertBuffer, vertFile.length );
	vertBuffer[vertFile.length] = '\0';

	char* fragBuffer = ( char* ) StackArenaAllocate( &tempArena, fragFile.length + 1 );
	NFileRead( &fragFile, fragBuffer, fragFile.length );
	fragBuffer[fragFile.length] = '\0';
	
	NFileClose( &vertFile );
	NFileClose( &fragFile );

	//Create the shaders
	u32 vProg, fProg;
	vProg = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vProg, 1, &vertBuffer, NULL );
	glCompileShader( vProg );
	ShaderCheckCompileErrors( vProg, "VERTEX" );

	fProg = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fProg, 1, &fragBuffer, NULL );
	glCompileShader( fProg );
	ShaderCheckCompileErrors( fProg, "FRAGMENT" );

	//Create the actual program
	shader->id = glCreateProgram();
	glAttachShader( shader->id, vProg );
	glAttachShader( shader->id, fProg );
	glLinkProgram( shader->id );
	ShaderCheckCompileErrors( fProg, "PROGRAM" );

	glDeleteShader( vProg );
	glDeleteShader( fProg );

	shader->flags = (SHADER_VERTEX | SHADER_FRAGMENT);
	shader->paths[0] = vertPath;
	shader->paths[1] = fragPath;

	return true;
}

ShaderArg* ShaderFindArg( Shader* shader, const char* name ) {
	renderer.frameInfos[renderer.currentFrameInfo].shaderArgsSet++;

	ShaderArg* current = shader->args;
	while ( current ) {
		if ( strcmp( name, current->name ) == 0 ) {
			return current;
		}
		current = current->next;
	}
	return 0;
}

#define CHECKARG() if ( !arg ) { \
	LOG_WARNING( LGS_RENDERER, "Uniform %s can not be found\n", name ); \
	return; \
}

#define CHECKSHADER() if ( renderer->currentShaderID != shader->id ) { \
	LOG_WARNING( LGS_RENDERER, "Setting Uniform %s of a non active shader\n", name ); \
	return; \
}

void ShaderSetMat4( Renderer* renderer, Shader* shader, const char* name, Mat4 mat ){
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG()
	CHECKSHADER()

	memcpy( arg->value, &mat, sizeof( Mat4 ) );
	glUniformMatrix4fv( arg->uniformLoc, 1, GL_FALSE, (GLfloat*) arg->value );
}

void ShaderSetMat3( Renderer* renderer, Shader* shader, const char* name, Mat3 mat ){
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &mat, sizeof( Mat3 ) );
	glUniformMatrix3fv( arg->uniformLoc, 1, GL_FALSE, ( GLfloat* ) arg->value );
}

void ShaderSetVec4( Renderer* renderer, Shader* shader, const char* name, Vec4 vec ){
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &vec, sizeof( Vec4 ) );
	glUniform4fv( arg->uniformLoc, 1, (GLfloat*) arg->value);
}

void ShaderSetVec3( Renderer* renderer, Shader* shader, const char* name, Vec3 vec ){
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &vec, sizeof( Vec3 ) );
	glUniform3fv( arg->uniformLoc, 1, ( GLfloat* ) arg->value );
}

void ShaderSetVec2( Renderer* renderer, Shader* shader, const char* name, Vec2 vec ) {
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &vec, sizeof( Vec2 ) );
	glUniform2fv( arg->uniformLoc, 1, ( GLfloat* ) arg->value );
}

void ShaderSetFloat( Renderer* renderer, Shader* shader, const char* name, float value ) {
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &value, sizeof( float ) );
	glUniform1f( arg->uniformLoc, value );
}

void ShaderSetFloat( Renderer* renderer, Shader* shader, const char* name, int value ) {
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &value, sizeof( int ) );
	glUniform1i( arg->uniformLoc, ( GLint) *arg->value );
}

void ShaderSetInt( Renderer* renderer, Shader* shader, const char* name, int value ) {
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &value, sizeof( int ) );
	glUniform1i( arg->uniformLoc, ( GLint ) *arg->value );
}

void ShaderSetMat4Array( class Renderer* renderer, Shader* shader, const char* name, Mat4* value, i32 count ) {
	ShaderArg* arg = ShaderFindArg( shader, name );
	CHECKARG();
	CHECKSHADER();

	memcpy( arg->value, &value, sizeof( int ) );
	Mat4* loc = ( Mat4* ) value;
	glUniformMatrix4fv( arg->uniformLoc, count, GL_FALSE, ( GLfloat* ) loc );
}

void ShaderSetIntArray(class Renderer* renderer, Shader* shader, const char* name, int* value, i32 count) {
	ShaderArg* arg = ShaderFindArg(shader, name);
	CHECKARG();
	CHECKSHADER();

	memcpy(arg->value, &value, sizeof(int));
	Mat4* loc = (Mat4*)value;
	glUniform1iv(arg->uniformLoc, count, (GLint*)loc);
}
