
struct Light{
	vec3 pos;
	float cutoff;
	vec3 dir;
	int type;
	vec3 color;
	float intensity;
	//Constant Linear Quadratic
	vec4 attenuation;
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