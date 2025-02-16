#include "def.h"
#include "Console.h"
#include "cstdarg"

Cvar* headCvar;
int numCvars;

void nprintf( const char* format, ... ) {
	static char buffer[MAX_CONSOLE_COMMAND_LENGTH];
	va_list argList;
	va_start( argList, format );
	int charsWritten = vsnprintf( buffer, MAX_CONSOLE_COMMAND_LENGTH, format, argList );
	printf( "%s", buffer );
	console.WriteString( "%s", buffer );
	va_end( argList );
}

Cvar* FindCvar( const char* name ) {
	for ( Cvar* c = headCvar; c != nullptr; c = c->next ) {
		if ( strcmp( name, c->name ) == 0 )
			return c;
	}

	return nullptr;
}

void RegisterCvar( const char* name, void* value, CVarType type ) {
	if ( !name ) return;

	Cvar* newCvar;
	newCvar = ( Cvar* ) ScratchArenaAllocate( &globalArena, sizeof( Cvar ) );
	memset( newCvar, 0, sizeof( Cvar ) );

	newCvar->type = type;
	newCvar->name = ( char* ) ScratchArenaAllocate( &globalArena, MAX_CVAR_NAME_LENGTH );
	memset( newCvar->name, 0, MAX_CVAR_NAME_LENGTH );
	strcpy( newCvar->name, name );

	switch ( type ) {
		//Leaks
	case CV_NONE:
		printf( "unkown type none. cant create cvar" );
		return;
		break;
	case CV_FUNC:
		newCvar->func = ( funcCmd ) value;
		break;
	default:
		newCvar->value = value;
		break;
	}

	if ( !headCvar ) {
		headCvar = newCvar;
		return;
	}

	Cvar* c;
	for ( c = headCvar; c->next != nullptr; c = c->next );
	c->next = newCvar;
}


bool IsConsoleTypeableKey( int k ) {
	if ( ( k >= 'A' && k <= 'Z' ) || ( k >= '0' && k <= '9' ) || k == ' ' )
		return true;

	switch ( k ) {
	case '.':
	case ',':
	case '/':
	case ';':
	case '[':
	case ']':
	case '\'':
		return true;
	default:
		return false;
	}

	return false;
}

char CaptialSymbol( char c ) {
	if ( c >= 'A' && c <= 'Z' ) return c;
	switch ( c ) {
	case '1':
		return '!';
		break;
	case '2':
		return '@';
		break;
	case '3':
		return '#';
		break;
	case '4':
		return '$';
		break;
	case '5':
		return '%';
		break;
	case '6':
		return '^';
		break;
	case '7':
		return '&';
		break;
	case '8':
		return '*';
		break;
	case '9':
		return '(';
		break;
	case '0':
		return ')';
		break;

	case '.':
		return '>';
		break;
	case ',':
		return '<';
		break;
	case '/':
		return '?';
		break;
	case ';':
		return ':';
		break;
	case '[':
		return '{';
		break;
	case ']':
		return '}';
		break;
	case '\'':
		return '|';
		break;
	default:
		printf( "key %d did not have a capital", c );
		return ' ';
	}
}


Console::Console() {
	color = Vec3( 0, .5, 0 );
	openMax = .4;
	openT = 0;
	openSpeed = 7.0f;
	currentState = CONSOLE_CLOSED;

	currentCommandIndex = 0;
	currentHistoryIndex = 0; // Console will start with this at the bottom and draw upwards

	numCommandsInHistory = 0;
	numLinesInHistory = 0;

	lineOffsetStartX = 10.0f;
	fontSize = 16.0f;

	console.blipLength = 0.6f;
}

void CExit() {
	exit( 0 );
}

void Console::Init() {
	active = true;
	cvarArgc = 0;
	RegisterCvar( "exit", CExit, CV_FUNC );
	RegisterCvar( "quit", CExit, CV_FUNC );
}

