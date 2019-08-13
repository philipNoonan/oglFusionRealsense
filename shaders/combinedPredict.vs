#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;

uniform mat4 inversePose[4];
uniform vec4 camPam[4]; // cx cy fx fy
uniform vec2 imSize;
uniform float maxDepth;
uniform float confThreshold;
uniform int time;
uniform int maxTime;
uniform int timeDelta;


out vec4 fragVertConf;
out vec4 fragNormRadi;
out vec4 fragColTimDev;

vec3 projectPoint(vec3 p)
{
    return vec3(((((camPam[0].z * p.x) / p.z) + camPam[0].x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((camPam[0].w * p.y) / p.z) + camPam[0].y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}

vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam[0].z * p.x) / p.z) + camPam[0].x,
                ((camPam[0].w * p.y) / p.z) + camPam[0].y,
                p.z);
}

void main()
{
    vec4 vPosHome = inversePose[0] * vec4(vertexConfidence.xyz, 1.0);
    
  //  if(vPosHome.z > maxDepth || vPosHome.z < 0 || vertexConfidence.w < confThreshold || time - colorTimeDevice.w > timeDelta || colorTimeDevice.w > maxTime)
  //  {
 //       gl_Position = vec4(1000.0f, 1000.0f, 1000.0f, 1000.0f);
  //      gl_PointSize = 0;
 //   }
 //   else
 //   {
	    gl_Position = vec4(projectPoint(vPosHome.xyz), 1.0);
	    
        fragColTimDev = colorTimeDevice;
	    fragVertConf = vec4(vPosHome.xyz, vertexConfidence.w);
	    fragNormRadi = vec4(normalize(mat3(inversePose[0]) * normalRadius.xyz), normalRadius.w);
	    
	    vec3 x1 = normalize(vec3((fragNormRadi.y - fragNormRadi.z), -fragNormRadi.x, fragNormRadi.x)) * fragNormRadi.w * 1.41421356;
	    
	    vec3 y1 = cross(fragNormRadi.xyz, x1);
	
	    vec4 proj1 = vec4(projectPointImage(vPosHome.xyz + x1), 1.0);
	    vec4 proj2 = vec4(projectPointImage(vPosHome.xyz + y1), 1.0);
	    vec4 proj3 = vec4(projectPointImage(vPosHome.xyz - y1), 1.0);
	    vec4 proj4 = vec4(projectPointImage(vPosHome.xyz - x1), 1.0);
	                
	    vec2 xs = vec2(min(proj1.x, min(proj2.x, min(proj3.x, proj4.x))), max(proj1.x, max(proj2.x, max(proj3.x, proj4.x))));
	    vec2 ys = vec2(min(proj1.y, min(proj2.y, min(proj3.y, proj4.y))), max(proj1.y, max(proj2.y, max(proj3.y, proj4.y))));
	
	    float xDiff = abs(xs.y - xs.x);
	    float yDiff = abs(ys.y - ys.x);
	
	    gl_PointSize = max(0, max(xDiff, yDiff));

  //  }
}