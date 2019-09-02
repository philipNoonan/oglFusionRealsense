#version 430 core
layout(points) in;
layout(points, max_vertices = 1) out;

flat in vec4 geoVertConf[];
flat in vec4 geoNormRadi[];
flat in vec4 geoColTimDev[];
flat in int geoVertexID[];

flat out vec4 fragVertConf;
flat out vec4 fragNormRadi;
flat out vec4 fragColTimDev;
flat out int fragVertexID;


void main()
{
	if (geoVertexID[0] > 0)
	{
	    gl_Position = gl_in[0].gl_Position;
		gl_PointSize = gl_in[0].gl_PointSize;
		fragVertConf = geoVertConf[0];
		fragNormRadi = geoNormRadi[0];
		fragColTimDev = geoColTimDev[0];
		fragVertexID = geoVertexID[0];
		EmitVertex();
        EndPrimitive(); 
	}

}