#version 330 core

out vec4 FragColor;

uniform sampler2D albedo;

in vec2 vtex;
in vec3 vnorm;

void main() {
	vec3 color = texture(albedo, vtex).rgb;
	FragColor = vec4(color,1.0);
}