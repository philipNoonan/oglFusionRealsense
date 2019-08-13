#version 430

layout(local_size_x = 32) in;

uint maxNumVerts;

layout(std430, binding = 0) buffer feedbackBuffer
{
    vec4 interleavedData [];
};

layout(std430, binding = 1) buffer outputBuffer
{
    vec4 outputInterleavedData [];
};


void main()
{
    // data is vec4 vec4 vec4
    uint vertID = gl_GlobalInvocationID.x;

    if (vertID < maxNumVerts)
    {
        outputInterleavedData[(vertID * 3)] = interleavedData[(vertID * 3)];
        outputInterleavedData[(vertID * 3) + 1] = interleavedData[(vertID * 3) + 1];
        outputInterleavedData[(vertID * 3) + 2] = vec4(interleavedData[(vertID * 3) + 2].x, 0, 1, interleavedData[(vertID * 3) + 2].w);
    }
}