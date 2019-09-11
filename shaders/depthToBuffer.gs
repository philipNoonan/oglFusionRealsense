#version 430 core
layout(points) in;
layout(points, max_vertices = 1) out;

in vec4 geoVertexPositionConfidence[];
in vec4 geoVertexNormalRadius[];
in vec4 geoVertexColorTimeDevice[];

flat in ivec2 imageCoord[];

out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;

layout(binding = 0, rgba32f) uniform image2D outImagePC;
layout(binding = 1, rgba32f) uniform image2D outImageNR;
layout(binding = 2, rgba32f) uniform image2D outImageCTD;


void main()
{
	if (geoVertexPositionConfidence[0].z > 0)
	{

		imageStore(outImagePC, imageCoord[0], geoVertexPositionConfidence[0]);
		imageStore(outImageNR, imageCoord[0], geoVertexNormalRadius[0]);
		imageStore(outImageCTD, imageCoord[0], geoVertexColorTimeDevice[0]);

		outVertPosConf = geoVertexPositionConfidence[0];
		outVertNormRadi = geoVertexNormalRadius[0];
		outVertColTimDev = geoVertexColorTimeDevice[0];

		EmitVertex();
        EndPrimitive(); 
	}

}