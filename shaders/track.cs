#version 430

layout (local_size_x = 32, local_size_y = 32) in;

//uniform mat4 view;
//uniform mat4 Ttrack;
uniform float dist_threshold;
uniform float normal_threshold;

//layout(binding = 0, rgba8ui) uniform image2D TrackData;
layout(binding = 0, rgba32f) uniform image2DArray inVertex;
layout(binding = 1, rgba32f) uniform image2DArray inNormal;
layout(binding = 2, rgba32f) uniform image2DArray refVertex;
layout(binding = 3, rgba32f) uniform image2DArray refNormal;
layout(binding = 4, r32f) uniform image2D differenceImage;
layout(binding = 5, rgba32f) uniform image2DArray trackImage;

uniform int numberOfCameras;
uniform mat4 cameraPoses[4];
uniform mat4 inverseCameraPoses[4];

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

//vec3 opMul(mat4 M, vec3 v)
//{
//    return vec3(
//        dot(vec3(M[0][0], M[1][0], M[2][0]), v) + M[3][0],
//        dot(vec3(M[0][1], M[1][1], M[2][1]), v) + M[3][1],
//        dot(vec3(M[0][2], M[1][2], M[2][2]), v) + M[3][2]);
//}

//vec3 rotate(mat4 M, vec3 V)
//{
//    // glsl and glm [col][row]
//    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
//                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
//                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
//}

// int oneDindex = (row * length_of_row) + column; // Indexes

void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    ivec3 imSize = imageSize(inVertex); // mipmapped sizes
    ivec3 refSize = imageSize(refVertex); // full depth size

    for (int camera = 0; camera < numberOfCameras; camera++)
    {
        uint offset = uint(camera * imSize.x * imSize.y) + ((pix.y * imSize.x) + pix.x);


        if (pix.x < imSize.x && pix.y < imSize.y)
        {
            vec4 normals = imageLoad(inNormal, ivec3(pix, camera));

            if (normals.x == 2)
            {
                trackOutput[offset].result = -1; // does this matter since we are in a low mipmap not full size???
                imageStore(trackImage, ivec3(pix, camera), vec4(0, 0, 0, 0));

            }
            else
            {
                vec4 projectedVertex = cameraPoses[camera] * vec4(imageLoad(inVertex, ivec3(pix, camera)).xyz, 1.0f); // CHECK ME AGAINT THE OLD CRAPPY OPMUL
                                                                                                                      //vec3 projectedVertex = opMul(Ttrack, imageLoad(inVertex, ivec2(pix)).xyz); // CHECK ME AGAINT THE OLD CRAPPY OPMUL

                vec4 projectedPos = inverseCameraPoses[camera] * projectedVertex;
                //vec3 projectedPos = opMul(view, projectedVertex);
                imageStore(trackImage, ivec3(pix, camera), vec4(projectedPos.xyz, 0.323f));


                //vec2 projPixel = vec2(projectedPos.x / projectedPos.z, projectedPos.y / projectedPos.z);



                //// vec2 projPixel = vec2(pix.x * 2, pix.y * 2);


                //if (projPixel.x < 0 || projPixel.x > refSize.x - 1 || projPixel.y < 0 || projPixel.y > refSize.y - 1)
                //{
                //    trackOutput[offset].result = -2;
                //    imageStore(trackImage, ivec3(pix, camera), vec4(1.0f, 0, 0, 1.0f));

                //}
                //else
                //{
                //    ivec3 refPixel = ivec3(projPixel.x, projPixel.y, camera);
                //    vec3 referenceNormal = imageLoad(refNormal, refPixel).xyz;
                //    vec3 tmp = imageLoad(refVertex, refPixel).xyz;
                //    //imageStore(differenceImage, ivec2(projPixel), vec4(tmp.z, 0.0f, 0.0f, 1.0f));


                //    if (referenceNormal.x == -2)
                //    {
                //        trackOutput[offset].result = -3;
                //        imageStore(trackImage, ivec3(pix, camera), vec4(0, 1.0f, 0, 1.0f));

                //    }
                //    else
                //    {
                //        vec3 diff = imageLoad(refVertex, refPixel).xyz - projectedVertex.xyz;
                //        vec4 currNormal = imageLoad(inNormal, ivec3(pix, camera));
                //        vec3 projectedNormal = vec3((cameraPoses[camera] * vec4(currNormal.xyz, 0.0f)).xyz); // input mipmap sized pixel

                //        if (length(diff) > dist_threshold)
                //        {
                //            trackOutput[offset].result = -4;
                //            imageStore(trackImage, ivec3(pix, camera), vec4(0, 0, 1.0f, 1.0f));

                //        }
                //        else if (dot(projectedNormal, referenceNormal) < normal_threshold)
                //        {
                //            trackOutput[offset].result = -5;
                //            imageStore(trackImage, ivec3(pix, camera), vec4(1.0f, 1.0f, 0, 1.0f));

                //        }
                //        else
                //        {
                //            imageStore(trackImage, ivec3(pix, camera), vec4(0.5f, 0.5f, 0.5f, 1.0f));


                //            trackOutput[offset].result = 1;
                //            trackOutput[offset].error = dot(referenceNormal, diff);

                //            trackOutput[offset].J[0] = referenceNormal.x;
                //            trackOutput[offset].J[1] = referenceNormal.y;
                //            trackOutput[offset].J[2] = referenceNormal.z;

                //            vec3 crossProjVertRefNorm = cross(projectedVertex.xyz, referenceNormal);
                //            trackOutput[offset].J[3] = crossProjVertRefNorm.x;
                //            trackOutput[offset].J[4] = crossProjVertRefNorm.y;
                //            trackOutput[offset].J[5] = crossProjVertRefNorm.z;
                //        }
                //    }
                //}
            }
        }
    }
}