#pragma once
#include "def.h"
class ALCdevice;
class ALCcontext;

struct WavFile {
	char chunkID[4];
	int chunkSize;
	char format[4];

	//fmt 
	char subChunk1ID[4];
	int subChunk1Size;
	i16 audioFormat;
	i16 numChannels;
	int sampleRate;
	int byteRate;
	i16 blockAlign;
	i16 bitsPerSample;

	//data
	char dataID[4];
	int dataSize;
};

struct Sound {
	char path[MAX_PATH_LENGTH];
	WavFile file;
	u32 bufferID;
	void* data;
};

struct AudioSource {
	Sound* sound;
	bool active;
	Vec3 pos;
	int alState;
	//Make sure this is at least 8bytes into struct so pointer does not override
	u32 alSourceIndex;
};

class SoundManager {
public:
	Sound sounds[MAX_SOUND_BUFFERS];
	PoolArena AudioSourceArena;
	AudioSource* activeSources[MAX_AUDIO_SOURCES];

	int numSounds;
	int numSources;
	int numActiveSources;
	
	u64 totalSoundBytes;
	ALCdevice* device;
	ALCcontext* context;
};

extern SoundManager soundManager;

AudioSource* NewAudioSource ();
void RemoveAudioSource ( AudioSource* source );
void SoundSetListenerPosition ( const Vec3& pos );
//If sound != 0, will change sound
//Else will play whatever is currently in source
void PlaySound( AudioSource* source, Sound* sound );
void UpdateSounds();

void CreateSoundManager ();
void LoadWavFile ( Sound* sound, const char* path );

inline float dBToVolume ( float dB ) {
	return powf ( 10.0f, 0.05f * dB );
}

inline float VolumeTodB ( float volume ) {
	return 20.0f * log10f ( volume );
}