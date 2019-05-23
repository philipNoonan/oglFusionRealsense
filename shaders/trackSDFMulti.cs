#version 430

layout(local_size_x = 32, local_size_y = 32) in;

// bind textures
layout(binding = 0) uniform sampler3D volumeDataTexture; 
layout(binding = 1) uniform sampler2DArray vertexTextureArray; 
layout(binding = 2) uniform sampler2DArray normalTextureArray; 
// bind images
layout(binding = 0, rgba16f) uniform image3D volumeData;
layout(binding = 1, rgba32f) uniform image2DArray SDFImage;
layout(binding = 2, rgba32f) uniform image2DArray trackImage;

    uniform mat4 cameraPoses[4];


struct reduSDFType
{
    int result;
    float h;
    float D;
    float J[6];
};
layout(std430, binding = 12) buffer TrackData
{
    reduSDFType trackOutput [];
};

uniform int numberOfCameras;

//uniform mat4 Ttrack;
uniform vec3 volDim;
uniform vec3 volSize; 
uniform float c;
uniform float eps;

uniform float dMax;
uniform float dMin;

uniform int devNumber;
uniform int mip;

uniform float robust_statistic_coefficent = 0.02f;


//float vs(uvec3 pos, inout bool interpolated)



bool invalidGradient(float inputSDF)
{
    if (inputSDF <= -0.2f || inputSDF >= 0.2f || isnan(inputSDF))
    {
        return true;
    }
    return false;
}

float SDF(vec3 location, inout bool validGradient)
{
    validGradient = true;
    float i, j, k;
    float x, y, z;

    x = modf(location.x, i);
    y = modf(location.y, j);
    z = modf(location.z, k);

    int I = int(i);
    int J = int(j);
    int K = int(k);

    vec3 locationInVolumeTexture = vec3(location / volSize.x);
    //float N0 = imageLoad(volumeData, ivec3(I, J, K)).x * 0.00003051944088f;
    //float N1 = imageLoad(volumeData, ivec3(I, J + 1.0, K)).x * 0.00003051944088f;
    //float N2 = imageLoad(volumeData, ivec3(I + 1.0, J, K)).x * 0.00003051944088f;
    //float N3 = imageLoad(volumeData, ivec3(I + 1.0, J + 1.0, K)).x * 0.00003051944088f;

    //float N4 = imageLoad(volumeData, ivec3(I, J, K + 1)).x * 0.00003051944088f;
    //float N5 = imageLoad(volumeData, ivec3(I, J + 1.0, K + 1)).x * 0.00003051944088f;
    //float N6 = imageLoad(volumeData, ivec3(I + 1.0, J, K + 1)).x * 0.00003051944088f;
    //float N7 = imageLoad(volumeData, ivec3(I + 1.0, J + 1.0, K + 1)).x * 0.00003051944088f;

    //float a0, a1, b0, b1;

    //a0 = N0 * (1.0 - z) + N4 * z;
    //a1 = N1 * (1.0 - z) + N5 * z;
    //b0 = N2 * (1.0 - z) + N6 * z;
    //b1 = N3 * (1.0 - z) + N7 * z;

    //if (invalidGradient(N0) ||
    //    invalidGradient(N1) ||
    //    invalidGradient(N2) ||
    //    invalidGradient(N3) ||
    //    invalidGradient(N4) ||
    //    invalidGradient(N5) ||
    //    invalidGradient(N6) ||
    //    invalidGradient(N7))
    //{
    //    validGradient = false;
    //}

    //return (a0 * (1.0 - y) + a1 * y) * (1.0 - x) + (b0 * (1.0 - y) + b1 * y) * x;
    float currSDF = float(textureLod(volumeDataTexture, locationInVolumeTexture, 0).x);
    //float currSDF = texelFetch(volumeDataTexture, ivec3(location), 0).x;
    //float currSDF = imageLoad(volumeData, ivec3(location)).x;
    //float currSDF = texelFetch(volumeDataTexture, ivec3(location), 0).x;

    if (abs(currSDF) < 0.0001)
    {
        validGradient = false;
    }
    return currSDF;
    //return imageLoad(volumeData, ivec3(I, J, K)).x * 0.000030517f;
}


