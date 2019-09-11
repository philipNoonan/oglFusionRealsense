#version 430 core

layout(local_size_x = 1024, local_size_y = 1) in; // 

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

//out vec4 geoVertPosConf;
//out vec4 geoVertNormRadi;
//out vec4 geoVertColTimDev;
//flat out int toFuse;

uniform vec4 camPam;
uniform float texDim;
uniform int time;
uniform uint currentGlobalNumber;
uniform uint currentNewUnstableNumber;

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

    //int toFuse = 0;

    // get the (if any) matching depth vertices index from the global-size matched update index buffer 

    uint invocationID = gl_GlobalInvocationID.x;

    if (invocationID > currentGlobalNumber + currentNewUnstableNumber)
    {
        return;
    }

    if (invocationID > currentGlobalNumber) // new unstable point
    {
        
        uint newUnstableInvocationID = invocationID - currentGlobalNumber; // CHECK THAT THIS ISNT A ERROR OF 1 OUT

        uint newUnstableID = updateIndexNewUnstableBuffer[newUnstableInvocationID];

        vec4 depthVertConf = interleavedDepthBuffer[(newUnstableID * 3)];
        vec4 depthNormRadi = interleavedDepthBuffer[(newUnstableID * 3) + 1];
        vec4 depthColDevTim = interleavedDepthBuffer[(newUnstableID * 3) + 2];

        interleavedGlobalBuffer[(invocationID * 3) + 0] = depthVertConf;
        interleavedGlobalBuffer[(invocationID * 3) + 1] = depthNormRadi;
        interleavedGlobalBuffer[(invocationID * 3) + 2] = depthColDevTim;

        updateIndexNewUnstableBuffer[newUnstableInvocationID] = 0; // SGHOULD THIS BE ZERO?

    }
    else // we either have a stable point to merge, or just need to copy/do nothing
    {
        int matchingIndex = updateIndexMatchingBuffer[invocationID];

        vec4 globalVertConf = interleavedGlobalBuffer[(invocationID * 3)];
        vec4 globalNormRadi = interleavedGlobalBuffer[(invocationID * 3) + 1];
        vec4 globalColDevTim = interleavedGlobalBuffer[(invocationID * 3) + 2];

        if (matchingIndex == 0) // no point to merge with
        {
            // copy accross, or rather, dont overwrite this global vert in the buffer
        }
        else // point to merge with
        {
            vec4 depthVertConf = interleavedDepthBuffer[(abs(matchingIndex) * 3)];
            vec4 depthNormRadi = interleavedDepthBuffer[(abs(matchingIndex) * 3) + 1];
            vec4 depthColDevTim = interleavedDepthBuffer[(abs(matchingIndex) * 3) + 2];

            vec3 pix = projectPointImage(depthVertConf.xyz);

            float globalConfidence = depthVertConf.w; // c_k
            vec3 globalVertex = depthVertConf.xyz; // v_k

            float depthConfidence = depthVertConf.w; // a
            vec3 depthVertex = depthVertConf.xyz; // v_g

            if (depthNormRadi.w < (1.0 + 0.5) * globalNormRadi.w)
            {
                // vert conf output
                interleavedGlobalBuffer[(matchingIndex * 3) + 0] = vec4(((globalConfidence * globalVertex) + (depthConfidence * depthVertex)) / (globalConfidence + depthConfidence), globalConfidence + depthConfidence);

                vec3 globalCol = decodeColor(globalColDevTim.x);
                vec3 depthCol = decodeColor(depthColDevTim.x);

                vec3 avgColor = ((globalConfidence * globalCol.xyz) + (depthConfidence * depthCol.xyz)) / (globalConfidence + depthConfidence);

                vec4 geoVertNormRadi = ((globalConfidence * globalNormRadi) + (depthConfidence * depthNormRadi)) / (globalConfidence + depthConfidence);
                interleavedGlobalBuffer[(matchingIndex * 3) + 1] = vec4(normalize(geoVertNormRadi.xyz), geoVertNormRadi.w);

                interleavedGlobalBuffer[(matchingIndex * 3) + 2] = vec4(encodeColor(avgColor), globalColDevTim.y, globalColDevTim.z, time);



            }
            else
            {

                imageStore(outImagePC, ivec2(pix.xy), vec4(0.6, 0.2, 0.3, 1));

                interleavedGlobalBuffer[(gl_GlobalInvocationID.x * 3) + 0] = vec4(globalVertConf.xyz, globalConfidence + depthConfidence);
                interleavedGlobalBuffer[(gl_GlobalInvocationID.x * 3) + 1] = globalNormRadi;
                interleavedGlobalBuffer[(gl_GlobalInvocationID.x * 3) + 2] = vec4(globalColDevTim.xyz, time);


            }

        }

        updateIndexMatchingBuffer[invocationID] = 0;

    }
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