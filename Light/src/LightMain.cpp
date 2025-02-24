#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include "def.h"
#include "Renderer/Renderer.h"
#include "lightdef.h"

#include "Core/parser.cpp"
//Undef my versions of RGB
#include <string.h>
bool Trace( Vec3 start, Vec3 end, int face, bool ignoreSelf );

Atlas atlas;
u32* imageRaw;
Vec3 imageColors[( int ) ( ATLAS_SIZE * ATLAS_SIZE )]; // Need this for 

StaticVertex* vertices;
//RenderBrushFace* faces;
LightMapFace* faces;
u32* indices;
u32 numVertices;
u32 numFaces;
u32 numIndices;
u32 numBrushes;

Vec2* lightmapCoords;
AtlasNode** fullNodes;
LightMapBrush* brushes;
LightSurface* surfaces;

std::vector<Vec4> texelLocations;
std::vector<Light> lights;


void LoadFile( const char* path ) {
    //Read Light File
    FILE* lightOut = fopen( path, "rb" );
    if( !lightOut ) {
        printf( "Can not find file %s\n", path );
        return;
    }

    fread( &numBrushes, 4, 1, lightOut );
    fread( &numVertices, 4, 1, lightOut );
    fread( &numFaces, 4, 1, lightOut );
    fread( &numIndices, 4, 1, lightOut );

    faces = ( LightMapFace* ) malloc( sizeof( LightMapFace ) * numFaces );
    vertices = ( StaticVertex* ) malloc( sizeof( StaticVertex ) * numVertices );
    indices = ( u32* ) malloc( 4 * numIndices );
    fullNodes = ( AtlasNode** ) malloc( numFaces * 8 );
    brushes = ( LightMapBrush* ) malloc( sizeof( LightMapBrush ) * numBrushes );

    memset( fullNodes, 0, 8 * numFaces );
    fread( brushes, sizeof( LightMapBrush ) * numBrushes, 1, lightOut );
    fread( faces, numFaces * sizeof( LightMapFace ), 1, lightOut );
    fread( vertices, numVertices * sizeof( StaticVertex ), 1, lightOut );
    fread( indices, 4 * numIndices, 1, lightOut );

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
            lights.push_back( light );
        }
        else {
            parser.LeaveCurrentBrackets( 1 );
            parser.ReadToken();
        }
    }
}

inline AtlasNode* NewAtlasNode() {
    AtlasNode* node = ( AtlasNode* ) malloc( sizeof( AtlasNode ) );
    memset( node, 0, sizeof( *node ) );
    node->faceIndex = -1;
    atlas.count++;
    return node;
}

AtlasNode* FindNode( const Vec2& size ) {
    AtlasNode** queue = ( AtlasNode** ) malloc( atlas.count * sizeof( AtlasNode* ) );
    int numQueue = 0;
    queue[numQueue++] = atlas.head;

    AtlasNode* best = 0;
    float bestPercent = -99999.0f;
    while( numQueue > 0 ) {
        AtlasNode* node = queue[--numQueue];
        if( node->faceIndex == -1 ) {
            Vec2 nodeSize = node->max - node->min;

            //Fits
            if( nodeSize.x >= size.x && nodeSize.y >= size.y ) {
                //This is a good enough node
                float percent = ( size.x * size.y ) / ( nodeSize.x * nodeSize.y );
                if( percent > bestPercent || !best ) {
                    best = node;
                    bestPercent = percent;
                }

                //Quick checks to see if its perfect
                if( size == nodeSize )
                    goto end;
                if( size.x == nodeSize.x ) {
                    best = node;
                    goto end;
                }
                if( size.y == nodeSize.y ) {
                    best = node;
                    goto end;
                }
            }
        }

        if( node->left )
            queue[numQueue++] = node->left;
        if( node->right )
            queue[numQueue++] = node->right;
    }

    if( !best ) {
        printf( "Could not find free node: " );
        printf( "%.2f %.2f\n", size.x, size.y );
    }
    return best;

end:
    free( queue );
    return best;
}

