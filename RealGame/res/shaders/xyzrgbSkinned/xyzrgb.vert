#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;
layout (location = 2) in vec2 atex;
layout (location = 3) in vec3 atan;
layout (location = 4) in ivec4 abone;
layout (location = 5) in vec4 aweight;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

uniform mat4 bones[100];

void main(){
    mat4 skinMatrix =
        aweight.x * bones[abone.x] +
        aweight.y * bones[abone.y] +
        aweight.z * bones[abone.z] +
        aweight.w * bones[abone.w];

	gl_Position =  projection * view * model * skinMatrix * vec4(apos,1.0);
}		