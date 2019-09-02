#version 430 core

in vec4 fragVertConf;
in vec4 fragNormRadi;
in vec4 fragColTimDev;

uniform vec4 camPam[4];
uniform float maxDepth;

layout(location = 0) out vec4 outVertConf;
layout(location = 1) out vec4 outNormRadi;
layout(location = 2) out vec4 outColTimDev;
layout(location = 3) out uint outTime;




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
    vec3 l = normalize(vec3((vec2(gl_FragCoord) - camPam[0].xy) / camPam[0].zw, 1.0f));
    
    vec3 corrected_pos = (dot(fragVertConf.xyz, fragNormRadi.xyz) / dot(l, fragNormRadi.xyz)) * l; 

    //check if the intersection is inside the surfel
    float sqrRad = pow(fragNormRadi.w, 2);
    vec3 diff = corrected_pos - fragVertConf.xyz;

    if(dot(diff, diff) > sqrRad)
    {
        discard;
    }

    outColTimDev = vec4(decodeColor(fragColTimDev.x), 1);
    
    float z = corrected_pos.z;
    
    outVertConf = vec4((gl_FragCoord.x - camPam[0].x) * z * (1.f / camPam[0].z), (gl_FragCoord.y - camPam[0].y) * z * (1.f / camPam[0].w), z, fragVertConf.w);
    
    outNormRadi = vec4(fragNormRadi.xyz, 1.0f);
    
    outTime = uint(fragColTimDev.z);
    
    gl_FragDepth = (corrected_pos.z / (2 * maxDepth)) + 0.5f;


}