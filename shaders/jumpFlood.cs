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

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; // 

layout(binding = 0) uniform sampler3D textureInitialRGB;

layout(binding = 0, rgba32f) uniform image3D im_dtnn_0;
layout(binding = 1, rgba32f) uniform image3D im_dtnn_1;
layout(binding = 2, rgba8) uniform image2D outImage;

layout(binding = 3, rgba32f) uniform image2D verts;

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

    vec3 imSize = vec3(imageSize(im_dtnn_0));

    vec4 vertex = imageLoad(verts, pix);

    vec4 vertexInVolume = scaleFactor * (trackMat * vertex) ;

    //if (vertexInVolume.x > 0 && vertexInVolume.x < imSize.x && vertexInVolume.y > 0 && vertexInVolume.y < imSize.y && vertexInVolume.z > 0 && vertexInVolume.z < imSize.z)
    //{
        imageStore(im_dtnn_0, ivec3(vertexInVolume.xyz), vec4(vertexInVolume.xyz, 1.0f));
    //}




}


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


void DTNN(const in vec3 off)
{
    vec3 dtnn_cur = vec3(imageLoad(im_dtnn_0, ivec3(off)).xyz);
    // THIS WAS A PAIN AND IS PROBABLY A HACK FIX. DTNN_CURR WOULD RETURN ZERO VALUES AND MESS EVERYTHING UP LEADING TO A 0,0 POINT PERSISTING
    if (dtnn_cur != vec3(0) && off != vec3(0))
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
void jumpFloodAlgorithmUpdate()
{
    pix = vec3(gl_GlobalInvocationID.xyz);

    dtnn = vec3(imageLoad(im_dtnn_0, ivec3(pix)).xyz);

    vec3 ddxy = dtnn - pix;
    dmin = LENGTH_SQ(ddxy);

    // z - 1 plane
    DTNN(pix + vec3(-jump, jump, -jump));     DTNN(pix + vec3(0, jump, -jump));     DTNN(pix + vec3(jump, jump, -jump));
    DTNN(pix + vec3(-jump, 0, -jump));        DTNN(pix + vec3(0, 0, -jump));        DTNN(pix + vec3(jump, 0, -jump));
    DTNN(pix + vec3(-jump, -jump, -jump));    DTNN(pix + vec3(0, -jump, -jump));    DTNN(pix + vec3(jump, -jump, -jump));

    // in plane
    DTNN(pix + vec3(-jump, jump, jump));     DTNN(pix + vec3(0, jump, jump));     DTNN(pix + vec3(jump, jump, jump));
    DTNN(pix + vec3(-jump, 0, jump));                                       DTNN(pix + vec3(jump, 0, jump));
    DTNN(pix + vec3(-jump, -jump, jump));    DTNN(pix + vec3(0, -jump, jump));    DTNN(pix + vec3(jump, -jump, jump));

    // z + 1 plane
    DTNN(pix + vec3(-jump, jump, jump));    DTNN(pix + vec3(0, jump, jump));      DTNN(pix + vec3(jump, jump, jump));
    DTNN(pix + vec3(-jump, 0, jump));       DTNN(pix + vec3(0, 0, jump));         DTNN(pix + vec3(jump, 0, jump));
    DTNN(pix + vec3(-jump, -jump, jump));   DTNN(pix + vec3(0, -jump, jump));     DTNN(pix + vec3(jump, -jump, jump));





    imageStore(im_dtnn_1, ivec3(pix), vec4(dtnn.x, dtnn.y, dtnn.z, 0));


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

}

void main()
{
    jumpFloodSubroutine();

}