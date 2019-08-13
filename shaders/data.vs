#version 430 core

out vec4 geoVertPosConf;
out vec4 geoVertNormRadi;
out vec4 geoVertColTimDev;
flat out int updateId;

layout (binding=0) uniform sampler2DArray colorSampler;
layout (binding=1) uniform sampler2DArray depthSampler;

layout (binding=2) uniform sampler2D vertConfSampler; // 4x size
layout (binding=3) uniform sampler2D colTimDevSampler; // 4x size
layout (binding=4) uniform sampler2D normRadiSampler; // 4x size
layout (binding=5) uniform usampler2D indexSampler; // 4x size

uniform vec4 camPam[4]; //cx, cy, 1/fx, 1/fy
uniform float scale; // index map scale = 4.0f
uniform mat4 pose;
uniform float maxDepth;
uniform float time;
uniform float weighting;

vec2 imSize;
float texDim = 3072; // why this nuber?

vec3 getVert(sampler2DArray depthTex, vec3 textureCoord, ivec3 texelCoord)
{
	float z = float(textureLod(depthTex, textureCoord, 0.0f).x); // SINGLE CAMERA MODE

	return vec3((texelCoord.x - camPam[texelCoord.z].x) * z * camPam[texelCoord.z].z,
			    (texelCoord.y - camPam[texelCoord.z].y) * z * camPam[texelCoord.z].w, 
			     z);
}

