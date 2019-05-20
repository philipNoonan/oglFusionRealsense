#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rgba16f) uniform image3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively
layout(binding = 1, r32f) uniform image2D depthImage;
layout(binding = 2, r16ui) uniform uimage2D depthImageShort;

layout(binding = 3, rgba32f) uniform image2D depthVertexImage;


layout(std430, binding = 5) buffer CameraData
{
    mat4 cameraMat [];
};

layout(binding = 0) uniform sampler2DArray depthTextureArray; // 


uniform float dMax;
uniform float dMin;

uniform mat4 invTrack;
uniform mat4 Kmat; 
uniform mat4 invK;

//uniform float mu; 
uniform float maxWeight;
uniform vec3 volDim; // vol dim real world span of the volume in meters
uniform vec3 volSize; // voxel grid size of the volume 
    // volDim.x / volSize.x = sizeOfVoxel
uniform int imageType;
uniform float depthScale;
uniform int forceIntegrate;

//uniform int cameraDevice;
uniform int numberOfCameras;


vec3 getVolumePosition(uvec3 p)
{
    return vec3((p.x + 0.5f) * volDim.x / volSize.x, (p.y + 0.5f) * volDim.y / volSize.y, (p.z + 0.5f) * volDim.z / volSize.z);
}

vec4 getSDF(uvec3 pos)
{
    return imageLoad(volumeData, ivec3(pos));
}


bool inFrustrum(in vec4 pClip)
{
    return abs(pClip.x) < pClip.w &&
           abs(pClip.y) < pClip.w; 
}


void integrateExperimental()
{
    ivec2 depthSize = ivec2(imageSize(depthImageShort).xy);
    uvec3 pix = gl_GlobalInvocationID.xyz;

    float deemax = invTrack[3][3];
    float deemin = invTrack[2][3];
    int cameraDevice = int(invTrack[1][3]);
    int numberOfCameras = int(invTrack[0][3]);
    mat4 itrack = invTrack;
    itrack[3][3] = 1.0f;
    itrack[2][3] = 0.0f;
    itrack[1][3] = 0.0f;
    itrack[0][3] = 0.0f;

    mat4 trackMat;// = cameraMat[0];
    float diff[4]; // max number of cameras on one system

    for (pix.z = 0; pix.z < volSize.z; pix.z++)
    {
        for (int cameraDevice = 0; cameraDevice < numberOfCameras; cameraDevice++)
        {
            trackMat = cameraMat[cameraDevice];
            
            // get world position of voxel 
            vec3 worldPos = vec3(trackMat * vec4(getVolumePosition(pix), 1.0f)).xyz;
            // get clip position
            vec4 pClip = Kmat * vec4(worldPos, 1.0f);
            // determin if valid pixel for each camera
            vec2 pixel = vec2(pClip.x / pClip.z, pClip.y / pClip.z);
     
            // if we dont check if we hit the image here and just assume that if pixel is out of bounds the resultant texture read will be zero
            if (pixel.x < 0 || pixel.x > depthSize.x - 1 || pixel.y < 0 || pixel.y > depthSize.y - 1)
            {
                diff[cameraDevice] = -10000.0f;
                continue;
            }

            float depth = float(texelFetch(depthTextureArray, ivec3(pixel.xy, cameraDevice), 0).x) * 65535.0f * 0.0001f; // this may be 65535

            //if (depth != 0)
            //{
           //     imageStore(volumeData, ivec3(pix), vec4(0.1, 0.6, 0.9, 1.0f));
            //}
            vec4 depthPoint = (depth) * (invK * vec4(pixel.x, pixel.y, 1.0f, 0.0f));
            if (depthPoint.z <= 0.0f)
            {
                diff[cameraDevice] = -10000.0f;
                continue;
            }
            //imageStore(volumeData, ivec3(pix), vec4(0.2, 0.3, 0.4, 1.0f));

            // if we get here, then the voxel is seen by this cameraDevice
            // determin best cameraDevice
            vec3 shiftVec = depthPoint.xyz - worldPos;
            float tdiff = length(shiftVec);
            diff[cameraDevice] = shiftVec.z > 0.0 ? tdiff : -tdiff;

        }

        float finalDiff = 10000.0f;
        for (int cameraDevice = 0; cameraDevice < numberOfCameras; cameraDevice++)
        {
            if (diff[cameraDevice] != 10000.0f)
            {
                finalDiff = abs(diff[cameraDevice]) < abs(finalDiff) ? diff[cameraDevice] : finalDiff;
            }
        }

        //finalDiff = diff[0];

        // get distance from voxel to depth
        //float diff = distance(depthPoint.xyz, worldPos);


        // if diff within TSDF range, write to volume
        if (finalDiff < dMax && finalDiff > dMin)
        {
            vec4 data = getSDF(pix);
            float weightedDistance = (data.y * data.x + finalDiff) / (data.y + 1);

            if (weightedDistance < dMax)
            {
                data.x = clamp(weightedDistance, dMin, dMax);
                //data.x = diff;
                data.y = min(data.y + 1, (maxWeight));
            }
            else
            {
                data.x = 0;
                data.y = 0;
            }
            imageStore(volumeData, ivec3(pix), data);
        }
        else
        {
            //pix.z += 50; // need to be clever here, but this could work nicely woudl like to jump to just before the 
            imageStore(volumeData, ivec3(pix), vec4(0.0f));
        }
    }

}

