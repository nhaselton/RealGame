#pragma once

enum logSeverity_t {
	LOG_SEVERITY_INFO,
	LOG_SEVERITY_WARNING,
	LOG_SEVERITY_ERROR,
	LOG_SEVERITY_ASSERT,//will call an assert
	LOG_SEVERITY_FORCE_WRITE, //Will always write to console
};

enum subSystem_t {
	LGS_NONE = 0,
	LGS_CORE = 0b1,
	LGS_MEMORY = 0b10,
	LGS_PROFILE = 0b100,
	LGS_IO = 0b1000,
	LGS_RENDERER = 0b10000,
	LGS_GAME = 0b100000,
	LGS_PHYSICS = 0b1000000,
	LGS_SOUND = 0b10000000,
};

//Which subsystems do we care about, starts as all 
void LogSetSeverity( logSeverity_t severity );
void LogAddSubsystem(subSystem_t system);
void LogRemoveSubsystem( subSystem_t system );


//Subsystem, format, args
void Log( subSystem_t sys, int severity, const char* format, ... );
#define LOG_PRINTF(s,f,...) Log(s,LOG_SEVERITY_FORCE_WRITE,f,##__VA_ARGS__)
#define LOG_INFO(s,f,...) Log(s,LOG_SEVERITY_INFO,f,##__VA_ARGS__)
#define LOG_WARNING(s,f,...) Log(s,LOG_SEVERITY_WARNING,f,##__VA_ARGS__)
#define LOG_ERROR(s,f,...) Log(s,LOG_SEVERITY_ERROR,f,##__VA_ARGS__)
#define LOG_ASSERT(s,f,...) Log(s,LOG_SEVERITY_ASSERT,f,##__VA_ARGS__)