//to get large console must manually call it
void Console::Toggle() {
	if ( openTTarget > 0 )
		console.SetState( CONSOLE_CLOSED );
	else
		console.SetState( CONSOLE_OPEN_SMALL );
}

void Console::SetState( ConsoleState state ) {
	if ( state == CONSOLE_CLOSED ) {
		openTTarget = 0;
	}
	else if ( state == CONSOLE_OPEN_SMALL ) {
		openTTarget = 1;
	}
	else if ( state == CONSOLE_OPEN_BIG )
		openTTarget = 2;
	else {
		printf( "unkownc onsole state %d\n", ( int ) state );
	}
	currentState = state;
	sub.numKeys = 0;
}

void Console::UpdateOpenness() {
	if ( openT < openTTarget ) {
		openT += dt * openSpeed;
		if ( openT > openTTarget )
			openT = openTTarget;
	}

	else if ( openT > openTTarget ) {
		openT -= dt * openSpeed;
		if ( openT < openTTarget ) {
			openT = openTTarget;
		}
	}
}
void Console::Update() {
	if ( openTTarget != openT ) {
		UpdateOpenness();
	}

	if ( KeyPressed( KEY_GRAVE_ACCENT ) ) {
		console.Toggle();
		sub.numKeys = 0;
		return;
	}

	if ( currentState == CONSOLE_CLOSED ) {
		sub.numKeys = 0;
		return;
	}

	blipTime += dt;
	if ( blipTime >= 2 * blipLength )
		blipTime = 0;

	KeyInput();
}

void Console::KeyInput() {
	for ( int i = 0; i < sub.numKeys; i++ ) {
		KeyInfo k = sub.keys[i];

		//Typeable
		if ( k.action <= 0 ) {
			continue;
		}

		if ( k.keycode < KEY_FUNCTIONS_START ) {
			//Captialize if shift pressed
			if ( ( k.flags & 0x0001 ) == 1 ) {
				k.keycode = CaptialSymbol( k.keycode );
			}
			//Lowercase if alpha key and shift not pressed
			else if ( isalpha( k.keycode ) )
				k.keycode = tolower( k.keycode );

			TypeKey( k.keycode );
		}
		//Functions
		else {
			if ( k.keycode == KEY_LEFT ) {
				MoveCursor( -1, ( k.flags & 0x0002 ) );//control
				//cursorLoc--;
			}
			else if ( k.keycode == KEY_RIGHT ) {
				MoveCursor( 1, ( k.flags & 0x0002 ) );//control
			}
			else if ( k.keycode == KEY_BACKSPACE ) {
				Backspace();
			}
			else if ( k.keycode == KEY_DELETE ) {
				DeleteKey();
			}
			else if ( k.keycode == KEY_ENTER ) {
				SendCommand();
			}
			else if ( k.keycode == KEY_UP ) {
				currentViewingCommandIndex -= 1;
				if ( currentViewingCommandIndex < 0 )
					currentViewingCommandIndex = numCommandsInHistory;
			}
			else if ( k.keycode == KEY_DOWN ) {
				currentViewingCommandIndex += 1;
				if ( currentViewingCommandIndex > numCommandsInHistory )
					currentViewingCommandIndex = 0;
			}
			else if ( k.keycode == KEY_SCROLLUP ) {
				scrollAmount--;
				if ( scrollAmount < -numLinesInHistory )
					scrollAmount = -numLinesInHistory;
			}
			else if ( k.keycode == KEY_SCROLLDOWN ) {
				scrollAmount++;
				if ( scrollAmount >= numCommandsInHistory )
					scrollAmount = numCommandsInHistory;
			}

		}
	}

	sub.numKeys = 0;
}

void Console::Backspace() {
	if ( cursorLoc == 0 )
		return;
	ConsoleCommand* command = GetCurrentCommand();

	if ( cursorLoc == command->length ) {
		command->data[command->length - 1] = '\0';
		command->length--;
		cursorLoc--;
		return;
	}

	for ( int i = cursorLoc - 1; i < command->length; i++ ) {
		command->data[i] = command->data[i + 1];
	}

	command->data[command->length] = '\0';
	cursorLoc--;
	command->length--;
}

