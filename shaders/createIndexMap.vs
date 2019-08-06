#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;

uniform mat4 MVP[4];
uniform mat4 inversePose[4];
uniform vec4 camPam[4];
uniform vec2 imSize;
uniform float threshold;
uniform int vertexType;
uniform int time;
uniform int unstable;


out vec4 vVertConf;
out vec4 vNormRadi;
out mat4 vMVP;
out int vTime;
out int vVertexType;


void main()
{
    vec4 vCurrentPosition = inversePose[0] * vec4(vertexConfidence.xyz, 1.0);
    
    float x = 0;
    float y = 0;
        
    if(vCurrentPosition.z > maxDepth || vCurrentPosition.z < 0)// || time - vColorTime.w > timeDelta)
    {
        x = -10;
        y = -10;
        vertexId = 0;
    }
    else
    {
        x = ((((camPam[0].z * vCurrentPosition.x) / vCurrentPosition.z) + camPam[0].x) - (imSize.x * 0.5)) / (imSize.x * 0.5);
        y = ((((camPam[0].w * vCurrentPosition.y) / vCurrentPosition.z) + camPam[0].y) - (imSize.y * 0.5)) / (imSize.y * 0.5);
        vertexId = gl_VertexID;
    }
    
    gl_Position = vec4(x, y, vCurrentPosition.z / maxDepth, 1.0);

    vPosition0 = vec4(vCurrentPosition.xyz, vertexConfidence.w);
    vColorTimeDevice0 = vColorTimeDevice;
    vNormRad0 = vec4(normalize(mat3(t_inv) * vNormRad.xyz), vNormRad.w);
}