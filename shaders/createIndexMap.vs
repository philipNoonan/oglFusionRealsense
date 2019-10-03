#version 430 core

layout(std430, binding = 0) buffer feedbackBuffer
{
    vec4 interleavedData [];
};

uniform mat4 inversePose[4];
uniform vec4 camPam; // cx, cy, fx, fy
uniform vec2 imSize;
uniform float maxDepth;
uniform int time;

layout(binding=0, rgba32f) uniform image2D outputVertConf;
layout(binding=1, rgba32f) uniform image2D outputNormRadi;
layout(binding=2, rgba32f) uniform image2D outputColTimDev;
layout(binding=3, r32i)    uniform iimage2D outputVertexID; 

vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam.z * p.x) / p.z) + camPam.x,
                ((camPam.w * p.y) / p.z) + camPam.y,
                p.z);
}

void main()
{

	int vertID = int(gl_GlobalInvocationID.x); // 0 to max number of valid depth verts

	vec4 vertexConfidence = interleavedData[(vertID * 3)];
	vec4 normalRadius = interleavedData[(vertID * 3) + 1];
	vec4 colorTimeDevice = interleavedData[(vertID * 3) + 2];

	int geoVertexID;

    vec4 vCurrentPosition = inversePose[0] * vec4(vertexConfidence.xyz, 1.0);
    vec3 pix;

    float x = 0;
    float y = 0;
        
    if(vCurrentPosition.z > maxDepth || vCurrentPosition.z < 0 || time - colorTimeDevice.w > timeDelta)
    {
        x = -10;
        y = -10;
        geoVertexID = 0;
    }
    else // get projected image pixel point in 4x image
    {
		pix = projectPointImage(vCurrentPosition.xyz);
        //x = ((((camPam.z * vCurrentPosition.x) / vCurrentPosition.z) + camPam.x) - (imSize.x * 0.5)) / (imSize.x * 0.5);
        //y = ((((camPam.w * vCurrentPosition.y) / vCurrentPosition.z) + camPam.y) - (imSize.y * 0.5)) / (imSize.y * 0.5);
        geoVertexID = gl_VertexID;
    }
    

    //gl_Position = vec4(x, y, vCurrentPosition.z / maxDepth, 1.0);
	//gl_PointSize = 1.0f;

	if (geoVertexID > 0)
	{
		imageStore(outputVertConf, ivec2(pix.xy), vec4(vCurrentPosition.xyz, vertexConfidence.w));
		imageStore(outputNormRadi, ivec2(pix.xy), vec4(normalize(mat3(inversePose[0]) * normalRadius.xyz), normalRadius.w));
		imageStore(outputColTimDev, ivec2(pix.xy), vec4(colorTimeDevice));
		imageStore(outputVertexID, ivec2(pix.xy), ivec4(geoVertexID));
	}

   // geoVertConf = vec4(vCurrentPosition.xyz, vertexConfidence.w);
   // geoNormRadi = vec4(normalize(mat3(inversePose[0]) * normalRadius.xyz), normalRadius.w);
   // geoColTimDev = colorTimeDevice;

}