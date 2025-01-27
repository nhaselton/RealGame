#include "def.h"
#include "Input.h"
#include "GLFW/glfw3.h"
#include "Window.h"
int keys[KEY_LAST];

void KeyCallback( GLFWwindow* window, int key, int scancode, int action, int flags ) {
	if ( key == KEY_ESCAPE ) {
		exit( 0 );
	}

	if ( key == -1 ) {
		printf( "key was -1 for some reason" );
		return;
	}
	if ( action == 1 && action == GLFW_PRESS  )
		keys[key] = 1;
	if ( action == 0 )
		keys[key] = 0;

	Window* win = (Window*) glfwGetWindowUserPointer( window );
	KeyInfo info;
	info.keycode = key;
	info.action = action;
	info.flags = flags;
	WindowNotifyKeySubscriptions( win, info );
}

void ScrollCallback( GLFWwindow* window, double xoffset, double yoffset ) {
	Window* win = ( Window* ) glfwGetWindowUserPointer( window );
	KeyInfo info;
	info.keycode = (yoffset > 0) ? KEY_SCROLLUP : KEY_SCROLLDOWN;
	info.action = 1;
	info.flags = 0;
	WindowNotifyKeySubscriptions( win, info );
}

bool KeyDown( int key ) {
	return keys[key] > 0;
}
bool KeyPressed( int key ) {
	return keys[key] == 1;
}

//Set pressed keys  to down
void KeysUpdate() {
	for ( int i = 0; i < KEY_LAST; i++ )
		if ( keys[i] == 1 )
			keys[i] = 2;
}