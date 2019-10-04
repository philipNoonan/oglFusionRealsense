#version 420

layout (binding = 0) uniform sampler2D vertMap;
layout (binding = 1) uniform sampler2D normMap;

uniform mat4 T;
uniform mat4 P;

uniform int mip;

out vec3 vsVert;
out vec3 vsNorm;

void main()
{
	ivec2 resolution = textureSize(vertMap, 0);
	vec2 texcoord = vec2(
		(float(gl_VertexID % resolution.x) + 0.5f) / float(resolution.x),
		(float(gl_VertexID / resolution.x) + 0.5f) / float(resolution.y)
	);	// NOTE: Texture coordinates: Left bottom origin

	vsVert = mat4x3(T) * textureLod(vertMap, texcoord, mip);
	vsNorm = mat3(T) * textureLod(normMap, texcoord, mip).xyz;

	gl_Position = P * vec4(vsVert.xy, -vsVert.z, 1.0);
}