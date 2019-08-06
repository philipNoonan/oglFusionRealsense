#version 430 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float signMult;

in vec4 vVertConf[];
in vec4 vNormRadi[];
in vec4 vColTimDev[];
in int vTime[];
in int vVertexType[];
in mat4 vMVP[];

out vec3 gVert; // output for rendering
out vec3 gNorm; // output for rendering
out vec2 texCoord;
out float radi;
flat out int unstablePoint;

void main()
{
    if(vVertexType[0] != -1)
    {
		vec3 x = normalize(vec3((vNormRadi[0].y - vNormRadi[0].z), -vNormRadi[0].x, vNormRadi[0].x)) * vNormRadi[0].w * 1.41421356;
        
        vec3 y = cross(vNormRadi[0].xyz, x);

        gNorm = signMult * vNormRadi[0].xyz;


		texCoord = vec2(-1.0, -1.0);
        gl_Position = vMVP[0] * vec4(vVertConf[0].xyz + x, 1.0);
        gVert = vVertConf[0].xyz + x;
        EmitVertex();

        texCoord = vec2(1.0, -1.0);
        gl_Position = vMVP[0] * vec4(vVertConf[0].xyz + y, 1.0);
        gVert = vVertConf[0].xyz + y;
        EmitVertex();

        texCoord = vec2(-1.0, 1.0);
        gl_Position = vMVP[0] * vec4(vVertConf[0].xyz - y, 1.0);
        gVert = vVertConf[0].xyz - y;
        EmitVertex();

        texCoord = vec2(1.0, 1.0);
        gl_Position = vMVP[0] * vec4(vVertConf[0].xyz - x, 1.0);
        gVert = vVertConf[0].xyz - x;
        EmitVertex();
        EndPrimitive();

	}

}