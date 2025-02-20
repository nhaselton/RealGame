/*
	NOTE: THIS IS NOT BEING USED CURRENTLY.
	STANDARD SKINNED USES THE STANDARD FRAGMENT SHADER
*/
DFJADFSAJDFJKLSA ///to error if trying to use by mistake
#version 330 core

out vec4 FragColor;

uniform sampler2D albedo;

in vec2 vtex;
in vec3 vnorm;

void main() {
	vec3 color = texture(albedo, vtex).rgb;
	color *= dot(vec3(0,1,1), vnorm);
	
	FragColor = vec4(color,1.0);
}