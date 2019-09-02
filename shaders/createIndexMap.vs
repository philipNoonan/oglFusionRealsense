#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;

uniform mat4 inversePose[4];
uniform vec4 camPam[4];
uniform vec2 imSize;
uniform float maxDepth;
uniform int time;


flat out vec4 geoVertConf;
flat out vec4 geoNormRadi;
flat out vec4 geoColTimDev;
flat out int geoVertexID;


void main()
{
    vec4 vCurrentPosition = inversePose[0] * vec4(vertexConfidence.xyz, 1.0);
    
    float x = 0;
    float y = 0;
        
    if(vCurrentPosition.z > maxDepth || vCurrentPosition.z < 0) // || time - colorTimeDevice.w > timeDelta)
    {
        x = -10;
        y = -10;
        geoVertexID = 0;
    }
    else
    {
        x = ((((camPam[0].z * vCurrentPosition.x) / vCurrentPosition.z) + camPam[0].x) - (imSize.x * 0.5)) / (imSize.x * 0.5);
        y = ((((camPam[0].w * vCurrentPosition.y) / vCurrentPosition.z) + camPam[0].y) - (imSize.y * 0.5)) / (imSize.y * 0.5);
        geoVertexID = gl_VertexID;
    }
    
	//if (x < -1.0f || x > 1.0f || y < -1.0f || y > 1.0f)
	//{
	//    geoVertexID = 0;
	//}


    gl_Position = vec4(x, y, vCurrentPosition.z / maxDepth, 1.0);
	gl_PointSize = 1.0f;

    geoVertConf = vec4(vCurrentPosition.xyz, vertexConfidence.w);
    geoNormRadi = vec4(normalize(mat3(inversePose[0]) * normalRadius.xyz), normalRadius.w);
	geoColTimDev = colorTimeDevice;

}