#version 450 core
#include "inc/lighting.inc"
#include "inc/world.inc"

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;
layout (location = 2) in vec2 atex;
layout (location = 3) in vec4 aweight;
layout (location = 4) in ivec4 abone;

uniform mat4 model;
uniform mat4 bones[100];

out vec2 vtex;
out vec3 vnorm;
out vec3 vpos;

void main(){
	vtex = atex;

    mat4 skinMatrix =
        aweight.x * bones[abone.x] +
        aweight.y * bones[abone.y] +
        aweight.z * bones[abone.z] +
        aweight.w * bones[abone.w];

	vec4 normalL = model * skinMatrix * vec4(anorm,0.0);
	vnorm = normalize(normalL.xyz);

	vec4 skinPos = model * skinMatrix * vec4(apos,1.0);
	vpos = skinPos.xyz;

	gl_Position =  projection * view * model * skinMatrix * vec4(apos,1.0);
}		