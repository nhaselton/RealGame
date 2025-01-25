#pragma once
#include "coredef.h"
#include <vector>
#include "input.h"

#if 0
class DLL Window {
public:
	Window();
	void Init( u32 width, u32 height, const char* title );
	void SwapBuffers();
	void PollInput();
	static void SetVsync( int sync );
	bool ShouldClose();
	u32 width;
	u32 height;
	void NotifyKeySubscriptions( KeyInfo sub );
	void AddKeySubscription(KeySub* sub);
private:
	std::vector<KeySub*> keySubscriptions;
	struct GLFWwindow* handle;
};
#endif

class DLL Window {
public:
	u32 width;
	u32 height;
	std::vector<KeySub*> keySubscriptions;
	struct GLFWwindow* handle;
};

void WindowInit( Window* window, u32 width, u32 height, const char* title );
void WindowSwapBuffers( Window* window );
void WindowPollInput( Window* window );
void WindowSetVsync( Window* window, int sync );
bool WindowShouldClose( Window* window );
void WindowNotifyKeySubscriptions( Window* window, KeyInfo sub );
void WindowAddKeySubscription( Window* window, KeySub* sub );
