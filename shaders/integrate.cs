#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rgba16f) uniform image3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively

layout(binding = 0) uniform sampler2DArray depthTextureArray; // 
layout(binding = 1) uniform sampler2DArray trackTextureArray; // 

layout(std140) uniform Configs
{
    int numberOfCameras;
    int d2p;
    int d2v;
    float dMax;
    float dMin;
    float maxWeight;
    float depthScale;
    float volDim; // vol dim real world span of the volume in meters
    float volSize; // voxel grid size of the volume 
};

uniform int forceIntegrate;



uniform mat4 cameraPoses[4];
uniform mat4 cameraIntrinsics[4];
uniform mat4 inverseCameraIntrinsics[4];




vec3 getVolumePosition(uvec3 p)
{
    return vec3((p.x + 0.5f) * volDim / volSize, (p.y + 0.5f) * volDim / volSize, (p.z + 0.5f) * volDim / volSize);
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
    ivec2 depthSize = ivec2(textureSize(depthTextureArray,0).xy);
    uvec3 pix = gl_GlobalInvocationID.xyz;

    mat4 trackMat;// = cameraMat[0];
    float diff[4]; // max number of cameras on one system
    //imageStore(volumeData, ivec3(pix), vec4(0.1f));
    vec4 track[4];
    int bestTrack;

    for (pix.z = 0; pix.z < volSize; pix.z++)
    {
        //imageStore(volumeData, ivec3(pix), vec4(0.2f));


        for (int cameraDevice = 0; cameraDevice < numberOfCameras; cameraDevice++)
        {
            //imageStore(volumeData, ivec3(pix), vec4(cameraDevice * 0.1));

            trackMat = cameraPoses[cameraDevice];
            
            // get world position of voxel 
            vec3 worldPos = vec3(trackMat * vec4(getVolumePosition(pix), 1.0f)).xyz;
            // get clip position
            vec4 pClip = cameraIntrinsics[cameraDevice] * vec4(worldPos, 1.0f);
            // determin if valid pixel for each camera
            vec2 pixel = vec2(pClip.x / pClip.z, pClip.y / pClip.z);
     
            // if we dont check if we hit the image here and just assume that if pixel is out of bounds the resultant texture read will be zero
            if (pixel.x < 0 || pixel.x > depthSize.x - 1 || pixel.y < 0 || pixel.y > depthSize.y - 1)
            {
                diff[cameraDevice] = -10000.0f;
                continue;
            }
            //imageStore(volumeData, ivec3(pix), vec4(volDim, volSize, volSize, volDim));
            track[cameraDevice] = texelFetch(trackTextureArray, ivec3(pixel.x, pixel.y, cameraDevice), 0); // this may be 6553

            //if (track == vec4(1.0f, 1.0f, 0.0f, 1.0f) && forceIntegrate == 0)
            //{
            //    continue;
            //}

            float depth = float(texelFetch(depthTextureArray, ivec3(pixel.x, pixel.y, cameraDevice), 0).x) * 65535.0f * depthScale; // this may be 65535

            //if (depth == 0)
            //{
                //imageStore(volumeData, ivec3(pix), vec4(pixel, 0.9, 1.0f));
            //}
            vec4 depthPoint = (depth) * (inverseCameraIntrinsics[cameraDevice] * vec4(pixel.x, pixel.y, 1.0f, 0.0f));
            if (depthPoint.z <= 0.0f)
            {
                diff[cameraDevice] = -10000.0f;
                continue;
            }
            //imageStore(volumeData, ivec3(pix), vec4(0.2, 0.3, 0.4, 1.0f));

            // if we get here, then the voxel is seen by this cameraDevice
            // determin best cameraDevice
            vec3 shiftVec = worldPos - depthPoint.xyz;
            float tdiff = length(shiftVec);
            diff[cameraDevice] = shiftVec.z < 0.0 ? tdiff : -tdiff;
        }

        float finalDiff = 10000.0f;
        float validCameras = 0;
        for (int cameraDevice = 0; cameraDevice < numberOfCameras; cameraDevice++)
        {
            if (diff[cameraDevice] != 10000.0f)
            {
                if (abs(diff[cameraDevice]) < abs(finalDiff))
                {
                    bestTrack = cameraDevice;
                    finalDiff = diff[cameraDevice];
                }

                // closest takes all
                 //   finalDiff = abs(diff[cameraDevice]) < abs(finalDiff) ? diff[cameraDevice] : finalDiff;
                // merge data from both cameras
                //finalDiff += diff[cameraDevice];
                //validCameras++;
            }
        }
        //finalDiff /= validCameras;
        float ctfo = 0.0f;
        if (track[bestTrack] == vec4(0.5f, 0.5f, 0.5f, 1.0 ))
        {
            ctfo = 0.1f;
        }
        else if (track[bestTrack] == vec4(1.0f, 1.0f, 0.0f, 1.0))
        {
            ctfo = 0.001f;
        }
        else if (track[bestTrack] == vec4(1.0f, 0.0f, 0.0f, 1.0))
        {
            ctfo = 0.001f;
        }
        // if diff within TSDF range, write to volume
        if (finalDiff < dMax && finalDiff > dMin)
        {
            vec4 data = getSDF(pix);
            float weightedDistance = 0.0f;
            if (d2p == 1)
            {
                weightedDistance = (data.y * data.x + finalDiff) / (data.y + 1);
            }
            else if (d2v == 1)
            {
                weightedDistance = (data.y * data.x + ctfo * finalDiff) / (data.y + ctfo);
            }

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
            //imageStore(volumeData, ivec3(pix), vec4(0.0f));
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
    integrateExperimental();
}

