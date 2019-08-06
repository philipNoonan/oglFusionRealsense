#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;

uniform mat4 MVP[4];
uniform float threshold;
uniform int vertexType;
uniform int time;
uniform int unstable;


out vec4 vVertConf;
out vec4 vNormRadi;
out vec4 vColTimDev;
out mat4 vMVP;
out int vTime;
out int vVertexType;


void main()
{
	if (vertexConfidence.w > threshold || unstable == 1)
	{
		vVertexType = vertexType;
		vTime = time;
		vMVP = MVP[0]; // do we need this?
		vVertConf = vertexConfidence;
		vNormRadi = normalRadius;
		gl_Position = MVP[0] * vec4(vertexConfidence.xyz, 1.0f);
	}
	else
	{
		vVertexType = -1;
	}


}
