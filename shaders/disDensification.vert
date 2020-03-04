#version 430

flat out vec4 flow;

layout(binding = 0, rgba32f) uniform image2D sparseFlowMap;
layout(binding = 1, rgba32f) uniform image2D testMap;

uniform int level;
uniform vec2 invDenseTexSize;
uniform ivec2 sparseTexSize;

// using point sprites, we are going to splat at patch centers into the framebuffer which is set to blend whatever is already in the framebuffer
void main()
{
	int idx = gl_VertexID;


	ivec2 sparseCoord = ivec2(
		idx % sparseTexSize.x,
		idx / sparseTexSize.x
		);

	// clip space
	gl_Position = 2.0 * (vec4((sparseCoord.x * 4) * invDenseTexSize.x,
					          (sparseCoord.y * 4) * invDenseTexSize.y,
						      0, 1)) - 1.0f;


	gl_PointSize = 8;

	flow = imageLoad(sparseFlowMap, sparseCoord);



}