#version 430 core

in vec4 vVertConf;
in vec4 vNormRadi;
in vec4 vColTimDev;
flat in int vVertexId;

layout(location = 0) out vec4 outVertConf;
layout(location = 1) out vec4 outNormRadi;
layout(location = 2) out vec4 outColTimDev;
layout(location = 3) out int outVertID;

void main() 
{
	outVertID = vVertexId;
	outVertConf = vVertConf;
	outNormRadi = vNormRadi;
	outColTimDev = vColTimDev;
}