#include "Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Log.h"
#include "Timer.h"

#include <iostream>
void ErrorCallback( int, const char* err_str ) {
	std::cout << "GLFW Error: " << err_str << std::endl;
}


void WindowInit( Window* window, u32 width, u32 height, const char* title ) {
	memset( window, 0, sizeof( Window ) );
	window->width = width;
	window->height = height;

	if ( !glfwInit() ) {
		LOG_ASSERT( LGS_CORE, "Could not init GLFW\n" );
		return;
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	window->handle = glfwCreateWindow( width, height, title, NULL, NULL );
	glfwSetErrorCallback( ErrorCallback );

	if ( window->handle == NULL ) {
		printf( "Failed to create GLFW window.handle\n" );
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent( ( GLFWwindow* ) window->handle );
	glfwSwapInterval( 1 );

	// load function pointers
	if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) ) {
		printf( "Failed to initialize GLAD" );
		return;
	}
	glViewport( 0, 0, width, height );

	//if ( glfwRawMouseMotionSupported() )
	//	glfwSetInputMode( ( GLFWwindow* ) handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );

	glfwSetWindowUserPointer( window->handle, window );
	////Key Callback
	glfwSetKeyCallback( ( GLFWwindow* ) window->handle, KeyCallback );
	glfwSetScrollCallback( ( GLFWwindow* ) window->handle, ScrollCallback );
}

void WindowSetVsync( Window* window, int v) {
	glfwSwapInterval( v );
}

void WindowSwapBuffers( Window* window ) {
	glfwSwapBuffers( ( GLFWwindow* ) window->handle );
}

void WindowPollInput( Window* window ) {
	//KeysUpdate();
	glfwPollEvents();
}

bool WindowShouldClose( Window* window ) {
	return glfwWindowShouldClose( window->handle );
}


void WindowNotifyKeySubscriptions( Window* window, KeyInfo sub ) {
	for ( int i = 0; i < window->keySubscriptions.size(); i++ )
		if ( window->keySubscriptions[i]->numKeys < MAX_KEYS_FRAME )
			window->keySubscriptions[i]->keys[window->keySubscriptions[i]->numKeys++] = sub;
}

void WindowAddKeySubscription( Window* window, KeySub* sub ) {
	window->keySubscriptions.push_back( sub );
}
