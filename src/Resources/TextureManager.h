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

class TextureManager {
public:
	PoolArena texurePool;
	Texture* head;
};
extern TextureManager textureManager;

void CreateTextureManager( void* data, u32 size );
Texture* TextureManagerDoesTextureExist( const char* path );
Texture* TextureManagerLoadTextureFromFile( const char* path );
//Make sure string is null terminated
Texture* TextureManagerLoadTextureFromMemory( u8* memory, u32 size, const char* name );
Texture* TextureManagerGetTexture( const char* path, bool loadIfNeeded );