float SDFGradient(vec3 location, vec3 location_offset, float numVoxels)
{
    float voxelLength = volDim.x / volSize.x;
    //vec3 location_offset = vec3(0, 0, 0);
    //location_offset(dim) = stepSize;
    bool valGradUpper;
    bool valGradLower;
    float gradient;

    float upperVal = SDF(vec3(location.xyz + location_offset), valGradUpper);

    float lowerVal = SDF(vec3(location.xyz - location_offset), valGradLower);

    if (valGradUpper == false && valGradLower == true)
    {
        gradient = lowerVal;
    }
    else if (valGradUpper == true && valGradLower == false)
    {
        gradient = upperVal;
    }
    else if (valGradUpper == false && valGradLower == false)
    {
        gradient = -2.0f;
    }
    else
    {
        gradient = (upperVal - lowerVal) / (2.0f * voxelLength * numVoxels);
    }
    return gradient;



}

float[6] getJ(vec3 dsdf, float[3][6] dxdxi)
{
    float oJ[6];
    oJ[0] = dsdf.x * dxdxi[0][0] + dsdf.y * dxdxi[1][0] + dsdf.z * dxdxi[2][0];
    oJ[1] = dsdf.x * dxdxi[0][1] + dsdf.y * dxdxi[1][1] + dsdf.z * dxdxi[2][1];
    oJ[2] = dsdf.x * dxdxi[0][2] + dsdf.y * dxdxi[1][2] + dsdf.z * dxdxi[2][2];
    oJ[3] = dsdf.x * dxdxi[0][3] + dsdf.y * dxdxi[1][3] + dsdf.z * dxdxi[2][3];
    oJ[4] = dsdf.x * dxdxi[0][4] + dsdf.y * dxdxi[1][4] + dsdf.z * dxdxi[2][4];
    oJ[5] = dsdf.x * dxdxi[0][5] + dsdf.y * dxdxi[1][5] + dsdf.z * dxdxi[2][5];
    return oJ;
}


