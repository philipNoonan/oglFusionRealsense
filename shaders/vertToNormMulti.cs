#version 430

layout(local_size_x = 32, local_size_y = 32) in;

uniform int numberOfCameras;

layout(binding = 0) uniform sampler2DArray vertexTextureArray;

layout(binding = 0, rgba32f) uniform image2DArray normImageOut0; // mip 0
layout(binding = 1, rgba32f) uniform image2DArray normImageOut1; // mip 1
layout(binding = 2, rgba32f) uniform image2DArray normImageOut2; // mip 2
    
    
void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;

    for (int camera = 0; camera < numberOfCameras; camera++)
    {
        for (int mip = 0; mip < 3; mip++)
        {
            ivec3 size = imageSize(normImageOut0);
            size.x = size.x / (1 << mip);
            size.y = size.y / (1 << mip);

            if (pix.x < size.x && pix.y < size.y)
            {
                // get centre pixel
                vec4 center = texelFetch(vertexTextureArray, ivec3(pix, camera), mip);
                // get pixel neighbours
                vec4 left = texelFetch(vertexTextureArray, ivec3(max(pix.x - 1, 0), pix.y, camera), mip);
                vec4 right = texelFetch(vertexTextureArray, ivec3(min(pix.x + 1, size.x - 1), pix.y, camera), mip);
                vec4 up = texelFetch(vertexTextureArray, ivec3(pix.x, max(pix.y - 1, 0), camera), mip);
                vec4 down = texelFetch(vertexTextureArray, ivec3(pix.x - 1, min(pix.y + 1, size.y - 1), camera), mip);

                // if the distance between vertices is greater than some threshold (dep on depth) then dont use it
                left = distance(center, left) > center.z * 0.05f ? vec4(center.x - 0.05f, center.yzw) : left;
                right = distance(center, right) > center.z * 0.05f ? vec4(center.x + 0.05f, center.yzw) : right;
                up = distance(center, up) > center.z * 0.05f ? vec4(center.x, center.y - 0.05f, center.zw) : up;
                down = distance(center, down) > center.z * 0.05f ? vec4(center.x, center.y + 0.05f, center.zw) : down;



                if (left.z == 0.0f || right.z == 0.0f || up.z == 0.0f || down.z == 0.0f)
                {
                    if (mip == 0)
                        imageStore(normImageOut0, ivec3(pix.x, pix.y, camera), vec4(2.0f, 0.0f, 0.0f, 0.0f));
                    if (mip == 1)
                        imageStore(normImageOut1, ivec3(pix.x, pix.y, camera), vec4(2.0f, 0.0f, 0.0f, 0.0f));
                    if (mip == 2)
                        imageStore(normImageOut2, ivec3(pix.x, pix.y, camera), vec4(2.0f, 0.0f, 0.0f, 0.0f));
                    continue;
                }
                else
                {
                    vec3 dxv = right.xyz - left.xyz;
                    vec3 dyv = down.xyz - up.xyz;

                    if (mip == 0)
                        imageStore(normImageOut0, ivec3(pix.x, pix.y, camera), vec4(normalize(cross(dyv, dxv)), 1.0f));
                    if (mip == 1)
                        imageStore(normImageOut1, ivec3(pix.x, pix.y, camera), vec4(normalize(cross(dyv, dxv)), 1.0f));
                    if (mip == 2)
                        imageStore(normImageOut2, ivec3(pix.x, pix.y, camera), vec4(normalize(cross(dyv, dxv)), 1.0f));
                }

            }
        }



    }

}
