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
uniform vec4 cam; // cx cy fx fy
uniform vec2 imSize;
uniform float maxDepth;
uniform float c_stable;

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


//flat out int index;
out vec4 gsVert;
out vec4 gsNorm;
out vec4 gsColor;
out vec4 gsData;



void main(void)
{
	int index = gl_VertexID;

	vec4 vPosHome = invT * vec4(elems[index].vert.xyz, 1.0);
	float conf = elems[index].data.x;
	float radius = elems[index].data.y;
	
	if(vPosHome.z > maxDepth || vPosHome.z < 0 || conf < c_stable)// || time - vColor.w > timeDelta || vColor.w > maxTime)
    {
        gl_Position = vec4(1000.0f, 1000.0f, 1000.0f, 1000.0f);
        gl_PointSize = 0;
    }
    else
    {
		gl_Position = vec4(projectPoint(vPosHome.xyz), 1.0);

		gsColor = vec4(elems[index].color.xyzw);

		gsVert = vPosHome;

		gsNorm = vec4(normalize(mat3(invT) * elems[index].norm.xyz), 0.0f);

		gsData = elems[index].data.xyzw;


		vec3 x1 = normalize(vec3((gsNorm.y - gsNorm.z), -gsNorm.x, gsNorm.x)) * radius * 1.41421356;
	    
	    vec3 y1 = cross(gsNorm.xyz, x1);
	
	    vec4 proj1 = vec4(projectPointImage(vPosHome.xyz + x1), 1.0);
	    vec4 proj2 = vec4(projectPointImage(vPosHome.xyz + y1), 1.0);
	    vec4 proj3 = vec4(projectPointImage(vPosHome.xyz - y1), 1.0);
	    vec4 proj4 = vec4(projectPointImage(vPosHome.xyz - x1), 1.0);
	                
	    vec2 xs = vec2(min(proj1.x, min(proj2.x, min(proj3.x, proj4.x))), max(proj1.x, max(proj2.x, max(proj3.x, proj4.x))));
	    vec2 ys = vec2(min(proj1.y, min(proj2.y, min(proj3.y, proj4.y))), max(proj1.y, max(proj2.y, max(proj3.y, proj4.y))));
	
	    float xDiff = abs(xs.y - xs.x);
	    float yDiff = abs(ys.y - ys.x);
	
	    gl_PointSize = max(0, max(xDiff, yDiff));
		//gl_PointSize = 100.0f;

	}
}