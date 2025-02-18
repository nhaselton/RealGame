#version 450 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 atex;

uniform mat4 model;
layout(std430, binding = 8 ) buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
};

out vec2 vtex;

void main(){
	vtex = atex;
	gl_Position =  projection * view * model * vec4(apos,1.0);
}		