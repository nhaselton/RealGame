#pragma once
#include "def.h"
#include "GLBuffer.h"
#include "Camera.h"
#include "Physics\Colliders.h"
#include "Resources\TextureManager.h"

#define MAX_FRAME_INFOS 1000
#define FONT_BATCH_SIZE 8192
#define MAX_PARTICLES 32768 //1MB of particles. Must change compute if this changes
#define MAX_PARTICLE_EMITTERS 64
#define PARTICLE_SIZE_GPU 64 //pos,vel,lifetime,active?

struct FontVert {
	Vec2 pos;
	Vec2 tex;
};

struct Skybox {
	u32 textureID;
	Texture faces[6];
	GLBuffer buffer;
};

enum builtInShaderList {
	SHADER_XYZRGB,
	SHADER_XYZRGB_SKINNED,
	SHADER_LINE_SHADER,
	SHADER_STANDARD,
	SHADER_STANDARD_SKINNED,
	SHADER_UI,
	SHADER_SKYBOX,
	SHADER_PARTICLES,
	
	SHADER_COMP_CREATE_PARTICLES,
	SHADER_COMP_UPDATE_PARTICLES,
	SHADER_LAST,
};

//Must be sets of Vec4s
struct ParticleEmitter {
	Vec3 pos;
	float speed;

	Vec3 color;
	float colorRange; //color = random(color-range,color+range)

	float currentTime;
	float lifeTime;
	float pad, pad2;
};

struct TextureChain {
	Texture* texture;
	u32* firstIndexOfTriangles;
	u32 numTriangles;
};

//These should always be bound to these values for consistency
enum samplerList {
	S2D_ALBEDO = 0,
	S3D_SKYBOX = 15,
	S2D_LAST
};

struct BitmapGlyph {
	int ascii;
	int x;
	int y;
	int width;
	int height;
	int xoffset;
	int yoffset;
	int xadvance;
	Vec2 tMin;
	Vec2 tMax;
};

struct BitmapFont {
	BitmapGlyph* glyphs;
	int numGlyphs;
	int imageSizeX;
	int imageSizeY;
	int lineHeight;
	int glyphSize;
	int base;
};

struct RenderBrushFace {
	u32 firstVertex;
	u32 firstIndex;
	//These are kept small becuase there are lots of faces in the maps. It adds up quickly
	u8 numVertices;
	u8 numIndices;
	u16 textureIndex;
};

struct RenderBrush {
	int firstFace;
	int numFaces;

	int firstIndex;
	int numIndices;

	BoundsMinMax bounds;
};

struct BrushVertex {
	Vec3 pos;
	Vec3 normal;
	Vec2 uv;
};

struct LevelRenderInfo {
	u32 numBrushes;
	u32 numVertices;
	u32 numIndices;
	u32 numFaces;
	u32 numTextures;

	GLBuffer buffer;
	struct RenderBrushFace* faces;
	struct RenderBrush* brushes;
	TextureChain* textureChains;
	//Need this for batching textures
	u32* indices;
};

struct FrameInfo {
	u32 drawArrayCalls;
	u32 drawArrayTriangleCount;

	u32 drawElementCalls;
	u32 drawElementTriangleCount;

	u32 activeTextureCalls;

	u32 bindTextureCalls;
	u32 bufferSubDataCalls;

	u32 bufferSubdataBytes;
	u32 bindVaoCalls;

	u32 shaderBinds;
	u32 shaderArgsSet;
};

class Renderer {
public:
	ScratchArena arena;

	struct Shader* shaders[SHADER_LAST];

	Camera camera;

	Mat4 projection;
	Mat4 orthographic;
	Mat4 mat4Array[100];

	LevelRenderInfo levelInfo;

	class Model* cube;
	class Model* sphere;
	u32 currentShaderID;

	Texture* crosshairTex;
	Texture* healthTex;
	Texture* fontTex;
	Texture* whiteNoiseTex;
	Texture* blankTexture;

	GLBuffer quadBuffer;
	BitmapFont font;

	Skybox skybox;

	bool drawStats;
	FrameInfo frameInfos[MAX_FRAME_INFOS];
	int currentFrameInfo;
	
	u32 particleSSBO;
	u32 particleEmitterSSBO;
	//Note: When uploading particle emiiters, dont forget the sizeof(Vec4) offset for count.
	ParticleEmitter particleEmitters[MAX_PARTICLE_EMITTERS];
	GLBuffer particleBuffer;

	//Font
	GLBuffer fontBuffer;
	FontVert glyphs[FONT_BATCH_SIZE * 4];
	u32 numGlyphsBatched;
};
extern Renderer renderer;

void CreateRenderer( Renderer* renderer, void* memory, u32 size );
void RenderStartFrame( Renderer* renderer );
void RenderDrawFrame( Renderer* renderer, float dt );
void RenderEndFrame( Renderer* renderer );

void RenderSetShader( Renderer* renderer, class Shader* newShader );
void RenderDrawModel(Renderer* renderer, class Model* model, Mat4 offset = Mat4(1.0), struct SkeletonPose* pose = 0 );
void RenderDrawLevel( Renderer* renderer );

void RenderDrawEntity( class Entity* entity );
void RenderDrawAllEntities();
void RenderDrawAllProjectiles();
void RenderDrawAllRigidBodies();
void RenderDrawGun();

void RenderDrawFontBatch();
void RenderDrawText( Vec2 pos, float fontSize, const char* string );
void RenderDrawChar( Vec2 pos, BitmapGlyph* glyph, float fontSize );

void RenderDrawHealthBar( Vec2 pos, Vec2 size, int hp, int maxHp );
void RenderDrawQuadColored( Vec2 pos, Vec2 size, Vec3 color ); 
void RenderDrawQuadTextured( Vec2 pos, Vec2 size, struct Texture* texture );

void RenderLoadLevel( class Level* level, class NFile* file );

void RenderInitFont();
void RenderLoadFontFromFile();

ParticleEmitter* NewParticleEmitter();

typedef unsigned int GLenum;
typedef int GLint;
typedef unsigned int GLuint;
typedef signed   long long int GLsizeiptr;
typedef int GLsizei;
typedef signed   long long int GLintptr;

void nglDrawArrays( GLenum mode, GLint first, GLsizei count );
void nglDrawElements( GLenum mode, GLsizei count, GLenum type, const void* indices );
void nglBindVertexArray( GLuint array );
void nglBindTexture( GLenum target, GLuint texture );
void nglBindBuffer( GLenum target, GLuint buffer );
void nglBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void* data );
void nglActiveTexture( GLenum texture );
