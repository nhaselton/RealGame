#pragma once
#include "Core/coredef.h"
#include "Log.h"

class DLL Timer {
public:
	//Starts the timer when created
	Timer();
	//Stops the timer
	void Tick();
	void Restart();

	u64 GetTimeCycles();
	f32 GetTimeSeconds();
	f32 GetTimeMiliSeconds();

private:
	u64 start;
	u64 stop;
};

#define PROFILE(x) ProfileTimer profileTimer(x);

class DLL ProfileTimer {
public:
	ProfileTimer( const char* name ) {
		strcpy_s( this->name, MAX_NAME_LENGTH, name );
	}
	~ProfileTimer() {
		t.Tick();
		LOG_INFO( LGS_PROFILE, "%s: %.4fms\n", name, t.GetTimeMiliSeconds() );
	}

	char name[MAX_NAME_LENGTH];
	Timer t;
};

u64 QueryHighFreqTimer();
u64 QueryCPUFrequency();
void NSpinLock( u32 fps );