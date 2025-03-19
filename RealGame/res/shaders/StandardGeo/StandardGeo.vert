#version 450 core
#include "inc/lighting.inc"
#include "inc/world.inc"

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;
layout (location = 2) in vec2 atex;
layout (location = 3) in vec2 alighttex;

uniform mat4 model;

out vec3 vnorm;
out vec2 vtex;
out vec2 vlightTex;
out vec3 vpos;

void main() {
	vpos = vec3(model * vec4(apos,1.0));
	vtex = atex;
	vlightTex = alighttex;
	vnorm = anorm;
	gl_Position =  projection * view * model * vec4(apos,1.0);
}		