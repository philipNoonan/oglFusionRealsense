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

layout(binding = 0, rgba32f) uniform image3D im_dtnn_0;
layout(binding = 1, rgba32f) uniform image3D im_dtnn_1;

layout(binding = 2, r32i) uniform iimage3D outImage;

layout(binding = 3, rgba32f) uniform image2D verts;

layout(binding = 4, r32ui) uniform uimage3D image0;
layout(binding = 5, r32ui) uniform uimage3D image1;

uniform mat4 trackMat;
//mat4 track = mat4(1.0f);


#define LENGTH_SQ(dir) ((dir).x*(dir).x + (dir).y*(dir).y + (dir).z*(dir).z)
#define POS_MAX 0x7FFF // == 32767 == ((1<<15) - 1)

vec3 dtnn = vec3(POS_MAX); // some large number

vec3 pix;
float dmin = LENGTH_SQ(dtnn);
uniform float jump;

uniform float scaleFactor;





subroutine void launchSubroutine();
subroutine uniform launchSubroutine jumpFloodSubroutine;

subroutine(launchSubroutine)
void jfaInitFromDepth()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);

    //ivec3 imSize = ivec3(imageSize(im_dtnn_0));
    ivec3 imSize = ivec3(imageSize(image0));

    vec4 vertex = imageLoad(verts, pix);

    vec4 vertexInVolume = scaleFactor * (trackMat * vertex) ;
    //vec4 vertexInVolume = scaleFactor * vertex;

    if (vertexInVolume.x > 0 && vertexInVolume.x < imSize.x && vertexInVolume.y > 0 && vertexInVolume.y < imSize.y && vertexInVolume.z > 0 && vertexInVolume.z < imSize.y)
    {
        //imageStore(im_dtnn_0, ivec3(vertexInVolume.xyz), vec4(vertexInVolume.xyz, 1.0f));
        uint encodedOriginal = uint(vertexInVolume.x) << 20 | uint(vertexInVolume.y) << 10 | uint(vertexInVolume.z);
        imageStore(image0, ivec3(vertexInVolume.xyz), ivec4(encodedOriginal));
    }

    // uint encodedOriginal = uint(vertexInVolume.x) << 20 | uint(vertexInVolume.y) << 10 | uint(vertexInVolume.z);

    // vec3 decodedOriginal = vec3((encodedOriginal & 1072693248) >> 20, (encodedOriginal & 1047552) >> 10, encodedOriginal & 1023);

}


// here we have just copied across a predefined meshgrid style volume and to init,
// we simple do the same as for the standard texture, 
// but instead of placing the texture coord in the texel, we set the texel to 0

subroutine(launchSubroutine)
void jfaInitFromDepthInverted()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);

    ivec3 imSize = ivec3(imageSize(im_dtnn_0));

    vec4 vertex = imageLoad(verts, pix);

    //vec4 vertexInVolume = scaleFactor * (trackMat * vertex) ;
    vec4 vertexInVolume = scaleFactor * vertex;

    if (vertexInVolume.x > 0 && vertexInVolume.x < imSize.x && vertexInVolume.y > 0 && vertexInVolume.y < imSize.y && vertexInVolume.z > 0 && vertexInVolume.z < imSize.y)
    {
        imageStore(im_dtnn_0, ivec3(vertexInVolume.xyz), vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

   // uint encodedOriginal = uint(vertexInVolume.x) << 20 | uint(vertexInVolume.y) << 10 | uint(vertexInVolume.z);

   // vec3 decodedOriginal = vec3((encodedOriginal & 1072693248) >> 20, (encodedOriginal & 1047552) >> 10, encodedOriginal & 1023);


}


//shared vec4 orthoProjectedData[128];

//subroutine(launchSubroutine)
//void jfsProjectVolume()
//{
//    ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);


//    vec4 dataToWrite = imageLoad(im_dtnn_0, pix);
//    float oriDistance = distance(dataToWrite.xyz, vec3(pix));

//    //orthoProjectedData[pix.z] = dataToWrite;

//    for (int z = 0; z < 128; z++)
//    {
//        vec4 dataFromOtherSlices = imageLoad(im_dtnn_0, ivec3(pix.xy, z));
//        if (distance(dataFromOtherSlices, vec3(pix)) < oriDistance)
//        {
//            dataToWrite = dataFromOtherSlices;
//        }
//        imageStore(im_dtnn_1, ivec3(pix.xy, z), dataToWrite);


//    }



//}

subroutine(launchSubroutine)
void jumpFloodAlgorithmInit()
{
    // http://rykap.com/graphics/skew/2016/02/25/voronoi-diagrams/
    // https://github.com/diwi/PixelFlow/blob/master/examples/Miscellaneous/DistanceTransform_Demo/DistanceTransform_Demo.java
    // https://github.com/diwi/PixelFlow/blob/master/src/com/thomasdiewald/pixelflow/glsl/Filter/distancetransform.frag
    // http://www.comp.nus.edu.sg/~tants/jfa/i3d06.pdf
    // https://www.shadertoy.com/view/4syGWK
    ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);

    vec4 tColor = texelFetch(textureInitialRGB, pix, 0);

    vec4 outPos = vec4(POS_MAX);
    imageStore(im_dtnn_1, pix, outPos); // wipe it

    if (tColor.x >= 1.0)
    {
        outPos = vec4(pix.xyz, 0);
    }

    imageStore(im_dtnn_0, pix, outPos);


}

