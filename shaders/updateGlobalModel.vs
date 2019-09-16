#version 430 core

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

// output index matching buffer
layout(std430, binding = 2) buffer indexMatchingBuffer
{
    int updateIndexMatchingBuffer [];
};

layout(std430, binding = 3) buffer indexNewUnstableBuffer
{
    int updateIndexNewUnstableBuffer [];
};

layout(binding = 0, rgba32f) uniform image2D outImagePC;

flat out vec4 outputVC;
flat out vec4 outputNR;
flat out vec4 outputCDT;
flat out int toFuse;

uniform vec4 camPam;
uniform float texDim;
uniform uint time;
uniform uint timeDelta;
uniform uint currentGlobalCount;
uniform uint currentNewUnstableCount;

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
    //int toFuse = 0;

    // get the (if any) matching depth vertices index from the global-size matched update index buffer 

    uint invocationID = gl_GlobalInvocationID.x;

    if (invocationID > currentGlobalNumber + currentNewUnstableNumber)
    {
        return;
    }

    vec4 globalVertConf;
    vec4 globalNormRadi;
    vec4 globalColDevTim;

    vec4 depthVertConf;
    vec4 depthNormRadi;
    vec4 depthColDevTim;


    if (invocationID > currentGlobalNumber) // new unstable point
    {
        
        uint newUnstableInvocationID = invocationID - currentGlobalNumber; // CHECK THAT THIS ISNT A ERROR OF 1 OUT

        uint newUnstableID = updateIndexNewUnstableBuffer[newUnstableInvocationID];

        depthVertConf = interleavedDepthBuffer[(newUnstableID * 3)];
        depthNormRadi = interleavedDepthBuffer[(newUnstableID * 3) + 1];
        depthColDevTim = interleavedDepthBuffer[(newUnstableID * 3) + 2];

        //interleavedGlobalBuffer[(invocationID * 3) + 0] = depthVertConf;
        //interleavedGlobalBuffer[(invocationID * 3) + 1] = depthNormRadi;
        //interleavedGlobalBuffer[(invocationID * 3) + 2] = depthColDevTim;

		outputVC = depthVertConf;
        outputNR = depthNormRadi;
        outputCDT = depthColDevTim;

        updateIndexNewUnstableBuffer[newUnstableInvocationID] = 0; // SGHOULD THIS BE ZERO?
        vec3 pix0 = projectPointImage(depthVertConf.xyz);

        imageStore(outImagePC, ivec2(pix0.xy), vec4(0.6, 0.5, 0.3, 1));


    }
    else // we either have a stable point to merge, or just need to copy/do nothing
    {
        int matchingIndex = updateIndexMatchingBuffer[invocationID];

        globalVertConf = interleavedGlobalBuffer[(invocationID * 3)];
        globalNormRadi = interleavedGlobalBuffer[(invocationID * 3) + 1];
        globalColDevTim = interleavedGlobalBuffer[(invocationID * 3) + 2];

        if (matchingIndex == 0) // no point to merge with
        {
            // copy accross, or rather, dont overwrite this global vert in the buffer
            vec3 pix1 = projectPointImage(globalVertConf.xyz);

            imageStore(outImagePC, ivec2(pix1.xy), vec4(0.6, 0.2, 0.7, 1));

        }
        else // point to merge with
        {
            depthVertConf = interleavedDepthBuffer[(abs(matchingIndex) * 3)];
            depthNormRadi = interleavedDepthBuffer[(abs(matchingIndex) * 3) + 1];
            depthColDevTim = interleavedDepthBuffer[(abs(matchingIndex) * 3) + 2];

            vec3 pix = projectPointImage(depthVertConf.xyz);

            float globalConfidence = depthVertConf.w; // c_k
            vec3 globalVertex = depthVertConf.xyz; // v_k

            float depthConfidence = depthVertConf.w; // a
            vec3 depthVertex = depthVertConf.xyz; // v_g

            if (depthNormRadi.w < (1.0 + 0.5) * globalNormRadi.w)
            {
                // vert conf output

                vec3 globalCol = decodeColor(globalColDevTim.x);
                vec3 depthCol = decodeColor(depthColDevTim.x);

                vec3 avgColor = ((globalConfidence * globalCol.xyz) + (depthConfidence * depthCol.xyz)) / (globalConfidence + depthConfidence);

                vec4 geoVertNormRadi = ((globalConfidence * globalNormRadi) + (depthConfidence * depthNormRadi)) / (globalConfidence + depthConfidence);
                
				
				//interleavedGlobalBuffer[(matchingIndex * 3) + 0] = vec4(((globalConfidence * globalVertex) + (depthConfidence * depthVertex)) / (globalConfidence + depthConfidence), globalConfidence + depthConfidence);
			    //interleavedGlobalBuffer[(matchingIndex * 3) + 1] = vec4(normalize(geoVertNormRadi.xyz), geoVertNormRadi.w);
                //interleavedGlobalBuffer[(matchingIndex * 3) + 2] = vec4(encodeColor(avgColor), globalColDevTim.y, globalColDevTim.z, time);


				outputVC = vec4(((globalConfidence * globalVertex) + (depthConfidence * depthVertex)) / (globalConfidence + depthConfidence), globalConfidence + depthConfidence);
			    outputNR = vec4(normalize(geoVertNormRadi.xyz), geoVertNormRadi.w);
                outputCDT = vec4(encodeColor(avgColor), globalColDevTim.y, globalColDevTim.z, time);

                imageStore(outImagePC, ivec2(pix.xy), vec4(0.2, 0.2, 0.3, 1));


            }
            else
            {

                imageStore(outImagePC, ivec2(pix.xy), vec4(0.6, 0.2, 0.3, 1));

                //interleavedGlobalBuffer[(gl_GlobalInvocationID.x * 3) + 0] = vec4(globalVertConf.xyz, globalConfidence + depthConfidence);
                //interleavedGlobalBuffer[(gl_GlobalInvocationID.x * 3) + 1] = globalNormRadi;
                //interleavedGlobalBuffer[(gl_GlobalInvocationID.x * 3) + 2] = vec4(globalColDevTim.xyz, time);

				outputVC = vec4(globalVertConf.xyz, globalConfidence + depthConfidence);
                outputNR = globalNormRadi;
                outputCDT = vec4(globalColDevTim.xyz, time);
            }

        }

        updateIndexMatchingBuffer[invocationID] = 0;

    }

    // we now have a buffer of stable and unstable verts, the new unstable verts are at the end of the buffer, but there may be unstable verts at any point in the buffer
    // so for each invocationID, we need to check whether they are stable (some flag in color) and if not, how long have they been in the buffer, and what is their confidence

    //if (time - globalColDevTim.w > timeDelta) 




}




//void main()
//{
//    if (toFuse[0] == 1)
//    {
//        outVertPosConf = geoVertPosConf[0];
//        outVertNormRadi = geoVertNormRadi[0];
//        outVertColTimDev = geoVertColTimDev[0];
//        EmitVertex();
//        EndPrimitive();
//    }

//}