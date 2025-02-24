#pragma once
#include "CoreDef.h"

// token types
enum TokenType {
	TT_UNKOWN =		-1,
	TT_EOF			= 0,
	TT_STRING		= 1,		// string
	TT_NUMBER		= 2,		// number
	TT_PUNCTUATION	= 3		// punctuation
};

enum TokenSubtype {
	TS_NONE = 0,
	TS_INT, //No decimal
	TS_FLOAT, // has decimal
	TS_STRING,
	TS_NAME, //These are inside of quotes. can also be paths
	//any puncuation subtype = it's asci

	TS_LB = '{',
	TS_RB = '}',
	TS_LP = '(',
	TS_RP = ')',
	TS_HT = '#',
	TS_FS = '/',
	TS_COL = ':',
	TS_SQ = '\'',
	TS_EQ = '=',
	TS_CM = ',',
	TS_LSB = '[',
	TS_RSB = ']',

};

enum tokenFlags {
	TF_NONE = 0,
	TF_SPACE = 1,
	TF_NEWLINE = 2
};


struct DLL Token {
	const char* data;
	TokenType type;
	TokenSubtype subType;
	Token* next;//index
	u32 line;
	u32 colStart;
	u32 length;
	u32 flags;

	void Print();
	int ToInt();
	float ToFloat();
	void ToString( char* buffer, int bufferLength );
	int StringEquals( char* other );
};

#define MAX_TOKENS 500000
class DLL Parser {
public:
	Parser( char* buffer, u64 size );
	Parser();
	//void ParseData();
	void SetBuffer( char* buffer, u32 size );

	//Reads token, checks value and advances
	bool ExpectedTokenString( const char* expected );
	bool ExpectedTokenTypePunctuation( char symbol );

	bool SkipUntilTokenOfType( TokenType type, TokenSubtype subType = TS_NONE );
	bool SkipUntilTokenWithString( const char* str );

	//Consumes current token and return a copy
	Token ReadToken();
	//Gives you the next token and resets the state	
	Token PeekNext();
	//Just print no reading
	void PrintCurrent();
	//Reads token value and advances
	int ParseInt();
	//Reads equal then next is int
	int ParseIntEqualInFront();
	float ParseFloat();
	float ParseFloatFromQuotes();//same as part float but wont throw error
	void ParseString( char* buffer, u32 bufferSize );
	u32 Cursor() { return cursor; }
	char* Data() { return buffer; }
	//Parse float of size and place inside of dests
	void ParseVec( float* dest, int VecSize, bool hasParenthesesSurrounding );
	bool AtEndOfFile();
	Token GetCurrent() { return current; };

	//assumes you are in a thing with depth ({}), it will keep reading until the depth you are in is 0
	//If the next } leaves the bracket it works
	//if there are more { brackets inside, it will make sure you leave those first
	Token LeaveCurrentBrackets(int currentDepth);

	//Reads token until you find a newLine or a space
	//Starts with the current token.
	//If Path does not fit, It will reading tokens until its suppose to stop but will no tadd them to the buffer
	Token ReadPath(char* buffer, u32 bufferLen);
	//Next token to parse
	Token current;

	void RemoveWhitespace();
	bool IsStartOfString( char c ) const;
	bool IsPartOfString( char c ) const;
	bool IsPartOfNumber( char c, bool hasDecimal ) const;
	char* buffer;
	u32 bufferSize;
	u32 cursor;
	u32 line;
	u32 lineStart;
};
Vec3 StringToVec3( const char* value, bool fix );