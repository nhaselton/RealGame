#include "def.h"
#include "lightdef.h"
#include "Renderer/Renderer.h"

bool PhysicsRaycastHull( Vec3 start, Vec3 end, int hull );
bool Trace( Vec3 start, Vec3 end, int face, bool ignoreSelf ) {
    for( int n = 0; n < world.numBrushes; n++ ) {
        if( ignoreSelf )
            if( face > world.brushes[n].firstFace && face < world.brushes[n].firstFace + world.brushes[n].numFaces )
                continue;

        //if( n == face ) continue;
        if( PhysicsRaycastHull( start, end, n ) )
            return false;
    }

    return true;
}

inline AtlasNode* NewAtlasNode() {
    AtlasNode* node = ( AtlasNode* ) malloc( sizeof( AtlasNode ) );
    memset( node, 0, sizeof( *node ) );
    node->faceIndex = -1;
    world.atlas.count++;
    return node;
}

AtlasNode* FindNode( const Vec2& size ) {
    AtlasNode** queue = ( AtlasNode** ) malloc( world.atlas.count * sizeof( AtlasNode* ) );
    int numQueue = 0;
    queue[numQueue++] = world.atlas.head;

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
        Vec2 vert = world.vertices[firstVertex + n].lightTex;
        for( int k = 0; k < 2; k++ ) {
            if( vert[k] > max[k] )
                max[k] = vert[k];
            if( vert[k] < min[k] )
                min[k] = vert[k];
        }
        //Vertex Bounds
        Vec3 pos = world.vertices[firstVertex + n].pos;
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

void GetMinOnAxes( LightMapFace* face, Vec2* minOut, Vec2* maxOut ) {
    Vec2 min( 9999999 );
    Vec2 max( -9999999 );

    Vec3 axes[2]{
        face->u,face->v
    };

    for( int i = 0; i < face->numVertices; i++ ) {
        Vec3 vert = world.vertices[face->firstVertex + i].pos;

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

    //What if n instead get the min position along the U & V axes?
    for( int f = 0; f < world.numFaces; f++ ) {
        LightMapFace* face = &world.faces[f];

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
        LightSurface* surface = &world.surfaces[f];
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
        Vec2 atlasUVMin = newNode->min / ( float ) ATLAS_SIZE;
        Vec2 atlasUVMax = newNode->max / ( float ) ATLAS_SIZE;

        for( int n = 0; n < face->numVertices; n++ ) {
            face->normal = glm::normalize( face->normal );

            StaticVertex& vertex = world.vertices[face->firstVertex + n];
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

Vec3 colors[] = {
    YELLOW     ,
    GOLD       ,
    ORANGE     ,
    PINK       ,
    RED        ,
    MAROON     ,
    GREEN      ,
    LIME       ,
    DARKGREEN  ,
    SKYBLUE    ,
    BLUE       ,
    DARKBLUE   ,
    PURPLE     ,
    VIOLET     ,
    DARKPURPLE ,
    BEIGE      ,
    BROWN      ,
    DARKBROWN  ,
};
int numColors = 18;

inline u32 FtoRGB( Vec3 v ) {
    u8 r = v.x * 255;
    u8 g = v.y * 255;
    u8 b = v.z * 255;

    u32 color = r + ( g << 8 ) + ( b << 16 ) + ( 255 << 24 );
    return color;
}

void DrawTexels() {
    for( int i = 0; i < world.numFaces; i++ ) {
        Vec3 c = colors[i % numColors];
        Vec3 c2 = colors[i % numColors]; c2.x /= 2; c2.y /= 2; c2.z /= 2;

        u32 colorNormal = FtoRGB( c );
        u32 colorDim = FtoRGB( c2 );

        LightSurface* surface = &world.surfaces[i];

        int startX = surface->node->min[0];
        int startY = surface->node->min[1] * ( int ) ATLAS_SIZE;

        for( int yy = 0; yy < surface->paddedTextureSize[1]; yy++ ) {
            for( int xx = 0; xx < surface->paddedTextureSize[0]; xx++ ) {
                u32 color = ( ( xx + yy ) % 2 == 0 ) ? colorDim : colorNormal;
                int y = yy * ATLAS_SIZE;
                world.imageRaw[startX + startY + xx + y] = color;
            }
        }
    }
}

void LightLevel() {
    for( int f = 0; f < world.numFaces; f++ ) {
        LightMapFace* face = &world.faces[f];
        LightSurface* surface = &world.surfaces[f];

        int samples, h, w;
        float starts, startt, step;

        if( USE_AA ) {
			samples = 2;
			h = ( surface->paddedTextureSize[1] ) * 2;
			w = ( surface->paddedTextureSize[0] ) * 2;
			starts = ( surface->paddedMin[0] - 0.5 ) * TEXEL_SIZE_WORLD_UNITS;
			startt = ( surface->paddedMin[1] - 0.5 ) * TEXEL_SIZE_WORLD_UNITS;
			step = TEXEL_SIZE_WORLD_UNITS / 2.0f;
        }
        else {
			samples = 1;
			h = surface->paddedTextureSize[1];
			w = surface->paddedTextureSize[0];

			starts = ( surface->paddedMin[0] - PAD ) * TEXEL_SIZE_WORLD_UNITS;
			startt = ( surface->paddedMin[1] - PAD ) * TEXEL_SIZE_WORLD_UNITS;
			step = TEXEL_SIZE_WORLD_UNITS;
        }

        Vec2 midTs = Vec2( surface->paddedMin + surface->paddedMax ) / 2.0f;
        Vec3 midPoint = surface->texOrigin + surface->texToWorld[0] * midTs[0] + surface->texToWorld[1] * midTs[1];
        midPoint += surface->axes[2] * .1f; //move slightly off surface for collision detection

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

                Vec2 UVPosition = surface->node->min + Vec2( u, v ) / Vec2( samples );
                int uvIndex = ( int ) UVPosition.x + ( ( int ) UVPosition.y * ( int ) ATLAS_SIZE );

                for( int i = 0; i < world.lights.size(); i++ ) {
                    bool trace = Trace( point, world.lights[i].pos, f, true );

                    if( trace ) {
                        float dist = glm::length( point - world.lights[i].pos );
                        float light = glm::max( glm::dot( glm::normalize( world.lights[i].pos - point ), surface->n ), 0.0f );
                        light *= world.lights[i].intensity;
                        float attenuation = 1.0f / ( 1.0 + world.lights[i].attenuation.y * dist + world.lights[i].attenuation.z * ( dist * dist ) );
                        light *= attenuation;
                        Vec3 thisColor = light * world.lights[i].color;
                        world.imageColors[uvIndex] += thisColor;
                    }
                }
            }
        }
    }

    float div = ( USE_AA ) ? 4.0f : 1.0f;
    for( int i = 0; i < ATLAS_SIZE * ATLAS_SIZE; i++ ) {
        world.imageColors[i] /= div;
        world.imageColors[i] += AMBIENT;
        world.imageColors[i] = glm::clamp( world.imageColors[i], 0.0f, 1.0f );

        u8 r = world.imageColors[i].x * 255.0f;
        u8 g = world.imageColors[i].y * 255.0f;
        u8 b = world.imageColors[i].z * 255.0f;
        u8 a = 255;

        u32 color = r + ( g << 8 ) + ( b << 16 ) + ( a << 24 );
        world.imageRaw[i] = color;
    }
}

bool PhysicsRaycastHull( Vec3 start, Vec3 end, int hull ) {
    float tmin = 0.0f;
    float tmax = 1.0f;
    Vec3 lightPos = end;//lights[light].pos;
    Vec3 dir = lightPos - start;

    for( int n = 0; n < world.brushes[n].numFaces; n++ ) {
        //Intersect each plane
        LightMapFace* face = &world.faces[world.brushes[hull].firstFace + n];
        Vec3 axis = glm::normalize( world.faces->normal );
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



void GenerateLightmap() {
    world.atlas.count = 0;
    world.atlas.head = NewAtlasNode();
    world.atlas.head->min = Vec2( 0 );
    world.atlas.head->faceIndex = -1;
    world.atlas.head->max = Vec2( ATLAS_SIZE, ATLAS_SIZE );

    CreateUVs();
    //DrawTexels();
    LightLevel();
}