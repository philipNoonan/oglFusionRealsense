#version 430 core

in vec4 fragVertConf;
in vec4 fragNormRadi;
in vec4 fragColTimDev;

uniform vec4 camPam;
uniform float maxDepth;

layout(location = 0) out vec4 outVertex;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColTimStab;
layout(location = 3) out vec4 outConRadDev;




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
    vec3 l = normalize(vec3((vec2(gl_FragCoord) - camPam.xy) / camPam.zw, 1.0f));
    
    vec3 corrected_pos = (dot(fragVertConf.xyz, fragNormRadi.xyz) / dot(l, fragNormRadi.xyz)) * l; 

    //check if the intersection is inside the surfel
    float sqrRad = pow(fragNormRadi.w, 2);
    vec3 diff = corrected_pos - fragVertConf.xyz;

    if(dot(diff, diff) > sqrRad) 
    {
        discard;
    }

    outColTimStab = vec4(fragColTimDev.xyz, 1); 
    
    float z = corrected_pos.z;
	// is this S.O. relevent?
    // https://stackoverflow.com/questions/701504/perspective-projection-determine-the-2d-screen-coordinates-x-y-of-points-in-3/701978#701978
    outVertex = vec4((gl_FragCoord.x - camPam.x) * z * (1.f / camPam.z), (gl_FragCoord.y - camPam.y) * z * (1.f / camPam.w), z, 1.0f);
    //outVertex = fragVertConf;
    outNormal = vec4(fragNormRadi.xyz, 1.0f); // this doesnt like being non 1.0f for w
    //outNormRadi = vec4(0.5f, 0.25f, 0.5f, 1.0f);
    //outTime = uint(fragColTimDev.z);
    
	outConRadDev = vec4(fragVertConf.w, fragNormRadi.w, fragColTimDev.w, 1.0f);

    gl_FragDepth = (corrected_pos.z / (2.0f * maxDepth)) + 0.5f; // scale to some max depth but still allow for the depth to be discarded if it is greater than other frags


}