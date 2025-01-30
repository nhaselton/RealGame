#include "def.h"

void PrintableBytes( u32 size, char* buffer ) {
	if ( size > GB( 1 ) ) {
		double newSize = ( double ) size / ( double ) GB( 1 );
		sprintf_s( ( char* ) buffer, 16, "%.2lf GB", newSize );
	}
	else if ( size > MB( 1 ) ) {
		double newSize = ( double ) size / ( double ) MB( 1 );
		sprintf_s( ( char* ) buffer, 16, "%.2lf MB", newSize );
	}
	else if ( size > KB( 1 ) ) {
		double newSize = ( double ) size / ( double ) KB( 1 );
		sprintf_s( ( char* ) buffer, 16, "%.2lf KB", newSize );
	}
	else {
		sprintf_s( ( char* ) buffer, 16, "%d B", size );
	}
}

//TODO pool
static inline void SetParentChildren(Arena* child, Arena* parent ) {
	//Add to pool allocator header's list of current children
	//This way when freeing to a checkpoint it can destroy all removed allocators
	if ( parent->type == STACK ) {
		StackArena* stack = ( StackArena* ) parent;
		StackHeader* header = stack->head;
		if ( header ) {
			for ( int i = 0; i < MAX_STACK_CHILDREN; i++ ) {
				if ( header->children[i] == 0 ) {
					header->children[i] = child;
					break;
				}
			}
		}
	}

	//Add to LL 
	//Note: This means the newest allocator appears first in the list, but thats okay
	child->sibling = parent->children;
	parent->children = child;
}

void CreateScratchArena( ScratchArena* scratchArena, u32 capacity, void* memory, Arena* parent, const char* name ) {
	//Must have at least enoguh spaces for pointers and stuff
	assert( capacity > 256 );
	assert( scratchArena );

	scratchArena->type = SCRATCH;
	scratchArena->capacityBytes = capacity;
	scratchArena->memory = (u8*) memory;
	scratchArena->usedBytes = 0;
	scratchArena->children = nullptr;

	strcpy_s( scratchArena->name, MAX_NAME_LENGTH, name );

	//If no parent was given, it must be the global arena
	if ( !parent ) {
		if ( scratchArena != &globalArena ) {
			LOG_ASSERT( LGS_MEMORY, "Arena requires parent");
			memset( scratchArena, 0, sizeof( *scratchArena ) );
		}
		return;
	}
	SetParentChildren( scratchArena, parent );
}


void* ScratchArenaAllocateZero( ScratchArena* arena, u32 size ) {
	void* data = ScratchArenaAllocate( arena, size );
	if ( data != 0 )
		memset( data, 0, size );
	return data;
}

void* ScratchArenaAllocate( ScratchArena* arena, u32 size ) {
	u32 newSize = arena->usedBytes + size;

	if ( newSize > arena->capacityBytes ) {
		//Todo Logging
		assert( 0 );
		return NULL;
	}

	void* ptr = arena->memory + arena->usedBytes;
	arena->usedBytes = newSize;
	return ptr;
}


void PoolArenaFreeAll( PoolArena* arena ) {
	CreatePoolArena( arena, arena->chunkSize, arena->numChunks, arena->memory, arena, arena->name );
}

void CreatePoolArena( PoolArena* poolArena, u32 chunkSize, u32 numChunks, void* memory, Arena* parent, const char* name ) {
	assert( chunkSize >= sizeof( void* ) ); //Must be able to store a pointer to the next one
	assert( parent );

	//Base
	poolArena->type = POOL;
	poolArena->capacityBytes = chunkSize * numChunks;
	poolArena->memory = (u8*) memory;
	poolArena->usedBytes = 0;
	poolArena->children = nullptr;
	strcpy_s( poolArena->name, 32, name );

	//Pool
	poolArena->numChunks= numChunks;
	poolArena->chunkSize = chunkSize;

	//Create Chunks out of the memory
	poolArena->head = (PoolChunk*) poolArena->memory;
	u8* memoryptr = poolArena->memory + poolArena->chunkSize;
	PoolChunk* current = poolArena->head;

	//TODO spend some time verifiyng if this is an off by 1 error.
	//  It's either going to write too much memory or miss a single node. Im not sure (poolArena->numChunks -1 or poolArena->numChunks)
	for ( int i = 0; i < poolArena->numChunks-1; i++, memoryptr += poolArena->chunkSize ) {
		current->next = ( PoolChunk* ) memoryptr;
		current = current->next;
	}
	current->next = 0;

	if ( parent != poolArena )//(Hack) Can set parent to self to reset arena
		SetParentChildren( poolArena, parent );
}

void* PoolArenaAllocateZero( PoolArena* arena ) {
	void* mem = PoolArenaAllocate( arena );
	memset( mem, 0, arena->chunkSize );
	return mem;
}
void* PoolArenaAllocate( PoolArena* arena ) {
	if ( !arena->head ) {
		LOG_ERROR( LGS_MEMORY, "Pool Arena %s is out of memory", arena->name );
		assert( 0 );
		return 0;
	}

	PoolChunk* chunk = arena->head;
	arena->head = arena->head->next;

	arena->usedBytes += arena->chunkSize;
	arena->usedChunks++;

	return chunk;
}

void ScratchArenaFree( ScratchArena* arena ) {
	arena->usedBytes = 0;
	arena->children = nullptr; //No longer exists because you nuked all of them
}

