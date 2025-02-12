#include "def.h"
#include "Parser.h"
#include <string.h>
#include <ctype.h>

void Token::Print() {
	printf( "%.*s \t:%d\n", length, data, type );
}

int Token::ToInt() {
	if ( type != TT_NUMBER ) {
		printf( "[WARNING] TOken is not type int: " );
		Print();
	}
	if ( length > 10 ) {
		printf( "Token Int is > 10 characters.\n" );
		return 0;
	}

	char i[10] = {'0' };
	memcpy( i, data, length );
	int n = atoi( i );
	return n;
}

int Token::StringEquals( char* other ) {
	char temp[64]{};
	memcpy( temp, data, length );
	return (strcmp( temp, other ) == 0);
}

float Token::ToFloat() {
	if ( type != TT_NUMBER ) {
		printf( "[WARNING] TOken is not type int: " );
		Print();
	}

	if ( length > 32 ) {
		printf( "Token Float is > 32 characters.\n" );
		return 0;
	}

	char f[32] = {};
	memcpy( f, data, length );
	float n = atof( f );
	return n;
}

void Token::ToString( char* buffer, int bufferLength ) {
	memcpy( buffer, data, ( length < bufferLength ) ? length : bufferLength );
}

Parser::Parser() {
	buffer = NULL;
	bufferSize = 0;
	cursor = 0;
	line = 0;
	lineStart = 0;
}

Parser::Parser( char* buffer, u64 size ) {
	bufferSize = size;
	this->buffer = buffer;
	cursor = 0;
	line = 0;
	lineStart = 0;
}

void Parser::RemoveWhitespace() {
	for ( ;; ) {
		if ( cursor >= bufferSize )
			return;

		if ( buffer[cursor] == ' ' || buffer[cursor] == '\r' || buffer[cursor] == '\t' ) {
			//current.flags |= ( u32 ) TF_SPACE;
			BIT_SET( current.flags, TF_SPACE );
			cursor++;
		}
		
		else if ( buffer[cursor] == '\n' ) {
			cursor++;
			line++;
			lineStart = cursor;
			BIT_SET( current.flags, TF_NEWLINE );
			//current.flags |= ( u32 ) TF_NEWLINE;
		//Comments
		}else if ( bufferSize - cursor >= 2 && buffer[cursor] == '/' && buffer[cursor + 1] == '/' ){
			while ( buffer[cursor] != '\n' ) {
				cursor++;
				if ( cursor >= bufferSize )
					return;
			}
		}
#if DOOM3
		else if ( buffer[cursor] == '/' && buffer[cursor + 1] == '*' ) {
			while ( buffer[cursor] != '*' && buffer[cursor] != '/' ) {
				cursor++;
			}
			cursor += 2;

			return;
		}
#endif
		else {
			return;
		}

	 }
}

bool Parser::IsStartOfString( char c ) const {
	return ( isalpha( c ) || c == '_' );
}

bool Parser::IsPartOfString( char c ) const {
	return ( isalnum( c ) || c == '_' );
}

bool Parser::IsPartOfNumber( char c, bool hasDecimal ) const {
	if ( isdigit( c ) )
		return true;
	if ( !hasDecimal && c == '.' )
		return true;
	//technically could break if i dont check for other ones
	if ( c == 'e' )
		return true;

	return false;
}

void Parser::SetBuffer( char* buffer, u32 size ) {
	this->buffer = buffer;
	this->bufferSize = size;
	this->cursor = 0;
}

