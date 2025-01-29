#pragma once
#include "def.h"
#include "GLBuffer.h"
#include "Camera.h"
#include "Physics\Colliders.h"
#include "Resources\TextureManager.h"

#define FONT_BATCH_SIZE 8192

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
	SHADER_LAST
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
	u32 numVertices;
	u32 firstIndex;
	u32 numIndices;
	struct Texture* texture;
	//todo tex,etc.
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

	GLBuffer buffer;
	struct RenderBrushFace* faces;
	struct RenderBrush* brushes;
};

class Renderer {
public:
	ScratchArena arena;

	struct Shader* shaders[SHADER_LAST];

	Camera camera;

	Mat4 mat4Array[100];
	Mat4 projection;
	Mat4 orthographic;

	LevelRenderInfo levelInfo;

	class Model* cube;
	class Model* sphere;
	u32 currentShaderID;

	Texture* crosshairTex;
	Texture* healthTex;
	Texture* fontTex;

	GLBuffer quadBuffer;
	BitmapFont font;

	Skybox skybox;

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

void RenderDrawFontBatch();
void RenderDrawText( Vec2 pos, float fontSize, const char* string );
void RenderDrawChar( Vec2 pos, BitmapGlyph* glyph, float fontSize );

void RenderDrawHealthBar( Vec2 pos, Vec2 size, int hp, int maxHp );
void RenderDrawQuadColored( Vec2 pos, Vec2 size, Vec3 color ); 
void RenderDrawQuadTextured( Vec2 pos, Vec2 size, struct Texture* texture );

void RenderLoadLevel( class Level* level, class NFile* file );

void RenderInitFont();
void RenderLoadFontFromFile();