void Console::DeleteKey() {
	ConsoleCommand* command = GetCurrentCommand();
	if ( cursorLoc == command->length )
		return;

	if ( cursorLoc == command->length - 1 ) {
		command->data[cursorLoc] = '\0';
		command->length--;
		return;
	}

	for ( int i = cursorLoc; i < command->length; i++ ) {
		command->data[i] = command->data[i + 1];
	}

	command->data[command->length] = '\0';
	command->length--;
}


void Console::TypeKey( char c ) {
	ConsoleCommand* command = GetCurrentCommand();

	if ( command->length == MAX_CONSOLE_COMMAND_LENGTH )
		return;

	//Check if we're at the end of the line
	if ( cursorLoc == command->length ) {
		command->data[command->length] = c;
	}
	else {
		//Probably a better way to shift array right?
		for ( int i = command->length; i >= cursorLoc; i-- )
			command->data[i + 1] = command->data[i];

		command->data[cursorLoc] = c;
	}

	command->length++;
	cursorLoc++;
}

ConsoleCommand* Console::GetCurrentCommand() {
	return &commandHistory[currentCommandIndex];
}

ConsoleCommand* Console::GetCurrentHistory() {
	return &consoleHistory[currentHistoryIndex];
}

void Console::SendCommand() {
	//Copy who you're looking at to your current command. (overrides it)
	if ( currentViewingCommandIndex != currentCommandIndex ) {
		CopyCommand( GetCurrentCommand(), &commandHistory[currentViewingCommandIndex] );
	}

	ParseCommand();

	//Copy and increment Information
	CopyCommand( GetCurrentHistory(), GetCurrentCommand() );
	currentCommandIndex++;
	currentHistoryIndex++;
	currentCommandIndex %= MAX_CONSOLE_COMMAND_HISTORY;
	currentHistoryIndex %= MAX_CONSOLE_COMMAND_HISTORY;
	currentViewingCommandIndex = currentCommandIndex;
	numCommandsInHistory++;
	numLinesInHistory++;
	cursorLoc = 0;
}

//Console commands work by having a 1 string name, then value is the rest of the "arguments"
void Console::ParseCommand() {
	cvarArgc = 0;
	//parse current command
	ConsoleCommand* command = GetCurrentCommand();
	parser.SetBuffer( command->data, command->length );
	Token t = parser.ReadToken();

	//This is our cvar name
	if ( t.type != TT_STRING ) {
		WriteLine( "Not a valid command." );
		return;
	}

	char name[MAX_CVAR_NAME_LENGTH]{};
	memcpy( name, t.data, t.length );
	Cvar* cvar = FindCvar( name );

	if ( !cvar ) {
		WriteString( "could not find cvar with name %s", name );
		return;
	}

	parser.ReadToken();
	for ( int i = 0; parser.GetCurrent().type != TT_EOF; i++, parser.ReadToken() ) {
		cvarArgc++;
		cvarArgv[i] = parser.GetCurrent();
	}

	//Actually do the command
	if ( cvar->type == CV_FUNC ) {
		cvar->func();
		return;
	}

	if ( cvarArgc == 0 ) {
		WriteLine( "must provide arguments for cvar" );
	}

	//Otherwise time to do data
	switch ( cvar->type ) {
	case CV_FLOAT:
	case CV_INT:
	{
		if ( cvarArgv[0].type != TT_NUMBER ) {
			WriteString( "%s: %d\n", name, *( ( int* ) cvar->value ) );
			//WriteLine( "Must provide number for cvar value" );
			return;
		}

		if ( cvar->type == CV_FLOAT ) {
			float* value = ( float* ) cvar->value;
			*value = cvarArgv[0].ToFloat();
		}
		else if ( cvar->type == CV_INT ) {
			int* value = ( int* ) cvar->value;
			*value = cvarArgv[0].ToFloat();
		}

	}break;
	case CV_VEC3:
	{
		if ( cvarArgc != 3 ) {
			Vec3* v = ( Vec3* ) cvar->value;
			WriteString( "%.2f %.2f %.2f: %d\n", name, v->x, v->y, v->z );
			//WriteLine( "not enough arguments for cvar" );
			return;
		}
		float temp[3];
		Token* t = &cvarArgv[0];

		for ( int i = 0; i < 3; i++, t++ ) {
			if ( t->type != TT_NUMBER ) {
				WriteLine( "invalid argument" );
				return;
			}
			temp[i] = t->ToFloat();
		}

		memcpy( cvar->value, temp, sizeof( Vec3 ) );
	}

	case CV_STRING:
	{
		if ( cvarArgv[0].type != TT_STRING ) {
			WriteLine( "Cvar requires a string" );
			return;
		}
		memcpy( cvar->value, cvarArgv[0].data, cvarArgv[0].length );
		char* v = ( char* ) cvar->value;
		v[cvarArgv[0].length] = '\0';
	}break;



	default:
		nprintf( "unkown cvar type %d\n", cvar->type );
	}

}


