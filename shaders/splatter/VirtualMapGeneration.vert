#version 420

layout (binding = 0) uniform sampler2D vertMap;
layout (binding = 1) uniform sampler2D normMap;

uniform mat4 T;
uniform mat4 P;

uniform int mip;

uniform vec2 imSize;
uniform vec4 cam;
uniform float maxDepth;

vec3 projectPoint(vec3 p)
{
    return vec3(((((cam.z * p.x) / p.z) + cam.x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((cam.w * p.y) / p.z) + cam.y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}

vec3 projectPointImage(vec3 p)
{
    return vec3(((cam.z * p.x) / p.z) + cam.x,
                ((cam.w * p.y) / p.z) + cam.y,
                p.z);
}


out vec3 vsVert;
out vec3 vsNorm;

void main()
{
	ivec2 resolution = textureSize(vertMap, mip);
	vec2 texcoord = vec2(
		(float(gl_VertexID % resolution.x) + 0.5f) / float(resolution.x),
		(float(gl_VertexID / resolution.x) + 0.5f) / float(resolution.y)
	);	// NOTE: Texture coordinates: Left bottom origin

	vsVert = mat4x3(T) * textureLod(vertMap, texcoord, mip);
	vsNorm = mat3(T) * textureLod(normMap, texcoord, mip).xyz;

	//gl_Position = P * vec4(vsVert.xy, -vsVert.z, 1.0);
	gl_Position = vec4(projectPoint(vec3(vsVert.xy, vsVert.z)), 1.0f);

}