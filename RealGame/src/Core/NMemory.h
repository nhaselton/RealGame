#include "coredef.h"

/*
		How Memory Works:
The game does 1 big allocation at the start with a scratch arena. This memory is divided into all the different subsystems
	These subsystems can then divide up the memory as needed
*/

//Typical Scratch Arena
extern class ScratchArena globalArena;

enum arena_t : u8 {
	SCRATCH,
	STACK,
	POOL
};

class Arena {
public:
	arena_t type;
	char name[MAX_NAME_LENGTH];

	u32 capacityBytes;
	u32 usedBytes;
	u8* memory;

	Arena* children;
	Arena* sibling;
};

class ScratchArena : public Arena {
public:
	
};

//This will silently "fail" by not adding it to tree, will still be a working arena and not corrupt memory
#define MAX_STACK_CHILDREN 4
struct StackHeader {
	u32 index;
	StackHeader* next;

	Arena* children[MAX_STACK_CHILDREN];
};

class StackArena : public Arena {
public:
	u32 numCheckpoints;
	StackHeader* head;
};

struct PoolChunk {
	PoolChunk* next;
};

class PoolArena: public Arena {
public:
	u32 numChunks;
	u32 chunkSize;
	u32 usedChunks;

	PoolChunk* head;
};

//Takes in a size and uses the buffer[16] supplied to return a string of how big it is in b/kb/gb
void PrintableBytes( u32 size, char* buffer );

//Note: scratchArena must already be allocated, this will not call parent.allocate()
void CreateScratchArena( ScratchArena* scratchArena, u32 capacity, void* memory, Arena* parent, const char* name = "");
void* ScratchArenaAllocate( ScratchArena* arena, u32 size );
void ScratchArenaFree( ScratchArena* arena );
void* ScratchArenaAllocateZero( ScratchArena* arena, u32 size );

//Note: poolAllocator must already be allocated, this will not call parent.allocate()
void CreatePoolArena( PoolArena* poolArena, u32 chunkSize, u32 numChunks, void* memory , Arena* parent, const char* name = "" );
void* PoolArenaAllocate( PoolArena* arena );
void* PoolArenaAllocateZero( PoolArena* arena );
void PoolArenaFree( PoolArena* arena, void* freeChunk );

void CreateStackArena( StackArena* scratchArena, u32 capacity, void* memory, Arena* parent, const char* name = "" );
void* StackArenaAllocate( StackArena* arena, u32 size );
void* StackArenaAllocateZero( StackArena* arena, u32 size );
int StackArenaSetCheckpoint( StackArena* arena );
void StackArenaFreeAll( StackArena* arena );
//If -1, goes back to the previous one
void StackArenaFreeToPrevious( StackArena* arena, int whichIndex = -1 );

void PrintAllocators( Arena* head );

#define TEMP_ARENA_SET StackArenaAutoKill __skill(&tempArena);
#define TEMP_ALLOC(x) StackArenaAllocate(&tempArena, x );
#define TEMP_ALLOC_ZERO(x) StackArenaAllocateZero(&tempArena, x );
class StackArenaAutoKill {
public:
	StackArena* arena;
	StackArenaAutoKill( StackArena* arena ) {
		this->arena = arena;
		StackArenaSetCheckpoint( arena );
	}
	~StackArenaAutoKill() {
		StackArenaFreeToPrevious( arena );
	}
};