AtlasNode* InsertIntoAtlas( const Vec2& size, int faceIndex ) {
    AtlasNode* bestNode = FindNode( size );
    if( !bestNode ) {
        return 0;
    }

    bestNode->faceIndex = faceIndex;
    Vec2 bestNodeSize = bestNode->max - bestNode->min;
    Vec2 sizeDiff = bestNodeSize - size;

    //If this takes up the entire size, we remove it
    if( sizeDiff == Vec2( 0 ) ) {
        return bestNode;
    }

    //Trim the top
    if( sizeDiff.x == 0 ) {
        AtlasNode* left = NewAtlasNode();
        left->min = bestNode->min + Vec2( 0, size.y );
        left->max = bestNode->max;
        bestNode->left = left;

        bestNode->max = bestNode->min + size;
        return bestNode;
    }

    if( sizeDiff.y == 0 ) {
        AtlasNode* left = NewAtlasNode();
        left->min = bestNode->min + Vec2( size.x, 0 );
        left->max = bestNode->max;
        bestNode->left = left;

        bestNode->max = bestNode->min + size;
        return bestNode;
    }

    //Figure out which direction to split in 
    if( sizeDiff.x >= sizeDiff.y ) {
        //Split Y
        AtlasNode* left = NewAtlasNode();
        left->min = bestNode->min + Vec2( size.x, 0 );
        left->max = Vec2( bestNode->max.x, bestNode->min.y + size.y );
        //left->max = bestNode->max;

        AtlasNode* right = NewAtlasNode();
        right->min = bestNode->min + Vec2( 0, size.y );
        right->max = bestNode->max;

        bestNode->left = left;
        bestNode->right = right;

        bestNode->max = bestNode->min + size;
    }
    else {
        //Split X
        AtlasNode* left = NewAtlasNode();
        left->min = bestNode->min + Vec2( 0, size.y );
        left->max = Vec2( bestNode->min.x + size.x, bestNode->max.y );
        //left->max = bestNode->max;

        AtlasNode* right = NewAtlasNode();
        right->min = bestNode->min + Vec2( size.x, 0 );
        right->max = bestNode->max;

        bestNode->left = right;
        bestNode->right = left;

        bestNode->max = bestNode->min + size;
    }
    return bestNode;
}

void GetFaceSize( int firstVertex, int count, Vec2* size, Vec2* outMin, Vec2* outMax, Vec3* outVertMin, Vec3* outVertMax ) {
    Vec3 min( 999999.0f );
    Vec3 max( -99999.0f );

    Vec3 vmin( 999999.0f );
    Vec3 vmax( -99999.0f );

    for( int n = 0; n < count; n++ ) {
        //LightMap Bounds
        Vec2 vert = vertices[firstVertex + n].lightTex;
        for( int k = 0; k < 2; k++ ) {
            if( vert[k] > max[k] )
                max[k] = vert[k];
            if( vert[k] < min[k] )
                min[k] = vert[k];
        }
        //Vertex Bounds
        Vec3 pos = vertices[firstVertex + n].pos;
        for( int k = 0; k < 3; k++ ) {
            if( pos[k] > vmax[k] )
                vmax[k] = pos[k];
            if( pos[k] < vmin[k] )
                vmin[k] = pos[k];
        }
    }
    //Shift bounds so min is (0,0)
    *outMin = min;
    *outMax = max;

    *outVertMin = vmin;
    *outVertMax = vmax;


    max -= min;
    *size = max / 32.0f;
}

void GetMinMaxVertices( LightMapFace* face, Vec3* outMin, Vec3* outMax ) {
    Vec3 min( 99999.0f );
    Vec3 max( -99999.0f );

    for( int n = 0; n < face->numVertices; n++ ) {
        Vec3 v = vertices[face->firstVertex + n].pos;
        min = glm::min( v, min );
        max = glm::max( v, max );
    }
    *outMin = min;
    *outMax = max;
}

