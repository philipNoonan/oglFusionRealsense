#version 430 core

layout(points) in;
layout(points, max_vertices = 1) out;

flat in int geoVertID[];

flat in vec4 geoVC[];
flat in vec4 geoNR[];
flat in vec4 geoCTD[];

flat in int updateID[];

out vec4 outVC;
out vec4 outNR;
out vec4 outCTD;
out uint outVertID;

void main() 
{
		if (updateID[0] > 0)
		{
			outVC = geoVC[0];
			outNR = geoNR[0];
			outCTD = geoCTD[0];
			outVertID = geoVertID[0];
			EmitVertex();
			EndPrimitive(); 
		} 
}