#include "Cum.h"

#include "c:\Workspace\Cpp\RealGame\RealGame\src\Resources\Level.h"
#include "c:\Workspace\Cpp\RealGame\RealGame\src\physics\physics.h"
#include "c:\Workspace\Cpp\RealGame\RealGame\src\Resources\ModelManager.h"
#include "c:\Workspace\Cpp\RealGame\RealGame\src\Renderer\Renderer.h"
#include "Resources\TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/gtx/norm.hpp> 

BVHTree ConstructBVH( std::vector<NPBrush>& brushes );

static inline bool ThreePlaneIntersection( DPlane* a, DPlane* b, DPlane* c, dVec3* out ) {
	double	denom;
	denom = glm::dot( c->n, glm::cross( a->n, b->n ) );

	if ( fabs( denom ) < FEPSILON ) {
		return false;
	}
	*out = ( ( glm::cross( a->n, b->n ) ) * -c->d -
		( glm::cross( b->n, c->n ) ) * a->d -
		( glm::cross( c->n, a->n ) ) * b->d ) / denom;
	return true;
}

static bool IsValid( dVec3 nonRoundedPoint, NPBrush* brush, std::vector<NPFace>& faces ) {
	for ( int j = 0; j < brush->numFaces; j++ ) {
		NPFace* face = &faces[brush->firstPlane + j];
		if ( glm::dot( nonRoundedPoint, face->n ) + face->d - FEPSILON > 0 ) {
			return false;
		}
	}
	return true;
}

static bool IsUnique( NPFace* face, const dVec3& check, const std::vector<DBrushVertex>& vertices ) {
	for ( int i = 0; i < face->numVertices; i++ ) {
		//Not sure if should do == or length2 < fepsilon
		if ( glm::length( vertices[face->firstVertex + i].pos - check ) < .001f )
			return false;

		//if ( check == vertices[face->firstVertex + i].pos ) {
		//	return false;
		//}
	}
	return true;
}

bool Compile( const char* input, const char* output ) {
	FILE* file = 0;
	fopen_s( &file, input, "rb" );
	if ( !file ) {
		printf( "[FATAL ERROR] Could not load map %s\n", input );
		return false;
	}
	//Setup allocators
	fseek( file, 0, SEEK_END );
	u32 len = ftell( file );
	fseek( file, 0, SEEK_SET );

	u8* buffer = ( u8* ) malloc( len );
	if ( !buffer ) {
		printf( "[FATAL ERROR] Could not allocate buffer of %u\n", len );
		return false;
	}

	fread_s( buffer, len, len, 1, file );
	fclose( file );

	Parser parser( ( char* ) buffer, len );
	parser.ReadToken();

	Token tok = parser.GetCurrent();
	while ( ( parser.GetCurrent().type != TT_EOF ) ) {
		parser.ExpectedTokenTypePunctuation( '{' );
		//Figure out what to make from this
		if ( parser.GetCurrent().StringEquals( ( char* ) "mapversion" ) ) {
			bool ws = LoadWorldSpawn( &parser, output );
			if ( !ws )
				return false;
			break;
		}
		else {
			return true;
		}
	}

	return true;
}

struct TexInfo {
	char name[NAME_BUF_LEN];
	IVec2 size;
};

bool CloseEnough( const Vec3& a, const Vec3& b ) {
	float dx = fabs( b.x - a.x );
	float dy = fabs( b.y - a.y );
	float dz = fabs( b.z - a.z );
	return (
		dx < .001f &&
		dy < .001f &&
		dz < .001f );
}

