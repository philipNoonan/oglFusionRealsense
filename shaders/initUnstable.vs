#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;


out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;


void main()
{
    outVertPosConf = vertexConfidence;
    outVertNormRadi = normalRadius;
	outVertColTimDev = vec4(colorTimeDevice.x, 0, 1, colorTimeDevice.w);
}