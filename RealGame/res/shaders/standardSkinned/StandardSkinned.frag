#version 330 core

out vec4 FragColor;

uniform sampler2D albedo;

in vec2 vtex;
in vec3 vnorm;
void main() {
	vec3 color = texture(albedo, vtex).rgb;
	color *= dot(vec3(0,0,1), vnorm);
	FragColor = vec4(color,1.0);
}