#version 430

layout(local_size_x = 32) in;

layout(std430, binding = 0) buffer feedbackBuffer
{
    vec4 interleavedData [];
};

layout(binding = 0, rgba32f) uniform image2D outVertConf;
layout(binding = 1, rgba32f) uniform image2D outNormRadi;
layout(binding = 2, rgba32f) uniform image2D outColTimDev;
layout(binding = 3, r32ui) uniform image2D outTime;

uniform uint maxNumVerts;
uniform mat4 inversePose[4];
uniform vec4 camPam[4]; // cx cy fx fy
uniform vec2 imSize;
uniform float maxDepth;
uniform float confThreshold;
uniform int time;
uniform int maxTime;
uniform int timeDelta;

vec3 projectPoint(vec3 p)
{
    return vec3(((((camPam[0].z * p.x) / p.z) + camPam[0].x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((camPam[0].w * p.y) / p.z) + camPam[0].y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}

vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam[0].z * p.x) / p.z) + camPam[0].x,
                ((camPam[0].w * p.y) / p.z) + camPam[0].y,
                p.z);
}


float encodeColor(vec3 c)
{
    int rgb = int(round(c.x * 255.0f));
    rgb = (rgb << 8) + int(round(c.y * 255.0f));
    rgb = (rgb << 8) + int(round(c.z * 255.0f));
    return float(rgb);
}

vec3 decodeColor(float c)
{
    vec3 col;
    col.x = float(int(c) >> 16 & 0xFF) / 255.0f;
    col.y = float(int(c) >> 8 & 0xFF) / 255.0f;
    col.z = float(int(c) & 0xFF) / 255.0f;
    return col;
}

void main()
{
    uint vertID = gl_GlobalInvocationID.x;


    vec4 position = vec4(interleavedData[(vertID * 3)].xyz, 1.0f);
    vec4 normal = vec4(interleavedData[(vertID * 3) + 1].xyz, 0.0f);

    float confidence = interleavedData[(vertID * 3)].w;
    float radius = interleavedData[(vertID * 3) + 1].w;

    float color = interleavedData[(vertID * 3) + 2].x;
    int device = int(interleavedData[(vertID * 3) + 2].y);
    int vertTime = int(interleavedData[(vertID * 3) + 2].z);
    int miscTime = int(interleavedData[(vertID * 3) + 2].w);


    vec4 vPosHome = inversePose[0] * position;

    if (vPosHome.z > maxDepth || vPosHome.z < 0 || confidence < confThreshold || time - vertTime > timeDelta || miscTime > maxTime)
    {

    }
    else
    {
        vec4 glPosition = vec4(projectPoint(vPosHome.xyz), 1.0);

        vec4 glNormal = normalize(inversePose[0] * normal); // Is this correct transform for normals?

        vec3 x1 = normalize(vec3((glNormal.y - glNormal.z), -glNormal.x, glNormal.x)) * radius * 1.41421356;

        vec3 y1 = cross(glNormal.xyz, x1);

        vec4 proj1 = vec4(projectPointImage(vPosHome.xyz + x1), 1.0);
        vec4 proj2 = vec4(projectPointImage(vPosHome.xyz + y1), 1.0);
        vec4 proj3 = vec4(projectPointImage(vPosHome.xyz - y1), 1.0);
        vec4 proj4 = vec4(projectPointImage(vPosHome.xyz - x1), 1.0);

        vec2 xs = vec2(min(proj1.x, min(proj2.x, min(proj3.x, proj4.x))), max(proj1.x, max(proj2.x, max(proj3.x, proj4.x))));
        vec2 ys = vec2(min(proj1.y, min(proj2.y, min(proj3.y, proj4.y))), max(proj1.y, max(proj2.y, max(proj3.y, proj4.y))));

        float xDiff = abs(xs.y - xs.x);
        float yDiff = abs(ys.y - ys.x);

        float glPointSize = max(0, max(xDiff, yDiff));
    }


    // frag coord is pixel coord
    vec3 l = normalize(vec3((vec2(glFragCoord) - camPam[0].xy) / camPam[0].zw, 1.0f)); // what is this doing with gl_fragcoord?
    // l defines the ray out at this pixel location for z = 1.0f

    vec3 corrected_pos = (dot(glPosition.xyz, glNormal.xyz) / dot(l, glNormal.xyz)) * l;

    //check if the intersection is inside the surfel
    float sqrRad = pow(radius, 2);
    vec3 diff = corrected_pos - position.xyz;



    if (dot(diff, diff) < sqrRad)
    {
        // check if fragdepth is lower than the current pixel fragdepth, i.e. compute shader depth culling
        float previousDepth = imageLoad(outVertConf, ivec2(glFragCoord)).z;

        if (previousDepth < (corrected_pos.z / (2 * maxDepth)) + 0.5f)
        {
            imageStore(outColTimDev, ivec2(glFragCoord), vec4(decodeColor(fragColTimDev.x), 1));

            float z = corrected_pos.z;

            imageStore(outVertConf, ivec2(glFragCoord), vec4((glFragCoord.x - camPam[0].x) * z * (1.f / camPam[0].z), (glFragCoord.y - camPam[0].y) * z * (1.f / camPam[0].w), z, confidence));

            imageStore(outNormRadi, ivec2(glFragCoord), vec4(normal.xyz, radius));

            imageStore(outTime, ivec2(glFragCoord), uvec4(vertTime));

            float glFragDepth = (corrected_pos.z / (2 * maxDepth)) + 0.5f;
        }


    }



}


















}