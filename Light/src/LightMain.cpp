#define _CRT_SECURE_NO_WARNINGS
#include "lightdef.h"

#include "Core/parser.cpp"

bool Trace( Vec3 start, Vec3 end, int face, bool ignoreSelf );

WorldInfo world;
int ATLAS_SIZE;
float TEXEL_SIZE_WORLD_UNITS;
float AMBIENT;

void LoadFile( const char* path ) {
    ATLAS_SIZE = 1024;
    TEXEL_SIZE_WORLD_UNITS = 1.0f;

    //Read Light File
    FILE* lightOut = fopen( path, "rb" );
    if( !lightOut ) {
        printf( "Can not find file %s\n", path );
        return;
    }

    fread( &world.numBrushes, 4, 1, lightOut );
    fread( &world.numVertices, 4, 1, lightOut );
    fread( &world.numFaces, 4, 1, lightOut );
    fread( &world.numIndices, 4, 1, lightOut );

    world.faces = ( LightMapFace* ) malloc( sizeof( LightMapFace ) * world.numFaces );
    world.vertices = ( StaticVertex* ) malloc( sizeof( StaticVertex ) * world.numVertices );
    world.indices = ( u32* ) malloc( 4 * world.numIndices );
    world.fullNodes = ( AtlasNode** ) malloc( world.numFaces * 8 );
    world.brushes = ( LightMapBrush* ) malloc( sizeof( LightMapBrush ) * world.numBrushes );
	world.imageColors = ( Vec3* ) malloc( sizeof( Vec3 ) * ATLAS_SIZE * ATLAS_SIZE );
    world.surfaces = ( LightSurface* ) malloc( sizeof( LightSurface ) * world.numFaces );
    world.imageRaw = ( u32* ) malloc( sizeof( u32 ) * ATLAS_SIZE * ATLAS_SIZE );
    memset( world.imageRaw, 0, sizeof( u32 ) * ATLAS_SIZE * ATLAS_SIZE );
    for( int n = 0; n < ATLAS_SIZE * ATLAS_SIZE; n++ )
        world.imageRaw[n] = ( 255 << 24 );

    memset( world.fullNodes, 0, 8 * world.numFaces );
    fread(  world.brushes, sizeof( LightMapBrush ) * world.numBrushes, 1, lightOut );
    fread(  world.faces, world.numFaces * sizeof( LightMapFace ), 1, lightOut );
    fread(  world.vertices, world.numVertices * sizeof( StaticVertex ), 1, lightOut );
    fread(  world.indices, 4 * world.numIndices, 1, lightOut );

    fclose( lightOut );

    //Read Entity File
    char entPath[256]{};
    int len = strlen( path );
    memcpy( entPath, path, len );
    entPath[len - 3] = 'e';
    entPath[len - 2] = 'n';
    entPath[len - 1] = 't';

    FILE* entFile = fopen( entPath, "rb" );
    if( !entFile ) {
        printf( "could not load entity file %s\n", entFile );
        return;
    }

    fseek( entFile, 0, SEEK_END );
    len = ftell( entFile );
    fseek( entFile, 0, SEEK_SET );

    char* buffer = ( char* ) malloc( len + 1 );
    fread( buffer, len, 1, entFile );;
    buffer[len] = '\0';
    fclose( entFile );

    Parser parser( buffer, len );
    parser.ReadToken();

    while( parser.GetCurrent().type != TT_EOF ) {
        parser.ExpectedTokenTypePunctuation( '{' );
        parser.ExpectedTokenString( "classname" );

        char className[32]{};
        parser.ParseString( className, 32 );

        if( !strcmp( className, "light" ) ) {
            Light light{};
            light.type = LIGHT_POINT;
            light.color = Vec3( 1 );
            light.cutoff = glm::cos( glm::radians( 20.0f ) );
            light.intensity = 1;
            LightSetAttenuation( &light, 20 );

            while( 1 ) {
                char key[32]{};
                char value[32]{};

                parser.ParseString( key, 32 );
                parser.ParseString( value, 32 );


                if( !strcmp( key, "origin" ) ) {
                    light.pos = StringToVec3( value, true );
                }
                else if( !strcmp( key, "type" ) ) {
                    light.type = ( float ) ( atoi( value ) );
                }
                //Should set float (0-1)
                else if( !strcmp( key, "_color" ) ) {
                    light.color = StringToVec3( value, false );
                }
                else if( !strcmp( key, "attenuation" ) ) {
                    LightSetAttenuation( &light, atoi( value ) );
                }
                else if( !strcmp( key, "intensity" ) ) {
                    light.intensity = atof( value );
                }
                else if( !strcmp( key, "direction" ) ) {
                    light.dir = StringToVec3( value, 0 );
                }
                else {
                }

                if( parser.GetCurrent().subType == '}' ) {
                    parser.ReadToken();
                    break;
                }
            }
            world.lights.push_back( light );
        }
        else {
            parser.LeaveCurrentBrackets( 1 );
            parser.ReadToken();
        }
    }
}

void EditCum( const char* lmoPath ) {
    char path[256]{};
    strcpy( path, lmoPath );
    int slen = strlen( path );
    path[slen - 3] = '.c';
    path[slen - 2] = '.u';
    path[slen - 1] = '.m';
    
    //Read in file data
    FILE* file = fopen( path, "rb" );

    if( !file ) {
        printf( "Could not find file at %s\n", path );
        return;
    }

    fseek( file, 0, SEEK_END );
    long len = ftell( file );
    fseek( file, 0, SEEK_SET );

	char* fileData = (char*) malloc( len );
    fread( fileData, len, 1, file );
    fclose( file );

    //Find first Vertex position
    char* verticesCharPtr = fileData + 20;
    StaticVertex* vertices = ( StaticVertex* ) verticesCharPtr;

    //Give vertices their lightmap information
    for( int i = 0; i < world.numVertices; i++ ) {
        vertices[i].lightTex = world.vertices[i].lightTex;
    }

    //write file back
    file = fopen( path, "wb" );
    fwrite( fileData, len, 1, file );
    fclose( file );
}



int main( void ) {
    ATLAS_SIZE = 1024;
    TEXEL_SIZE_WORLD_UNITS = 4.0f;

    const char* mapFilePath = "c:/workspace/cpp/realgame/realgame/res/maps/demo.lmo";
    LoadFile( mapFilePath );

    GenerateLightmap();
    EditCum(mapFilePath);

    char outPath[128]{};
    strcpy( outPath, mapFilePath );
    int len = strlen( outPath );
    outPath[len - 3] = 'l';
    outPath[len - 2] = 'g';
    outPath[len - 1] = 't';

    u32 numTexels = world.texelLocations.size();
    FILE* outFile = fopen( outPath, "wb" );
    fwrite( &ATLAS_SIZE, 4, 1, outFile );
    fwrite( world.imageRaw, ATLAS_SIZE * ATLAS_SIZE * 4, 1, outFile );
    fclose( outFile );
    return 0;
}
