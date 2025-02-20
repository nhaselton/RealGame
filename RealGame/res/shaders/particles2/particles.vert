#version 440 

struct Particle	{
	vec4 pos;		//xyz = pos.	w = lifeTime [0,1]
	vec4 color;		//xyz color		w: scale.x
	vec4 velocity;	//xyz = vel		w = scale.y 
	vec4 acceleration;
	vec4 UVs;
};

layout(std430, binding = 3 ) buffer particleBuffer {
	Particle particles[];
};

layout(std430, binding = 5 ) buffer sortedBuffer {
	int sortCount;
	uint sortIndices[];
};

uniform mat4 model;
layout(std430, binding = 8 ) readonly buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
};

out vec2 vtex;

const vec3 billboard[] = {
  vec3(-1, -1, 0),
  vec3(1, -1, 0),
  vec3(-1, 1, 0),
  vec3(-1, 1, 0),
  vec3(1, -1, 0),
  vec3(1, 1, 0),
};

const vec2 uvs[] = {
  vec2(0, 0),
  vec2(1, 0),
  vec2(0, 1),
  vec2(0, 1),
  vec2(1, 0),
  vec2(1, 1),
};

vec2 CorrectUVs(int index) {
	uint vertexID = index % 6;
	uint instanceID = index / 6;

	vec2 texCoords = vec2(0,0);

	//X
	if ( vertexID == 0 || vertexID == 2 || vertexID == 3)
		texCoords.x = particles[instanceID].UVs.x;
	else
		texCoords.x = particles[instanceID].UVs.z;
	//Y
	if ( vertexID == 0 || vertexID == 1 || vertexID == 4)
		texCoords.y = particles[instanceID].UVs.y;
	else
		texCoords.y = particles[instanceID].UVs.w;



	return texCoords;
}

void main() {
	int index = gl_VertexID;
	uint vertexID = index % 6;
	uint instanceID = index / 6;

	if ( instanceID > uint(sortCount))
		instanceID =0;

	//instanceID = sortIndices[instanceID];
	Particle particle = particles[instanceID];

	vec3 quadPos = billboard[vertexID];
	quadPos.x *= particles[instanceID].color.w;
	quadPos.y *= particles[instanceID].velocity.w;

	vec3 realPos = particle.pos.xyz + quadPos;

	vtex = CorrectUVs(index);
	mat4 viewModel = view * model;

	realPos = particle.pos.xyz + ( quadPos * mat3(view) );
	gl_Position = projection * view * vec4(realPos, 1.0);
}