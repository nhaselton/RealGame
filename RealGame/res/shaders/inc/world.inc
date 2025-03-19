//"inc/lighting.inc" MUST BE READ IN FIRST

layout(std430, binding = 8 ) readonly buffer worldViewBuffer {
	mat4 projection;
	mat4 view;
	
	int numDynamicLights;
	int numStaticLights;
	float gameTime;
	int pad1;
	
	//Static comes first Then comes dynamic
	Light dynamicLights[512];
	Light staticLights[512];
};