Token Parser::ReadToken() {
	memset( &current, 0, sizeof( Token ) );
	RemoveWhitespace();

	current.data = buffer + cursor;
	current.line = line;
	current.colStart = cursor;

	if ( cursor >= bufferSize ) {
		current.type = TT_EOF;
		return current;
	}
	
	char c = buffer[cursor];


	//String
	if ( IsStartOfString( c ) ) {
		current.type = TT_STRING;
		current.subType = TS_STRING;	
		while ( IsPartOfString( buffer[cursor] ) ) {
			cursor++;
		}
		current.length = cursor - current.colStart;
		return current;
	}

	//Numbers
	if ( IsPartOfNumber( c, false ) || c == '-' ) {
		bool hasDecimal = ( c == '.' );
		current.type = TT_NUMBER;
		cursor++;

		while ( IsPartOfNumber( buffer[cursor], hasDecimal ) ) {
			if ( cursor >= bufferSize )
				break;

			if ( buffer[cursor] == '.' ) hasDecimal = true;
			
			if ( buffer[cursor] == 'e' )
				cursor++;

			cursor++;

		}

		current.subType = ( hasDecimal == true ) ? TS_FLOAT : TS_INT;
		current.length = cursor - current.colStart;
		return current;
	}

	//Names (Quotes)
	if ( c == '"' ) {
		current.type = TT_STRING;
		current.subType = TS_NAME;

		//if the last thing is a " with no ending brace just return EOF
		if ( cursor + 1 > bufferSize ) {
			current.type = TT_EOF;
			return current;
		}

		cursor++;
		while ( buffer[cursor] != '"' ) {
			if ( cursor > bufferSize ) {
				break;
			}

			cursor++;
		}
		cursor++;

		//Remove front "
		current.data++;
		current.colStart += 1;
		//Removes back "
		current.length = cursor - current.colStart - 1;
		return current;
	}

	//Puncuatipn
	
	if ( c == '{' || c == '}' || c == '(' || c == ')' || c == '#' || c == '/' || c == ':' || c == '\'' || c == '=' || c == ',' || c == '[' || c== ']' ) {
		current.type = TT_PUNCTUATION;
		current.subType = ( TokenSubtype ) c;
		current.length = 1;
		cursor++;
		return current;
	}
	
	current.type = TT_UNKOWN;
	while ( c != ' ' && c != '\n' ) {
		current.length++;
		c = buffer[++cursor];
	}

	return current;
}


bool Parser::ExpectedTokenString( const char* expected ) {
#if 1
	if ( current.type != TT_STRING ) {
		printf( "Error parser expected string %s but token was type %d\n", expected, ( int ) current.type );
		current.Print();
		return false;
	}

	char c[64]{ 0 };
	current.ToString( c, 64 );

	if ( strcmp( expected, c ) != 0 ) 		{
		printf( "Error parser expected token with string %s, but token had string %s\n", expected, c );
	}
#endif
	ReadToken();
	return true;
}


bool Parser::SkipUntilTokenOfType( TokenType type, TokenSubtype subType ) {
	//We're going to assume that this token is not the token being searched for.
	Token start = ReadToken();

	while ( start.type != EOF ) {
		if ( start.type == type ) {
			if ( subType == TS_NONE || start.subType == subType ) {
				current = start;
				return true;
			}
		}
		start = ReadToken();
	}
	printf( "error parser could not find token of type %d\n", ( int ) type );
	return false;
}

bool Parser::SkipUntilTokenWithString( const char* lookFor ) {
	ReadToken();

	if ( strlen( lookFor ) > 64 ) {
		printf( "Lookfor string too big, max 64 characters\n" );
		return false;
	}

	char temp[64]{};
	while ( current.type != TT_EOF) {
		if ( current.type != TT_STRING ) {
			ReadToken();
			continue;
		}

		memset( temp, 0, 64 );
		memcpy( temp, current.data, ( current.length < 64 ) ? current.length : 64 );

		if ( strcmp( ( const char* ) temp, lookFor ) == 0 ) {
			return true;
		}
		ReadToken();
	}
	printf( "Parser could not find token with string %s\n", lookFor );
	return false;
}

int Parser::ParseIntEqualInFront() {
	if ( !ExpectedTokenTypePunctuation( '=' ) ) {
		printf( "Error can not parseIntEqualInFront if frist char is not =\n" );
		return 0;
	}
		return ParseInt();
}

int Parser::ParseInt() {
	int i = current.ToInt();
	ReadToken();
	return i;
}

