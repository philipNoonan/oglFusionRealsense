#version 430

layout (local_size_x = 32, local_size_y = 32) in;



//layout(binding = 0, rgba8ui) uniform image2D TrackData;
layout(binding = 0, rgba32f) uniform image2D inVertex;
layout(binding = 1, rgba32f) uniform image2D inNormal;
layout(binding = 2, rgba32f) uniform image2D refVertex;
layout(binding = 3, rgba32f) uniform image2D refNormal;
layout(binding = 4, r32f) uniform image2D differenceImage;
layout(binding = 5, rgba32f) uniform image2DArray trackImage;

    layout(binding = 6, rgba32f) uniform image2D outImagePC;

uniform mat4 cameraPoses;
uniform vec4 camPam; // cx cy fx fy
uniform mat4 inverseVP;
//uniform mat4 Ttrack;
uniform float dist_threshold;
uniform float normal_threshold;

int numberOfCameras = 1;


struct reduType
{
    int result;
    float error;
    float J[6];
};
layout(std430, binding = 0) buffer TrackData
{
    reduType trackOutput[];
};


vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam.z * p.x) / p.z) + camPam.x,
                ((camPam.w * p.y) / p.z) + camPam.y,
                p.z);
}

void main()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    vec2 imSize = imageSize(inVertex); // mipmapped sizes
    ivec2 refSize = imageSize(refVertex); // full depth size

    for (int camera = 0; camera < numberOfCameras; camera++)
    {
        uint offset = uint(camera * uint(imSize.x * imSize.y)) + ((pix.y * uint(imSize.x)) + pix.x);

        if (pix.x < imSize.x && pix.y < imSize.y)
        {
            vec4 normals = imageLoad(inNormal, pix);

            if (normals.x == 2)
            {
                trackOutput[offset].result = -1; // does this matter since we are in a low mipmap not full size???
                imageStore(trackImage, ivec3(pix, camera), vec4(0, 0, 0, 1));
            }
            else
            {
                // invertex is depth verts in previous depth frames space
                vec4 projectedVertex = cameraPoses * vec4(imageLoad(inVertex, ivec2(pix)).xyz, 1.0f); // CHECK ME AGAINT THE OLD CRAPPY OPMUL
                // projectedVertex is depth verts transformed to estimated global space                                                                                                      //vec3 projectedVertex = opMul(Ttrack, imageLoad(inVertex, ivec2(pix)).xyz); // CHECK ME AGAINT THE OLD CRAPPY OPMUL

                // either ...
                //vec4 projectedPos = inverseVP * projectedVertex;
                //vec2 projPixel = vec2(projectedPos.x / projectedPos.z, projectedPos.y / projectedPos.z);

                // or ...
                vec3 projPixel = projectPointImage(projectedVertex.xyz);
                // projpixel is the estimated location in 2d global space where this depth vert thinks it should be


                if (projPixel.x < 0 || projPixel.x > refSize.x - 1 || projPixel.y < 0 || projPixel.y > refSize.y - 1)
                {
                    trackOutput[offset].result = -2;
                    imageStore(trackImage, ivec3(pix, camera), vec4(1.0f, 0.0f, 0.0f, 1.0f));
                }
                else
                {
                    ivec2 refPixel = ivec2(projPixel.x, projPixel.y);
                    vec3 referenceNormal = imageLoad(refNormal, refPixel).xyz;
                    //vec3 tmp = imageLoad(refVertex, refPixel).xyz;
                    //imageStore(differenceImage, ivec2(projPixel), vec4(tmp.z, 0.0f, 0.0f, 1.0f));
                    //imageStore(outImagePC, ivec2(pix), vec4(referenceNormal.xyz, 1.0f));
                    if (referenceNormal.x == 2)
                    {
                        trackOutput[offset].result = -3;
                        imageStore(trackImage, ivec3(pix, camera), vec4(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                    else
                    {
                        //vec4 refVert = imageLoad(refVertex, refPixel); // this is in depth space...
                        vec3 diff = imageLoad(refVertex, refPixel).xyz - projectedVertex.xyz;
                        vec4 currNormal = imageLoad(inNormal, pix);
                        vec3 projectedNormal = vec3((cameraPoses * vec4(currNormal.xyz, 0.0f)).xyz); // input mipmap sized pixel

                        //imageStore(outImagePC, ivec2(pix), vec4(diff, 1.0f));
                        if (length(diff) > dist_threshold)
                        {
                            trackOutput[offset].result = -4;
                            imageStore(trackImage, ivec3(pix, camera), vec4(0, 0, 1.0f, 1.0f));

                        }
                        else if (dot(projectedNormal, referenceNormal) < normal_threshold)
                        {
                            trackOutput[offset].result = -5;
                            imageStore(trackImage, ivec3(pix, camera), vec4(1.0f, 1.0f, 0, 1.0f));
                        }
                        else
                        {
                            imageStore(trackImage, ivec3(pix, camera), vec4(0.5f, 0.5f, 0.5f, 1.0f));

                            trackOutput[offset].result = 1;
                            trackOutput[offset].error = dot(referenceNormal, diff);

                            trackOutput[offset].J[0] = referenceNormal.x;
                            trackOutput[offset].J[1] = referenceNormal.y;
                            trackOutput[offset].J[2] = referenceNormal.z;

                            vec3 crossProjVertRefNorm = cross(projectedVertex.xyz, referenceNormal);
                            trackOutput[offset].J[3] = crossProjVertRefNorm.x;
                            trackOutput[offset].J[4] = crossProjVertRefNorm.y;
                            trackOutput[offset].J[5] = crossProjVertRefNorm.z;
                        }
                    }
                }
            }
        }
    }

    
}