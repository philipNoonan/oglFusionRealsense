#version 430 core

layout(points) in;
layout(points, max_vertices = 1) out;

in vec4 geoVertPosConf[];
in vec4 geoVertNormRadi[];
in vec4 geoVertColTimDev[];
flat in int updateId[];

out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;
flat out int outUpdateId;

void main() 
{
    //Emit a vertex if either we have an update to store, or a new unstable vertex to store
    if(updateId[0] > 0)
    {
	    outVertPosConf = geoVertPosConf[0];
	    outVertNormRadi = geoVertNormRadi[0];
	    outVertColTimDev = geoVertColTimDev[0];
	    outUpdateId = updateId[0];
	    
	    //This will be -10, -10 (offscreen) for new unstable vertices, so they don't show in the fragment shader
	    gl_Position = gl_in[0].gl_Position;

	    EmitVertex();
	    EndPrimitive(); 
    }
}