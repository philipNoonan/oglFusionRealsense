#version 430 core
layout(points) in;
layout(points, max_vertices = 1) out;

in vec4 geoVertPosConf[];
in vec4 geoVertNormRadi[];
in vec4 geoVertColTimDev[];
flat in int toFuse[];

out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;


void main()
{
	if (toFuse[0] == 1)
	{
		outVertPosConf = geoVertPosConf[0];
		outVertNormRadi = geoVertNormRadi[0];
		outVertColTimDev = geoVertColTimDev[0];
		EmitVertex();
        EndPrimitive(); 
	}

}