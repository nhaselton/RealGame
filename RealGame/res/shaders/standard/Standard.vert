#version 450 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anorm;
layout (location = 2) in vec2 atex;
layout (location = 3) in vec2 alighttex;

uniform mat4 model;

out vec3 vnorm;
out vec2 vtex;
out vec2 vlightTex;
out vec3 vpos;

struct Light{
	vec3 pos;
	float cutoff;
	vec3 dir;
	float type;
	vec3 color;
	float intensity;
	//Constant Linear Quadratic
	vec4 attenuation;
};

layout(std430, binding = 8 ) readonly buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
	ivec4 counts;
	Light lights[];
};

void main() {
	vpos = vec3(model * vec4(apos,1.0));
	vtex = atex;
	vlightTex = alighttex;
	vnorm = anorm;
	gl_Position =  projection * view * model * vec4(apos,1.0);
}		