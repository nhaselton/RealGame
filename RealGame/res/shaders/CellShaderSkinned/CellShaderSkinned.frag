#version 450 core
#include "inc/lighting.inc"
#include "inc/world.inc"

out vec4 FragColor;

uniform sampler2D albedo;
uniform bool fullbright;

in vec2 vtex;
in vec3 vnorm;
in vec3 vpos;

//We want to store the light's color (.xyz) and the intensity (.w)
//The total intensity will determime if we use light or dark shade
//The color of the light will be averaged in main() to get a color for the pixel
//This will mean lights not being used are still averaged...
vec4 Point2( Light light, vec3 pos, vec3 norm ) {
	vec3 dir = normalize( light.pos.xyz - pos );
	float diffuse = (max(dot(dir, norm) * light.intensity,0));

	if ( diffuse == 0 )
		return vec4(0);

	diffuse *= Attenuation( length(light.pos.xyz - pos), light.attenuation.xyz );
	vec4 color = vec4(0);
	color.xyz = light.color.xyz;
	color.w = diffuse;

	//Try Clamping each light color?
	if ( diffuse <= .2 )
		color.w *= .2;
	

	return color;
}
	
void main() {
	vec3 rawcolor = texture(albedo, vtex).rgb;

	vec4 totalLight = vec4(0.0);
	int numLights = 0;
	for ( int i = 0; i < numStaticLights; i++)  
		if ( staticLights[i].type == 2 ){
			vec4 add = Point2(staticLights[i], vpos, vnorm);
			totalLight += add;
			if ( add.w > 0 ) numLights++;
		}
	for ( int i = 0; i < numDynamicLights; i++)  
		if ( dynamicLights[i].type == 2 ){
			vec4 add = Point2(dynamicLights[i], vpos, vnorm);
			totalLight += add;
			if ( add.w > 0 ) numLights++;
		}
	numLights = max(numLights,1);

	if ( totalLight.w > .6 )
		totalLight.w = 1.0f;
	else
		totalLight.w = 0.4;
	
	if ( totalLight.xyz == vec3(0))
		totalLight.xyz = vec3(1);

	totalLight.xyz /= float(numLights);
	vec3 color = (totalLight.w * rawcolor) * totalLight.xyz;

	FragColor = vec4(color,1.0);
}
