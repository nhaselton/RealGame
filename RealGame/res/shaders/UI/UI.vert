#version 330 core

layout (location = 0) in vec2 apos;
layout (location = 1) in vec2 atex;

uniform mat4 model;
uniform mat4 projection;

out vec2 vtex;

void main(){
	vtex = atex;	
	gl_Position =  projection * model * vec4(apos,0.0,1.0);
}		