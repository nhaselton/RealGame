#version 450 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;
layout (location = 2) in vec2 atex;
layout (location = 3) in vec2 alighttex;

uniform mat4 lightSpace;
uniform mat4 model;

void main() {
	gl_Position =  lightSpace * model * vec4(apos,1.0);
}		