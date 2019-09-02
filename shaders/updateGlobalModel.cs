#version 430

layout(local_size_x = 32) in;

uint maxNumVerts;

layout(std430, binding = 0) buffer feedbackBuffer
{
    vec4 interleavedData [];
};

layout(std430, binding = 1) buffer updateIndexMapBuffer
{
    vec4 updateIndexInterleaved [];
};

layout(std430, binding = 2) buffer outputBuffer
{
    vec4 outputInterleavedData [];
};

uniform float texDim;
uniform int time;

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

    // data is vec4 vec4 vec4
    //vec2 imSize = vec2(imageSize(imVertConf));

    int vertID = int(gl_GlobalInvocationID.x);

    //ivec2 pix;
    //pix.y = vertID / int(texDim);
    //pix.x = vertID - (pix.y * int(texDim));


    vec4 newColor = updateIndexInterleaved[(vertID * 3) + 2];

    //Do averaging here
    if (newColor.w == -1)
    {
        vec4 vertexConfidence = interleavedData[(vertID * 3)];
        vec4 normalRadius = interleavedData[(vertID * 3) + 1];
        vec4 colorTimeDevice = interleavedData[(vertID * 3) + 2];

        vec4 newPos = updateIndexInterleaved[(vertID * 3)];
        vec4 newNorm = updateIndexInterleaved[(vertID * 3) + 1];

        float c_k = vertexConfidence.w;
        vec3 v_k = vertexConfidence.xyz;
        
        float a = newPos.w;
        vec3 v_g = newPos.xyz;
        
        if(newNorm.w < (1.0 + 0.5) * normalRadius.w)
        {
            // vert conf output
            outputInterleavedData[(vertID * 3)] = vec4(((c_k * v_k) + (a * v_g)) / (c_k + a), c_k + a);
	        
	        vec3 oldCol = decodeColor(colorTimeDevice.x);
	        vec3 newCol = decodeColor(newColor.x);
           
            vec3 avgColor = ((c_k * oldCol.xyz) + (a * newCol.xyz)) / (c_k + a);

            // norm radius output
            outputInterleavedData[(vertID * 3) + 1] = vec4(encodeColor(avgColor), colorTimeDevice.y, colorTimeDevice.z, time);
	        
	        vec4 outVertNormRadi = ((c_k * normalRadius) + (a * newNorm)) / (c_k + a);

            // color time device output
            outputInterleavedData[(vertID * 3) + 1] = vec4(normalize(outVertNormRadi.xyz), outVertNormRadi.w);
        }
        else
        {
            outputInterleavedData[(vertID * 3)    ] = vec4(vertexConfidence.xyz, c_k + a);
            outputInterleavedData[(vertID * 3) + 1] = normalRadius;
            outputInterleavedData[(vertID * 3) + 2] = vec4(colorTimeDevice.xyz, colorTimeDevice);
        }
    }
    else
    {
        //This point isn't being updated, so just transfer it
        outputInterleavedData[(vertID * 3)    ] = interleavedData[(vertID * 3)];
        outputInterleavedData[(vertID * 3) + 1] = interleavedData[(vertID * 3) + 1];
        outputInterleavedData[(vertID * 3) + 2] = interleavedData[(vertID * 3) + 2];
    }


    // need to wipe the update buffer after every update, this should always ensure that the active buffer values are set off after use
    updateIndexInterleaved[(vertID * 3)] = vec4(0.0f);
    updateIndexInterleaved[(vertID * 3) + 1] = vec4(0.0f);
    updateIndexInterleaved[(vertID * 3) + 2] = vec4(0.0f);

}