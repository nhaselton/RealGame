#version 450 core
#include "inc/lighting.inc"
#include "inc/world.inc"

out vec4 FragColor;

uniform sampler2D albedo;
uniform bool fullbright;

in vec2 vtex;
in vec3 vnorm;
in vec3 vpos;
	
void main() {
	vec3 rawcolor = texture(albedo, vtex).rgb;
	vec3 ambient = vec3(0.1);
	vec3 diffuse = vec3(0.0);

	for ( int i = 0; i < numStaticLights; i++)	{
		if		( staticLights[i].type == 1 ) diffuse += Directional(staticLights[i], vnorm);
		else if ( staticLights[i].type == 2 ) diffuse += Point(staticLights[i], vpos, vnorm);
		else if ( staticLights[i].type == 3 ) diffuse += Spot(staticLights[i], vpos, vnorm);
	}

	for ( int i = 0; i < numDynamicLights; i++)	{
		if		( dynamicLights[i].type == 1 ) diffuse += Directional(dynamicLights[i], vnorm);
		else if ( dynamicLights[i].type == 2 ) diffuse += Point(dynamicLights[i], vpos, vnorm);
		else if ( dynamicLights[i].type == 3 ) diffuse += Spot(dynamicLights[i], vpos, vnorm);
	}


	vec3 color = (diffuse + ambient) * rawcolor;
	//vec3 color = rawcolor;
	if ( fullbright ){
		color = rawcolor;
	}
	FragColor = vec4(color,0.1);
}