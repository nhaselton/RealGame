#version 450 core

out vec4 FragColor;

uniform sampler2D albedo;
uniform sampler2D lightmap;

in vec2 vtex;
in vec3 vnorm;
in vec3 vpos;
in vec2 vlightTex;

struct Light{
	vec3 pos;
	float cutoff;
	vec3 dir;
	float type;
	vec3 color;
	float intensity;
	//Constant Linear Quadratic
	vec4 attenuation;
};

layout(std430, binding = 8 ) readonly buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
	ivec4 counts;
	Light lights[];
};

float Attenuation( float dist, vec3 at ) {
	return 1.0 / ( at.x + at.y * dist + at.z * (dist*dist)) ;
}

vec3 Directional( Light light, vec3 norm ) {
	return dot(light.dir.xyz,norm) * light.intensity * light.color.xyz;
}

vec3 Point( Light light, vec3 pos, vec3 norm ) {
	vec3 dir = normalize( light.pos.xyz - pos );
	vec3 color = (max(dot(dir, norm) * light.intensity,0) * light.color.xyz);
	color *= Attenuation( length(light.pos.xyz - pos), light.attenuation.xyz );
	return color;
}

vec3 Spot( Light light, vec3 pos, vec3 norm ){
	vec3 dir = normalize( light.pos.xyz - pos );
	float theta = dot(-light.dir.xyz, dir);

	if ( theta < light.cutoff ){
		return vec3(0);
	}

	vec3 color = (max(dot(dir, norm)* light.intensity,0) * light.color.xyz);
	color *= Attenuation( length(light.pos.xyz - pos), light.attenuation.xyz );
	return color;
}

void main() {
	vec3 rawcolor = texture(albedo, vtex).rgb;
	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);

	for ( int i = 0; i < counts.x; i++)	{
		if ( lights[i].type == 1 )
			diffuse += Directional(lights[i], vnorm);
		else if ( lights[i].type == 2 )
			diffuse += Point(lights[i], vpos, vnorm);
		else if ( lights[i].type == 3 )
			diffuse += Spot(lights[i], vpos, vnorm);
	}

	vec3 color = (diffuse + ambient) * rawcolor;

	vec3 lightmapColor = texture(lightmap,vlightTex).rgb;
	color = rawcolor * diffuse;
	color = lightmapColor * rawcolor;	
	FragColor = vec4(color,1.0);
}