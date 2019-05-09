#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0) uniform sampler3D volumeDataTexture; // interp access

layout(binding = 0, rgba16f) uniform image3D volumeData; // texel access, lets define r as main sdf, g as running fused, b as to be fused, a as weights for each, 0-1000 for running, 1000-2000 for to be fused
layout(binding = 1, rgba32f) uniform image2D inVertex;
layout(binding = 2, rgba32f) uniform image2D inNormal;

layout(binding = 3, rgba32f) uniform image2D testImage;
layout(binding = 4, rgba32f) uniform image2D trackImage;

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

uniform ivec2 imageSize; 

uniform mat4 Ttrack;
uniform vec3 volDim;
uniform vec3 volSize; 
uniform float c;
uniform float eps;

uniform float dMax;
uniform float dMin;

uniform int devNumber;

uniform float robust_statistic_coefficent = 0.02f;

mat3 r1p;
mat3 r1m;

mat3 r2p;
mat3 r2m;

mat3 r3p;
mat3 r3m;

void setMats()
{
    float w_h = 0.01f;
    mat3 rot;
    rot[0] = vec3(Ttrack[0][0], Ttrack[0][1], Ttrack[0][2]);
    rot[1] = vec3(Ttrack[1][0], Ttrack[1][1], Ttrack[1][2]);
    rot[2] = vec3(Ttrack[2][0], Ttrack[2][1], Ttrack[2][2]);

    mat3 Rotdiff;
    Rotdiff[0][0] = 1.0f; Rotdiff[1][0] = 0.0f; Rotdiff[2][0] = 0.0f;
    Rotdiff[0][1] = 0.0f; Rotdiff[1][1] = 1.0f; Rotdiff[2][1] = -w_h;
    Rotdiff[0][2] = 0.0f; Rotdiff[1][2] = w_h; Rotdiff[2][2] = 1.0f;

    r1p = Rotdiff * rot;

    Rotdiff[2][1] = w_h;
    Rotdiff[1][2] = -w_h;

    r1m = Rotdiff * rot;

    Rotdiff[2][1] = 0;
    Rotdiff[1][2] = 0;
    Rotdiff[2][0] = w_h;
    Rotdiff[0][2] = -w_h;

    r2p = Rotdiff * rot;

    Rotdiff[2][0] = -w_h;
    Rotdiff[0][2] = w_h;

    r2m = Rotdiff * rot;

    Rotdiff[2][0] = 0;
    Rotdiff[0][2] = 0;
    Rotdiff[1][0] = -w_h;
    Rotdiff[0][1] = w_h;

    r3p = Rotdiff * rot;

    Rotdiff[1][0] = w_h;
    Rotdiff[0][1] = -w_h;

    r3m = Rotdiff * rot;
}

//float vs(uvec3 pos, inout bool interpolated)

float vs(uvec3 pos)
{
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    //return data.x; // convert short to float

    vec4 data = imageLoad(volumeData, ivec3(pos));
    //if (data.y > 0)
    //{
    //    interpolated = true;
    //}
    //else
    //{
    //    interpolated = false;
    //}
    //return 0.0f;
    return data.x; // convert short to float

}

float vsGrad(uvec3 pos)
{
    vec4 data = imageLoad(volumeData, ivec3(pos));
    return data.y;
}




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
    //float currSDF = float(textureLod(volumeDataTexture, locationInVolumeTexture, 0).x) * 0.000030517f;
    //float currSDF = texelFetch(volumeDataTexture, ivec3(location), 0).x;
    float currSDF = imageLoad(volumeData, ivec3(location)).x;
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

