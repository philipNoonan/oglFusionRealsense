#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;

out vec4 outVertPosConf;
out vec4 outVertNormRadi;
out vec4 outVertColTimDev;

uniform float texDim;
uniform int time;

uniform sampler2D vertSamp;
uniform sampler2D colorSamp;
uniform sampler2D normSamp;

//#include "color.glsl"

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
    int intY = gl_VertexID / int(texDim);
    int intX = gl_VertexID - (intY * int(texDim));

    float halfPixel = 0.5 * (1.0f / texDim);
    float y = (float(intY) / texDim) + halfPixel;
    float x = (float(intX) / texDim) + halfPixel;
    
    vec4 newColor = textureLod(colorSamp, vec2(x, y), 0);

    //Do averaging here
    if(newColor.w == -1)
    {
        vec4 newPos = textureLod(vertSamp, vec2(x, y), 0);
        vec4 newNorm = textureLod(normSamp, vec2(x, y), 0);
        
        float c_k = vertexConfidence.w;
        vec3 v_k = vertexConfidence.xyz;
        
        float a = newPos.w;
        vec3 v_g = newPos.xyz;
        
        if(newNorm.w < (1.0 + 0.5) * normalRadius.w)
        {
	        outVertPosConf = vec4(((c_k * v_k) + (a * v_g)) / (c_k + a), c_k + a);
	        
	        vec3 oldCol = decodeColor(colorTimeDevice.x);
	        vec3 newCol = decodeColor(newColor.x);
           
            vec3 avgColor = ((c_k * oldCol.xyz) + (a * newCol.xyz)) / (c_k + a);
            
	        outVertColTimDev = vec4(encodeColor(avgColor), colorTimeDevice.y, colorTimeDevice.z, time);
	        
	        outVertNormRadi = ((c_k * normalRadius) + (a * newNorm)) / (c_k + a);
	        
	        outVertNormRadi.xyz = normalize(outVertNormRadi.xyz);
        }
        else
        {
            outVertPosConf = vertexConfidence;
            outVertNormRadi = normalRadius;
            outVertColTimDev = colorTimeDevice;

            outVertPosConf.w = c_k + a;
            outVertColTimDev.w = time; // CHECK ME LATER TO CHANGE TO W
        }
    }
    else
    {
        //This point isn't being updated, so just transfer it
	    outVertPosConf = vertexConfidence;
	    outVertNormRadi = normalRadius;
		outVertColTimDev = colorTimeDevice;
    }
}