#version 450 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;

layout(std430, binding = 8 ) readonly buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

out vec3 vnorm;

void main(){
	vnorm = anorm;
	gl_Position =  projection * view * model * vec4(apos,1.0);
}		