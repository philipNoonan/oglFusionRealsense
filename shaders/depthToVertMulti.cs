#version 430

layout (local_size_x = 32, local_size_y = 32) in;

uniform mat4 invK[4];
//uniform mat4 colorK;

//uniform mat4 colorIntrinsics;
//uniform mat4 depthToColor;

//uniform mat4 depthToDepth;

//uniform vec4 camPamsColor;
//uniform vec4 camPamsDepth;
    // camPams.x = fx, camPams.y = fy, camPams.z = cx, camPams.w = cy
//uniform int imageType; // 0 = short, 1 = float
uniform float depthScale; // value to convert whatever unit the depth data comes in to be in units of metres
uniform int numberOfCameras;

layout(binding = 0) uniform sampler2DArray depthTextureArray;

layout(binding = 0, rgba32f) uniform image2DArray vertImageOut0; // mip 0
layout(binding = 1, rgba32f) uniform image2DArray vertImageOut1; // mip 1
layout(binding = 2, rgba32f) uniform image2DArray vertImageOut2; // mip 2

layout(std430, binding=0) buffer pos3D
{
    vec4 Position3D[];
}; 






void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;

    for (int camera = 0; camera < numberOfCameras; camera++)
    {
        for (int mip = 0; mip < 3; mip++)
        {
            ivec3 size = imageSize(vertImageOut0);
            size.x = size.x / (1 << mip);
            size.y = size.y / (1 << mip);

            if (pix.x < size.x && pix.y < size.y)
            {
                float depth = float(texelFetch(depthTextureArray, ivec3(pix.xy, camera), mip).x) * 65535.0f * depthScale; // this may be 65535

                if (depth == 0 || depth < 0)
                {
                    if (mip == 0)
                        imageStore(vertImageOut0, ivec3(pix, camera), vec4(0.0f, 0.0f, 0.0f, 0.0f));
                    if (mip == 1)
                        imageStore(vertImageOut1, ivec3(pix, camera), vec4(0.0f, 0.0f, 0.0f, 0.0f));
                    if (mip == 2)
                        imageStore(vertImageOut2, ivec3(pix, camera), vec4(0.0f, 0.0f, 0.0f, 0.0f));

                    continue;
                }

                mat4 modInvK = invK[camera];
                modInvK[0][0] *= float(1 << mip);
                modInvK[1][1] *= float(1 << mip);

                vec4 tPos = (depth) * (modInvK * vec4(pix.x, pix.y, 1.0f, 0.0f));

                if (mip == 0)
                    imageStore(vertImageOut0, ivec3(pix, camera), vec4(tPos.xyz, 1.0f));
                if (mip == 1)
                    imageStore(vertImageOut1, ivec3(pix, camera), vec4(tPos.xyz, 1.0f));
                if (mip == 2)
                    imageStore(vertImageOut2, ivec3(pix, camera), vec4(tPos.xyz, 1.0f));
            }
        }
    }
    

    

}