void integrateStandard()
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

    float deemax = invTrack[3][3];
    float deemin = invTrack[2][3];
    int cameraDevice = int(invTrack[1][3]);
    int numberOfCameras = int(invTrack[0][3]);

    mat4 itrack = invTrack;
    itrack[3][3] = 1.0f;
    itrack[2][3] = 0.0f;
    itrack[1][3] = 0.0f;
    itrack[0][3] = 0.0f;

    vec3 pos = vec3(itrack * vec4(getVolumePosition(pix), 1.0f)).xyz;
    vec3 cameraX = vec3(Kmat * vec4(pos, 1.0f)).xyz;
    vec3 delta = vec3(itrack * vec4(0.0f, 0.0f, volDim.z / volSize.z, 0.0f)).xyz;
    vec3 cameraDelta = vec3(Kmat * vec4(delta, 0.0f)).xyz;

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
        if (diff < deemax && diff > deemin)
        {
            //if ((diff) < 0.1)
            //{
            //float sdf = diff / mu;
            vec4 data = getSDF(pix);
            //float weightedDistance = (data.y * data.x + sdf) / (data.y + 1);
            float weightedDistance = (data.y * data.x + diff) / (data.y + 1);

            if (weightedDistance < 0.1f)
            {
                data.x = clamp(weightedDistance, -0.04f, 0.1f);
                // data.x = diff;
                data.y = min(data.y + 1, (maxWeight));
            }
            else
            {
                data.x = 0;
                data.y = 0;
            }

            imageStore(volumeData, ivec3(pix), data);

            //setSDF(pix, data);
        }
        else
        {
            imageStore(volumeData, ivec3(pix), vec4(0.0f));

        }
    }
}


void fuse(in vec4 previousTSDF, in vec4 currentTSDF, out vec4 outputTSDF)
{
    if (abs(previousTSDF.z) == 0.0f || abs(currentTSDF.x) < abs(previousTSDF.z))
    {
        outputTSDF = vec4(previousTSDF.x, previousTSDF.y, currentTSDF.x, currentTSDF.y);
    }
    else
    {
        outputTSDF = previousTSDF;
    }
}

void main()
{

    //integrateSubroutine();

    integrateExperimental();
        //}
        //else // new method where we dont sweep through the whole texture only a few voxels either side of the array
        //{
        //    // the input depth image z value should be projected into 3d then tranlated to volume space
        //    // from cameraX and camera delta, traverse the ray from +SDF to -SDF
        //    // at each delta position, update the SDF from the current depth

    //    mat4 track = inverse(invTrack);

    //    vec4 depth = imageLoad(depthVertexImage, ivec2(pix.xy));

    //    vec4 depthInVolumeSpace = track * vec4(depth.xyz, 1.0f);



    //    float voxelLength = volDim.x / volSize.x;

    //    for (float z = -voxelLength * 5.0; z < voxelLength * 5.0; z += voxelLength / 10.0)
    //    {
    //        vec3 volPos = vec3(depthInVolumeSpace.xy, depthInVolumeSpace.z + z);

    //        vec4 previousSDF = getSDF(uvec3(volPos * volSize / volDim));
    //        float weightedDistance = (previousSDF.y * previousSDF.x + z / mu) / (previousSDF.y + 1);

    //        vec4 newSDF = vec4(clamp(weightedDistance, -voxelLength * 5.0, voxelLength * 5.0),
    //                           min(previousSDF.y + 1.0f, maxWeight),
    //                           0.0f,
    //                           0.0f);

    //        setSDF(uvec3(volPos * volSize / volDim), newSDF);
    //    }




    //}

    // }



}

