#version 430 core

// from global TFO in global space
//layout(location = 0) in vec4 vertexConfidence;
//layout(location = 1) in vec4 normalRadius;
//layout(location = 2) in vec4 colorTimeDevice;

// from new unstable buffer in global space
layout(std430, binding = 0) buffer newUnstableBuffer
{
    vec4 newUnstable [];
};


layout(std430, binding = 1) buffer globalModelBuffer
{
    vec4 globalModel [];
};

//layout(binding = 1, r32i) uniform image2D outImagePC;
//layout(binding = 2, rgba32f) uniform image2D outImagePC;
//layout(binding = 3, rgba32f) uniform image2D outImagePC;
//layout(binding = 4, rgba32f) uniform image2D outImagePC;

// from 4x index map in depth space
layout(binding = 0, rgba32f) uniform image2D imageVC;
layout(binding = 1, rgba32f) uniform image2D imageNR;
layout(binding = 2, rgba32f) uniform image2D imageCTD;
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

vec3 projectPointImage(vec3 p)
{
    return vec3((((1.0f / camPam.z) * p.x) / p.z) + camPam.x,
                (((1.0f / camPam.w) * p.y) / p.z) + camPam.y,
                p.z);
}



void main()
{
	uint vertID = gl_VertexID;

	if (vertID >= currentGlobalCount) // new unstable
	{
		// these are in global space, they have been transformed by pose from current depth space to global space
		geoVertPosConf   = newUnstable[((vertID - currentGlobalCount) * 3) + 0];
		geoVertNormRadi  = newUnstable[((vertID - currentGlobalCount) * 3) + 1];
		geoVertColTimDev = newUnstable[((vertID - currentGlobalCount) * 3) + 2];
	}
	else
	{	
		geoVertPosConf   = globalModel[((vertID) * 3) + 0];
		geoVertNormRadi  = globalModel[((vertID) * 3) + 1];
		geoVertColTimDev = globalModel[((vertID) * 3) + 2];
	}



	test = 1;

	// 4x size
	vec2 imSize = vec2(imageSize(imageIndex).xy);

	// in depth space
	vec3 localPos = (inversePose[0] * vec4(geoVertPosConf.xyz, 1.0f)).xyz;

    // project to 1x image space
    vec3 pix = projectPointImage(localPos.xyz); // depth space
	
	// in depth space
	vec3 localNorm = normalize(mat3(inversePose[0]) * geoVertNormRadi.xyz);

    int count = 0;
    int zCount = 0;
							
	//imageStore(outImagePC, ivec2(pix.xy), vec4(geoVertColTimDev.xyzw));	


	// IS THIS WORKING??? this should run over all the verts in the gloabl model, checking all the projected poitns in the index map, but what about free space violations???
	// how do we check for points that indexs dont show int he index map
	if(time - geoVertColTimDev.w < timeDelta && localPos.z > 0 && pix.x > 0 && pix.y > 0 && pix.x < (imSize.x / 4.0f) && pix.y < (imSize.y / 4.0f))
	{
       for (int i = int(((pix.x * 4.0f) - 4.0f) + 0.5f); i < int(((pix.x * 4.0f) + 4.0f) + 0.5f); i += 1)
       {
           for (int j = int(((pix.y * 4.0f) - 4.0f) + 0.5f); j < int(((pix.y * 4.0f) + 4.0f) + 0.5f); j += 1)
           {
				uint current = uint(imageLoad(imageIndex, ivec2(i, j)).x);


			   if(current > 0U)
               {

			   		vec4 vertConf = imageLoad(imageVC, ivec2(i, j)); //  depth space
					vec4 colorTime = imageLoad(imageCTD, ivec2(i, j));

					//imageStore(outImagePC, ivec2(pix.xy), vec4(abs(localNorm.z), vertConf.w, vertConf.z, localPos.z));	

					// if the global vert is older than the current vert, and, the global vert has higher confidence, and it is furtehr away, and the diff in depth is greater than 1 cm, and the 
					if(colorTime.z < geoVertColTimDev.z && // .z is the original time this vert was observed, so this is a check to see if the current manifestation is newer or older than the match it has found in the index map
                      vertConf.w > confThreshold && 
                      vertConf.z > localPos.z && 
                      vertConf.z - localPos.z < 0.01 &&
                      sqrt(dot(vertConf.xy - localPos.xy, vertConf.xy - localPos.xy)) < geoVertNormRadi.w * 2.4)
					{ // this sees if close verts should be merged?

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


	if(count > 8 || zCount > 4) // this never gets triggered, and the model gets poinsoned
    {
		//imageStore(outImagePC, ivec2(pix.xy), vec4(0.1,0.2,0.3, 1));	
        test = 0;
		return;
    }
	


	//New unstable point
    if(vertID > currentGlobalCount) // it has recently been appended
    {
        geoVertColTimDev.w = time;
    }

	//Degenerate case or too unstable , degenerate cases having now never been added to the new unstable buffer, since we do merging in data.vs
	// but if the time since vert found is over 20 and its confidence is still low, get rid of it

    if((time - geoVertColTimDev.w) > 20 && geoVertPosConf.w < confThreshold)
    {
        test = 0;
		return;
		//imageStore(outImagePC, ivec2(pix.xy), vec4(0.9,0.7,geoVertPosConf.w, 1));
    }
	
	
	if(geoVertColTimDev.w > 0 && time - geoVertColTimDev.w > timeDelta)
    {
        test = 1;
		//imageStore(outImagePC, ivec2(pix.xy), vec4(0.9,geoVertColTimDev.w,0.8, 1));	
    }

								imageStore(outImagePC, ivec2(pix.xy), vec4(count, zCount, test, 1));	


}