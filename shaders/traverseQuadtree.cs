#version 430

layout(local_size_x = 32, local_size_y = 1) in; // 

// here we will adapt the shader from https://www.shadertoy.com/view/MlffW8 to achieve quadtree partitioning using the SDF as the metric for determining if there is an object of interest in the quad space

// inputs to shader 
// sdf image

// outputs from shader
// a 2d texture that contains rgb values that act as pointers to the nodes/leaves - an indirection pool  https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter37.html
// or
// a 2d texture ontainsing colours that indicate what level the quad is on


// images
//layout(binding = 0, r32f) uniform image2D hpBaseLevel;

    // textures
layout(binding = 0) uniform sampler2D hpVolumeTexture; 

    // buffers 
layout(std430, binding = 0) buffer posBuf
{
    // DONT USE VEC3 IN SSBO https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
    vec4 pos [];
};

uniform uint totalSum;
uniform uint cutoff;

uniform uvec4 cubeOffsets[4] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    //{0, 0, 1, 1},
   // {1, 0, 1, 1},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    //{0, 1, 1, 1},
   // {1, 1, 1, 1},    
    };

subroutine bool launchSubroutine();
subroutine uniform launchSubroutine quadlistSubroutine;


// current = ivec4 x y z sum
bool scanHPLevel(in uint target, inout int lod, inout uvec4 current, inout vec2 range)
{
    // when we scan through the layers we need to check the target index against the START value in the START:END range as described (poorly) in their paper
    // -1 is counted as 1 (not described in the paper)
    // the range is EXCLUSIVE of the END value. to get the correct current position for the leaf, it needs to be target == START

    float inputNode = texelFetch(hpVolumeTexture, ivec2(current.xy + cubeOffsets[current.w].xy), lod).x;

    if (inputNode == -1.0f && target == uint(range.x)) // if we are at a leaf node and target == START
    {
        return true;
    }
    else
    {
        range.y = abs(inputNode) + range.x; // asign END

        if (range.x <= target && target < range.y) // descend
        {
            if (lod == cutoff)
            {
                lod--;
                return false;
            }
            else
            {
                lod--;
                current.xy = (current.xy + cubeOffsets[current.w].xy) * 2; // shift the current corner block origin along
                current.w = 0; // reset the cubeOffset
                return false;
            }

        }
        else // traverse
        {
            //range.xy = range.yx; // swizzle end and start
            range.x = range.y;
            // make CURRENT SWEEP THROUGH
            //uint currentOffset = current.w;
            current.w++;// += cubeOffsets[currentOffset]; // current.w iterates the current through the sweep
            return false;
        }
    }
}

subroutine(launchSubroutine)
bool traverseHPLevel()
{
    //ivec2 texSize = textureSize(hpVolumeTexture, 0);
    uint target = uint(gl_GlobalInvocationID.x);

    if (target >= totalSum)
    {
        target = 0;
    }

    uvec4 cubePosition = uvec4(0,0,0,0); // x y z iterator

    // now traverse pyr from top to bottom depending on image size
    bool leafReached = false;
    int lod = 10;
    vec2 range = vec2(0.0f,0.0f);

    for (int k = 10*4; k >= 0; k--)
    {
        leafReached = scanHPLevel(target, lod, cubePosition, range);
        if (leafReached || lod < cutoff)
        {
            break;
        }
    }

    //cubePosition.x /= 2;
    //cubePosition.y /= 2;
    //cubePosition.z /= 2;

    if (leafReached || lod < cutoff)
    {
        if (lod < cutoff)
        {
            pos[target] = vec4(cubePosition.xy + cubeOffsets[cubePosition.w].xy, lod + 1, -1.0);
        }
        else
        {
            pos[target] = vec4(cubePosition.xy + cubeOffsets[cubePosition.w].xy, lod, -1.0);

        }


    }
    else
    {
        pos[target] = vec4(-1, -2,-3, -4);

    }

    return true;

}

void main()
{
    bool done = quadlistSubroutine();
}