void GetMinOnAxes( LightMapFace* face, Vec2* minOut, Vec2* maxOut ) {
    Vec2 min( 9999999 );
    Vec2 max( -9999999 );

    Vec3 axes[2]{
        face->u,face->v
    };

    for( int i = 0; i < face->numVertices; i++ ) {
        Vec3 vert = vertices[face->firstVertex + i].pos;

        for( int n = 0; n < 2; n++ ) {
            float dist = glm::dot( vert, axes[n] );
            if( dist > max[n] )
                max[n] = dist;
            if( dist < min[n] )
                min[n] = dist;
        }
    }
    *minOut = min;
    *maxOut = max;
}


void CreateUVs() {
    surfaces = ( LightSurface* ) malloc( sizeof( LightSurface ) * numFaces );

    //What if n instead get the min position along the U & V axes?
    for( int f = 0; f < numFaces; f++ ) {
        LightMapFace* face = &faces[f];

        face->texNormal = glm::cross( face->u, face->v );
        face->texNormal = glm::normalize( face->texNormal );

        float distScale = glm::dot( face->texNormal, face->normal );
        if( distScale < 0 ) {
            face->texNormal *= -1;
            distScale *= -1;
        }
        distScale = 1.0f / distScale;


        float distU = glm::dot( face->u, face->normal );
        float distV = glm::dot( face->v, face->normal );

        Vec3 texToWorldU = face->u + face->texNormal * ( -distScale * distU );
        Vec3 texToWorldV = face->v + face->texNormal * ( -distScale * distV );

        float planeDist = glm::dot( Vec3( 0 ), face->normal ) - face->d;
        planeDist *= distScale;
        Vec3 origin = planeDist * face->texNormal;

        //Copy Into Surface
        LightSurface* surface = &surfaces[f];
        surface->axes[0] = face->u;
        surface->axes[1] = face->v;
        surface->axes[2] = face->texNormal;
        surface->d = face->d;
        surface->n = face->normal;
        surface->texOrigin = origin;
        surface->texToWorld[0] = texToWorldU;
        surface->texToWorld[1] = texToWorldV;

        //These are the extreme DISTANCES of each axis
        //there are NOT points 
        Vec2 min, max;
        GetMinOnAxes( face, &min, &max );
        surface->realMin = min;
        surface->realMax = max;

        //min -= TEXEL_SIZE_WORLD_UNITS;
        //max +=  TEXEL_SIZE_WORLD_UNITS;

        for( int i = 0; i < 2; i++ ) {
            surface->paddedMin[i] = floor( min[i] / TEXEL_SIZE_WORLD_UNITS ) - PAD;
            surface->paddedMax[i] = ceil( max[i] / TEXEL_SIZE_WORLD_UNITS ) + PAD;
        }
        surface->paddedTextureSize = surface->paddedMax - surface->paddedMin;

        //Now that the worldspace stuff is done
        //Go through the vertices and give them texture coordinates
        AtlasNode* newNode = InsertIntoAtlas( surface->paddedTextureSize, f );
        Vec2 atlasUVMin = newNode->min / ATLAS_SIZE;
        Vec2 atlasUVMax = newNode->max / ATLAS_SIZE;

        for( int n = 0; n < face->numVertices; n++ ) {
            face->normal = glm::normalize( face->normal );

            StaticVertex& vertex = vertices[face->firstVertex + n];
            //Get the percentage of how far in the larger box this is. That is it's face UV
            float verU = glm::dot( vertex.pos, face->u );
            float verV = glm::dot( vertex.pos, face->v );

            float tU = ( verU - min[0] ) / ( max[0] - min[0] );
            float tV = ( verV - min[1] ) / ( max[1] - min[1] );

            //Now with the face UV, figure out where it is inside of the global atlas 
            vertex.lightTex = atlasUVMin + ( atlasUVMax - atlasUVMin ) * Vec2( tU, tV );
            //printf( "(%.2f %.2f)\n", vertex.lightMapTex.x, vertex.lightMapTex.y );
        }
        surface->node = newNode;
    }
}

bool PhysicsRaycastHull( Vec3 start, Vec3 end, int hull );

