#version 430 core

flat in vec4     fragVertConf;
flat in vec4     fragNormRadi;
flat in vec4     fragColTimDev;
flat in int      fragVertexID;

layout(location = 0) out vec4 outVertConf;
layout(location = 1) out vec4 outNormRadi;
layout(location = 2) out vec4 outColTimDev;
layout(location = 3) out int outVertID;

void main() 
{
	outVertConf  = fragVertConf;
	outNormRadi  = fragNormRadi;
	outColTimDev = fragColTimDev;
	outVertID    = fragVertexID;

}