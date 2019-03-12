#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rg16i) uniform iimage3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively
layout(binding = 1, r32f) uniform image2D depthImage;
layout(binding = 2, r16ui) uniform uimage2D depthImageShort;

layout(binding = 3, rgba32f) uniform image2D depthVertexImage;

uniform mat4 invTrack;
uniform mat4 Kmat; 
uniform float mu; 
uniform float maxWeight;
uniform vec3 volDim; // vol dim real world span of the volume in meters
uniform vec3 volSize; // voxel grid size of the volume 
    // volDim.x / volSize.x = sizeOfVoxel
uniform int imageType;
uniform float depthScale;





// this returns the normailsed float distance for the volume 0 - 1
vec3 getVolumePosition(uvec3 p)
{
    return vec3((p.x + 0.5f) * volDim.x / volSize.x, (p.y + 0.5f) * volDim.y / volSize.y, (p.z + 0.5f) * volDim.z / volSize.z);
}

//vec3 rotate(mat4 M, vec3 V)
//{
//    // glsl and glm [col][row]
//    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
//                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
//                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
//}

//vec3 opMul(mat4 M, vec3 v)
//{
//    return vec3(
//        dot(vec3(M[0][0], M[1][0], M[2][0]), v) + M[3][0],
//        dot(vec3(M[0][1], M[1][1], M[2][1]), v) + M[3][1],
//        dot(vec3(M[0][2], M[1][2], M[2][2]), v) + M[3][2]);
//}

vec4 getSDF(uvec3 pos)
{
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    ivec4 data = imageLoad(volumeData, ivec3(pos));

    return vec4(float(data.x) * 0.00003051944088f, data.y, data.zw); 
}

void setSDF(uvec3 _pix, vec4 _data)
{
    //imageStore(volumeData, vec3(_pix), ivec4(_data.x, _data.y, _data.zw));
    imageStore(volumeData, ivec3(_pix), ivec4(int(_data.x * 32766.0f), int(_data.y), _data.zw));

}

void main()
{
    ivec2 depthSize;
    if (imageType == 0)
    {
        depthSize = ivec2(imageSize(depthImageShort).xy);
    }
    else
    {
        depthSize = ivec2(imageSize(depthImage).xy);
    }
    uvec3 pix = gl_GlobalInvocationID.xyz;

   // imageStore(volumeData, ivec3(pix.x, pix.y, 1), ivec4(123));

    vec3 pos = vec3(invTrack * vec4(getVolumePosition(pix), 1.0f)).xyz;
    //vec3 pos = opMul(invTrack, getVolumePosition(pix));

    vec3 cameraX = vec3(Kmat * vec4(pos, 1.0f)).xyz;
    //vec3 cameraX = opMul(Kmat, pos);

    vec3 delta = vec3(invTrack * vec4(0.0f, 0.0f, volDim.z / volSize.z, 0.0f)).xyz;
    //vec3 delta = rotate(invTrack, vec3(0.0f, 0.0f, volDim.z / volSize.z));

    vec3 cameraDelta = vec3(Kmat * vec4(delta, 0.0f)).xyz;
    //vec3 cameraDelta = rotate(Kmat, delta);

    bool useOld = true;
    if (useOld)
    {
        for (pix.z = 0; pix.z < volSize.z; ++pix.z, pos += delta, cameraX += cameraDelta)
        {
            if (pos.z < 0.0001f)
                continue;

            vec2 pixel = vec2(cameraX.x / cameraX.z, cameraX.y / cameraX.z);

            if (pixel.x < 0 || pixel.x > depthSize.x - 1 || pixel.y < 0 || pixel.y > depthSize.y - 1)
                continue;

            uvec2 px = uvec2(pixel.x, pixel.y);

            float depth;
            if (imageType == 0)
            {
                depth = float(imageLoad(depthImageShort, ivec2(px)).x) * depthScale;
            }
            else if (imageType == 1)
            {
                depth = imageLoad(depthImage, ivec2(px)).x / 1000.0f; // chnage me to whatever float type images have scales, if they do
            }

            if (depth.x == 0)
                continue;

            float diff = (depth.x - cameraX.z) * sqrt(1 + pow(pos.x / pos.z, 2) + pow(pos.y / pos.z, 2));
            //if (abs(diff) < 0.1f)
            if (diff > -mu)
            {
                //if ((diff) < 0.1)
                //{
                float sdf = diff / mu;
                vec4 data = getSDF(pix);
                //float weightedDistance = (data.y * data.x + sdf) / (data.y + 1);
                float weightedDistance = (data.y * data.x + diff) / (data.y + 1);

                if (weightedDistance < 0.2f)
                {
                    data.x = clamp(weightedDistance, -0.2f, 0.2f);
                    // data.x = diff;
                    data.y = min(data.y + 1, maxWeight);
                }
                else
                {
                    data.x = 0;
                    data.y = 0;
                }

                setSDF(pix, data);
            }
            else
            {
                vec4 data;// = vs(pix);

                data.x = 0;

                data.y = 0;

                setSDF(pix, data);
            }
        }



    }
    else // new method where we dont sweep through the whole texture only a few voxels either side of the array
    {
        // the input depth image z value should be projected into 3d then tranlated to volume space
        // from cameraX and camera delta, traverse the ray from +SDF to -SDF
        // at each delta position, update the SDF from the current depth

        mat4 track = inverse(invTrack);

        vec4 depth = imageLoad(depthVertexImage, ivec2(pix.xy));

        vec4 depthInVolumeSpace = track * vec4(depth.xyz, 1.0f);



        float voxelLength = volDim.x / volSize.x;

        for (float z = -voxelLength * 5.0; z < voxelLength * 5.0; z+= voxelLength / 10.0)
        {
            vec3 volPos = vec3(depthInVolumeSpace.xy, depthInVolumeSpace.z + z);

            vec4 previousSDF = getSDF(uvec3(volPos * volSize / volDim));
            float weightedDistance = (previousSDF.y * previousSDF.x + z / mu) / (previousSDF.y + 1);

            vec4 newSDF = vec4(clamp(weightedDistance, -voxelLength * 5.0, voxelLength * 5.0),
                               min(previousSDF.y + 1.0f, maxWeight), 
                               0.0f,
                               0.0f);

            setSDF(uvec3(volPos * volSize / volDim), newSDF);
        }




    }



}

