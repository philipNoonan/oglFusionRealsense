#version 430 core

// from global TFO
layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;

// from new unstable buffer
layout(std430, binding = 0) buffer newUnstableBuffer
{
    vec4 newUnstable [];
};




//layout(binding = 1, r32i) uniform image2D outImagePC;
//layout(binding = 2, rgba32f) uniform image2D outImagePC;
//layout(binding = 3, rgba32f) uniform image2D outImagePC;
//layout(binding = 4, rgba32f) uniform image2D outImagePC;

// from 4x index map

//layout (binding = 0) uniform isampler2D indexSampler; // 4x
//layout (binding = 1) uniform sampler2D vertConfSampler; // 4x
//layout (binding = 2) uniform sampler2D normRadiSampler; // 4x
//layout (binding = 3) uniform sampler2D colTimDevSampler; // 4x

layout(binding = 0, rgba32f) uniform image2D imageVC;
layout(binding = 1, rgba32f) uniform image2D imageNR;
layout(binding = 2, rgba32f) uniform image2D imageCDT;
layout(binding = 3, r32i)    uniform iimage2D imageIndex; 

layout(binding = 4, rgba32f) uniform image2D outImagePC;



flat out vec4 geoVertPosConf;
flat out vec4 geoVertNormRadi;
flat out vec4 geoVertColTimDev;
flat out int test;

uniform int time;
uniform int timeDelta;
uniform mat4 inversePose[4];
uniform vec4 camPam; //cx, cy, 1 / fx, 1 / fy
uniform float confThreshold;
uniform uint currentGlobalCount;
//uniform uint currentNewUnstableCount;

vec3 projectPointImage(vec3 p)
{
    return vec3((((1.0f / camPam.z) * p.x) / p.z) + camPam.x,
                (((1.0f / camPam.w) * p.y) / p.z) + camPam.y,
                p.z);
}



void main()
{
	uint vertID = gl_VertexID;

	if (vertID > currentGlobalCount) // new unstable
	{
		geoVertPosConf   = newUnstable[((vertID - currentGlobalCount) * 3) + 0];
		geoVertNormRadi  = newUnstable[((vertID - currentGlobalCount) * 3) + 1];
		geoVertColTimDev = newUnstable[((vertID - currentGlobalCount) * 3) + 2];

		//vec3 pixx = projectPointImage(geoVertPosConf.xyz);
		//imageStore(outImagePC, ivec2(pixx.xy), vec4(0.1,0.6,0.5, 1));	
	}
	else
	{
	
	    geoVertPosConf = vertexConfidence;
		geoVertNormRadi = normalRadius;
		geoVertColTimDev = colorTimeDevice;
	
		
	
	}



	test = 1;
	vec2 imSize = vec2(imageSize(imageIndex).xy);
	vec3 localPos = (inversePose[0] * vec4(geoVertPosConf.xyz, 1.0f)).xyz;

    // project to 1x image space
    vec3 pix = projectPointImage(localPos.xyz); // MAYBE THIS SHOULD BE TRANSFORMED??
	
	vec3 localNorm = normalize(mat3(inversePose[0]) * geoVertNormRadi.xyz);

    int count = 0;
    int zCount = 0;

			vec3 pixx = projectPointImage(geoVertPosConf.xyz);




	if(time - geoVertColTimDev.z < timeDelta && localPos.z > 0 && pix.x > 0 && pix.y > 0 && pix.x < imSize.x && pix.y < imSize.y)
	{

        for (int i = int((pix.x * 4) - 2); i < int((pix.x * 4) + 2); i += 1)
        {
            for (int j = int((pix.y * 4) - 2); j < int((pix.y * 4) + 2); j += 1)
            {
                //uint current = uint(texelFetch(indexSampler, ivec2(i, j), 0));
				uint current = uint(imageLoad(imageIndex, ivec2(i, j)).x);

								//	vec4 vertConf = textureLod(vertConfSampler, vec2(i, j), 0);


			   if(current > 0U)
               {
			   					           	//imageStore(outImagePC, ivec2(i / 4, j / 4), vec4(current, 0, 0.5, 1));	

			   		vec4 vertConf = imageLoad(imageVC, ivec2(i, j));
					vec4 colorTime = imageLoad(imageNR, ivec2(i, j));

					//vec4 vertConf = textureLod(vertConfSampler, vec2(i, j), 0);
					//vec4 colorTime = textureLod(colTimDevSampler, vec2(i, j), 0);

					if(colorTime.z < geoVertColTimDev.z && 
                      vertConf.w > confThreshold && 
                      vertConf.z > localPos.z && 
                      vertConf.z - localPos.z < 0.01 &&
                      sqrt(dot(vertConf.xy - localPos.xy, vertConf.xy - localPos.xy)) < geoVertNormRadi.w * 1.4)
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
				imageStore(outImagePC, ivec2(pix.xy), vec4(0.1,0.2,0.3, 1));	

        test = 0;
    }
					imageStore(outImagePC, ivec2(pixx.xy), vec4(geoVertColTimDev.xyzw));	


	//New unstable point
    if(vertID > currentGlobalCount) // it has recently been appended
    {
        geoVertColTimDev.w = time;
		imageStore(outImagePC, ivec2(pix.xy), vec4(0.7,0.2,0.3, 1));	

    }

	//Degenerate case or too unstable , degenerate cases having now never been added to the new unstable buffer, since we do merging in data.vs
	// but if the time since vert found is over 20 and its confidence is still low, get rid of it
    if((time - geoVertColTimDev.w) > 20 && vertexConfidence.w < confThreshold)
    {
        test = 0;
			imageStore(outImagePC, ivec2(pix.xy), vec4(0.9,0.7,0.8, 1));	

    }

	if(geoVertColTimDev.w > 0 && time - geoVertColTimDev.w > timeDelta)
    {
        test = 1;
					imageStore(outImagePC, ivec2(pix.xy), vec4(0.9,geoVertColTimDev.w,0.8, 1));	

    }

}