void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    ivec3 imSize = imageSize(trackImage) >> mip;

    for (int camera = 0; camera < numberOfCameras; camera++)
    { 
        imageStore(SDFImage, ivec3(pix, camera), vec4(0.0f, 0.0f, 0.0, 1.0));
        imageStore(trackImage, ivec3(pix, camera), vec4(0.0f, 0.0, 0.0, 1.0));

        uint offset = uint(camera * imSize.x * imSize.y) + ((pix.y * imSize.x) + pix.x);


        //if (pix.x >= 0 && pix.x < imageSize.x - 1 && pix.y >= 0 && pix.y < imageSize.y)
        if (all(greaterThanEqual(pix, uvec2(0))) && all(lessThan(pix, uvec2(imSize.xy))))
        {
            //vec4 normals = imageLoad(inNormal, ivec2(pix));
            vec4 normals = texelFetch(normalTextureArray, ivec3(pix, camera), mip);

            if (normals.x == 2)
            {
                trackOutput[offset].result = -4;
                imageStore(trackImage, ivec3(pix, camera), vec4(0, 0, 0, 0));

            }
            else
            {
                //setMats(camera);

                //vec4 cameraPoint = imageLoad(inVertex, ivec2(pix));
                vec4 cameraPoint = texelFetch(vertexTextureArray, ivec3(pix, camera), mip);

                vec4 projectedVertex = cameraPoses[camera] * vec4(cameraPoint.xyz, 1.0f);

                bool isInterpolated;
                // float sdf_der[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
                //  float D;
                //  getPartialDerivative(cameraPoint.xyz, projectedVertex.xyz, isInterpolated, sdf_der, D);
                //  vec3 dSDF_dx = vec3(sdf_der[0], sdf_der[1], sdf_der[2]);

                // cvp is in texel space 0 - 127
                vec3 cvp = (volSize.x / volDim.x) * projectedVertex.xyz;

                //if (cvp.x < 0.0f || cvp.x > volSize.x || cvp.y < 0.0f || cvp.y > volSize.y || cvp.z < 0.0f || cvp.z > volSize.z)
                if (any(lessThan(cvp, vec3(0.0f))) || any(greaterThan(cvp, volSize)))
                {
                    //imageStore(testImage, ivec2(pix), vec4(0.5f));
                    imageStore(trackImage, ivec3(pix, camera), vec4(0.0f, 0.0f, 1.0, 1.0f));
                    trackOutput[offset].result = -4;

                    continue;
                }

                //vec3 cvp = vec3(((projectedVertex.x) * volSize.x / volDim.x), ((projectedVertex.y) * volSize.y / volDim.y), ((projectedVertex.z) * volSize.z / volDim.z));
                bool valSDF = true;
                float D = SDF(cvp.xyz, valSDF);

                //float Dup = SDF(cvp + vec3(0,0,1));

                //float voxelLength = volDim.x / volSize.x; // in meters
                                                          // umvox in 2.5 cm
                //float numVox = (min(dMax, dMin) / voxelLength) * 0.05f;



                vec3 dSDF_dx = vec3(SDFGradient(cvp, vec3(1, 0, 0), 1), SDFGradient(cvp, vec3(0, 1, 0), 1), SDFGradient(cvp, vec3(0, 0, 1), 1));



                vec3 rotatedNormal = vec3(cameraPoses[camera] * vec4(normals.xyz, 0.0f)).xyz;

                if (dot(dSDF_dx, rotatedNormal) < 0.8 && !any(equal(dSDF_dx, vec3(0.0f))))
                {
                    imageStore(trackImage, ivec3(pix, camera), vec4(1.0f, 1.0f, 0, 1.0f));

                    trackOutput[offset].result = -4;

                    continue;
                }

                //if (any(greaterThan(rotatedNormal, vec3(1.0f))))
                //{
                //    //imageStore(testImage, ivec2(pix), vec4(0.5f));
                //    imageStore(trackImage, ivec2(pix), vec4(1.0f, 0.0f, 0, 1.0f));

                //    trackOutput[(pix.y * imageSize.x) + pix.x].result = -4;

                //    return;
                //}

                if (any(equal(dSDF_dx, vec3(-2.0f))))
                {
                    trackOutput[offset].result = -4;
                    imageStore(trackImage, ivec3(pix, camera), vec4(1.0f, 0.0f, 0, 1.0f));

                    continue;
                }

                imageStore(SDFImage, ivec3(pix, camera), vec4(dSDF_dx, 1.0f));

                float absD = abs(D);// get abs depth

                //imageStore(testImage, ivec2(pix), vec4(absD.xxx,1.0f));

                if (D < dMax && D > dMin)
                {
                    vec3 nDSDF = normalize(dSDF_dx);

                    // 3 cols 6 rows 
                    float dx_dxi[3][6];

                dx_dxi[0][0] = 0;                   dx_dxi[1][0] = -projectedVertex.z;  dx_dxi[2][0] = projectedVertex.y;    
                dx_dxi[0][1] = projectedVertex.z;   dx_dxi[1][1] = 0;                   dx_dxi[2][1] = -projectedVertex.x; 
	            dx_dxi[0][2] = -projectedVertex.y;  dx_dxi[1][2] = projectedVertex.x;   dx_dxi[2][2] = 0;  
                dx_dxi[0][3] = 1;	                dx_dxi[1][3] = 0;                   dx_dxi[2][3] = 0;
                dx_dxi[0][4] = 0;	                dx_dxi[1][4] = 1;                   dx_dxi[2][4] = 0;
                dx_dxi[0][5] = 0;	                dx_dxi[1][5] = 0;                   dx_dxi[2][5] = 1;

                float J[6] = getJ(dSDF_dx, dx_dxi);

                float huber = absD < 0.003f ? 1.0f : 0.003f / absD;

                trackOutput[offset].result = 1;

                trackOutput[offset].h = huber;
                trackOutput[offset].D = D;
                trackOutput[offset].J = J;

                imageStore(trackImage, ivec3(pix, camera), vec4(0.5f, 0.5f, 0.5f, 1.0f));

               
            }
            else
            {
                  // imageStore(testImage, ivec2(pix), vec4(0,0,0, 1.0f));

               // imageStore(testImage, ivec2(pix), vec4(0.0f, 1.0f, 0.0f, 1.0f));
                imageStore(trackImage, ivec3(pix, camera), vec4(0.0, 0.0, 0.0f, 1.0f));

                float J0[6] = { 0, 0, 0, 0, 0, 0 };
                trackOutput[offset].result = -4;
                trackOutput[offset].h = 0;
                trackOutput[offset].D = 0;
                trackOutput[offset].J = J0;
            }
        } 
    }
    }


}




