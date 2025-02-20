#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include "Cum.h"


#include "Core\Parser.h"
#include "Core\Parser.cpp"

#include "Core\Timer.h"
#include "Core\Timer.cpp"
const char* help = "cum.exe Input -args\n\
-output : Where to store the final result. Defualt is same folder\n\
-scale: Scale brushes by 1/factor. Default is 16\n\
-defaultNormals: Will not change normals to OGL version\
TODO-texpath: Where the texture folder is stored\n\
Note: All paths with spaces but be surrounded by \" \" ";

//TODO
//	"UI"
//		Look for argv[1]
//		Check for -Whatever
	//Texture scale/offset wrong?

char* input;
char* output;
double scale = ( 1.0f / 32.0f );
bool fixNormals = true;

void Help() {
	printf( "help: %s\n", help );
}
#define _DEBUG
int main( int argc, const char** argv ) {
#ifdef _DEBUG
	if ( argc == 1 ) {
		input = ( char* ) malloc( 1000 );
		memset( input, 0, 1000 );
		strcpy_s( input, 1000, "C:\\Workspace\\Cpp\\RealGame\\RealGame\\res\\maps\\lighting.map" );
	}else
		input = (char*) argv[1];
#else
	if ( argc < 2 ) {
		Help();
		return 0;
	}
	input = ( char* ) argv[1];
#endif

	Timer timer;


	int inputlen = strlen( input );
	for ( int i = 0; i < inputlen; i++ ) {
		if ( input[i] == '\\' )
			input[i] = '/';
	}

		
	for ( int i = 2; i < argc; i++ ) {
		if ( !strcmp( argv[i], "-output" ) && i+1 != argc ) {
			output = (char*) argv[i + 1];
			i++;
		}

		else if ( !strcmp( argv[i], "-scale" ) && i + 1 != argc ) {
			scale = 1.0f / atof(argv[i + 1]);
			i++;
		}

		else if ( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "-help" ) ) {
			Help();
		}

		else if ( !strcmp( argv[i], "-defaultNormals" ) ) {
			fixNormals = false;
		}

	}

	if ( !output ) {
		//Add a tiny pad to be safe
		u32 outputLen = strlen( input );
		output = (char*) malloc( outputLen + 2 );
		output[outputLen] = '\0';
		memset( output, outputLen, 0 );
		strcpy_s( output, outputLen + 1, input );
		output[outputLen - 3] = 'c';
		output[outputLen - 2] = 'u';
		output[outputLen - 1] = 'm';
	}

	bool result = Compile( input, output );
	if ( result ) {
		printf( "Map Compiled Successfully to %s\n", output );

		timer.Tick();
		float ms = timer.GetTimeMiliSeconds();
		if ( ms > 60000.0f ) {
			ms /= 60000.0f;
			printf( "Time Taken: %.2f Minutes\n", ms );
		}
		else if ( ms > 1000.0f ) {
			printf( "Time Taken: %.2f Seconds\n", timer.GetTimeSeconds() );
		}
		else {
			printf( "Time Taken: %.2f ms\n", ms );
		}
	}
	else {
		printf( "[WARNING] Map Was Not Compiled" );
	}

}

