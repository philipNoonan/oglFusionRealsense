#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;


out vec4 geoVertPosConf;
out vec4 geoVertNormRadi;
out vec4 geoVertColTimDev;
flat out int test;

uniform int time;
uniform int timeDelta;

uniform float scale;
uniform mat4 inversePose[4];
uniform vec4 camPam[4]; //cx, cy, fx, fy
uniform vec2 imSize;
uniform float confThreshold;

uniform usampler2D indexSampler;
uniform sampler2D vertConfSampler;
uniform sampler2D colTimDevSampler;
uniform sampler2D normRadiSampler;
uniform sampler2D depthSampler;

void main()
{
    geoVertPosConf = vertexConfidence;
    geoVertNormRadi = normalRadius;
	geoVertColTimDev = colorTimeDevice;

	test = 1;

	vec3 localPos = (inversePose[0] * vec4(vertexConfidence.xyz, 1.0f)).xyz;

    float x = ((camPam[0].z * localPos.x) / localPos.z) + camPam[0].x;
    float y = ((camPam[0].w * localPos.y) / localPos.z) + camPam[0].y;

    vec3 localNorm = normalize(mat3(inversePose[0]) * normalRadius.xyz);

    float indexXStep = (1.0f / (imSize.x * scale)) * 0.5f;
    float indexYStep = (1.0f / (imSize.y * scale)) * 0.5f;

    float windowMultiplier = 2;
	
    int count = 0;
    int zCount = 0;

	if(time - colorTimeDevice.z < timeDelta && localPos.z > 0 && x > 0 && y > 0 && x < imSize.x && y < imSize.y)
	{
		for(float i = x / imSize.x - (scale * indexXStep * windowMultiplier); i < x / imSize.x + (scale * indexXStep * windowMultiplier); i += indexXStep)
        {
            for(float j = y / imSize.y - (scale * indexYStep * windowMultiplier); j < y / imSize.y + (scale * indexYStep * windowMultiplier); j += indexYStep)
            {
               uint current = uint(textureLod(indexSampler, vec2(i, j), 0));
               
			   if(current > 0U)
               {
					vec4 vertConf = textureLod(vertConfSampler, vec2(i, j), 0);
					vec4 colorTime = textureLod(colTimDevSampler, vec2(i, j), 0);

					if(colorTime.z < colorTimeDevice.z && 
                      vertConf.w > confThreshold && 
                      vertConf.z > localPos.z && 
                      vertConf.z - localPos.z < 0.01 &&
                      sqrt(dot(vertConf.xy - localPos.xy, vertConf.xy - localPos.xy)) < normalRadius.w * 1.4)
					{
						count++;				
					}
					if(colorTime.w == time &&
                      vertConf.w > confThreshold && 
                      vertConf.z > localPos.z && 
                      vertConf.z - localPos.z > 0.01 &&
                      abs(localNorm.z) > 0.85f)
					{
						zCount++;
					}
				}
			}
		}
	}
	if(count > 8 || zCount > 4)
    {
        test = 0;
    }

	//New unstable point
    if(colorTimeDevice.w == -2)
    {
        geoVertColTimDev.w = time;
    }

	//Degenerate case or too unstable
    if((colorTimeDevice.w == -1 || ((time - colorTimeDevice.w) > 20 && vertexConfidence.w < confThreshold)))
    {
        test = 0;
    }

	if(colorTimeDevice.w > 0 && time - colorTimeDevice.w > timeDelta)
    {
        test = 1;
    }



}