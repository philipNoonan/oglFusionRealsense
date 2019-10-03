#version 430 core

// the global model which contains stable and unstable vertices
//layout(location = 0) in vec4 vertexConfidence;
//layout(location = 1) in vec4 normalRadius;
//layout(location = 2) in vec4 colorTimeDevice;

layout(std430, binding = 0) buffer globalModelBuffer
{
    vec4 globalModel [];
};

uniform mat4 inversePose[4];
uniform vec4 camPam; // cx cy fx fy
uniform vec2 imSize;
uniform float maxDepth;
uniform float confThreshold;
uniform int time;
uniform int maxTime;
uniform int timeDelta;

layout(binding = 0, rgba32f) uniform image2D outImagePC;

out vec4 fragVertConf;
out vec4 fragNormRadi;
out vec4 fragColTimDev;

vec3 projectPoint(vec3 p)
{
    return vec3(((((camPam.z * p.x) / p.z) + camPam.x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((camPam.w * p.y) / p.z) + camPam.y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}

vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam.z * p.x) / p.z) + camPam.x,
                ((camPam.w * p.y) / p.z) + camPam.y,
                p.z);
}

void main()
{
   // get the position of the global vert in the current estimated camera position fov

    vec4 vertexConfidence	= globalModel[(gl_VertexID * 3) + 0];
	vec4 normalRadius		= globalModel[(gl_VertexID * 3) + 1];
	vec4 colorTimeDevice	= globalModel[(gl_VertexID * 3) + 2];

	vec4 vPosHome = inversePose[0] * vec4(vertexConfidence.xyz, 1.0);

	// THIS TIME THING IS A HACK || (vertexConfidence.w < confThreshold && time > 10)
    if(vPosHome.z > maxDepth || vPosHome.z < 0 || vertexConfidence.w < confThreshold || time - colorTimeDevice.w > timeDelta || colorTimeDevice.w > maxTime)
    {
        gl_Position = vec4(1000.0f, 1000.0f, 1000.0f, 1000.0f);
        gl_PointSize = 0;
    }
    else
    {
		// project the current view global point to opengl image space (-1 to 1)
	    gl_Position = vec4(projectPoint(vPosHome.xyz), 1.0);
		vec3 pix = projectPointImage(vPosHome.xyz);


	    fragColTimDev = colorTimeDevice;
		fragVertConf = vec4(vPosHome.xyz, vertexConfidence.w);
		fragNormRadi = vec4(normalize(mat3(inversePose[0]) * normalRadius.xyz), normalRadius.w);

		//imageStore(outImagePC, ivec2(pix.xy), vec4(vertexConfidence.www,1));

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
		


    }
}