float Parser::ParseFloat() {
	float f = current.ToFloat();
	ReadToken();
	return f;
}

float Parser::ParseFloatFromQuotes() {
	char f[32] = {};
	memcpy( f, current.data, 32 );
	float n = atof( f );
	ReadToken();
	return n;
}


bool Parser::ExpectedTokenTypePunctuation( char symbol ){
	if ( current.type != TT_PUNCTUATION ) {
		printf( "Token not type puncation. token is: " );
		current.Print();
		return false;
	}

	if ( current.data[0] != symbol ) {
		printf( "Token expected to have %c, but had %c instead", symbol, current.data[0] );
		return false;
	}

	ReadToken();
	return true;
}

void Parser::ParseString( char* buffer, u32 bufferSize ) {
	if ( current.type != TT_STRING ) {
		printf( "error Current token is not type string\n" );
		current.Print();
		return;
	}

	memcpy( buffer, current.data, ( current.length < bufferSize ) ? current.length : bufferSize );
	ReadToken();
}

void Parser::ParseVec(float* dest, int size, bool hasParenthesesSurrounding ){
	if ( hasParenthesesSurrounding ) {
		if ( current.subType == TS_NAME ) {
			cursor -= (current.length + 1);
			ReadToken();
		}
		//ExpectedTokenTypePunctuation( '(' );
		else {
			if ( current.subType != '(' && current.subType != '[' )
				printf( "bad parentheses around vec" );
			ReadToken();
		}

	}

	for ( int i = 0; i < size; i++ ) {
		float f = ParseFloat();
		dest[i] = f;
	}

	if ( hasParenthesesSurrounding ) {
		if ( current.subType == TS_NAME ) {
			cursor -= ( current.length + 1 );
			ReadToken();
		}
		else {

		if ( current.subType != ')' && current.subType != ']' )
			printf( "bad parentheses around vec" );
			ReadToken();
		}
		//ExpectedTokenTypePunctuation( ')' );
	}
}

void Parser::PrintCurrent() {
	current.Print();
}

bool Parser::AtEndOfFile() {
	return current.type == TT_EOF;
}

Token Parser::LeaveCurrentBrackets( int currentDepth ) {
	Token t = GetCurrent();
	if ( t.subType == '}' )
		currentDepth--;
	if ( t.subType == '{' )
		currentDepth++;

	while ( 1 ) {
		Token t = ReadToken();

		if ( t.type == TT_EOF ) {
			printf( "end of file. brackets never end\n" );
			return t;
		}

		if ( t.subType == '}' ) {
			currentDepth--;
		}
		if ( t.subType == '{' ) 
			currentDepth++;

		if ( currentDepth == 0 )
			break;
	}
	
	return t;
}

Token Parser::ReadPath( char* buffer, u32 pathbufferLen ) {
	RemoveWhitespace();
	pathbufferLen = -1; //room for \0
	Token t = GetCurrent();
	int currentLength = 0;

	memcpy( buffer, t.data, t.length );
	currentLength = t.length;

	while ( 1 ) {
		t = ReadToken();

		if ( t.type == TT_EOF ) {
			printf( "Could not finish path\n" );
			return t;
		}

		if ( BIT_CHECK( t.flags, TF_SPACE ) || BIT_CHECK( t.flags, TF_NEWLINE ) || t.subType == ',' ) {
			return t;
		}

		else {
			int sizeAfter = currentLength + t.length;
			if ( sizeAfter > pathbufferLen ) {
				printf( "Could not fit entire path." );
				continue;
			}
			else {
				memcpy( buffer + currentLength, t.data, t.length );
				currentLength += t.length;
			}

		}
	}
}

//This is very slow. Try not to use it
//Too much state to properly nuke and regain. Look into later i suppose
//One thing it loses are the flags, that caused me a pretty bad headache for half an hour
Token Parser::PeekNext() {
	//Copy Entire parser
	Parser parser2 = *this;
	Token t = ReadToken();
	*this = parser2;
	return t;
}
