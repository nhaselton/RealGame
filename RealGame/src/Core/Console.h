#pragma once
#include "coredef.h"
#include <string>
#include <map>
#include "Input.h"

#define MAX_CONSOLE_COMMAND_HISTORY 32
#define MAX_CONSOLE_COMMAND_LENGTH 512
#define MAX_CVAR_NAME_LENGTH 32
#define MAX_CVAR_STRING_LENGTH 32

#define MAX_CVAR_NAME_LENGTH 64
#define MAX_CVAR_STRING_LENGTH 64
#define MAX_CVAR_ARGC 10

typedef void ( *funcCmd ) ( void );
enum CVarType {
	CV_NONE = 0,
	CV_FUNC,
	CV_INT,
	CV_FLOAT,
	CV_STRING,
	CV_VEC3
};

struct Cvar {
	char* name;
	void* value; //current value in correct form
	//all cvars are in 1 big list
	struct Cvar* next;
	funcCmd func;
	CVarType type;
};

enum ConsoleState {
	CONSOLE_CLOSED,
	CONSOLE_OPEN_SMALL,
	CONSOLE_OPEN_BIG
};

enum commandType {
	CT_EDIT_VAR,
	CT_CALL_FUNC,
};
enum commandSubType {
	CST_NONE,
	CST_INT,
	CST_FLOAT,
	CST_STRING,
	CST_VEC3
};

struct ConsoleCommand {
	char data[MAX_CONSOLE_COMMAND_LENGTH + 1];
	int length;
};

struct CommandInfo {
	char name[64];
	commandType type;
	commandSubType subType;
	void* data0;
	void* data1;
};


void RegisterCvar( const char* name, void* value, CVarType type );
Cvar* FindCvar( const char* name );


class DLL Console {
public:
	bool active;

	Console();
	void Init();
	void SetState( ConsoleState state );
	void Update();
	bool IsOpen() { return openT > 0.0f; }
	void Toggle();
	void UpdateOpenness();
	KeySub sub;

	//===================
	//	Console Functionality
	//===================
	void KeyInput(); //takes keys from sub and types them into console
	void TypeKey( char c );
	void Backspace();
	void DeleteKey();
	void SendCommand();
	void ParseCommand();
	void MoveCursor( int direction, bool controlHeld );
	//==================
	//	Console Commands
	//===================
	ConsoleCommand* GetCurrentCommand();
	ConsoleCommand* GetCurrentHistory();

	ConsoleCommand* GetCommandRelative( int r );
	ConsoleCommand* GetHistoryRelative( int r );

	//Writes a line. does not touch currentCommand
	void WriteLine( const char* format );
	void WriteString( const char* format, ... );
	//Memcpys data and copies length
	void CopyCommand( ConsoleCommand* dst, ConsoleCommand* src );

	Parser parser;
	ConsoleCommand commandHistory[MAX_CONSOLE_COMMAND_HISTORY]; //There are just the past commands you inputted.
	ConsoleCommand consoleHistory[MAX_CONSOLE_COMMAND_HISTORY]; //This is everything the console has written in it
	ConsoleCommand tempCommand; //For pressing up and down

	int currentCommandIndex;
	int currentHistoryIndex; // Console will start with this at the bottom and draw upwards
	int currentViewingCommandIndex; //This allows you to look at other commands without doing anythign to your actual command

	int numCommandsInHistory;
	int numLinesInHistory;

	int cursorLoc;

	Token cvarArgv[MAX_CVAR_ARGC];
	u32		cvarArgc;


	//===============
	//Console Draw
	//===============
	ConsoleState currentState;
	Vec3 color;
	//what percent of screen space should it cover
	float openMax;
	//percent opened
	float openT;
	//Open speed
	float openSpeed;
	float openTTarget;

	float fontSize;
	int lineOffsetStartX;

	int scrollAmount;

	//how long in seconds should it be on screen or off
	float blipLength;
	float blipTime;
};
extern Console console;