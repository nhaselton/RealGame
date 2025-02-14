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

//Mouse
extern float lastX;
extern float lastY;
extern float xOffset;
extern float yOffset;

void KeyCallback( struct GLFWwindow* window,int key, int scancode, int action, int flags );
void ScrollCallback( GLFWwindow* window, double xoffset, double yoffset );
void MouseButtonCallback( GLFWwindow* window, int button, int action, int mods );

bool KeyDown(int key );
bool KeyPressed( int key );
void KeysUpdate();
bool MousePressed( int key );
bool MouseDown( int key );