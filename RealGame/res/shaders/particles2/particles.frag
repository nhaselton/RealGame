#version 440

out vec4 FragColor;
in vec2 vtex;

uniform sampler2D albedo;
void main() {
	vec4 color = texture(albedo,vtex);
	FragColor = vec4(color);
}