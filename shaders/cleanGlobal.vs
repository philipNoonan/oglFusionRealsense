#version 430 core

// from global TFO
layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;


uniform int time;
uniform int timeDelta;
uniform mat4 inversePose[4];
uniform vec4 camPam; //cx, cy, fx, fy
uniform float confThreshold;
uniform uint currentGlobalCount;
uniform uint currentNewUnstableCount;


// from 4x index map
uniform usampler2D indexSampler;
uniform sampler2D vertConfSampler;
uniform sampler2D colTimDevSampler;
uniform sampler2D normRadiSampler;

flat out vec4 geoVertPosConf;
flat out vec4 geoVertNormRadi;
flat out vec4 geoVertColTimDev;
flat out int test;



vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam.z * p.x) / p.z) + camPam.x,
                ((camPam.w * p.y) / p.z) + camPam.y,
                p.z);
}



void main()
{
    geoVertPosConf = vertexConfidence;
    geoVertNormRadi = normalRadius;
	geoVertColTimDev = colorTimeDevice;

	uint vertID = gl_VertexID;

	test = 1;
	vec2 imSize = vec2(textureSize(indexSampler, 0));
	vec3 localPos = (inversePose[0] * vec4(vertexConfidence.xyz, 1.0f)).xyz;

    // project to 1x image space
    vec3 pix = projectPointImage(vertexConfidence.xyz); // MAYBE THIS SHOULD BE TRANSFORMED??
	
	vec3 localNorm = normalize(mat3(inversePose[0]) * normalRadius.xyz);

    int count = 0;
    int zCount = 0;

	if(time - colorTimeDevice.z < timeDelta && localPos.z > 0 && pix.x > 0 && pix.y > 0 && pix.x < imSize.x && pix.y < imSize.y)
	{
        for (int i = int((pix.x * 4) - 2); i < int((pix.x * 4) + 2); i += 1)
        {
            for (int j = int((pix.y * 4) - 2); j < int((pix.y * 4) + 2); j += 1)
            {
                uint current = uint(texelFetch(indexSampler, ivec2(i, j), 0));
           
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
    if(vertID > currentGlobalCount) // it has recently been appended
    {
        geoVertColTimDev.w = time;
    }

	//Degenerate case or too unstable
    if((geoVertColTimDev.w == -1 || ((time - geoVertColTimDev.w) > 20 && vertexConfidence.w < confThreshold)))
    {
        test = 0;
    }

	if(colorTimeDevice.w > 0 && time - colorTimeDevice.w > timeDelta)
    {
        test = 1;
    }

}