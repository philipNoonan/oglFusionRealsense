#version 430 core

layout(binding = 0, rgba32f) uniform image2D outImagePC;
layout(binding = 1, rgba32f) uniform image2D outImageNR;

out vec4 geoVertexPositionConfidence;
out vec4 geoVertexNormalRadius;
out vec4 geoVertexColorTimeDevice;

flat out ivec2 imageCoord;

uniform sampler2DArray depthTexture;

uniform vec4 camPam[4]; // cx, cy, 1 / fx, 1 / fy
uniform mat4 invK[4];

uniform uint time;
uniform float depthScale;

vec3 getVert(sampler2DArray depthTex, vec3 textureCoord, ivec3 texelCoord)
{
	//float z = float(textureLod(depthTex, textureCoord, 0.0f).x); // SINGLE CAMERA MODE
	float z = float(texelFetch(depthTex, ivec3(texelCoord.xy, 0), 0).x) * 65535.0f * depthScale; // this may be 65535
	vec4 tPos = z * (invK[0] * vec4(texelCoord.xy, 1.0f, 0.0f));

					return tPos.xyz;

	//return vec3((float(texelCoord.x) - camPam[texelCoord.z].x) * z * camPam[texelCoord.z].z,
	//		    (float(texelCoord.y) - camPam[texelCoord.z].y) * z * camPam[texelCoord.z].w, 
	//		     z);
}

vec3 getNorm(sampler2DArray depthTex, vec4 centreVertexPosition, vec3 textureCoord, vec3 textureShift, ivec3 texelCoord)
{
	vec3 posXF = getVert(depthTex, textureCoord + vec3(textureShift.x, 0, 0), texelCoord + ivec3(1, 0, 0));
	vec3 posXB = getVert(depthTex, textureCoord - vec3(textureShift.x, 0, 0), texelCoord - ivec3(1, 0, 0));

	vec3 posYF = getVert(depthTex, textureCoord + vec3(0, textureShift.y, 0), texelCoord + ivec3(0, 1, 0));
	vec3 posYB = getVert(depthTex, textureCoord - vec3(0, textureShift.y, 0), texelCoord - ivec3(0, 1, 0));

	vec3 dX = ((posXB + centreVertexPosition.xyz) / 2.0) - ((posXF + centreVertexPosition.xyz) / 2.0);
    vec3 dY = ((posYB + centreVertexPosition.xyz) / 2.0) - ((posYF + centreVertexPosition.xyz) / 2.0);
    
    return normalize(cross(dY, dX));
}

float getRadi(float depth, float normZ, int camNumber)
{
    float meanFocal = ((1.0 / abs(camPam[camNumber].z)) + (1.0 / abs(camPam[camNumber].w))) / 2.0;
    const float sqrt2 = 1.41421356237f;
    float radius = (depth / meanFocal) * sqrt2;
    
	float radius_n = radius;
    radius_n = radius_n / abs(normZ);
    radius_n = min(2.0f * radius, radius_n);

    return radius_n;


}

float getConf(vec3 texelCoord, float weighting)
{
    const float maxRadDist = 400; //sqrt((width * 0.5)^2 + (height * 0.5)^2)
    const float twoSigmaSquared = 0.72; //2*(0.6^2) from paper
    
    vec2 pixelPosCentered = texelCoord.xy - camPam[int(texelCoord.z)].xy;
    
    float radialDist = sqrt(dot(pixelPosCentered, pixelPosCentered)) / maxRadDist;
    
    return exp((-(radialDist * radialDist) / twoSigmaSquared)) * weighting;
}

void main()
{


    // Components of a texture coordinate that reference an array layer are not normalized to the number of layers. They specify a layer by index.
	ivec3 texSize = textureSize(depthTexture, 0);
	int vertID = gl_VertexID;
	ivec3 texelCoord = ivec3(vertID % texSize.x, (vertID / texSize.x) % texSize.y, vertID / (texSize.y * texSize.x));
	vec3 textureCoord = vec3(float(texelCoord.x + 0.5f) / float(texSize.x), float(texelCoord.y + 0.5) / float(texSize.y), float(texelCoord.z + 0.5f));

	geoVertexPositionConfidence.xyz = getVert(depthTexture, textureCoord, texelCoord);

	geoVertexNormalRadius.xyz = getNorm(depthTexture, geoVertexPositionConfidence, textureCoord, 1.0f / vec3(texSize.xyz), texelCoord);
	
	geoVertexPositionConfidence.w = getConf(texelCoord, 1.0f);

	geoVertexNormalRadius.w = getRadi(geoVertexPositionConfidence.z, geoVertexNormalRadius.z, texelCoord.z);

	geoVertexColorTimeDevice= vec4(0.2, 0.6, time, texelCoord.z);

	imageCoord = texelCoord.xy;

		// wipe previous frame
	imageStore(outImagePC, imageCoord, vec4(0.0f));
	imageStore(outImageNR, imageCoord, vec4(2.0f, 0.0f, 0.0f, 0.0f));

}