#version 430 core
layout(points) in;
layout(points, max_vertices = 1) out;

in vec4 geoVertexPositionConfidence[];
in vec4 geoVertexNormalRadius[];
in vec4 geoVertexColorTimeDevice[];

out vec4 outVertexPositionConfidence;
out vec4 outVertexNormalRadius;
out vec4 outVertexColorTimeDevice;


void main()
{
	if (geoVertexPositionConfidence[0].z > 0)
	{
		outVertexPositionConfidence = geoVertexPositionConfidence[0];
		outVertexNormalRadius = geoVertexNormalRadius[0];
		outVertexColorTimeDevice = geoVertexColorTimeDevice[0];
		EmitVertex();
        EndPrimitive(); 
	}

}