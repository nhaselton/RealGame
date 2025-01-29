#pragma once
#include "def.h"

struct Texture {
	char path[MAX_PATH_LENGTH];
	u32 id;
	i32 width; //STBI uses i32
	i32 height;
	i32 channels;
	u32 refrences; //How many materials(/models?) point to this texture
	Texture* next;
};

struct TextureInfo {
	i32 textureType; //Texture_2D / Texture_cube_map
	i32 target; //Texture2D / Texture_CubeMap_Postive_X + 1 etc.
	i32 wrapS;
	i32 wrapT;
	i32 wrapR;
	i32 minFilter;
	i32 magFilter;

};

class TextureManager {
public:
	PoolArena texurePool;
	Texture* head;
};
extern TextureManager textureManager;

void CreateTextureManager( void* data, u32 size );
Texture* TextureManagerDoesTextureExist( const char* path );
Texture* TextureManagerLoadTextureFromFile( const char* path, TextureInfo* info = 0 );
//Make sure string is null terminated
Texture* TextureManagerLoadTextureFromMemory( u8* memory, u32 size, const char* name, TextureInfo* texture = 0 );
Texture* TextureManagerGetTexture( const char* path, bool loadIfNeeded );