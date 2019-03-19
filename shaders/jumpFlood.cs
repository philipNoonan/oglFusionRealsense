// adapted from 
/**
 * 
 * PixelFlow | Copyright (C) 2017 Thomas Diewald - http://thomasdiewald.com
 * 
 * A Processing/Java library for high performance GPU-Computing (GLSL).
 * MIT License: https://opensource.org/licenses/MIT
 * 
 */


//
// resouces
//
// Jumpflood Algorithm (JFA)
//
// Jump Flooding in GPU with Applications to Voronoi Diagram and Distance Transform
// www.comp.nus.edu.sg/~tants/jfa/i3d06.pdf
//


#version 430

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in; // 

layout(binding = 0) uniform sampler3D textureInitialRGB;

layout(binding = 0, r32ui) uniform uimage3D image0;
layout(binding = 1, r32ui) uniform uimage3D image1;

layout(binding = 2, r32i) uniform iimage3D outImage;

layout(binding = 3, rgba32f) uniform image2D verts;
layout(binding = 4, rgba32f) uniform image2D norms;

uniform mat4 trackMat;


#define LENGTH_SQ(dir) ((dir).x*(dir).x + (dir).y*(dir).y + (dir).z*(dir).z)
#define POS_MAX 0x7FFF // == 32767 == ((1<<15) - 1)

vec3 dtnn = vec3(POS_MAX); // some large number

vec3 pix;
float dmin = LENGTH_SQ(dtnn);
uniform float jump;

uniform float scaleFactor;

// https://www.rapidtables.com/convert/number/binary-to-decimal.html?x=11111111110000000000000000000000
uint encodeValues(vec3 inVec)
{
    return uint(inVec.x) << 22 | uint(inVec.y) << 12 | uint(inVec.z) << 2;
}

vec3 decodeValues(uint inUint)
{
    return vec3((inUint & 4290772992) >> 22, (inUint & 4190208) >> 12, (inUint & 4092) >> 2); ;
}



subroutine void launchSubroutine();
subroutine uniform launchSubroutine jumpFloodSubroutine;

subroutine(launchSubroutine)
void jfaSetBlankVolume()
{

    ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);

    imageStore(image0, pix, ivec4(4294967292));


}

subroutine(launchSubroutine)
void jfaInitFromDepth()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);

    ivec3 imSize = ivec3(imageSize(image0));

    vec4 vertex = imageLoad(verts, pix);

    vec4 vertexInVolume = scaleFactor * (trackMat * vertex) ;
    //vec4 vertexInVolume = scaleFactor * vertex;



    vec4 normal = imageLoad(norms, pix);

    //vec3 vectorToNearestPoint = normalize(dtnn.xyz - pix);
    //float dotAngle = dot(normal.xyz, vectorToNearestPoint);


    if (all(greaterThan(vertexInVolume.xyz, vec3(0.0f))) && all(lessThan(vertexInVolume.xyz, vec3(imSize.x))))
    {
        uint encodedOriginal = encodeValues(vertexInVolume.xyz);

        imageStore(image0, ivec3(vertexInVolume.xyz), ivec4(encodedOriginal));
    }

}

subroutine(launchSubroutine)
void jfaUpscale()
{
    ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);

    uint encodedIn = imageLoad(image0, pix/2).x;
    vec3 decodedIn = decodeValues(encodedIn);
    uint encodedOut = encodeValues(vec3(decodedIn * 2.0f));

    imageStore(image1, pix, uvec4(encodedOut));
}

void DTNN(const in vec3 off)
{
    //if (any(lessThan(off, vec3(0.0f))))
    //{
    //    return;
    //}

    uint encodedIn = imageLoad(image0, ivec3(off)).x;
    vec3 dtnn_cur = decodeValues(encodedIn);

    // THIS WAS A PAIN AND IS PROBABLY A HACK FIX. DTNN_CURR WOULD RETURN ZERO VALUES AND MESS EVERYTHING UP LEADING TO A 0,0 POINT PERSISTING
    if (dtnn_cur != vec3(0))
    {
        vec3 ddxy = dtnn_cur - pix;
        float dcur = LENGTH_SQ(ddxy);
        if (dcur < dmin)
        {
            dmin = dcur;
            dtnn = dtnn_cur;
        }
    }
}



subroutine(launchSubroutine)
void jfsFastUpdate()
{
    // the idea here is that each pixel is essentially swept though the image and uses many repeat memory reads of the same pixel, this can be slow, esspecialy on large step sizes where texture caches are poorly utilised
    // if we first sweep through 3 rows per y axis can we load all rows into a shared memory array.
    // after barrier(), each local invocation then outputs the standard jfa from the shared memory arrays
}

subroutine(launchSubroutine)
void jumpFloodAlgorithmUpdate()
{
    pix = vec3(gl_GlobalInvocationID.xyz);

    uint encodedIn = imageLoad(image0, ivec3(pix)).x;
    dtnn = decodeValues(encodedIn);

    vec3 ddxy = dtnn - pix;
    dmin = LENGTH_SQ(ddxy);

    // z - 1 plane
    DTNN(pix + vec3(-jump, jump,  -jump));    DTNN(pix + vec3(0, jump, -jump));     DTNN(pix + vec3(jump, jump, -jump));
    DTNN(pix + vec3(-jump, 0,     -jump));    DTNN(pix + vec3(0, 0, -jump));        DTNN(pix + vec3(jump, 0, -jump));
    DTNN(pix + vec3(-jump, -jump, -jump));    DTNN(pix + vec3(0, -jump, -jump));    DTNN(pix + vec3(jump, -jump, -jump));

    // in plane
    DTNN(pix + vec3(-jump, jump,  0));    DTNN(pix + vec3(0, jump, 0));     DTNN(pix + vec3(jump, jump, 0));
    DTNN(pix + vec3(-jump, 0,     0));                                         DTNN(pix + vec3(jump, 0, 0));
    DTNN(pix + vec3(-jump, -jump, 0));    DTNN(pix + vec3(0, -jump, 0));    DTNN(pix + vec3(jump, -jump, 0));

    // z + 1 plane
    DTNN(pix + vec3(-jump, jump,  jump));    DTNN(pix + vec3(0, jump,  jump));      DTNN(pix + vec3(jump, jump, jump));
    DTNN(pix + vec3(-jump, 0,     jump));    DTNN(pix + vec3(0, 0,     jump));         DTNN(pix + vec3(jump, 0, jump));
    DTNN(pix + vec3(-jump, -jump, jump));    DTNN(pix + vec3(0, -jump, jump));     DTNN(pix + vec3(jump, -jump, jump));

    uint encodedOutput = encodeValues(dtnn); 
    imageStore(image1, ivec3(pix), uvec4(encodedOutput));
}

subroutine(launchSubroutine)
void getColorFromRGB()
{
    // THIS HAS NOT BEEN DECODED
    pix = vec3(gl_GlobalInvocationID.xyz);
    vec4 tData = imageLoad(image1, ivec3(pix));

    //// FOR SDF VOLUME
    float distCol = distance(vec3(pix), tData.xyz);
    if (pix.z > tData.z)
    {
        distCol *= -1.0f;
    }

    imageStore(outImage, ivec3(pix), ivec4(distCol.xxx, 1));
}

void main()
{
    jumpFloodSubroutine();
}