void LightLevel() {
    for( int f = 0; f < numFaces; f++ ) {
        LightMapFace* face = &faces[f];
        LightSurface* surface = &surfaces[f];

#if 1
        int samples = 2;
        int h = ( surface->paddedTextureSize[1] ) * 2;
        int w = ( surface->paddedTextureSize[0] ) * 2;
        float starts = ( surface->paddedMin[0] - 0.5 ) * TEXEL_SIZE_WORLD_UNITS;
        float startt = ( surface->paddedMin[1] - 0.5 ) * TEXEL_SIZE_WORLD_UNITS;
        float step = TEXEL_SIZE_WORLD_UNITS / 2.0f;
#else
        int samples = 1;
        int h = surface->paddedTextureSize[1];
        int w = surface->paddedTextureSize[0];

        float starts = ( surface->paddedMin[0] - PAD ) * TEXEL_SIZE_WORLD_UNITS;
        float startt = ( surface->paddedMin[1] - PAD ) * TEXEL_SIZE_WORLD_UNITS;
        float step = TEXEL_SIZE_WORLD_UNITS;

#if 0
        if( starts > 0 ) starts -= TEXEL_SIZE_WORLD_UNITS * PAD;
        else starts += TEXEL_SIZE_WORLD_UNITS * PAD;

        if( startt > 0 ) startt -= TEXEL_SIZE_WORLD_UNITS * PAD;
        else startt += TEXEL_SIZE_WORLD_UNITS * PAD;
#endif


#endif

        Vec2 midTs = Vec2( surface->paddedMin + surface->paddedMax ) / 2.0f;
        Vec3 midPoint = surface->texOrigin + surface->texToWorld[0] * midTs[0] + surface->texToWorld[1] * midTs[1];
        midPoint += surface->axes[2] * .1f; //move slightly off surface for collision detection

        texelLocations.push_back( Vec4( midPoint, 1 ) );

        for( int v = 0; v < h; v++ ) {
            for( int u = 0; u < w; u++ ) {
                float stepU = starts + u * step;
                float stepV = startt + v * step;

                Vec3 point = surface->texOrigin + surface->texToWorld[0] * stepU + surface->texToWorld[1] * stepV;
                point += surface->axes[2] * .1f;
                Vec3 startPoint = point;

                bool good = false;
                float us = starts + u * step;
                float ut = startt + v * step;

                bool adjusted = false;
                for( int t = 0; t < 4; t++ ) {
                    point = surface->texOrigin + surface->texToWorld[0] * us + surface->texToWorld[1] * ut;
                    point += surface->axes[2] * .1f;

                    if( Trace( point, midPoint, f, true ) ) {
                        good = true;
                        break;
                    }
                    adjusted = true;
#define HT (TEXEL_SIZE_WORLD_UNITS / 2.0f )
                    //Try and move by half of a texel to squirm towards the center of the block
                    if( t % 2 == 1 )
                        if( us > midTs[0] )
                            us -= HT;
                        else us += HT;
                    else
                        if( ut > midTs[1] )
                            ut -= HT;
                        else ut += HT;

                    Vec3 move = glm::normalize( midPoint - point );
                    point += move * TEXEL_SIZE_WORLD_UNITS / 2.0f;
                }
                if( !good ) {
                    point = startPoint;
                }

                if( adjusted && !Trace( point, lights[0].pos, f, false ) )
                    point = startPoint;

                Vec2 UVPosition = surface->node->min + Vec2( u, v ) / Vec2( samples );
                int uvIndex = ( int ) UVPosition.x + ( ( int ) UVPosition.y * ( int ) ATLAS_SIZE );

                for( int i = 0; i < lights.size(); i++ ) {
                    bool trace = Trace( point, lights[i].pos, f, true );

                    if( trace ) {
                        float dist = glm::length( point - lights[i].pos );
                        float light = glm::max( glm::dot( glm::normalize( lights[i].pos - point ), surface->n ), 0.0f );
                        light *= lights[i].intensity;
                        float attenuation = 1.0f / ( 1.0 + lights[i].attenuation.y * dist + lights[i].attenuation.z * ( dist * dist ) );
                        light *= attenuation;
                        Vec3 thisColor = light * lights[i].color;
                        imageColors[uvIndex] += thisColor;
                    }
                }
                //if ( adjusted )
                //	texelLocations.push_back( Vec4( point, good ) );
            }
        }
    }

    for( int i = 0; i < ATLAS_SIZE * ATLAS_SIZE; i++ ) {
        imageColors[i] /= 4;
        imageColors[i] += AMBIENT;
        imageColors[i] = glm::clamp( imageColors[i], 0.0f, 1.0f );

        u8 r = imageColors[i].x * 255.0f;
        u8 g = imageColors[i].y * 255.0f;
        u8 b = imageColors[i].z * 255.0f;
        u8 a = 255;


        u32 color = r + ( g << 8 ) + ( b << 16 ) + ( a << 24 );
        imageRaw[i] = color;
    }

    texelLocations.push_back( Vec4( lights[0].pos, 1 ) );
}

