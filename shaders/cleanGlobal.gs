#version 430 core
layout(points) in;
layout(points, max_vertices = 1) out;

flat in vec4 geoVertPosConf[];
flat in vec4 geoVertNormRadi[];
flat in vec4 geoVertColTimDev[];
flat in int test[];

out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;

void main()
{
	if (test[0] > 0)
	{
		outVertPosConf = geoVertPosConf[0];
		outVertNormRadi = geoVertNormRadi[0];
		outVertColTimDev = geoVertColTimDev[0];
		EmitVertex();
        EndPrimitive(); 
	}
}