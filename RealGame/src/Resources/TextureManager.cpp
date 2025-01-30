#include "TextureManager.h"
#include "Renderer\Renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad\glad.h>

void CreateTextureManager( void* data, u32 size ) {
	CreatePoolArena( &textureManager.texurePool, sizeof( Texture ), MAX_TEXTURES, data, &globalArena, "Texture Manager" );
	textureManager.head = 0;
}

Texture* TextureManagerDoesTextureExist( const char* path ) {
	Texture* head = textureManager.head;
	while ( head ) {
		if ( strcmp( path, head->path ) == 0 )
			return head;
		head = head->next;
	}
	return 0;
}

Texture* TextureManagerLoadTextureFromMemory( u8* memory, u32 size, const char* name, TextureInfo* info ) {
	Texture* texture = (Texture*) PoolArenaAllocate( &textureManager.texurePool );
	memset( texture, 0, sizeof( Texture ) );
	//LL it
	texture->next = textureManager.head;
	textureManager.head = texture;

	//Load Texture
	u8* data = stbi_load_from_memory( memory, size, &texture->width, &texture->height, &texture->channels, 0 );
	strcpy_s( texture->path, MAX_PATH_LENGTH, name );

	//Now Upload it to opengl
	if ( !data ) {
		LOG_ASSERT( LGS_IO, "Could not load texture from memory" );
		return 0;
	}
	if ( !info ) {
		//OpenGLify it
		glGenTextures( 1, &texture->id );
		glBindTexture( GL_TEXTURE_2D, texture->id );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		switch ( texture->channels ) {
		case 3:
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
			glGenerateMipmap( GL_TEXTURE_2D );
			break;
		case 4:
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			glGenerateMipmap( GL_TEXTURE_2D );
			break;
		}
	}
	else {
		//OpenGLify it
		glGenTextures( 1, &texture->id );
		glBindTexture( info->textureType, texture->id );

		glTexParameteri( info->textureType, GL_TEXTURE_WRAP_S, info->wrapS );
		glTexParameteri( info->textureType, GL_TEXTURE_WRAP_T, info->wrapT );
		glTexParameteri( info->textureType, GL_TEXTURE_WRAP_R, info->wrapR );
		glTexParameteri( info->textureType, GL_TEXTURE_MIN_FILTER, info->minFilter );
		glTexParameteri( info->textureType, GL_TEXTURE_MAG_FILTER, info->magFilter );

		switch ( texture->channels ) {
		case 3:
			glTexImage2D( info->target, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
			glGenerateMipmap( GL_TEXTURE_2D );
			break;
		case 4:
			glTexImage2D( info->target, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			glGenerateMipmap( GL_TEXTURE_2D );
			break;
		}
	}


	stbi_image_free( data );
	return texture;
}

Texture* TextureManagerLoadTextureFromFile( const char* path, TextureInfo* info ) {
	TEMP_ARENA_SET
	NFile file;
	CreateNFile( &file, path, "rb" );

	u8* data = ( u8* ) StackArenaAllocate( &tempArena, file.length );
	NFileRead( &file, data, file.length );
	Texture* texture = TextureManagerLoadTextureFromMemory( data, file.length, path, info );

	return texture;
}

Texture* TextureManagerGetTexture( const char* path, bool loadIfNeeded ) {
	Texture* doesExist = TextureManagerDoesTextureExist( path );
	
	if ( !doesExist ) {
		if ( loadIfNeeded )
			doesExist = TextureManagerLoadTextureFromFile( path );
		else
			LOG_ERROR( LGS_IO, "Could not get and load texture %s\n", path );
	}
	if ( !doesExist )
		doesExist = renderer.blankTexture;

	return doesExist;
}
