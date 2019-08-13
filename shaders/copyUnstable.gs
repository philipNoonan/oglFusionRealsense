#version 430 core
layout(points) in;
layout(points, max_vertices = 1) out;

in vec4 geoVertexPositionConfidence[];
in vec4 geoVertexNormalRadius[];
in vec4 geoVertexColorTimeDevice[];
flat in int test[];

out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;


void main()
{
	if (test[0] > 0)
	{
		outVertPosConf = geoVertexPositionConfidence[0];
		outVertNormRadi = geoVertexNormalRadius[0];
		outVertColTimDev = geoVertexColorTimeDevice[0];
		EmitVertex();
        EndPrimitive(); 
	}
}