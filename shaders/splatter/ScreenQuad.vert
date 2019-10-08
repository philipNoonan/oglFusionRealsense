#version 440 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;

layout (location = 2) in vec3 posePoints;

out vec2 vsTexCoord;

uniform bool isYFlip;
uniform int renderType;

uniform mat4 T;
uniform vec4 cam; // cx cy fx fy
uniform vec2 imSize;
uniform float maxDepth;

vec3 projectPoint(vec3 p)
{
    return vec3(((((cam.z * p.x) / p.z) + cam.x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((cam.w * p.y) / p.z) + cam.y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}


void main()
{
	if (renderType == 0)
	{
		vsTexCoord = vec2(vTexCoord.x, isYFlip ? 1.0 - vTexCoord.y : vTexCoord.y);
		gl_Position = vec4(vPosition, 1.0);
	}
	else if (renderType == 1)
	{
		vec4 transPoints = T * vec4(posePoints, 1.0f);
		gl_Position = vec4(projectPoint(transPoints.xyz), 1.0f);
	}
	else 
	{
		gl_Position = vec4(10000,10000,10000, 1.0);
	}

}