vec3 getNorm(sampler2DArray depthTex, vec4 centreVertexPosition, vec3 textureCoord, vec3 textureShift, ivec3 texelCoord)
{
	vec3 posXF = getVert(depthTex, textureCoord + vec3(textureShift.x, 0, 0), texelCoord + ivec3(1, 0, 0));
	vec3 posXB = getVert(depthTex, textureCoord - vec3(textureShift.x, 0, 0), texelCoord - ivec3(1, 0, 0));

	vec3 posYF = getVert(depthTex, textureCoord + vec3(0, textureShift.y, 0), texelCoord + ivec3(0, 1, 0));
	vec3 posYB = getVert(depthTex, textureCoord - vec3(0, textureShift.y, 0), texelCoord - ivec3(0, 1, 0));

	vec3 dX = ((posXB + centreVertexPosition.xyz) / 2.0) - ((posXF + centreVertexPosition.xyz) / 2.0);
    vec3 dY = ((posYB + centreVertexPosition.xyz) / 2.0) - ((posYF + centreVertexPosition.xyz) / 2.0);
    
    return normalize(cross(dX, dY));
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

bool checkNeighbours(vec3 texCoord, sampler2DArray depth)
{
    float z = float(textureLod(depth, vec3(texCoord.x - (1.0 / imSize.x), texCoord.y, texCoord.z), 0.0));
    if(z == 0)
        return false;
        
    z = float(textureLod(depth, vec3(texCoord.x, texCoord.y - (1.0 / imSize.y), texCoord.z), 0.0));
    if(z == 0)
        return false;

    z = float(textureLod(depth, vec3(texCoord.x + (1.0 / imSize.x), texCoord.y, texCoord.z), 0.0));
    if(z == 0)
        return false;
        
    z = float(textureLod(depth, vec3(texCoord.x, texCoord.y + (1.0 / imSize.y), texCoord.z), 0.0));
    if(z == 0)
        return false;
        
    return true;
}

float angleBetween(vec3 a, vec3 b)
{
    return acos(dot(a, b) / (length(a) * length(b)));
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
	ivec3 texSize = textureSize(depthSampler, 0);
	imSize = vec2(texSize).xy;
	int vertID = gl_VertexID;
	ivec3 texelCoord = ivec3(vertID % texSize.x, (vertID / texSize.x) % texSize.y, vertID / (texSize.y * texSize.x));
	vec3 textureCoord = vec3(float(texelCoord.x + 0.5f) / float(texSize.x), float(texelCoord.y + 0.5) / float(texSize.y), float(texelCoord.z));

	//Vertex position integrated into model transformed to global coords
    vec3 vPosLocal = getVert(depthSampler, textureCoord, texelCoord);
    geoVertPosConf = pose * vec4(vPosLocal, 1);

	geoVertColTimDev = textureLod(colorSampler, textureCoord, 0.0f);
    geoVertColTimDev.x = encodeColor(geoVertColTimDev.xyz);
	geoVertColTimDev.y = time;
	geoVertColTimDev.z = texelCoord.z;
	geoVertColTimDev.w = 0;

	//Normal and radius computed with filtered position / depth map transformed to global coords
    vec3 vNormLocal = getNorm(depthSampler, geoVertPosConf, textureCoord, 1.0f / vec3(texSize.xyz), texelCoord);
    geoVertNormRadi = vec4(mat3(pose) * vNormLocal, getRadi(geoVertPosConf.z, vNormLocal.z, texelCoord.z));

	geoVertPosConf.w = getConf(texelCoord, weighting);

	updateId = 0;

	uint best = 0U;


	//If this point is actually a valid vertex (i.e. has depth)
    if(texelCoord.x % 2 == int(time) % 2 && texelCoord.y % 2 == int(time) % 2 && 
       checkNeighbours(textureCoord.xyz, depthSampler) && vPosLocal.z > 0 && 
       vPosLocal.z <= maxDepth)
	{
		int counter = 0;

	    float indexXStep = (1.0f / (imSize.x * scale)) * 0.5f;
	    float indexYStep = (1.0f / (imSize.y * scale)) * 0.5f;

	    float bestDist = 1000;
	   
	    float windowMultiplier = 2;   

	    float xl = (texelCoord.x - camPam[0].x) * camPam[0].z;
        float yl = (texelCoord.y - camPam[0].y) * camPam[0].w;
	   
	    float lambda = sqrt(xl * xl + yl * yl + 1);

	    vec3 ray = vec3(xl, yl, 1);

	    // find best

		for(float i = textureCoord.x - (scale * indexXStep * windowMultiplier); i < textureCoord.x + (scale * indexXStep * windowMultiplier); i += indexXStep)
		{
	        for(float j = textureCoord.y - (scale * indexYStep * windowMultiplier); j < textureCoord.y + (scale * indexYStep * windowMultiplier); j += indexYStep)
			{
		       uint current = uint(textureLod(indexSampler, vec2(i, j), 0.0));
			   if(current > 0U)
				{
			        vec4 vertConf = textureLod(vertConfSampler, vec2(i, j), 0.0);
			        if(abs((vertConf.z * lambda) - (vPosLocal.z * lambda)) < 0.05)
					{
				        float dist = length(cross(ray, vertConf.xyz)) / length(ray);
					    vec4 normRad = textureLod(normRadiSampler, vec2(i, j), 0.0);

					    if(dist < bestDist && (abs(normRad.z) < 0.75f || abs(angleBetween(normRad.xyz, vNormLocal.xyz)) < 0.5f))
					    {
						    counter++;
                            bestDist = dist;
							best = current;
						}
					}
				}
			}
		}
		
	    //We found a point to merge with
	    if(counter > 0)
	    {
	       updateId = 1;
	       geoVertColTimDev.w = -1;
	    }
	    else
	    {
	       //New unstable vertex
	       updateId = 2;
	       geoVertColTimDev.w = -2;
	    }


	}

	//Output vertex id of the existing point to update
    if(updateId == 1)
    {
	    uint intY = best / uint(texDim);
	    uint intX = best - (intY * uint(texDim));
	    
	    float halfPixel = 0.5 * (1.0f / texDim);
	    
	    //should set gl_Position here to the 2D index for the updated vertex ID
	    gl_Position = vec4(float(int(intX) - (int(texDim) / 2)) / (texDim / 2.0) + halfPixel, 
	                       float(int(intY) - (int(texDim) / 2)) / (texDim / 2.0) + halfPixel, 
	                       0, 
	                       1.0);
    }
    else
    {
        //Either don't render anything, or output a new unstable vertex offscreen
        gl_Position = vec4(-10, -10, 0, 1);
    }
}