bool LoadWorldSpawn( Parser* parser, const char* output ) {
	parser->ReadToken();//220
	parser->ReadToken();//classname
	parser->ReadToken();//worldspawn
	parser->SkipUntilTokenOfType( TT_PUNCTUATION );

	std::vector<NPFace> faces;
	faces.reserve( 10000 );
	std::vector<DBrushVertex> vertices;
	vertices.reserve( 30000 );
	std::vector<NPBrush> brushes;
	brushes.reserve( 2000 );
	std::vector<u32> indices;
	indices.reserve( 45000 );
	std::vector<TexInfo> textures;
	textures.reserve( 512 );

	//FACE_VERT = position 
	//FACE_VERT_ = entier vertex. used when moving it's position around in arrays
#define FACEVERT(i) vertices[face->firstVertex+i].pos
#define FACEVERT_(i) vertices[face->firstVertex+i]

	//==========
	// Parse File
	//==========
	while ( 1 ) {
		NPBrush brush{};
		brush.firstPlane = faces.size();

		//New Brush
		if ( parser->PeekNext().subType == '}' ) {
			return true;
		}

		parser->ExpectedTokenTypePunctuation( '{' );

		while ( 1 ) {
			NPFace face{};
			Vec3 p[3]{};

			//Get plane information
			parser->ParseVec( &p[0][0], 3, true );
			parser->ParseVec( &p[1][0], 3, true );
			parser->ParseVec( &p[2][0], 3, true );

			Vec3 normal = glm::cross( p[2] - p[0], p[1] - p[0] );
			normal = glm::normalize( normal );
			float dist = -glm::dot( normal, p[0] );

			dist *= scale;

			if ( fixNormals )
				normal = Vec3( -normal.x, normal.z, normal.y );

			face.n = normal;
			face.d = dist;

			//Read name
			char texName[MAX_PATH_LENGTH]{};
			while ( parser->GetCurrent().subType != '[' ) {
				char data[MAX_PATH_LENGTH]{};
				parser->GetCurrent().ToString( data, 64 );
				strcat_s( texName, 64, data );
				parser->ReadToken();
			}

			//Get texture index
			u32 texIndex = U32MAX;

			for ( int i = 0; i < textures.size(); i++ ) {
				if ( !strcmp( texName, textures[i].name ) ) {
					texIndex = i;
					break;
				}
			}

			//Texture was not found
			if ( texIndex == U32MAX ) {
				//Try to load texture to get size
				char texPath[MAX_PATH_LENGTH * 2]{};
				strcpy_s( texPath, MAX_PATH_LENGTH * 2, "c:/workspace/cpp/realgame/realgame/res/textures/" );
				strcat_s( texPath, MAX_PATH_LENGTH * 2, texName );
				strcat_s( texPath, MAX_PATH_LENGTH * 2, ".png" );

				int x = 0, y = 0, c = 0;
				if ( !stbi_info( texPath, &x, &y, &c ) ) {
					printf( "[WARNING] Could not load texture %s\n", texPath );
					x = 512;
					y = 512;
				}
				//Create new Texture
				texIndex = textures.size();
				TexInfo info{};
				memset( info.name, 0, NAME_BUF_LEN );
				memcpy( info.name, texName, strlen( texName ) );
				info.size = IVec2( x, y );
				textures.push_back( info );
			}

			face.textureIndex = texIndex;

			//U,V,Offsets
			parser->ParseVec( &face.texU[0], 4, true );
			parser->ParseVec( &face.texV[0], 4, true );
			parser->ParseVec( &face.info[0], 3, false );

			brush.numFaces++;
			faces.push_back( face );

			if ( parser->GetCurrent().subType == '}' ) {
				parser->ReadToken();
				break;
			}
		}
		brushes.push_back( brush );
		if ( parser->GetCurrent().subType == '}' )
			break;
	}
	parser->ExpectedTokenTypePunctuation( '}' );
	// ======
	// Build Polygon Vertices
	// ======

	for ( int b = 0; b < brushes.size(); b++ ) {
		//Dont add any more brushes or ptr becomes invalid. Should NOT matter in this func
		NPBrush* brush = &brushes[b];
		brush->firstVertex = vertices.size();

		for ( int i = 0; i < brush->numFaces; i++ ) {
			NPFace* face = &faces[brush->firstPlane + i];
			face->firstVertex = vertices.size();

			for ( int k = 0; k < brush->numFaces; k++ ) {
				for ( int n = 0; n < brush->numFaces; n++ ) {
					if ( ( i == n ) || ( i == k ) || ( n == k ) ) continue;

					//Check if the 3 planes intesrect
					//if they intersect make sure they are all inside of the hulls of the planes
					//if so make sure we dont already have this vertex in the list
					dVec3 out( 0 );
#define BRUSHPLANE(brush,index) {faces[brush->firstPlane+index].n,faces[brush->firstPlane+index].d}
					DPlane a = BRUSHPLANE( brush, i );
					DPlane b = BRUSHPLANE( brush, k );
					DPlane c = BRUSHPLANE( brush, n );
					if ( !ThreePlaneIntersection( &a, &b, &c, &out ) ) continue;
					if ( !IsValid( out, brush, faces ) ) continue;

					//use the rounded out, quake1 only works on int and the intersection may have fp error
					dVec3 roundedOut = glm::round( out );
					if ( !IsUnique( face, out, vertices ) )continue;

					DBrushVertex vertex;
					vertex.pos = out;

					Vec2 texSize = textures[face->textureIndex].size;

					double u = ( glm::dot( dVec3( face->texU ), roundedOut ) ) / ( float ) texSize.x;
					u /= face->info.z;
					u += face->texU.w / ( float ) texSize.x;

					double v = ( glm::dot( dVec3( face->texV ), roundedOut ) ) / ( float ) texSize.y;
					v /= face->info.y;
					v += face->texV.w / ( float ) texSize.y;

					u /= scale;
					v /= scale;

					vertex.uv = Vec2( u, v );
					vertex.normal = face->n;
					vertices.push_back( vertex );

					face->numVertices++;
					brush->numVertices++;
				}
			}
		}
	}

	// ======
	// Sort Polygons
	// ======
	for ( int b = 0; b < brushes.size(); b++ ) {
		NPBrush* brush = &brushes[b];

		for ( int f = 0; f < brush->numFaces; f++ ) {
			NPFace* face = &faces[brush->firstPlane + f];

			dVec3 center( 0 );
			for ( int n = 0; n < face->numVertices; n++ )
				center += vertices[face->firstVertex + n].pos;
			center /= face->numVertices;

			for ( int n = 0; n < face->numVertices - 2; n++ ) {
				dVec3 a = glm::normalize( FACEVERT( n ) - center );

				DPlane p;
				dVec3 pa = FACEVERT( n );
				dVec3 pb = center;
				dVec3 pc = center + face->n;
				p.n = glm::cross( pc - pa, pb - pa );
				p.n = glm::normalize( p.n );
				p.d = glm::dot( pa, -p.n );

				double smallestAngle = -1;
				int smallestIndex = -1;

				for ( int m = n + 1; m < face->numVertices; m++ ) {
					if ( glm::dot( FACEVERT( m ), p.n ) + p.d < FEPSILON ) {
						dVec3 b = glm::normalize( FACEVERT( m ) - center );
						double angle = glm::dot( a, b );

						if ( angle > smallestAngle ) {
							smallestAngle = angle;
							smallestIndex = m;
						}
					}
				}
				if ( smallestIndex == -1 ) {
					printf( "Fatal Error: smallsetIndex -1 when sorting" );
					return false;
				}
				DBrushVertex temp = FACEVERT_( smallestIndex );
				FACEVERT_( smallestIndex ) = FACEVERT_( n + 1 );
				FACEVERT_( n + 1 ) = temp;
			}

			/// ==========
			//  Ensure Vertices are in the correct order
			/// ==========
			//Calculate DPlane
			DPlane oldPlane;
			oldPlane.n = face->n;
			oldPlane.d = face->d;

			DPlane plane = {};

			if ( face->numVertices < 3 ) {
				printf( "[FATAL ERROR] Face has bad amount of vertices: %u\n", face->numVertices );
				return false;
				assert( 0 );
			}

			dVec3 centerOfMass( 0 );
			double mag = 0;

			for ( int n = 0; n < face->numVertices; n++ ) {
				int j = n + 1;

				if ( j == face->numVertices )
					j = 0;
				plane.n.x += ( FACEVERT( n ).y - FACEVERT( j ).y ) * ( FACEVERT( n ).z + FACEVERT( j ).z );
				plane.n.y += ( FACEVERT( n ).z - FACEVERT( j ).z ) * ( FACEVERT( n ).x + FACEVERT( j ).x );
				plane.n.z += ( FACEVERT( n ).x - FACEVERT( j ).x ) * ( FACEVERT( n ).y + FACEVERT( j ).y );

				centerOfMass.x += FACEVERT( n ).x;
				centerOfMass.y += FACEVERT( n ).y;
				centerOfMass.z += FACEVERT( n ).z;
			}

			if ( ( fabs( plane.n.x ) < FEPSILON ) && ( fabs( plane.n.y ) < FEPSILON ) && ( fabs( plane.n.z ) < FEPSILON ) ) {
				printf( "Fatal Error: Assert" );
				assert( 0 );
			}

			mag = sqrt( plane.n.x * plane.n.x + plane.n.y * plane.n.y + plane.n.z * plane.n.z );

			if ( mag < FEPSILON ) {
				printf( "Fatal Error: Assert" );
				assert( 0 );
			}

			plane.n /= mag;
			centerOfMass /= face->numVertices;
			plane.d = -( glm::dot( centerOfMass, plane.n ) );
			//face->p.n = plane.n;
			//face->p.d = plane.d;

			//Reverse Vertice If Needed
			if ( glm::dot( plane.n, oldPlane.n ) < 0 ) {
				for ( int n = 0; n < face->numVertices / 2; n++ ) {
					DBrushVertex temp = FACEVERT_( n );
					FACEVERT_( n ) = FACEVERT_( face->numVertices - 1 - n );
					FACEVERT_( face->numVertices - 1 - n ) = temp;
				}
			}
		}
	}

	// ======
	// Indices
	// ======
	std::vector<u32> index2;
	for ( int b = 0; b < brushes.size(); b++ ) {
		NPBrush* brush = &brushes[b];
		brush->firstIndex = indices.size();
		brush->numIndices = 0;

		for ( int i = 0; i < brush->numFaces; i++ ) {
			NPFace* face = &faces[brush->firstPlane + i];
			face->firstIndex = indices.size();

			u32 triangleOrigin = face->firstVertex;
			u32 triangleHelper = face->firstVertex + 1;
			for ( int n = 2; n < face->numVertices; n++ ) {
				u32 finalVertex = face->firstVertex + n;

				//Create Triangle
				indices.push_back( triangleOrigin );
				indices.push_back( triangleHelper );
				indices.push_back( finalVertex );

				index2.push_back( triangleOrigin - brush->firstVertex );
				index2.push_back( triangleHelper - brush->firstVertex );
				index2.push_back( finalVertex - brush->firstVertex );


				triangleHelper = finalVertex;

				face->numIndices += 3;
				brush->numIndices += 3;
			}
		}
	}

	for ( int i = 0; i < faces.size(); i++ ) {
		NPFace* face = &faces[i];
		for ( int n = 0; n < face->numIndices; n += 3 ) {
			Vec3 A = vertices[indices[face->firstIndex + n + 0]].pos;
			Vec3 B = vertices[indices[face->firstIndex + n + 1]].pos;
			Vec3 C = vertices[indices[face->firstIndex + n + 2]].pos;

			Vec3 expectedNormal = face->n;
			Vec3 thisNormal = glm::normalize( glm::cross( B - A, C - A ) );
			if ( CloseEnough( thisNormal, expectedNormal ) ) {
			}
			else if ( CloseEnough( thisNormal, -expectedNormal ) ) {
				int t = indices[face->firstIndex + n];
				indices[face->firstIndex + n] = indices[face->firstIndex + n + 2];
				indices[face->firstIndex + n + 2] = t;

				Vec3 A = vertices[indices[face->firstIndex + n + 0]].pos;
				Vec3 B = vertices[indices[face->firstIndex + n + 1]].pos;
				Vec3 C = vertices[indices[face->firstIndex + n + 2]].pos;
				Vec3 v = glm::normalize( glm::cross( B - A, C - A ) );

				if ( !CloseEnough( v, expectedNormal ) ) {
					printf( "Flipped tri still bad!\n" );
					printf( "F.N: %.2f %.2f %.2f\n", face->n.x, face->n.y, face->n.z );
					printf( "Was: %.2f %.2f %.2f\n\n", v.x, v.y, v.z );
				}

			}
		}
	}

	// ========
	// MIN/MAX
	// ========
	for ( int b = 0; b < brushes.size(); b++ ) {
		NPBrush* brush = &brushes[b];

		Vec3 min( vertices[brush->firstVertex].pos );
		Vec3 max( vertices[brush->firstVertex].pos );
		for ( int i = 0; i < brush->numVertices; i++ ) {
			Vec3 vert = vertices[brush->firstVertex + i].pos;

			for ( int k = 0; k < 3; k++ ) {
				if ( vert[k] > max[k] )
					max[k] = vert[k];
				if ( vert[k] < min[k] )
					min[k] = vert[k];
			}
		}
		brush->bounds.min = min;
		brush->bounds.max = max;
	}

	u32 fileSize = 0;
	//=======================
	//       INIT RENDERER 
	//=======================
	u32* numTrianglesPerTexture = ( u32* ) malloc( textures.size() * sizeof( u32 ) );
	memset( numTrianglesPerTexture, 0, textures.size() * sizeof( u32 ) );

	RenderBrush* rBrushes = ( RenderBrush* ) malloc( brushes.size() * sizeof( RenderBrush ) );
	for ( int i = 0; i < brushes.size(); i++ ) {
		RenderBrush* brush = &rBrushes[i];
		brush->bounds = brushes[i].bounds;
		brush->firstFace = brushes[i].firstPlane;
		brush->firstIndex = brushes[i].firstIndex;
		brush->numFaces = brushes[i].numFaces;
		brush->numIndices = brushes[i].numIndices;
	}


	RenderBrushFace* rfaces = ( RenderBrushFace* ) malloc( faces.size() * sizeof( RenderBrushFace ) );

	for ( int i = 0; i < faces.size(); i++ ) {
		RenderBrushFace* face = &rfaces[i];
		rfaces[i].firstIndex = faces[i].firstIndex;
		rfaces[i].firstVertex = faces[i].firstVertex;
		rfaces[i].numVertices = faces[i].numVertices;
		rfaces[i].numIndices = faces[i].numIndices;
		rfaces[i].texture = ( Texture* ) faces[i].textureIndex;
		face->textureIndex = faces[i].textureIndex;
		numTrianglesPerTexture[faces[i].textureIndex] += faces[i].numIndices / 3;
	}

	DrawVertex* drawVertices = ( DrawVertex* ) malloc( vertices.size() * sizeof( DrawVertex ) );
	for ( int i = 0; i < vertices.size(); i++ ) {
		drawVertices[i].pos = vertices[i].pos;
		drawVertices[i].normal = vertices[i].normal;
		drawVertices[i].tex = vertices[i].uv;
	}

	u32 numVertices = vertices.size();
	u32 numIndices = indices.size();
	u32 numFaces = faces.size();
	u32 numBrushes = brushes.size();
	u32 numTextures = textures.size();



	FILE* out = 0;
	fopen_s( &out, output, "wb" );

	fwrite( &numVertices, sizeof( u32 ), 1, out );
	fwrite( &numIndices, sizeof( u32 ), 1, out );
	fwrite( &numFaces, sizeof( u32 ), 1, out );
	fwrite( &numBrushes, sizeof( u32 ), 1, out );
	fwrite( &numTextures, sizeof( u32 ), 1, out );

	fwrite( drawVertices, sizeof( DrawVertex ) * vertices.size(), 1, out );
	fwrite( indices.data(), sizeof( u32 ), indices.size(), out );
	fwrite( rfaces, sizeof( RenderBrushFace ) * faces.size(), 1, out );
	fwrite( rBrushes, sizeof( RenderBrush ) * brushes.size(), 1, out );

	//Write out all textures
	for ( int i = 0; i < numTextures; i++ )
		fwrite( textures[i].name, NAME_BUF_LEN, 1, out );
	fwrite( numTrianglesPerTexture, textures.size() * sizeof( u32 ), 1, out );

	//=======================
	//      INIT PHYSICS 
	//=======================

	u32 numTriangles = numIndices / 3;

	Vec3* outVertices = ( Vec3* ) malloc( sizeof( Vec3 ) * numVertices );
	Brush* outBrushes = ( Brush* ) malloc( sizeof( Brush ) * numBrushes );
	BrushTri* outTriangles = ( BrushTri* ) malloc( sizeof( BrushTri ) * numTriangles );
	Polygon* polygons = ( Polygon* ) malloc( sizeof( Polygon ) * numFaces );

	for ( int i = 0; i < numVertices; i++ )
		outVertices[i] = vertices[i].pos;

	int triangleOffset = 0;
	int fVertexOffset = 0;

	for ( int i = 0; i < numTriangles; i++ ) {
		outTriangles[i].v[0] = index2[( i * 3 ) + 0];
		outTriangles[i].v[1] = index2[( i * 3 ) + 1];
		outTriangles[i].v[2] = index2[( i * 3 ) + 2];
	}

	for ( int i = 0; i < numFaces; i++ ) {
		polygons[i].n = faces[i].n;
		polygons[i].d = faces[i].d;
		polygons[i].numTriangles = faces[i].numIndices / 3;
		polygons[i].triangles = ( BrushTri* ) triangleOffset;
		triangleOffset += polygons[i].numTriangles;
	}

	int vertexOffset = 0;
	int polygonOffset = 0;
	for ( int i = 0; i < numBrushes; i++ ) {
		outBrushes[i].polygons = ( Polygon* ) polygonOffset;
		outBrushes[i].vertices = ( Vec3* ) vertexOffset;
		outBrushes[i].numPolygons = brushes[i].numFaces;
		outBrushes[i].numVertices = brushes[i].numVertices;

		vertexOffset += outBrushes[i].numVertices;
		polygonOffset += outBrushes[i].numPolygons;
	}

	BVHTree tree = ConstructBVH( brushes );

	fwrite( &numBrushes, sizeof( u32 ), 1, out );
	fwrite( &numFaces, sizeof( u32 ), 1, out );
	fwrite( &numBrushes, sizeof( u32 ), 1, out );
	fwrite( &numTriangles, sizeof( u32 ), 1, out );
	fwrite( &numVertices, sizeof( u32 ), 1, out );

	fwrite( outBrushes, sizeof( Brush ) * numBrushes, 1, out );
	fwrite( polygons, sizeof( Polygon ) * numFaces, 1, out );
	fwrite( outTriangles, sizeof( BrushTri ) * numTriangles, 1, out );
	fwrite( outVertices, sizeof( Vec3 ) * numVertices, 1, out );

	fwrite( &tree, sizeof( BVHTree ), 1, out );
	fwrite( tree.nodes, sizeof( BVHNode )* tree.numNodes, 1, out );

	fclose( out );
	printf( "Map Compiled\n" );
	printf( "-----Stats--------\n" );
	printf( "Num Brushes: %u\n", numBrushes );
	printf( "Num Faces: %u\n", numFaces );
	printf( "Num Vertices: %u\n", numVertices );
	printf( "Num Indices: %u\n", numIndices );
	printf( "BVHNodes %u\n", tree.numNodes );

	return true;
}

