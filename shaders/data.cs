#version 430 core

layout(local_size_x = 1024) in;

uint maxNumVerts;

// this is the buffer vec4 vec4 vec4 [vert conf][norm rad][col time dev] from the current depth frame from the previous depth to buffer call 
layout(std430, binding = 0) buffer feedbackBuffer
{
    vec4 interleavedData [];
};

layout(std430, binding = 1) buffer updateIndexMapBuffer
{
    vec4 outputUpdateIndexInterleaved [];
};



layout (binding=2) uniform sampler2D vertConfSampler; // 4x
layout (binding=3) uniform sampler2D colTimDevSampler; // 4x
layout (binding=4) uniform sampler2D normRadiSampler; // 4x
layout (binding=5) uniform usampler2D indexSampler; // 4x

uniform vec4 camPam[4]; //cx, cy, 1/fx, 1/fy
uniform float scale; // index map scale = 4.0f
uniform mat4 pose;
uniform float maxDepth;
uniform float time;
uniform float weighting;

vec2 imSize;

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

bool checkNeighboursFIXME(int someNumber)
{
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

// this gets run for every valid depth vertex from the depth TFO
void main()
{
	ivec2 texSize = textureSize(vertConfSampler, 0); // this is the 4x size
	imSize = vec2(texSize).xy / 4.0f;

	int vertID = int(gl_GlobalInvocationID.x); // 0 to max number of valid depth verts


	//ivec2 texelCoord = ivec2(vertID % texSize.x, (vertID / texSize.x) % texSize.y);
	//vec2 textureCoord = vec2(float(texelCoord.x + 0.5f) / float(texSize.x), float(texelCoord.y + 0.5) / float(texSize.y));

    //Vertex position integrated into model transformed to global coords
    vec4 vPosLocal = interleavedData[(vertID * 3)];

    vec4 geoVertPosConf = pose * vec4(vPosLocal.xyz, 1);
    geoVertPosConf.w = vPosLocal.w;

    vec4 geoColTimDev = interleavedData[(vertID * 3) + 1];


    //Normal and radius computed with filtered position / depth map transformed to global coords
    //vec3 vNormLocal = getNorm(depthSampler, geoVertPosConf, textureCoord, 1.0f / vec3(texSize.xyz), texelCoord);
    vec4 vNormLocal = interleavedData[(vertID * 3) + 2];// vec4(mat3(pose) * vNormLocal, getRadi(geoVertPosConf.z, vNormLocal.z, texelCoord.z));
    vec4 geoVertNormRadi = vec4(mat3(pose) * vNormLocal.xyz, vNormLocal.w);

    vec4 geoVertColTimDev;

    int updateId = 0;

	uint best = 0U;


	//If this point is actually a valid vertex (i.e. has depth)
    // this should now be valid depth since we TFBO'd it previously
    //if(texelCoord.x % 2 == int(time) % 2 && texelCoord.y % 2 == int(time) % 2 && 
    //   checkNeighboursFIXME(1) && 
    if(vPosLocal.z > 0 && vPosLocal.z <= maxDepth) // only proceed if every other pixel, and every other frame, for some reason
	{

        // back project to get pixel coords in the 4x space so we can work out local area to search, ooo we could use local shared memory perhaps?

        vec3 pix = projectPoint(geoVertPosConf.xyz);
        

		int counter = 0;

	    float indexXStep = (1.0f / (imSize.x * scale)) * 0.5f; // this is a half pixel 
	    float indexYStep = (1.0f / (imSize.y * scale)) * 0.5f;

	    float bestDist = 1000;
	   
	    float windowMultiplier = 2;   

	    float xl = (pix.x - camPam[0].x) * camPam[0].z;
        float yl = (pix.y - camPam[0].y) * camPam[0].w;
	   
	    float lambda = sqrt(xl * xl + yl * yl + 1);

	    vec3 ray = vec3(xl, yl, 1);

	    // find best

		for(int i = int(pix.x - 2); i < int(pix.x + 2); i += 1)
		{
	        for(int j = int(pix.y - 2); j < int(pix.y + 2); j += 1)
			{
		       uint current = uint(texelFetch(indexSampler, ivec2(i, j), 0));
			   if(current > 0U)
				{
			        vec4 vertConf = texelFetch(vertConfSampler, ivec2(i, j), 0);
			        if(abs((vertConf.z * lambda) - (vPosLocal.z * lambda)) < 0.05)
					{
				        float dist = length(cross(ray, vertConf.xyz)) / length(ray);
					    vec4 normRad = texelFetch(normRadiSampler, ivec2(i, j), 0);

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
	    //if(counter > 0)
        if(false)
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

   // vec4 glPosition = vec4(-10, -10, 0, 1);

    ////Output vertex id of the existing point to update
    //if (updateId == 1)
    //{
    //    // intX annd Y are just a 2D mapping of the index onto a 2D array of size 3072 * 3072
	   // uint intY = best / uint(texDim);
	   // uint intX = best - (intY * uint(texDim));
	    
	   // float halfPixel = 0.5 * (1.0f / texDim);
	    
	   // //should set gl_Position here to the 2D index for the updated vertex ID
	   // glPosition = vec4(float(int(intX) - (int(texDim) / 2)) / (texDim / 2.0) + halfPixel, 
	   //                    float(int(intY) - (int(texDim) / 2)) / (texDim / 2.0) + halfPixel, 
	   //                    0, 
	   //                    1.0);
    //}
    //else
    //{
    //    //Either don't render anything, or output a new unstable vertex offscreen
    //    glPosition = vec4(-10, -10, 0, 1);
    //}


    //Emit a vertex if either we have an update to store, or a new unstable vertex to store
    //if (updateId > 0) !!!!! USE THIS
    if(true)
    {
        // if stable vertex, then pass its details so that it can be found in the ICP algorithm
        if (true)
        {
            // we want to store the stable vertices in some buffer/image 
            // old way is to add the vert info to a large sparse texture
            // we would have to use glClearTexImage after every frame, or directly set the value back to zeros after its used in the next shader! ooo efficient maybe otherwise we would have historics

            outputUpdateIndexInterleaved[best * 3]       = geoVertPosConf;
            outputUpdateIndexInterleaved[(best * 3) + 1] = geoVertNormRadi;
            outputUpdateIndexInterleaved[(best * 3) + 2] = geoVertColTimDev;

        }
        else
        {
            // unstable vertex doesnt get passed to the ICP algorithm, but it is not deleted from the global vbo
            // we can append unstable to the end of the vbo, since we know the county offset from the current global vert size
            // how about
            // index = county + current vertID (from depth ID, not global ID)
            // this will contain blanks for sure, but will pass all info without overwriting
            outputUpdateIndexInterleaved[best * 3] = geoVertPosConf;
            outputUpdateIndexInterleaved[(best * 3) + 1] = geoVertNormRadi;
            outputUpdateIndexInterleaved[(best * 3) + 2] = geoVertColTimDev;
        }




    }

}