#version 330 core

out vec4 FragColor;

uniform sampler2D albedo;
uniform bool solidColor;
uniform vec3 color;

in vec2 vtex;

void main(){
	
	if ( solidColor ) {
		FragColor = vec4(color,1.0);
	} else {
		FragColor = texture(albedo, vtex);
		if ( FragColor.a < .75 )
			discard;
	}
	
}