subroutine(launchSubroutine)
void jfaUpscale()
{
    ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);

    //vec4 lowResData = imageLoad(im_dtnn_0, pix/2);

    //imageStore(im_dtnn_1, pix, vec4(lowResData.xyz * 2.0f, 1.0));

    uint encodedIn = imageLoad(image0, pix/2).x;
    vec3 decodedIn = vec3((encodedIn & 1072693248) >> 20, (encodedIn & 1047552) >> 10, encodedIn & 1023);

    uint encodedOut = uint(decodedIn.x * 2.0f) << 20 | uint(decodedIn.y * 2.0f) << 10 | uint(decodedIn.z * 2.0f);

    imageStore(image1, pix, uvec4(encodedOut));
}

void DTNN(const in vec3 off)
{
    //vec3 dtnn_cur = vec3(imageLoad(im_dtnn_0, ivec3(off)).xyz);

    uint encodedIn = imageLoad(image0, ivec3(off)).x;
    vec3 dtnn_cur = vec3((encodedIn & 1072693248) >> 20, (encodedIn & 1047552) >> 10, encodedIn & 1023);

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

    //dtnn = vec3(imageLoad(im_dtnn_0, ivec3(pix)).xyz);
    uint encodedInput = imageLoad(image0, ivec3(pix)).x;
    dtnn = vec3((encodedInput & 1072693248) >> 20, (encodedInput & 1047552) >> 10, encodedInput & 1023);

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





    //imageStore(im_dtnn_1, ivec3(pix), vec4(dtnn.x, dtnn.y, dtnn.z, 0));

    uint encodedOutput = uint(dtnn.x) << 20 | uint(dtnn.y) << 10 | uint(dtnn.z);
    imageStore(image1, ivec3(pix), uvec4(encodedOutput));


    // for pixel pix, get the 8 points at +- k distance away and see if the distance they store is less than the distance pixel pix has

    // store the lowest distance as the new distance for pixel pix in output buffer

    // next iteration flip the input and output buffer

    // after the last iteration the distance transform can be determined for each pixel by getting the length of the vec between pixel pix and distance

}

subroutine(launchSubroutine)
void getColorFromRGB()
{
    //pix = vec2(gl_GlobalInvocationID.xy);
    //vec2 jfa = vec2(imageLoad(im_dtnn_1, ivec2(pix)).xy);

    //vec4 tColor = texelFetch(textureInitialRGB, ivec2(jfa), 0);

    //float distCol = distance(pix, jfa);

    //imageStore(outImage, ivec2(pix), vec4(distCol.xxx / 100, 1.0));

    pix = vec3(gl_GlobalInvocationID.xyz);
    vec4 tData = imageLoad(im_dtnn_1, ivec3(pix));
    //ivec4 tData = imageLoad(volumeData, ivec3(TexCoord.x * 128.0f, TexCoord.y * 128.0f, slice));

    //// FOR TSDF VOLUME
    //return vec4(1.0f * float(tData.x) * 0.00003051944088f, 1.0f * float(tData.x) * -0.00003051944088f, 0.0, 1.0f);

    //// FOR SDF VOLUME
    //vec3 texSize = vec3(textureSize(currentTextureVolume, 0));
    float distCol = distance(vec3(pix), tData.xyz);
    if (pix.z > tData.z)
    {
        distCol *= -1.0f;
    }
    //return vec4(tData.xyz*100.0, 1.0f);
    imageStore(outImage, ivec3(pix), ivec4(distCol.xxx, 1));
}

void main()
{
    jumpFloodSubroutine();

}