void PoolArenaFree( PoolArena* arena, void* freeChunk ) {
	PoolChunk* chunk = ( PoolChunk* ) freeChunk;
	chunk->next = arena->head;
	arena->head = chunk;

	arena->usedBytes -= arena->chunkSize;
	arena->usedChunks--;
}

void CreateStackArena( StackArena* stackArena, u32 capacity, void* memory, Arena* parent, const char* name ) {
	stackArena->type = STACK;
	stackArena->capacityBytes = capacity;
	stackArena->memory = ( u8* ) memory;
	stackArena->usedBytes = 0;
	stackArena->children = nullptr;
	strcpy_s( stackArena->name, MAX_NAME_LENGTH, name );
	
	stackArena->numCheckpoints = 0;
	stackArena->head = NULL;

	StackArenaSetCheckpoint( stackArena );
	SetParentChildren( stackArena, parent );
}

void* StackArenaAllocateZero( StackArena* arena, u32 size ) {
	void* mem = StackArenaAllocate( arena, size );
	memset( mem, 0, size );
	return mem;
}
void* StackArenaAllocate( StackArena* arena, u32 size ) {
	u32 newSize = arena->usedBytes + size;

	if ( newSize > arena->capacityBytes ) {
		LOG_ASSERT( LGS_MEMORY, "No space to allocate %u bytes in arena %s\n", size, arena->name );
		assert( 0 );
		return NULL;
	}

	void* ptr = arena->memory + arena->usedBytes;
	arena->usedBytes = newSize;
	return ptr;
}

int StackArenaSetCheckpoint( StackArena* arena ) {
	//Make sure we have room to allocate a chunk
	StackHeader* header = (StackHeader*) StackArenaAllocate( arena, sizeof( StackHeader ) );
	if ( header == nullptr ) {
		LOG_ASSERT( LGS_MEMORY, "No space to allocate stack header in arena %s\n", arena->name );
		return 0;
	}
	memset( header, 0, sizeof( StackHeader ) );

	header->index = arena->numCheckpoints++;
	header->next = nullptr;

	//Add to end of list
	if ( arena->head ) {
		StackHeader* head = arena->head;
		while ( head->next ) head = head->next;
		head->next = header;
	}
	else
		arena->head = header;

	return header->index;
}

void StackArenaFreeAll( StackArena* arena ) {
	arena->numCheckpoints = 0;
	arena->head = 0;
	arena->usedBytes = 0;
	arena->children = 0;

	//Make sure theres always a checkpoint?
	StackArenaSetCheckpoint( arena );
}

//If -1, goes back to the previous one
void StackArenaFreeToPrevious( StackArena* arena, int whichIndex ) {
	if ( whichIndex == -1 )
		whichIndex = arena->numCheckpoints - 1;

	//Get the previous header
	StackHeader* header = arena->head;
	StackHeader* prev = 0;
	while ( header->index != whichIndex ) {
		prev = header;
		header = header->next;

		if ( header == nullptr ) {
			//assert probably?
			StackArenaFreeAll( arena );
			return;
		}
	}

	//If theres no previous then it was all deleted, just call the free all for simplicity
	if ( prev == 0 ) {
		StackArenaFreeAll( arena );
		return;
	}

	//Remove all children in this chunk
	//TODO check that this actually works.
	for ( int i = 0; i < MAX_STACK_CHILDREN; i++ ) {
		if ( header->children[i] == 0 )
			break;

		Arena* prev = 0;
		Arena* current = arena->children;
		while ( current != 0 ) {
			if ( current == header->children[i] ) {
				if ( !prev )
					arena->children = current->sibling;
				else
					prev->sibling = current->sibling;
				break;
			}
			current = current->sibling;
		}
	}


	//Find how far away we are from the start of the memory block
	prev->next = header->next;
	u64 location = ( u64 ) ( ( u8* ) header - ( u8* ) arena->memory );// +sizeof( StackHeader );
	arena->usedBytes = location;
	arena->numCheckpoints--;
}

//#define MEM_FIRST
void PrintAllocators( Arena* head ) {
	char used[16];
	char max[16];

	int depth = 0;
	Arena* queue[16];
	int queueSize = 1;
	queue[0] = head;

	while ( queueSize > 0 ) {
		Arena* current = queue[--queueSize];

		//First Print Current
		PrintableBytes( current->usedBytes, used );
		PrintableBytes( current->capacityBytes, max );

		for ( int i = 0; i < depth; i++ )
#if MEM_FIRST
			LOG_PRINTF( LGS_NONE, "\t" );
#else
		LOG_PRINTF( LGS_NONE, "  " );
#endif

#if MEM_FIRST
		LOG_PRINTF( LGS_NONE, "%s / %s", used, max );
		if ( current->name[0] != '\0' )
			LOG_PRINTF( LGS_NONE, ": %s\n", current->name );
		else
			LOG_PRINTF( LGS_NONE, "\n" );
#else
		if ( current->name[0] != '\0' )
			LOG_PRINTF( LGS_NONE, "%s:  ", current->name );
		LOG_PRINTF( LGS_NONE, "%s / %s\n", used, max );
#endif


		//Add to queue
		if ( current->sibling ) {
			queue[queueSize++] = current->sibling;
		}

		if ( current->children ) {
			queue[queueSize++] = current->children;
			depth++;
		}

		//do we go up a level
		if ( !current->sibling && !current->children )
			depth--;
	}
}
