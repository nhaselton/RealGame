#pragma once

//Key subscription will be informed each time a key is pressed
struct DLL KeyInfo {
	int keycode;
	int action;
	int flags;
};

#define MAX_KEYS_FRAME 25
struct DLL KeySub {
	KeyInfo keys[MAX_KEYS_FRAME];
	int numKeys;
};

void DLL KeyCallback( struct GLFWwindow* window,int key, int scancode, int action, int flags );
void DLL ScrollCallback( GLFWwindow* window, double xoffset, double yoffset );
bool DLL KeyDown(int key );
bool DLL KeyPressed( int key );
void DLL KeysUpdate();