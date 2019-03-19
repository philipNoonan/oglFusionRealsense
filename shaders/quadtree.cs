#version 430

layout(local_size_x = 8, local_size_y = 8) in; // 

// here we will adapt the shader from https://www.shadertoy.com/view/MlffW8 to achieve quadtree partitioning using the SDF as the metric for determining if there is an object of interest in the quad space

// inputs to shader 
// sdf image

// outputs from shader
// a 2d texture that contains rgb values that act as pointers to the nodes/leaves - an indirection pool  https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter37.html
// or
// a 2d texture ontainsing colours that indicate what level the quad is on
// or
// can we just re-use the histo pyramid stuff for storing the indirection pool in a ND mip mapped texture

    // lets try a 2D histopyramid

// read in the mipmap image
// use 32 x 32 threads
// from coarse to fine
// if non interpolated texel fetch from each mipmap pixel = 0 then there are no interesting pixels in the finer levels
// hmm woudl this work.....................................






// images
layout(binding = 0, r32f) uniform image2D hpVolume;
layout(binding = 1, r32f) uniform image2D hpVolumeOutput;
//layout(binding = 2, r32ui) uniform uimage2D hpBaseLevel;

//textures
layout(binding = 0) uniform sampler2D hpVolumeTexture; 
layout(binding = 1) uniform sampler2D originalDataVolumeTexture; 
layout(binding = 2) uniform sampler2D originalDataVolumeTextureRG; 


//uniform int baseLevel;
uniform int hpLevel;

uniform uvec4 cubeOffsets[8] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {0, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 1, 0},    
 };
   
subroutine bool launchSubroutine();
subroutine uniform launchSubroutine hpQuadtreeSubroutine;


subroutine(launchSubroutine)
bool hpDiscriminator()
{
    vec2 texSize = vec2(textureSize(originalDataVolumeTextureRG, 0).xy);

    vec3 readPix = vec3(gl_GlobalInvocationID.xyz);
    if (readPix.x > texSize.x || readPix.y > texSize.y)
    {
        return false;
    }

    //float inputValue = texelFetch(originalDataVolumeTexture, ivec2(readPix.xy), 0).x;
    vec2 inputVec2 = texelFetch(originalDataVolumeTextureRG, ivec2(readPix.x, readPix.y), 0).xy;

    float inputValue = length(inputVec2);

    float writeValue;

    if (inputValue < 0.05f)
    {
        writeValue = -1.0f;
    }
    else
    {
        writeValue = 0.0f;
    }

    imageStore(hpVolumeOutput, ivec2(readPix.xy), vec4(writeValue));

    return true;

}

subroutine(launchSubroutine)
bool hpBuilder()
{
    vec2 writePos = vec2(gl_GlobalInvocationID.xy);
    vec2 readPix = writePos * 2;

    vec2 texSize = vec2(textureSize(hpVolumeTexture, hpLevel));


    // we can use the linear interp trick here, if we have a pre-mipmapped image volume, read one value from the middle of all 4, if its -1, then all 4 are minus 1, if its all 0, then all are zero
    // if its -0.5 then the ouput is 2
    float inputValue;

    vec2 readPosTexture = writePos.xy * 2 + 1.0f;
    readPosTexture /= texSize;
    inputValue = textureLod(hpVolumeTexture, readPosTexture, hpLevel).x;
    float writeValue;

    if (inputValue == -1.0) // all neighbours are -1
    {
        writeValue = -1.0f;
    }
    else // take the abs of the neighbours and sum them
    {
        vec4 oneVec = vec4(1.0f);
        vec4 inputVec;
        inputVec.x = texelFetch(hpVolumeTexture, ivec2(readPix), hpLevel).x;
        inputVec.y = texelFetch(hpVolumeTexture, ivec2(readPix) + ivec2(cubeOffsets[1].xy), hpLevel).x;
        inputVec.z = texelFetch(hpVolumeTexture, ivec2(readPix) + ivec2(cubeOffsets[4].xy), hpLevel).x;
        inputVec.w = texelFetch(hpVolumeTexture, ivec2(readPix) + ivec2(cubeOffsets[5].xy), hpLevel).x;

        writeValue = dot(oneVec, abs(inputVec)); // faster way to sum? just takes one cycle on gpu, pub abs here or on each line above?
        //writeValue = 32.1f;
    }


    //float writeValue = 0;

    //writeValue = imageLoad(volumeData, readPos).x +
    //             imageLoad(volumeData, readPos + ivec3(cubeOffsets[1].xyz)).x +
    //             //imageLoad(volumeData, readPos + ivec3(cubeOffsets[2].xyz)).x +
    //             //imageLoad(volumeData, readPos + ivec3(cubeOffsets[3].xyz)).x +

    //             imageLoad(volumeData, readPos + ivec3(cubeOffsets[4].xyz)).x +
    //             imageLoad(volumeData, readPos + ivec3(cubeOffsets[5].xyz)).x
    //             //imageLoad(volumeData, readPos + ivec3(cubeOffsets[6].xyz)).x +
    //             //imageLoad(volumeData, readPos + ivec3(cubeOffsets[7].xyz)).x
    //             ;




    imageStore(hpVolumeOutput, ivec2(writePos.xy), vec4(writeValue));

    return true;
}

void main()
{
    bool done = hpQuadtreeSubroutine();
    

}