float interpDistance(vec3 pos, inout bool interpolated)
{
    //vec3 scaled_pos = vec3((pos.x * volSize.x / volDim.x) - 0.5f, (pos.y * volSize.y / volDim.y) - 0.5f, (pos.z * volSize.z / volDim.z) - 0.5f);

    float i = pos.x;
    float j = pos.y;
    float k = pos.z;
    float w_sum = 0.0;
    float sum_d = 0.0;

    ivec3 current_voxel;
    float w = 0;
    float volume;
    int a_idx;
    interpolated = false;

    for (int i_offset = 0; i_offset < 2; i_offset++)
    {
        for (int j_offset = 0; j_offset < 2; j_offset++)
        {
            for (int k_offset = 0; k_offset < 2; k_offset++)
            {
                current_voxel.x = int(i) + i_offset;
                current_voxel.y = int(j) + j_offset;
                current_voxel.z = int(k) + k_offset;
                volume = abs(current_voxel.x - i) + abs(current_voxel.y - j) + abs(current_voxel.z - k);

                vec4 data = imageLoad(volumeData, current_voxel);
                if (data.y > 0)
                {
                    interpolated = true;
                    if (volume < 0.00001)
                    {
                        return float(data.x);
                    }
                    w = 1.0f / volume;
                    w_sum += float(w);
                    sum_d += float(w) * float(data.x);
                }

            }
        }
    }
    return sum_d / w_sum;
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

    uint offset = uint(devNumber * imageSize.x * imageSize.y);

    uvec2 pix = gl_GlobalInvocationID.xy;
    //ivec2 inSize = imageSize(inVertex); // mipmapped sizes
    imageStore(testImage, ivec2(pix), vec4(0.0f));
    imageStore(trackImage, ivec2(pix), vec4(0.0f));


    //if (pix.x >= 0 && pix.x < imageSize.x - 1 && pix.y >= 0 && pix.y < imageSize.y)
    if (all(greaterThanEqual(pix, uvec2(0))) && all(lessThan(pix, uvec2(imageSize))))
    {
        vec4 normals = imageLoad(inNormal, ivec2(pix));

        if (normals.x == 2)
        {
            trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].result = -4; 
            imageStore(trackImage, ivec2(pix), vec4(0, 0, 0, 0));

        }
        else
        {
            setMats();

            vec4 cameraPoint = imageLoad(inVertex, ivec2(pix));

            vec4 projectedVertex = Ttrack * vec4(cameraPoint.xyz, 1.0f);

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
                imageStore(trackImage, ivec2(pix), vec4(0.0f, 0.0f, 1.0, 1.0f));
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].result = -4;

                return;
            }

            //vec3 cvp = vec3(((projectedVertex.x) * volSize.x / volDim.x), ((projectedVertex.y) * volSize.y / volDim.y), ((projectedVertex.z) * volSize.z / volDim.z));
            bool valSDF = true;
            float D = SDF(cvp.xyz, valSDF);

            //float Dup = SDF(cvp + vec3(0,0,1));

            float voxelLength = volDim.x / volSize.x; // in meters
            // umvox in 2.5 cm
            float numVox = (min(dMax,dMin) / voxelLength) * 0.05f;



            vec3 dSDF_dx = vec3(SDFGradient(cvp, vec3(numVox, 0, 0), numVox), SDFGradient(cvp, vec3(0, numVox, 0), numVox), SDFGradient(cvp, vec3(0, 0, numVox), numVox));



            vec3 rotatedNormal = vec3(Ttrack * vec4(normals.xyz, 0.0f)).xyz;

            if (dot(dSDF_dx, rotatedNormal) < 0.8 && !any(equal(dSDF_dx, vec3(0.0f))))
            {
                imageStore(trackImage, ivec2(pix), vec4(1.0f, 1.0f, 0, 1.0f));

                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].result = -4;

                return;
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
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].result = -4;
                imageStore(trackImage, ivec2(pix), vec4(1.0f, 0.0f, 0, 1.0f));

                return;
            }

            imageStore(testImage, ivec2(pix), vec4(dSDF_dx, 1.0f));

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

trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].result = 1;

                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].h = huber;
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].D = D;
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].J = J;

                imageStore(trackImage, ivec2(pix), vec4(0.5f, 0.5f, 0.5f, 1.0f));

               
            }
            else
            {
                  // imageStore(testImage, ivec2(pix), vec4(0,0,0, 1.0f));

               // imageStore(testImage, ivec2(pix), vec4(0.0f, 1.0f, 0.0f, 1.0f));
                imageStore(trackImage, ivec2(pix), vec4(0, 0, 1.0f, 1.0f));

                float J0[6] = { 0, 0, 0, 0, 0, 0 };
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].result = -4;

                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].h = 0;
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].D = 0;
                trackOutput[offset + ((pix.y * imageSize.x) + pix.x)].J = J0;
            }
        } 
    }
}