/*	File Format
*
*		Renderer
* u32 NumVertices
* u32 NumIndices
* u32 NumFaces
* u32 NumBrushes
* u32 NumTextures
* DrawVertex[NumVertices]
* RenderBrushFace[NumFaces]
* u32 indices[NumIndices]
* char[MAX_NAME_LEN+4] Textures[NumTextures]
* u32[NumTextures] NumTrianglesPerTexture
* 
*
*						Physics
* u32 numBrushes
* u32 numFaces
* u32 numTriangles
* u32 numVertices
*
* Brush brushes[numBrushes]
* Polygon[numFaces]
* Triangles[numTriangles]
* Vec3 vertices[numVertices]
* 
* BVHTree	tree
* BVHNode	numNodes[tree.numNodes]
*/

float BoundsArea( const BoundsMinMax& A ) {
	Vec3 d = A.max - A.min;
	return 2.0f * ( d.x * d.y + d.y * d.z + d.z * d.x );
}

BoundsMinMax BoundsUnion( const BoundsMinMax& a, const BoundsMinMax& b ) {
	BoundsMinMax mm;
	mm.max = glm::max( a.max, b.max );
	mm.min = glm::min( a.min, b.min );
	return mm;
}

//THIS IS SO BAD LOL
void BVHRemove( BVHNode** arr, int* size, int index1, int index2 ) {
	if ( index1 > index2 ) {
		int temp = index1;
		index1 = index2;
		index2 = temp;
	}

	for ( int i = index1; i < *size - 1; i++ ) {
		arr[i] = arr[i + 1];
	}

	for ( int i = index2 - 1; i < *size - 2; i++ ) {
		arr[i] = arr[i + 1];
	}

	*size -= 2;
}

