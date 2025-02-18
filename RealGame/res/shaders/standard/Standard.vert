#version 450 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;
layout (location = 2) in vec2 atex;
layout (location = 3) in vec3 atan;

uniform mat4 model;

out vec3 vnorm;
out vec2 vtex;

layout(std430, binding = 8 ) buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
};

void main() {
	vtex = atex;
	vnorm = anorm;
	gl_Position =  projection * view * model * vec4(apos,1.0);
}		