bool PhysicsRaycastHull( Vec3 start, Vec3 end, int hull ) {
    float tmin = 0.0f;
    float tmax = 1.0f;
    Vec3 lightPos = end;//lights[light].pos;
    Vec3 dir = lightPos - start;

    for( int n = 0; n < brushes[n].numFaces; n++ ) {
        //Intersect each plane
        LightMapFace* face = &faces[brushes[hull].firstFace + n];
        Vec3 axis = glm::normalize( faces->normal );
        float denom = glm::dot( face->normal, dir );
        float dist = -face->d - glm::dot( face->normal, start );
        //Parallel
        if( fabs( denom ) <= .000000005f ) {
            if( -dist > 0 ) //??
                return 0;
        }
        else {
            float t = dist / denom;
            if( denom < .001f ) {
                if( t > tmin ) tmin = t;
            }
            else {
                if( t < tmax ) tmax = t;
            }
            if( tmin > tmax )
                return 0;
        }
    }
    return 1;
}

bool Trace( Vec3 start, Vec3 end, int face, bool ignoreSelf ) {
    for( int n = 0; n < numBrushes; n++ ) {
        if( ignoreSelf )
            if( face > brushes[n].firstFace && face < brushes[n].firstFace + brushes[n].numFaces )
                continue;

        //if( n == face ) continue;
        if( PhysicsRaycastHull( start, end, n ) )
            return false;
    }

    return true;
}

int main( void ) {
    const char* mapFilePath = "c:/workspace/cpp/realgame/realgame/res/maps/demo.lmo";
    LoadFile( mapFilePath );

    imageRaw = ( u32* ) malloc( sizeof( u32 ) * ATLAS_SIZE * ATLAS_SIZE );
    memset( imageRaw, 0, sizeof( u32 ) * ATLAS_SIZE * ATLAS_SIZE );
    for( int n = 0; n < ATLAS_SIZE * ATLAS_SIZE; n++ )
        imageRaw[n] = ( 255 << 24 );

    const int screenWidth = ATLAS_SIZE * 2;
    const int screenHeight = ATLAS_SIZE;
    atlas.count = 0;
    atlas.head = NewAtlasNode();
    atlas.head->min = Vec2( 0 );
    atlas.head->faceIndex = -1;
    atlas.head->max = Vec2( ATLAS_SIZE, ATLAS_SIZE );

    CreateUVs();
    LightLevel();

    Vec2* lightmapUVsOut = ( Vec2* ) malloc( sizeof( Vec2 ) * numVertices );
    for( int n = 0; n < numVertices; n++ ) {
        lightmapUVsOut[n] = vertices[n].lightTex;
        //printf( "%.2f %.2f\n", lightmapUVsOut[n].x, lightmapUVsOut[n].y );
    }

    char outPath[128]{};
    strcpy( outPath, mapFilePath );
    int len = strlen( outPath );
    outPath[len - 3] = 'l';
    outPath[len - 2] = 'g';
    outPath[len - 1] = 't';

    u32 numTexels = texelLocations.size();
    FILE* outFile = fopen( outPath, "wb" );
    fwrite( lightmapUVsOut, sizeof( Vec2 ), numVertices, outFile );
    fwrite( imageRaw, ATLAS_SIZE * ATLAS_SIZE * 4, 1, outFile );
    fwrite( &numTexels, 4, 1, outFile );
    fwrite( texelLocations.data(), sizeof( Vec4 ) * numTexels, 1, outFile );
    fclose( outFile );
    return 0;
}
