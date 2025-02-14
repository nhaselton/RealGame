#pragma once
#include "coredef.h"
#include <vector>
#include "input.h"

class Window {
public:
	u32 width;
	u32 height;
	std::vector<KeySub*> keySubscriptions;
	struct GLFWwindow* handle;
	bool cursorLocked;
};
extern Window window;
void WindowInit( Window* window, u32 width, u32 height, const char* title );
void WindowSwapBuffers( Window* window );
void WindowPollInput( Window* window );
void WindowSetVsync( Window* window, int sync );
bool WindowShouldClose( Window* window );
void WindowNotifyKeySubscriptions( Window* window, KeyInfo sub );
void WindowAddKeySubscription( Window* window, KeySub* sub );
