#version 430

// Data structure
struct gMapData
{
	vec4 data;	// Confidence, radius, timestamp, and empty data 
	vec4 vert;	// Vertex
	vec4 norm;	// Normal
	vec4 color;	// Color
};
// Distance global map
layout(std430, binding = 0) buffer gMap
{
	gMapData elems[];
};
uniform mat4 invT;
uniform mat4 P;

uniform vec2 imSize;
uniform vec4 cam;
uniform float maxDepth;

flat out int idx;

vec3 projectPoint(vec3 p)
{
    return vec3(((((cam.z * p.x) / p.z) + cam.x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((cam.w * p.y) / p.z) + cam.y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}

vec3 projectPointImage(vec3 p)
{
    return vec3(((cam.z * p.x) / p.z) + cam.x,
                ((cam.w * p.y) / p.z) + cam.y,
                p.z);
}
vec4 transPtForGL(vec4 v)
{
	v = invT * v;
	return vec4(projectPoint(vec3(v.xy, v.z)), 1.0f);
	//return P * vec4(v.xy, -v.z, 1.0);
}

void main()
{
	idx = gl_VertexID;

	vec4 tempPos = transPtForGL(elems[idx].vert);

	if (tempPos.z < 0)
	{
		gl_Position = vec4(10000,10000,0,0);
	}
	else
	{
		gl_Position = tempPos;
	}

	
}