#version 430 core

// input depth frame data
// layout(location = 0) in vec4 vertexConfidence;
// layout(location = 1) in vec4 normalRadius;
// layout(location = 2) in vec4 colorTimeDevice;

// global buffer
layout(std430, binding = 0) buffer globalBuffer
{
    vec4 interleavedGlobalBuffer [];
};

// new depth frame buffer
layout(std430, binding = 1) buffer depthBuffer
{
    vec4 interleavedDepthBuffer [];
};

layout(std430, binding = 2) buffer indexMatchingBuffer
{
    int updateIndexMatchingBuffer [];
};

layout(binding = 0, rgba32f) uniform image2D outImagePC;

out vec4 geoVertPosConf;
out vec4 geoVertNormRadi;
out vec4 geoVertColTimDev;
flat out int toFuse;

vec4 camPam = vec4(424, 240, 424, 424);
uniform float texDim;
uniform int time;

vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam.z * p.x) / p.z) + camPam.x,
                ((camPam.w * p.y) / p.z) + camPam.y,
                p.z);
}

float encodeColor(vec3 c)
{
    int rgb = int(round(c.x * 255.0f));
    rgb = (rgb << 8) + int(round(c.y * 255.0f));
    rgb = (rgb << 8) + int(round(c.z * 255.0f));
    return float(rgb);
}

vec3 decodeColor(float c)
{
    vec3 col;
    col.x = float(int(c) >> 16 & 0xFF) / 255.0f;
    col.y = float(int(c) >> 8 & 0xFF) / 255.0f;
    col.z = float(int(c) & 0xFF) / 255.0f;
    return col;
}

void main()
{
	// this gets run for every vertex in the global model
	
	toFuse = 0;

	// get the (if any) matching depth vertices index from the global-size matched update index buffer 
	int matchingIndex = updateIndexMatchingBuffer[gl_VertexID];

	vec4 globalVertConf  = interleavedGlobalBuffer[(gl_VertexID * 3)];
	vec4 globalNormRadi  = interleavedGlobalBuffer[(gl_VertexID * 3) + 1];
	vec4 globalColDevTim = interleavedGlobalBuffer[(gl_VertexID * 3) + 2];
		
	//Do averaging here
	if (matchingIndex != 0) // this defines whether this is a stable (+ve) or unstable (-ve) or non matching (0)
	{

		vec4 depthVertConf  = interleavedDepthBuffer[(abs(matchingIndex) * 3)];
		vec4 depthNormRadi  = interleavedDepthBuffer[(abs(matchingIndex) * 3) + 1];
		vec4 depthColDevTim = interleavedDepthBuffer[(abs(matchingIndex) * 3) + 2];

		vec3 pix = projectPointImage(depthVertConf.xyz);


		toFuse = 1;

		float globalConfidence = depthVertConf.w; // c_k
		vec3 globalVertex = depthVertConf.xyz; // v_k
        
		float depthConfidence = depthVertConf.w; // a
		vec3 depthVertex = depthVertConf.xyz; // v_g
        

		if(matchingIndex > 0 && depthNormRadi.w < (1.0 + 0.5) * globalNormRadi.w)
		{



			// vert conf output
			geoVertPosConf = vec4(((globalConfidence * globalVertex) + (depthConfidence * depthVertex)) / (globalConfidence + depthConfidence), globalConfidence + depthConfidence);
	        


			vec3 globalCol = decodeColor(globalColDevTim.x);
			vec3 depthCol = decodeColor(depthColDevTim.x);
           
			vec3 avgColor = ((globalConfidence * globalCol.xyz) + (depthConfidence * depthCol.xyz)) / (globalConfidence + depthConfidence);

			geoVertNormRadi = ((globalConfidence * globalNormRadi) + (depthConfidence * depthNormRadi)) / (globalConfidence + depthConfidence);
			geoVertNormRadi = vec4(normalize(geoVertNormRadi.xyz), geoVertNormRadi.w);
			
			imageStore(outImagePC, ivec2(pix.xy), vec4(geoVertPosConf.www, 1));

			geoVertColTimDev = vec4(encodeColor(avgColor), globalColDevTim.y, globalColDevTim.z, time);
		}
		else
		{
			imageStore(outImagePC, ivec2(pix.xy), vec4(0.6,0.2,0.3,1));

			geoVertPosConf = vec4(globalVertConf.xyz, globalConfidence + depthConfidence);
			geoVertNormRadi = globalNormRadi;
			geoVertColTimDev = vec4(globalColDevTim.xyz, time);
		}
	}
	else // no matching in depth frame so just copy over
	{
		//This point isn't being updated, so just transfer it
		geoVertPosConf = globalVertConf;
		geoVertNormRadi = globalNormRadi;
		geoVertColTimDev = globalColDevTim;
	}

	//updateIndexMatchingBuffer[gl_VertexID] = 0;

}




	//imageStore(outImagePC, ivec2(i / 4, j / 4), vec4(dist, 0, 0, 1));
	

    // need to wipe the update buffer after every update, this should always ensure that the active buffer values are set off after use
  //  updateIndexInterleaved[(gl_VertexID * 3)] = vec4(0.0f);
  //  updateIndexInterleaved[(gl_VertexID * 3) + 1] = vec4(0.0f);
  //  updateIndexInterleaved[(gl_VertexID * 3) + 2] = vec4(0.0f);
//}
