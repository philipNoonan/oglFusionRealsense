#version 430

layout(local_size_x = 1024) in;

layout(binding = 0, rg32f) uniform image2D prefixSumFlow;

uniform uint quadListCount;

layout(std430, binding = 0) buffer quadlistBuf
{
    vec4 quadlist [];
};

layout(std430, binding = 1) buffer quadlistMeanTempBuf
{
    vec2 outputMeanTemp [];
};

subroutine void launchSubroutine(in uint index, in vec2 xSum, in float quadSideLength);
subroutine uniform launchSubroutine stdDevSubroutine;

subroutine(launchSubroutine)
void firstPass(in uint index, in vec2 xSum, in float quadSideLength)
{
    vec2 xMean = xSum / (quadSideLength * quadSideLength);

    if (xSum.x == 0 || xSum.y ==0)
    {
        outputMeanTemp[index] = vec2(0);
    }
    else
    {
        outputMeanTemp[index] = xMean;
    }

}

subroutine(launchSubroutine)
void secondPass(in uint index, in vec2 xSum, in float quadSideLength)
{
    vec2 stdDev = sqrt(xSum / ((quadSideLength * quadSideLength) - 1)); // sqrt(xSum / ((quadSideLength * quadSideLength) - 1.0));

    if (isinf(stdDev.x) || isnan(stdDev.x) || isinf(stdDev.y) || isnan(stdDev.y))
    {
        outputMeanTemp[index] = vec2(0.0f);
    }
    else
    {
        outputMeanTemp[index] = stdDev;
    }
}

void main()
{
    uint index = gl_GlobalInvocationID.x;

    if (index > quadListCount)
    {
        return;
    }

    ivec2 pos = ivec2(quadlist[index].x, quadlist[index].y);



    //uint xPos = uint(quadlist.x);
    //uint yPos = uint(quadlist.y);
    uint lod = uint(quadlist[index].z);

    float quadSideLength = float(pow(2, lod)); //
    float shiftedQSL = quadSideLength - 1.0f;

    vec2 origin = vec2(pos * quadSideLength) - 1.0f; // 

    if (origin.x > 1280 - quadSideLength || origin.y > 720 - quadSideLength)
    {
        return;
    }
    //vec2 origin = ((vec2(xPos, yPos) * quadSideLength) + (quadSideLength * 0.5f)); // 

    // vec2 A, B, C, D;

    // for a quad of the prefix sum image where
    //      A -- B
    //      -    -
    //      -    -
    //      C -- D
    // sum of quad of the original image = A + D - B - C

    // Actually the calc is a bit different, we need to go
    // A -> A - (1, 1)
    // B -> B - (0, 1)
    // C -> C - (1, 0)

    // If any of these go out of bounds (because we are on the image edge, then return a Zero)
    // from the spec Note: Load operations from any texel that is outside of the boundaries of the bound image will return all zeros.
    // This is desirable for us

    vec2 A = imageLoad(prefixSumFlow, ivec2(origin)).xy;
    vec2 B = imageLoad(prefixSumFlow, ivec2(origin.x + quadSideLength, origin.y)).xy;
    vec2 C = imageLoad(prefixSumFlow, ivec2(origin.x, origin.y + quadSideLength)).xy;
    vec2 D = imageLoad(prefixSumFlow, ivec2(origin.x + quadSideLength, origin.y + quadSideLength)).xy;


    vec2 xSum = A + D - B - C;
    //vec2 xSum = C + B - D - A;


    stdDevSubroutine(index, xSum, quadSideLength);
}