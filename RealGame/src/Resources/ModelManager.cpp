#include <string> //Only used for names from the JSON

#include "ModelManager.h"
#include "TextureManager.h"

void CreateModelManager( ModelManager* manager, u32 memorySize, void* memory, u32 animationSize, void* animMemory ) {
	u8* mmemory = (u8*) memory;
	u32 a = MODEL_MANAGER_SIZE;
	mmemory += MODEL_MANAGER_SIZE;
	*mmemory++ = '1';
	*mmemory++ = '2';
	*mmemory++ = '3';
	CreatePoolArena( &manager->modelArena, MAX_MODEL_SIZE, MAX_MODELS, memory, &globalArena, "Model Manager");

	CreatePoolArena( &manager->animationArena, MAX_ANIMATION_SIZE, MAX_ANIMATIONS, animMemory, &globalArena, "AnimationManager" );
}

struct GLTFHeader {
	u32 magic;
	u32 version;
	u32 length;
};

enum GLTF_COMPONENT_TYPE {
	GLTF_BYTE = 5120,
	GLTF_UBYTE = 5121,
	GLTF_SHORT = 5122,
	GLTF_USHORT = 5123,
	GLTF_UINT = 5125,
	GLTF_FLOAT = 5126
};

enum GLTF_STRUCT_TYPE {
	GLTF_VEC4,
	GLTF_VEC3,
	GLTF_VEC2,
	GLTF_SCALAR,
	GLTF_MAT4,
};

struct Accessor {
	bool inUse;
	u32 bufferView;
	u32 bufferViewOffset;
	u32 bufferViewLength;
	u32 byteOffset;
	u32 count;
	GLTF_COMPONENT_TYPE componentType;
	GLTF_STRUCT_TYPE type;
};

//U32MAX for any value means it does not exist
struct GLTFMesh {
	char name[MAX_NAME_LENGTH];
	Accessor positionAccessor;
	Accessor normalAccessor;
	Accessor tangentAccessor;
	Accessor texcoordAccessor;
	Accessor jointAccessor;
	Accessor weightAccessor;
	Accessor indicesAccessor;
	u32 material;
};

static inline void FillBuffer( NFile* file, u8** dst, u32 binOffset, Accessor* accessor ) {
	NFileSetSeek( file, accessor->bufferViewOffset + accessor->byteOffset + binOffset );
	NFileRead( file, *dst, accessor->bufferViewLength );
	//memcpy( *dst, ( src + accessor->bufferViewOffset + accessor->byteOffset ), accessor->bufferViewLength );
}

static inline void TempAllocAndFillBuffer( NFile* file, u8** dst, u32 binOffset, Accessor* accessor ) {
	*dst = (u8*) StackArenaAllocate( &tempArena, accessor->bufferViewLength );
	FillBuffer( file,dst,binOffset,accessor );
}

static inline GLTF_STRUCT_TYPE AccessorStringToEnum( std::string str ) {
	if ( str == "SCALAR" )
		return GLTF_SCALAR;
	else if ( str == "VEC2" )
		return GLTF_VEC2;
	else if ( str == "VEC3" )
		return GLTF_VEC3;
	else if ( str == "VEC4" )
		return GLTF_VEC4;
	else if ( str == "MAT4" )
		return GLTF_MAT4;
	else {
		printf( "unkown accesor type %s\n", str.c_str() );
	}
}

static inline void FillOutAccessor( JSON& basejson, u32 accessorIdx, Accessor* accessor ) {
	JSON jAccessor = basejson["accessors"][accessorIdx];
	accessor->bufferView = jAccessor["bufferView"];
	accessor->componentType = jAccessor["componentType"];
	accessor->count = jAccessor["count"];
	accessor->type = AccessorStringToEnum(jAccessor["type"]);

	if ( jAccessor.find( "byteOffset" ) != jAccessor.end() )
		accessor->byteOffset = jAccessor["byteOffset"];
	else
		accessor->byteOffset = 0;

	JSON jBuffer = basejson["bufferViews"][accessor->bufferView];
	accessor->bufferViewOffset = jBuffer["byteOffset"];
	accessor->bufferViewLength = jBuffer["byteLength"];
	accessor->inUse = true;
}

