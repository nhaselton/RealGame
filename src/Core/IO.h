#pragma once
#include "CoreDef.h"

class DLL NFile {
public:
//private:
	const char* mode;
	const char* path;
	bool open;
	u32 length;
	FILE* file;
};

//Tries to Create a file and tells if it was a success
bool CreateNFile( NFile* file, const char* path, const char* readMode );
void NFileRead( NFile* file, char* dst, u32 length );
bool NFileValid( NFile* file );
void NFileClose(NFile* file);
u32 NFileReadU32(NFile* file);
void NFileRead( NFile* file, void* dst, u32 size );
u32 NFileGetPos( NFile* file );
void NFileSetSeek( NFile* file, u32 offset );
void NFileWriteU32( NFile* file, u32 u32 );