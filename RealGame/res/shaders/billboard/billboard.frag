#version 330 core

out vec4 FragColor;

uniform sampler2D albedo;
in vec2 vtex;

uniform vec4 mapTex;//From [0,1] to [0,?]
void main(){
	vec4 tex = texture(albedo,vtex);

	FragColor = vec4( tex);
}