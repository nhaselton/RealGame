#version 330 core

out vec4 FragColor;

uniform sampler2D albedo;

in vec2 vtex;
in vec3 vnorm;

void main() {
	vec3 rawcolor = texture(albedo, vtex).rgb;
	vec3 color = rawcolor * dot(vec3(0,.707,-.707), vnorm);
	color += vec3(.2);
	color = min(color,rawcolor);
	FragColor = vec4(color,1.0);
}