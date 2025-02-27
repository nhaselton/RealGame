#include "def.h"
#include "Input.h"
#include "GLFW/glfw3.h"
#include "Window.h"
int keys[KEY_LAST];
int mouseButtons[MOUSE_BUTTON_LAST];

float lastX;
float lastY;
float xOffset;
float yOffset;

#include "Game/EntityManager.h"
#include "Game/Player.h"

void MouseCallback( GLFWwindow* window, double xposIn, double yposIn ) {
	float xpos = static_cast< float >( xposIn );
	float ypos = static_cast< float >( yposIn );

	static bool firstMouse = true;
	if( firstMouse ) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	xOffset = xpos - lastX;
	yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
}

void MouseButtonCallback( GLFWwindow* window, int button, int action, int mods ) {
	if( button >= MOUSE_BUTTON_LAST || button < 0 ) {
		return;
	}

	if( action == GLFW_PRESS ) {
		mouseButtons[button] = 1;
	}
	else if( action == GLFW_RELEASE ) {
		mouseButtons[button] = 0;
	}

}

void KeyCallback( GLFWwindow* window, int key, int scancode, int action, int flags ) {
	if ( key == KEY_ESCAPE ) {
		exit( 0 );
	}

	if ( key == -1 ) {
		printf( "key was -1 for some reason" );
		return;
	}
	if (  action == GLFW_PRESS  )
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

	for( int i = 0; i < MOUSE_BUTTON_LAST; i++ ) {
		if( mouseButtons[i] == 1 )
			mouseButtons[i] = 2;
	}
}

bool MousePressed( int key ) {
	return mouseButtons[key] == 1;
}

bool MouseDown( int key ) {
	return mouseButtons[key] > 0;
}