#version 440 

struct Particle	{
	vec4 pos;		//xyz = pos.	w = lifeTime [0,1]
	vec4 color;		//xyz color		w: ?
	vec4 velocity;	//xyz = vel		w = ? 
	vec4 pad;
};

layout(std430, binding = 1 ) buffer particleBuffer {
	Particle particles[];
};

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec3 vcolor;
void main() {
	int index = gl_VertexID;
	vcolor = particles[index].color.xyz;
	gl_Position = projection * view * model * vec4(particles[index].pos.xyz,1.0);
}