void Console::CopyCommand( ConsoleCommand* dst, ConsoleCommand* src ) {
	memcpy( dst->data, src->data, MAX_CONSOLE_COMMAND_LENGTH );
	dst->length = src->length;
}

ConsoleCommand* Console::GetCommandRelative( int r ) {
	int cmd = ( currentCommandIndex + r + 512 ) % MAX_CONSOLE_COMMAND_HISTORY;
	return &commandHistory[cmd];
}

ConsoleCommand* Console::GetHistoryRelative( int r ) {
	int cmd = ( currentHistoryIndex + r + 512 ) % MAX_CONSOLE_COMMAND_HISTORY;
	return &consoleHistory[cmd];
}

void Console::MoveCursor( int direction, bool controlHeld ) {
	ConsoleCommand* cmd = GetCurrentCommand();

	int newSpot = cursorLoc;

	//Keep going until you encounter a space or the EOL
	if ( controlHeld ) {
		//Inbounds
		while ( 1 ) {

			newSpot += direction;

			if ( cmd->data[newSpot] == ' ' ) {
				break;
			}

			if ( newSpot >= cmd->length || newSpot <= 0 ) {
				cursorLoc = newSpot;
				return;
			}
		}

	}
	else
		newSpot += direction;

	newSpot = glm::clamp( newSpot, 0, cmd->length );
	cursorLoc = newSpot;

}


void Console::WriteString( const char* format, ... ) {
	char msg[MAX_CONSOLE_COMMAND_LENGTH]{};
	va_list    args;

	va_start( args, format );
	int length = vsnprintf( msg, MAX_CONSOLE_COMMAND_LENGTH, format, args );
	va_end( args );

	//Seperate it by newlines
	char temp[MAX_CONSOLE_COMMAND_LENGTH];
	int strIndx = 0;
	for ( int i = 0; i < length; i++ ) {
		//If not \n add
		char c = temp[i];

		if ( c == '\n' ) {
			temp[strIndx++] = '\0';
			WriteLine( temp );
			memset( temp, 0, strIndx );
			strIndx = 0;
		}
		else {
			temp[strIndx++] = msg[i];
		}
	}

	temp[strIndx++] = '\0';
	if ( strIndx > 1 )
		WriteLine( temp );

}

void Console::WriteLine( const char* line ) {
	ConsoleCommand* command = GetCurrentHistory();
	memset( command->data, 0, MAX_CONSOLE_COMMAND_LENGTH );
	memcpy( command->data, line, strlen( line ) );

	currentHistoryIndex++;
	numLinesInHistory++;
	currentCommandIndex %= MAX_CONSOLE_COMMAND_HISTORY;
	currentHistoryIndex %= MAX_CONSOLE_COMMAND_HISTORY;

}

