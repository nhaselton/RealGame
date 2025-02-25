#version 450 core
#include "inc/lighting.inc"

out vec4 FragColor;

uniform sampler2D albedo;
uniform bool fullbright;

in vec2 vtex;
in vec3 vnorm;
in vec3 vpos;

layout(std430, binding = 8 ) readonly buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
	ivec4 counts;
	Light lights[];
};

void main() {
	vec3 rawcolor = texture(albedo, vtex).rgb;
	vec3 ambient = vec3(0.1);
	vec3 diffuse = vec3(0.0);

	for ( int i = 0; i < counts.x; i++)	{
		if ( lights[i].type == 1 )
			diffuse += Directional(lights[i], vnorm);
		else if ( lights[i].type == 2 )
			diffuse += Point(lights[i], vpos, vnorm);
		else if ( lights[i].type == 3 )
			diffuse += Spot(lights[i], vpos, vnorm);
	}
	vec3 color = (diffuse + ambient) * rawcolor;

	//vec3 color = rawcolor;

	if ( fullbright ){
		color = rawcolor;
	}
	FragColor = vec4(color,1.0);
}