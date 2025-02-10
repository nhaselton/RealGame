#version 440

out vec4 FragColor;
in vec2 vtex;

uniform sampler2D albedo;
void main() {
	vec4 color = texture(albedo,vtex);
	if ( color.a < 1)
		discard;

	FragColor = vec4(color);
}