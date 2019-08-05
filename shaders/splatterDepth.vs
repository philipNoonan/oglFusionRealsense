#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;

uniform mat4 model[4];
uniform vec4 camPam[4]; // cx cy fx fy
uniform vec2 imSize;
uniform float maxDepth;

out vec3 positions;
out vec3 normals;


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
	vec4 pos = model[0] * vec4(vertexConfidence.xyz, 1.0f);

	 
   // if(pos.z > maxDepth || pos.z < 0 || vPosition.w < confThreshold || time - vColor.w > timeDelta || vColor.w > maxTime)
   // {
   //     gl_Position = vec4(1000.0f, 1000.0f, 1000.0f, 1000.0f);
   //     gl_PointSize = 0;
   // }
   // else
   // {

   gl_Position = vec4(projectPoint(pos.xyz), 1.0f);
   gl_PointSize = normalRadius.w;
   normals = vec3(normalRadius.x * -1.0, normalRadius.y * -1.0, normalRadius.z);
   positions = gl_Position.xyz;
	//}
	//gl_Position = MVP[0] * vec4(vertex_confidence.xyz,1.0f);
	//UV = normal_radius.xy;
}