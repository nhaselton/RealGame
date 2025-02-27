#include "Timer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"

u64 QueryHighFreqTimer() {
	u64 time = 0;
	QueryPerformanceCounter( ( LARGE_INTEGER* ) &time );
	return time;
}

u64 QueryCPUFrequency() {
	u64 freq = 0;
	QueryPerformanceFrequency( ( LARGE_INTEGER* ) &freq );
	return freq;
}

//Starts the timer when created
Timer::Timer() {
	start = QueryHighFreqTimer();
	stop = 0;
}
//Stops the timer
void Timer::Tick() {
	stop = QueryHighFreqTimer();
}

void Timer::Restart() {
	stop = 0;
	start = QueryHighFreqTimer();	
}

u64 Timer::GetTimeCycles(){
	if ( stop == 0 || start == 0 ) {
		printf( "Timer not completed. start: %llu\tend: %llu\n", start, stop );
	}
	return stop - start;
}

f32 Timer::GetTimeSeconds(){
	u64 t = GetTimeCycles();

	float timeSeconds = ( float ) t / ( float ) QueryCPUFrequency();
	return timeSeconds;
}

f32 Timer::GetTimeMiliSeconds() {
	return GetTimeSeconds() * 1000.0f;
}

void NSpinLock( u32 time ) {
	u32 last = QueryHighFreqTimer();
	while( time > 0 ) {
		u32 now = QueryHighFreqTimer();
		if( now > last + QueryCPUFrequency() / time )
			break;
	}
}