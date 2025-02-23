#include "SoundManager.h"
#include <AL/al.h>
#include <AL/alc.h>

void CreateSoundManager () {
    //Set up OpenAL devices 
    soundManager.device = alcOpenDevice ( nullptr );
    if (!(soundManager.device)) {
        LOG_ASSERT ( LGS_SOUND, "NO SOUND DEVICES\n" );
    }

    soundManager.context = alcCreateContext ( soundManager.device, nullptr );
    if (!soundManager.context) {
        LOG_ASSERT ( LGS_SOUND, "Could not create sound device context\n" );
    }

    if (!alcMakeContextCurrent ( soundManager.context )) {
        LOG_ASSERT ( LGS_SOUND, "Could not set sound context to current\n" );
    }

    soundManager.numSounds = 0;
    soundManager.totalSoundBytes = 0;

    //Error check (finish this)
    alGetError ();
	alcGetError ( soundManager.device );
    
    u32 sources[MAX_AUDIO_SOURCES];

    //Generate all sources up front
    alGenSources ( MAX_AUDIO_SOURCES, sources );
    
    alGenSources( 1, &soundManager.deadSource.alSourceIndex );

    CreatePoolArena ( &soundManager.AudioSourceArena, sizeof ( AudioSource ), MAX_AUDIO_SOURCES,
        ScratchArenaAllocate ( &globalArena, sizeof ( AudioSource ) * MAX_AUDIO_SOURCES ), &globalArena, "Audio Source Arena" );

	AudioSource* sourcePoolMemory = ( AudioSource* )soundManager.AudioSourceArena.memory;
    soundManager.numActiveSources = 0;
    //Create free lsit
    for (int i = 0; i < MAX_AUDIO_SOURCES; i++) {
		soundManager.activeSources[i] = 0;
        sourcePoolMemory[i].alSourceIndex = sources[i];
    }
}

void SoundSetListenerPosition ( const Vec3& pos ) {
    alListenerfv ( AL_POSITION, &pos.x );
}

//http://soundfile.sapp.org/doc/WaveFormat/
void LoadWavFile ( Sound* sound, const char* path ) {
    NFile file;
    CreateNFile ( &file, path, "rb" );

    if (!NFileValid ( &file )) {
        LOG_ERROR ( LGS_SOUND, "Could not load sound %s\n", path );
        return;
    }

    strcpy_s ( sound->path, MAX_PATH_LENGTH, path );
    WavFile& wav = sound->file;

    NFileRead ( &file, &wav, sizeof ( WavFile ) );

    if (wav.audioFormat != 1) {
        LOG_ASSERT ( LGS_SOUND, ".Wav Audio must be PCM" );
    }

    StackArenaSetCheckpoint ( &tempArena );
	sound->data = TEMP_ALLOC ( wav.dataSize );
    NFileRead ( &file, sound->data, wav.dataSize );

    //Create Buffer
    alGenBuffers ( 1, &sound->bufferID );

    ALenum format;
    if (wav.numChannels == 1 && wav.bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if (wav.numChannels == 1 && wav.bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if (wav.numChannels == 2 && wav.bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if (wav.numChannels == 2 && wav.bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else {
        LOG_ASSERT ( LGS_SOUND, "Bad .wav form %s\n", path );
        return;
    }

    //Upload data to buffer
    alBufferData ( sound->bufferID, format, sound->data, wav.dataSize, wav.sampleRate );
    StackArenaFreeToPrevious ( &tempArena );
	memset ( sound->data, 0, wav.dataSize );
}

AudioSource* CreateTempAudioSource( Vec3 pos, Sound* sound ) {
    AudioSource* source = NewAudioSource();
    if( !source ) {
		LOG_WARNING( LGS_SOUND, "Could not create temp audio source\n" );
        return &soundManager.deadSource;
    }
    source->pos = pos;
    source->sound = sound;
    source->flags = AUDIO_SOURCE_TEMP;
    PlaySound( source, sound );
    return source;
}

AudioSource* NewAudioSource () {
    if (soundManager.numSources == MAX_AUDIO_SOURCES) {
		LOG_WARNING ( LGS_SOUND, "Can't create new audio sources\n" );
        return &soundManager.deadSource;
    }
    
    soundManager.numSources++;
	AudioSource* source = ( AudioSource* )PoolArenaAllocate ( &soundManager.AudioSourceArena );

    if( !source ) {
        return 0;
    }

    source->active = false;
    source->sound = 0;
    source->flags = 0;

    alSourcef( source->alSourceIndex, AL_PITCH, 1 );
    alSourcef( source->alSourceIndex, AL_GAIN, 1.0f );
    alSourcefv( source->alSourceIndex, AL_POSITION, &source->pos.x );
    alSource3f( source->alSourceIndex, AL_VELOCITY, 0, 0, 0 );
    alSourcei( source->alSourceIndex, AL_LOOPING, AL_FALSE );
    return source;
}

void PlaySound( AudioSource* source, Sound* sound ) {
    return;
    if( source == &soundManager.deadSource )
        return;

    if( !sound ) {
        if( !source->sound ) {
			LOG_WARNING( LGS_SOUND, "Trying to play audio source with no sound\n" );
            return;
        }
    }
    else {
        source->sound = sound;
		alSourcei( source->alSourceIndex, AL_BUFFER, sound->bufferID );
    }

    //Add to active list
    if( !source->active ) {
		source->active = true;
        soundManager.activeSources[soundManager.numActiveSources++] = source;
    }

    alSourcePlay( source->alSourceIndex );
}

void StopSound ( AudioSource* source ) {
    if( source == &soundManager.deadSource )
        return;

    source->active = false;

    int indexInActiveList = -1;
    for( int i = 0; i < soundManager.numActiveSources; i++ ) {
        AudioSource* current = soundManager.activeSources[i];
        if( current == source ) {
            indexInActiveList = i;
            break;
        }
    }

    if( indexInActiveList == -1 ) {
        LOG_ERROR( LGS_SOUND, "Trying to remove active sound that does not exist\n" );
        return;
    }

    soundManager.activeSources[indexInActiveList] = soundManager.activeSources[--soundManager.numActiveSources];

    if( ( source->flags & AUDIO_SOURCE_TEMP ) == AUDIO_SOURCE_TEMP ) {
        RemoveAudioSource( source );
        soundManager.numSources--;
    }
}

void UpdateSounds () {
    for (int i = 0; i < soundManager.numActiveSources; i++) {
        AudioSource* source = soundManager.activeSources[i];
		alSourcefv( source->alSourceIndex, AL_POSITION, &source->pos.x );
		alGetSourcei( source->alSourceIndex, AL_SOURCE_STATE, &source->alState );

        if (source->alState != AL_PLAYING) {
            StopSound( source );
        }
    }
}

void RemoveAudioSource( AudioSource* source ) {
    //TODO end audio if still going
    //  Also remove from active if so
    if( source == &soundManager.deadSource )
        return;

    if( source->active ) {
        StopSound( source );
    }

    PoolArenaFree( &soundManager.AudioSourceArena, source );
}