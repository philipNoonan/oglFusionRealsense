#version 430 core

layout(points) in;
layout(points, max_vertices = 1) out;

flat in vec4 geoVC[];
flat in vec4 geoNR[];
flat in vec4 geoCTD[];

out vec4 outVC;
out vec4 outNR;
out vec4 outCTD;

void main() 
{
		if (geoCTD[0].w == -2) //we dont need the degenerate cases anymore since they have already been merged into the global model
		{
			outVC = geoVC[0];
			outNR = geoNR[0];
			outCTD = geoCTD[0];

			EmitVertex();
			EndPrimitive(); 
		} 
}