BVHTree ConstructBVH( std::vector<NPBrush>& brushes ) {
	printf( "Constructing BVH...\n" );
	BVHTree tree{};
	tree.numNodes = 2 * brushes.size();
	tree.nodes = ( BVHNode* ) malloc( tree.numNodes * sizeof( BVHNode ) );
	BVHNode** workingSet = ( BVHNode** ) malloc( tree.numNodes * sizeof( BVHNode* ) );
	int nodesCreated = 0;
	int numWorkingSet = 0;
	int numTops = 0;

	//Create a working set of all leaf nodes
	for ( int i = 0; i < brushes.size(); i++ ) {
		BVHNode* node = &tree.nodes[i];
		node->bounds = brushes[i].bounds;
		node->isLeaf = true;
		node->object = i;
		node->child1 = -1;
		node->child2 = -1;
		node->nodeIndex = i;
		node->parent = -1;

		workingSet[numWorkingSet++] = node;
	}
	nodesCreated = numWorkingSet;

	//Find the closest AABBS
	//For determining what should be grouped,
	//It will take the sum of the starting surface area of the two boxes
	//and the surface area of the union of the boxes and pick the one with the lowest amount
	//It will go through all the leaf nodes, then all the 2nd level nodes, then all the 3rd level nodes etc.
	//The remainders of each level (if any) are then grouped with the next highest node level
	while ( numWorkingSet > 1 ) {
		int bestA = -1, bestB = -1;
		int lowestDiffInSurfaceArea = 9999999.0f;
		//Find the best Bounds
		for ( int i = 0; i < numWorkingSet - 1; i++ ) {
			float surfaceAreaA = BoundsArea( workingSet[i]->bounds );
			for ( int n = i + 1; n < numWorkingSet; n++ ) {
				BoundsMinMax boxUnion = BoundsUnion( workingSet[i]->bounds, workingSet[n]->bounds );

				float surfaceAreaB = BoundsArea( workingSet[n]->bounds );
				float surfaceArenaUnion = BoundsArea( boxUnion );

				float diff = surfaceArenaUnion - ( surfaceAreaA + surfaceAreaB );
				if ( surfaceArenaUnion < lowestDiffInSurfaceArea ) {
					lowestDiffInSurfaceArea = diff;
					bestA = i;
					bestB = n;
				}
			}
		}

		//Remove the two root nodes and add a new Node
		assert( bestA != -1 );
		assert( bestB != -1 );
		BVHNode* node = &tree.nodes[nodesCreated];
		node->nodeIndex = nodesCreated++;
		node->bounds = BoundsUnion( workingSet[bestA]->bounds, workingSet[bestB]->bounds );//todo Faster to recalc or not?
		node->child1 = workingSet[bestA]->nodeIndex;
		node->child2 = workingSet[bestB]->nodeIndex;
		node->parent = -1;

		workingSet[bestA]->parent = node->nodeIndex;
		workingSet[bestB]->parent = node->nodeIndex;

		if ( bestA > bestB ) {
			int temp = bestA;
			bestA = bestB;
			bestB = temp;
		}

		//TODO Do Something faster
		BVHRemove( workingSet, &numWorkingSet, bestA, bestB );

		//Add the higher level node onto the end, we dont want to use it in this loop
		workingSet[tree.numNodes - 1 - numTops] = node;
		numTops++;

		//If the loop is going to break, we should move the topnodes back
		if ( numWorkingSet > 1 ) continue;

		//Copy them to start of array (TODO just memcpy)
		for ( int n = 0; n < numTops; n++ ) {
			workingSet[numWorkingSet++] = workingSet[tree.numNodes - numTops + n];
		}
		numTops = 0;
	}

	tree.root = workingSet[0]->nodeIndex;
	free( workingSet );
	return tree;
}