struct Sampler {
	Accessor input;
	Accessor output;
	//interp should always be LINEAR
};

static inline void ReadSampler( JSON& baseJson, JSON& animJson, Sampler* sampler, int samplerIndex ) {
	int input = animJson["samplers"][samplerIndex]["input"];
	int output = animJson["samplers"][samplerIndex]["output"];

	FillOutAccessor( baseJson, input, &sampler->input );
	FillOutAccessor( baseJson, output, &sampler->output );
}

Model* ModelManagerAllocate( ModelManager* manager, const char* path ) {
	TEMP_ARENA_SET

	NFile file;
	if ( !CreateNFile( &file, path, "rb" ) ) {
		LOG_ERROR( LGS_IO, "Could not open model %s\n", path );
		StackArenaFreeToPrevious( &tempArena );
		return 0;
	}

	ModelInfo* info = ( ModelInfo* ) PoolArenaAllocate( &manager->modelArena );
	Model* model = &info->model;
	memset( model, 0, sizeof( Model ) );
	//Allocate enough memory that the model only requires 1 allocation to handle everything including joints
	CreateScratchArena( &info->arena, MAX_MODEL_SIZE - sizeof( ModelInfo ), ( u8* ) info + sizeof( ModelInfo ), &manager->modelArena, path );
	strcpy_s( model->path, MAX_PATH_LENGTH, path );

	// ==========================================
	//				Load GLTF Model
	// ==========================================
	GLTFHeader header;
	NFileRead( &file, &header, sizeof(header));

	if ( header.magic != 0x46546C67 || header.version != 2) {
		LOG_ASSERT( LGS_RENDERER, "Bad glb file %s\n", path );
	}

	//Load in JSON
	u32 jsonChunkLength = NFileReadU32( &file );
	u32 chunkType = NFileReadU32( &file );
	
	u8* data = (u8*) StackArenaAllocate( &tempArena, jsonChunkLength + 1 );
	data[jsonChunkLength] = '\0';
	NFileRead( &file, data, jsonChunkLength );

	JSON json = JSON::parse( ( const char* ) data );
	//Load in bin
	u32 binLength = NFileReadU32( &file );
	u32 binType = NFileReadU32( &file );
	assert( binType == 0x004E4942 ); //Make sure this chunk is actually the bin
	u32 binOffset = NFileGetPos(&file);

	//Make sure GLTF valid here
	if ( json["scenes"].size() > 1 )
		LOG_WARNING( LGS_RENDERER, "json %s contains more than 1 scene\n",path );
	if ( json["scenes"][0]["nodes"].size() > 1 )
		LOG_WARNING( LGS_RENDERER, "json %s contains more than 1 root node\n",path );
	JSON& scene = json["scenes"][0];

	//Load Materials
	u32 numTextures = 0;
	if ( json.find( "images" ) != json.end() )
		numTextures = json["images"].size();

	Texture** textures = (Texture**) StackArenaAllocate( &tempArena, numTextures * sizeof( Texture* ) );

	for ( int i = 0; i < numTextures; i++ ) {
		TEMP_ARENA_SET

		JSON& jtexture = json["images"][i];
		std::string path = jtexture["name"];
		u32 bufferViewIndex = jtexture["bufferView"];
		
		JSON& bufferView = json["bufferViews"][bufferViewIndex];
		u32 bufferViewOffset = bufferView["byteOffset"];
		u32 bufferViewLength = bufferView["byteLength"];

		u8* memory = (u8*) StackArenaAllocate( &tempArena, bufferViewLength );
		NFileSetSeek( &file, bufferViewOffset + binOffset );
		NFileRead( &file, memory, bufferViewLength );

		textures[i] = TextureManagerLoadTextureFromMemory( memory, bufferViewLength, path.c_str() );
		
	}

	// Load the skin if the model has one
 	Skeleton* skeleton = 0;
	if ( json.find( "skins" ) != json.end() ) {
		if ( json["skins"].size() > 1 )
			LOG_WARNING( LGS_RENDERER, "More than 1 skin in %s\n", path );

		skeleton = (Skeleton*) ScratchArenaAllocateZero( &info->arena, sizeof( Skeleton ) );
		model->skeleton = skeleton;

		JSON& skin = json["skins"][0];
		if ( skin.find( "name" ) != skin.end() ) {
			std::string name = skin["name"];
			strcpy_s( skeleton->name, MAX_NAME_LENGTH, name.c_str() );
		}
		
		//Get inverse bind matrices
		skeleton->numBones = skin["joints"].size();
		skeleton->numNodes = json["nodes"].size();
		skeleton->root = scene["nodes"][0];
		
		if ( skeleton->numBones > 100 ) {
			LOG_ASSERT( LGS_RENDERER, "Too many bones with %d %s\n", skeleton->numBones, path );
		}

		//Load in inverse bind matrices
		Accessor invBindAccessor{};
		FillOutAccessor( json, skin["inverseBindMatrices"], &invBindAccessor );
		skeleton->inverseBinds = ( Mat4* ) ScratchArenaAllocate( &info->arena, invBindAccessor.bufferViewLength );
		FillBuffer( &file, ( u8** ) &skeleton->inverseBinds, binOffset, &invBindAccessor );

		skeleton->joints = ( Node* ) ScratchArenaAllocateZero( &info->arena, skeleton->numNodes * sizeof( Node ) );
		for ( int i = 0; i < skeleton->numNodes; i++ ) {
			JSON& jnode = json["nodes"][i];
			Node* joint = &skeleton->joints[i];

			joint->index = i;

			joint->boneID = -1; //if this is a bone, it will get set soon
			joint->numChildren = 0;

			if ( jnode.find( "name" ) != jnode.end() ) {
				std::string name = jnode["name"];
				assert( name.length() <= 32 );

				strcpy_s( joint->name, MAX_NAME_LENGTH, name.c_str() );
			}

			joint->s = Vec3( 1 );

			if ( jnode.find( "translation" ) != jnode.end() ) {
				JSON& t = jnode["translation"];
				joint->t = Vec3( t[0], t[1], t[2] );
			}
			else
				joint->t = Vec3( 0.0f );

			if ( jnode.find( "rotation" ) != jnode.end() ) {
				JSON& r = jnode["rotation"];
				joint->r = Quat( r[3], r[0], r[1], r[2]);
			}
			else
				joint->r = Quat( 1.0f, 0.0f, 0.0f, 0.0f );

			if ( jnode.find( "scale" ) != jnode.end() ) {
				JSON& s = jnode["scale"];
				joint->s = Vec3( s[0], s[1], s[2] );
			}
			else
				joint->s = Vec3( 1 );

			if ( jnode.find( "children" ) != jnode.end() ) {
				JSON& children = jnode["children"];

				joint->numChildren = children.size();
				joint->children = ( Node** ) ScratchArenaAllocate( &info->arena, joint->numChildren * sizeof( void* ) );
			
				for ( int n = 0; n < joint->numChildren; n++ ) {
					int child = children[n];
					joint->children[n] = &skeleton->joints[ child ];
				}
			}
		}

		//Load in bone indices
		skeleton->bones = ( Node** ) ScratchArenaAllocateZero( &info->arena, skeleton->numBones * sizeof( *skeleton->bones ) );
		for ( int i = 0; i < skeleton->numBones; i++ ) {
			int index = skin["joints"][i];
			skeleton->joints[index].boneID = i;
			//Put it in it's own array for easy lookups
			skeleton->bones[i] = &skeleton->joints[index];

		}
	}

	//Load in meshes
	u32 numMeshesItterate = json["meshes"].size(); 
	//A mesh can have multiple primitives, for this engine's sake each primitive will be a mesh
	model->numMeshes = 0;
	//Figure out how many total meshes there are. 

	for ( int i = 0; i < numMeshesItterate; i++ )
		model->numMeshes += json["meshes"][i]["primitives"].size();

	model->meshes = (Mesh*) ScratchArenaAllocate( &info->arena, model->numMeshes * sizeof( Mesh ) );
	
	//This just contains the accessor indices. Do this now and get the actual real data after
	GLTFMesh* tempMeshes = (GLTFMesh*) StackArenaAllocate( &tempArena, model->numMeshes * sizeof( GLTFMesh ) );
	memset( tempMeshes, 0, model->numMeshes * sizeof( GLTFMesh ) );

	u32 currentMesh = 0;
	for ( int i = 0; i < numMeshesItterate; i++ ) {
		std::string name = "";
		if ( json["meshes"][i].find( "name" ) != json["meshes"][i].end() )
			name = json["meshes"][i]["name"];
			u32 nameLen = name.length();
			if ( nameLen > MAX_NAME_LENGTH ) 
				nameLen = MAX_NAME_LENGTH;

		for ( int n = 0; n < json["meshes"][i]["primitives"].size(); n++ ) {
			JSON& prim = json["meshes"][i]["primitives"][n];
			GLTFMesh* mesh = &tempMeshes[currentMesh++];

			memset( mesh->name, 0, MAX_NAME_LENGTH );
			memcpy( mesh->name, name.c_str(), nameLen );

			//Get attribute data
			FillOutAccessor( json, prim["indices"], &mesh->indicesAccessor );
			
			JSON& attributes = prim["attributes"];
			if ( attributes.find( "POSITION" ) != attributes.end() )
				FillOutAccessor( json, attributes["POSITION"], &mesh->positionAccessor );
			if ( attributes.find( "NORMAL" ) != attributes.end() )
				FillOutAccessor( json, attributes["NORMAL"], &mesh->normalAccessor );
			if ( attributes.find( "TANGENT" ) != attributes.end() )
				FillOutAccessor( json, attributes["TANGENT"], &mesh->tangentAccessor );
			if ( attributes.find( "TEXCOORD_0" ) != attributes.end() )
				FillOutAccessor( json, attributes["TEXCOORD_0"], &mesh->texcoordAccessor );
			if ( attributes.find( "JOINTS_0" ) != attributes.end() )
				FillOutAccessor( json, attributes["JOINTS_0"], &mesh->jointAccessor );
			if ( attributes.find( "WEIGHTS_0" ) != attributes.end() )
				FillOutAccessor( json, attributes["WEIGHTS_0"], &mesh->weightAccessor );
		}
	}
	//Take accessor data and turn it into actual meshes
	//TODO turn this into 1 big GLBuffer in model, then store the start & length of each mesh
	for ( int i = 0; i < model->numMeshes; i++ ) {
		GLTFMesh* gltfMesh = &tempMeshes[i];
		Mesh* mesh = &model->meshes[i];

		strcpy_s( mesh->name, MAX_NAME_LENGTH, gltfMesh->name );

		TEMP_ARENA_SET

		//Find the base color texture in the material json.
		//Note: If normalmap/metallicmap are used this is where you would do it
		//todo Maybe load in the texture here?
		mesh->texture = 0;
		bool successlyFoundTexture = false;
		if ( gltfMesh->material != U32MAX ) {
			JSON& jmat = json["materials"][gltfMesh->material];
			if ( jmat.find( "pbrMetallicRoughness" ) != jmat.end() ) {
				JSON& pbr = jmat["pbrMetallicRoughness"];
				if ( pbr.find( "baseColorTexture" ) != pbr.end() ) {
					mesh->texture = textures[pbr["baseColorTexture"]["index"]];
					successlyFoundTexture = true;
				}
			}
		}
		//Make sure the material we found actually has a texture on it.
		if ( !successlyFoundTexture )
			gltfMesh->material = U32MAX;
			
		Vec3* positions = 0;
		Vec3* normals = 0;
		Vec4* tangents = 0;
		Vec2* texCoords = 0;
		u8* joints = 0;
		Vec4* weights = 0;

		//This will have to be done separately because indices can be either u16 or u32
		u8* indicesTmp = 0;
		TempAllocAndFillBuffer( &file, &indicesTmp, binOffset, &gltfMesh->indicesAccessor );

		if ( gltfMesh->positionAccessor.inUse ) 
			TempAllocAndFillBuffer( &file,( u8** ) &positions, binOffset, &gltfMesh->positionAccessor );
		if ( gltfMesh->normalAccessor.inUse )
			TempAllocAndFillBuffer( &file, ( u8** ) &normals, binOffset, &gltfMesh->normalAccessor );
		if ( gltfMesh->tangentAccessor.inUse )
			TempAllocAndFillBuffer( &file, ( u8** ) &tangents, binOffset, &gltfMesh->tangentAccessor );
		if ( gltfMesh->texcoordAccessor.inUse )
			TempAllocAndFillBuffer( &file, ( u8** ) &texCoords, binOffset, &gltfMesh->texcoordAccessor );
		if ( gltfMesh->jointAccessor.inUse )
			TempAllocAndFillBuffer( &file, ( u8** ) &joints, binOffset, &gltfMesh->jointAccessor );
		if ( gltfMesh->weightAccessor.inUse )
			TempAllocAndFillBuffer( &file, ( u8** ) &weights, binOffset, &gltfMesh->weightAccessor );

		//Now fill out a struct of vertices
		mesh->numVertices = gltfMesh->positionAccessor.count;
		mesh->numIndices = gltfMesh->indicesAccessor.count;

		if ( gltfMesh->material != U32MAX )
			mesh->texture = textures[gltfMesh->material];
		else
			mesh->texture = 0;

		DrawVertex* vertices = (DrawVertex*) StackArenaAllocate( &tempArena, mesh->numVertices * sizeof( vertices[0] ) );
		u32* indices = ( u32* ) StackArenaAllocate( &tempArena, gltfMesh->indicesAccessor.count * sizeof( u32 ) );

		if ( positions )
			for ( int n = 0; n < mesh->numVertices; n++ )
				vertices[n].pos = positions[n];
		if ( normals )
			for ( int n = 0; n < mesh->numVertices; n++ ) {
				vertices[n].normal = normals[n];
			}
		if ( texCoords )
			for ( int n = 0; n < mesh->numVertices; n++ )
				vertices[n].tex = texCoords[n];

		if ( tangents )
			for ( int n = 0; n < mesh->numVertices; n++ )
				vertices[n].tangents = tangents[n];

		if ( weights )
			for ( int n = 0; n < mesh->numVertices; n++ )
				vertices[n].weights = weights[n];

		if ( joints ) {
			assert( gltfMesh->jointAccessor.componentType == GLTF_UBYTE );
			for ( int n = 0; n < mesh->numVertices; n++ )
				for ( int k = 0; k < 4; k++ )
					vertices[n].bones[k] = (int) joints[n * 4 + k];
		}


		//Indices can be different types, but opengl will ALWAYS take U32
		if ( gltfMesh->indicesAccessor.componentType == GLTF_USHORT ) {
			//Cast loaded indices to u16 (what they loaded in as) and export them as u32
			u16* indicesCasted = ( u16* ) indicesTmp;
			for ( int n = 0; n < mesh->numIndices; n++ ) {
				indices[n] = *indicesCasted;
				indicesCasted++;
			}
		}
		else if ( gltfMesh->indicesAccessor.componentType == GLTF_UINT ) {
			//Just move the pointer, it doesn't matter that we allocated indices already, its a scratchalloc
			indices = (u32*) indicesTmp;
		}
		else {
			LOG_ASSERT( LGS_MEMORY, "Unkown type of indices accessor %d\n", gltfMesh->indicesAccessor.type);
		}

		CreateGLBuffer( &mesh->buffer,
			mesh->numVertices, mesh->numIndices,
			mesh->numVertices * sizeof( vertices[0] ), vertices,
			mesh->numIndices * sizeof( u32 ), indices,
			true, true );
		GLBufferAddDefaultAttribs( &mesh->buffer );
		if ( model->skeleton != 0 )
			GLBufferAddDefaultSkinnedAttribs( &mesh->buffer );

	}

	//Add to model List info
	info->next = manager->modelHead;
	manager->modelHead = info;

	// ============
	// Animations
	// ============
	if ( json["animations"].size() == 0 ) {
		StackArenaFreeToPrevious( &tempArena );
		return model;
	}

	model->numAnimations = json["animations"].size();
	
	model->animations = ( AnimationClip** ) ScratchArenaAllocate( &info->arena, model->numAnimations * sizeof( AnimationClip* ) );

	for ( int i = 0; i < model->numAnimations; i++ ) {
		AnimationClipInfo* animationInfo = (AnimationClipInfo*) PoolArenaAllocate( &manager->animationArena );
		AnimationClip* animation = &animationInfo->animation;
		CreateScratchArena( &animationInfo->arena, MAX_ANIMATION_SIZE - sizeof( AnimationClipInfo ), ( u8* ) animationInfo + sizeof( AnimationClipInfo ), &manager->animationArena, path );

		animationInfo->next = manager->animHead;
		manager->animHead = animationInfo;

		model->animations[i] = animation;

		assert( model->skeleton );
		animation->skeleton = model->skeleton;

		if ( !animation ) {
			LOG_ASSERT( LGS_RENDERER, "OUT OF ANIMATIONS %s\n", path );
			return 0;
		}

		JSON& animJson = json["animations"][i];

		//Name
		if ( animJson.find( "name" ) != animJson.end() ) {
			std::string name = animJson["name"];
			if ( name.length() >= MAX_NAME_LENGTH )
				name[MAX_NAME_LENGTH-1] = '\0';

			strcpy_s( animation->name, MAX_NAME_LENGTH, name.c_str() );
			strcpy_s( animationInfo->arena.name, MAX_NAME_LENGTH, name.c_str() );
		}

		//Channels
		int numChannels = animJson["channels"].size();
		char targetPath[32];

		animation->numKeyframes = model->skeleton->numNodes;
		animation->jointKeyFrames = ( JointKeyFrames* ) ScratchArenaAllocateZero( &animationInfo->arena, animation->numKeyframes * sizeof( *animation->jointKeyFrames ) );

		float longestDuration = 0.0f;

		for ( int i = 0; i < numChannels; i++ ) {
			JSON& channelJson = animJson["channels"][i];
			int target = channelJson["target"]["node"];
			std::string path = channelJson["target"]["path"];
			Sampler sampler{};
			ReadSampler( json, animJson, &sampler, channelJson["sampler"] );

			JointKeyFrames* frames = &animation->jointKeyFrames[target];

			assert( sampler.input.count == sampler.output.count );

			float** inputBuffer = 0;
			void** outputBuffer = 0;

			if ( path == "translation" ) {
				frames->numPosKeys = sampler.input.count;
				inputBuffer = ( float** ) &frames->posTimes;
				outputBuffer = ( void** ) &frames->posKeys;
			}
			else if ( path == "rotation" ) {
				frames->numRotKeys = sampler.input.count;
				inputBuffer = (float** ) & frames->rotTimes;
				outputBuffer = ( void** ) &frames->rotKeys;
			}
			else if ( path == "scale" ) {
				frames->numScaleKeys = sampler.input.count;
				inputBuffer = ( float** ) &frames->scaleTimes;
				outputBuffer = ( void** ) &frames->scaleKeys;
			}

			assert( inputBuffer && outputBuffer );

			*inputBuffer = ( float* ) ScratchArenaAllocate( &animationInfo->arena, sampler.input.bufferViewLength );
			FillBuffer( &file, ( u8** ) inputBuffer, binOffset, &sampler.input );

			*outputBuffer = ScratchArenaAllocate( &animationInfo->arena, sampler.output.bufferViewLength );
			FillBuffer( &file, ( u8** ) outputBuffer, binOffset, &sampler.output);

			if ( path == "rotation" ) {
				Quat* q = ( Quat* ) *outputBuffer;
				for ( int i = 0 ; i < sampler.output.count; i++ ){
					q[i] = Quat( q[i][3], q[i][0], q[i][1], q[i][2] );
				}
			}

			float lastKeyframeTime = ( ( float* ) ( *inputBuffer ) )[sampler.input.count-1];
			if ( lastKeyframeTime > longestDuration )
				longestDuration = lastKeyframeTime;
		}
		animation->duration = longestDuration;
		animation->looping = false;
	}

	return model;
}

//todo better solution
Model* ModelManagerGetModel( const char* path ) {
	for ( ModelInfo* model = modelManager.modelHead; model != 0; model = model->next ) {
		if ( strcmp( path, model->model.path ) == 0 )
			return &model->model;

		if ( model == 0 ) {
			LOG_WARNING( LGS_RENDERER, "COULD NOT FIND MODEL %s\n", path );
			return 0 ;
		}
	}

	//unreachable but whatever
	return 0;
}