#include "IO.h"
#include "def.h"
#include <sys/stat.h>
#include <time.h>

bool CreateNFile( NFile* file, const char* path, const char* readMode ) {
	file->path = path;
	file->mode = readMode;
	fopen_s( &file->file, file->path, file->mode );
	
	if ( !NFileValid( file ) ) {
		LOG_ERROR( LGS_IO, "Could not open file %s\n", path );
		memset( file, 0, sizeof( *file ) );
		return false;
	}

	file->open = true;
	//Get Length
	fseek( file->file, 0, SEEK_END );
	file->length = ftell( file->file );
	fseek( file->file, 0, SEEK_SET );

	return true;
}

void NFileRead( NFile* file, char* dst, u32 length ) {
	if ( !file->open )
		LOG_ASSERT( LGS_IO, "Can't read unopend file %s\n", file->path );
	fread( dst, length, 1, file->file );
}

bool NFileValid( NFile* file ) {
	return ( file->file != NULL );
}

void NFileClose( NFile* file ) {
	if ( !file->open ) {
		LOG_WARNING( LGS_IO, "trying to close a closed file %s\n", file->path );
		return;
	}

	fclose( file->file );
	file->open = false;
}

u32 NFileReadU32( NFile* file ) {
	if ( !file->open ) {
		LOG_WARNING( LGS_IO, "trying to read from a closed file %s\n", file->path );
		return 0;
	}

	u32 temp;
	fread( &temp, sizeof( u32 ), 1, file->file );
	return temp;
}

void NFileRead( NFile* file, void* dst, u32 size ) {
	fread( dst, size, 1, file->file );
}

u32 NFileGetPos( NFile* file ) {
	return ftell( file->file );
}

void NFileSetSeek( NFile* file, u32 offset ) {
	fseek( file->file, offset, SEEK_SET );
}

void NFileWriteU32( NFile* file, u32 u32 ) {
	fwrite( &u32, sizeof( u32 ), 1, file->file );
}