#include <stdio.h>
#include "Log.h"
#include "def.h"
#include "cstdarg"
#include "Log.h"

static u32 activeSubsystems = U32MAX;
static u32 minSeverity = 0;

void LogSetSeverity( logSeverity_t severity ) {
	minSeverity = severity;
}
void LogAddSubsystem( subSystem_t system ) {
	activeSubsystems |= system;
}

void LogRemoveSubsystem( subSystem_t system ) {
	activeSubsystems &= ~( 1 << ( system ) );
}

int VPrint( const char* format, va_list argList ) {
	const u32 MAX_CHARS = 1024;
	static char buffer[MAX_CHARS];

	int charsWritten = vsnprintf( buffer, MAX_CHARS, format, argList );
	printf( "%s", buffer );
	return charsWritten;
}


void Log( subSystem_t sys, int severity, const char* format, ... ) {

	switch ( severity ) {
		case LOG_SEVERITY_WARNING:	printf( "\x1B[33m" ); break;
		case LOG_SEVERITY_ERROR:	printf( "\x1B[31m" ); break;
		case LOG_SEVERITY_ASSERT:	printf( "\x1B[91m" ); break;
	}

	//Make sure its severe enough to write
	if ( severity < minSeverity && severity != LOG_SEVERITY_FORCE_WRITE ) {
		return;
	}

	//Make sure the subsystem is active || its a NONE
	if ( sys != 0 && ( (sys & activeSubsystems) == 0 ) )
		return;

	va_list argList;
	va_start( argList, format );
	int charsWritten = VPrint( format, argList );
	va_end( argList );

	//Reset color
	printf( "\033[0m" );

	if ( severity == LOG_SEVERITY_ASSERT )
		assert( 0 );
}
