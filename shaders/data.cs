#version 430 core

layout(local_size_x = 1024) in;

uint maxNumVerts;

layout(binding = 0, offset = 0) uniform atomic_uint ac;

// this is the buffer vec4 vec4 vec4 [vert conf][norm rad][col time dev] from the current depth frame from the previous depth to buffer call 
layout(std430, binding = 0) buffer feedbackBuffer
{
    vec4 interleavedData [];
};

//layout(std430, binding = 1) buffer updateIndexMapBuffer
//{
//    vec4 outputUpdateIndexInterleaved [];
//};

layout(std430, binding = 1) buffer indexMatchingBuffer
{
    int outputIndexMatchingBuffer [];
};


layout (binding = 0) uniform usampler2D indexSampler; // 4x
layout (binding = 1) uniform sampler2D vertConfSampler; // 4x
layout (binding = 2) uniform sampler2D normRadiSampler; // 4x
layout (binding = 3) uniform sampler2D colTimDevSampler; // 4x

layout(binding = 0, rgba32f) uniform image2D outImagePC;


uniform vec4 camPam; //cx, cy, fx, fy
uniform float scale; // index map scale = 4.0f
uniform mat4 pose[4];
uniform float maxDepth;
uniform float time;
uniform float weighting;

vec2 imSize;

vec3 getVert(sampler2DArray depthTex, vec3 textureCoord, ivec3 texelCoord)
{
	float z = float(textureLod(depthTex, textureCoord, 0.0f).x); // SINGLE CAMERA MODE

	return vec3((texelCoord.x - camPam.x) * z * camPam.z,
			    (texelCoord.y - camPam.y) * z * camPam.w, 
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
    float meanFocal = ((1.0 / abs(camPam.z)) + (1.0 / abs(camPam.w))) / 2.0;
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
    
    vec2 pixelPosCentered = texelCoord.xy - camPam.xy;
    
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
    return vec3(((((camPam.z * p.x) / p.z) + camPam.x) - (imSize.x * 0.5)) / (imSize.x * 0.5),
                ((((camPam.w * p.y) / p.z) + camPam.y) - (imSize.y * 0.5)) / (imSize.y * 0.5),
                p.z / maxDepth);
}

vec3 projectPointImage(vec3 p)
{
    return vec3(((camPam.z * p.x) / p.z) + camPam.x,
                ((camPam.w * p.y) / p.z) + camPam.y,
                p.z);
}

// this gets run for every valid depth vertex from the depth TFO
void main()
{
	ivec2 bigTexSize = textureSize(vertConfSampler, 0); // this is the 4x size
	imSize = vec2(bigTexSize).xy / 4.0f;

	int vertID = int(gl_GlobalInvocationID.x); // 0 to max number of valid depth verts

	//ivec2 texelCoord = ivec2(vertID % texSize.x, (vertID / texSize.x) % texSize.y);
	//vec2 textureCoord = vec2(float(texelCoord.x + 0.5f) / float(texSize.x), float(texelCoord.y + 0.5) / float(texSize.y));

    //Vertex position integrated into model transformed to global coords
    vec4 vPosLocal = interleavedData[(vertID * 3)];

    vec4 geoVertPosConf = pose[0] * vec4(vPosLocal.xyz, 1);
    geoVertPosConf.w = vPosLocal.w;

    //Normal and radius computed with filtered position / depth map transformed to global coords
    //vec3 vNormLocal = getNorm(depthSampler, geoVertPosConf, textureCoord, 1.0f / vec3(texSize.xyz), texelCoord);
    vec4 vNormLocal = interleavedData[(vertID * 3) + 1];// vec4(mat3(pose) * vNormLocal, getRadi(geoVertPosConf.z, vNormLocal.z, texelCoord.z));
    vec4 geoVertNormRadi = vec4(mat3(pose[0]) * vNormLocal.xyz, vNormLocal.w);



    vec4 geoColTimDev = interleavedData[(vertID * 3) + 2];






    vec4 geoVertColTimDev;

    int updateId = 0;

    uint best = 0U;


    //If this point is actually a valid vertex (i.e. has depth)
    // this should now be valid depth since we TFBO'd it previously
    //if(texelCoord.x % 2 == int(time) % 2 && texelCoord.y % 2 == int(time) % 2 && 
    //   checkNeighboursFIXME(1) && 
    if (vPosLocal.z > 0 && vPosLocal.z <= maxDepth) // only proceed if every other pixel, and every other frame, for some reason
    {
        ///// outputUpdateIndexInterleaved[(vertID * 3)] = interleavedData[(vertID * 3)];
        ///// outputUpdateIndexInterleaved[(vertID * 3) + 1] = interleavedData[(vertID * 3) + 1];

        // project to 1x image space
        vec3 pix = projectPointImage(geoVertPosConf.xyz);

        //imageStore(outImagePC, ivec2(pix.xy), vec4(geoVertPosConf.xyz, 2.0f));


        int counter = 0;

        float bestDist = 1000;

        float windowMultiplier = 2;

        // find the ray in the 1x image
        float xl = ((pix.x - camPam.x) / camPam.z);
        float yl = ((pix.y - camPam.y) / camPam.w);

        float lambda = sqrt(xl * xl + yl * yl + 1);

        //imageStore(outImagePC, ivec2(pix.xy), vec4(1, 0, 0, 1));

        vec3 ray = vec3(xl, yl, 1);
        // ray is direction of ray in pixel space out from each pixel from camera centre

        // find best
        for (int i = int((pix.x * 4) - 2); i < int((pix.x * 4) + 2); i += 1)
        {
            for (int j = int((pix.y * 4) - 2); j < int((pix.y * 4) + 2); j += 1)
            {
                uint current = uint(texelFetch(indexSampler, ivec2(i, j), 0));
                if (current > 0U)
                {
                    vec4 vertConf = texelFetch(vertConfSampler, ivec2(i, j), 0);
                    //imageStore(outImagePC, ivec2(i / 4, j / 4), vec4((vertConf.z * lambda) - (vPosLocal.z * lambda), 0, 0, 1));

                    // check to see if the camera space pixel ray that goes between each potential global pixel (vertConf) is in range of the current depth map (vposlocal)
                    if (abs((vertConf.z * lambda) - (vPosLocal.z * lambda)) < 0.05)
                    {
                        //imageStore(outImagePC, ivec2(i / 4, j / 4), vec4(vertConf.z, vPosLocal.z, 0, 1));

                        float dist = length(cross(ray, vertConf.xyz)) / length(ray);
                        vec4 normRad = texelFetch(normRadiSampler, ivec2(i, j), 0);

                        if (dist < bestDist && (abs(normRad.z) < 0.75f || abs(angleBetween(normRad.xyz, vNormLocal.xyz)) < 0.5f))
                        {
                            //imageStore(outImagePC, ivec2(i / 4, j / 4), vec4(dist, vertConf.z, vPosLocal.z, 1));

                            counter++;
                            bestDist = dist;
                            best = current;
                        }
                    }
                }
            }
        }

        //We found a point to merge with
        if (counter > 0)
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

    //Emit a vertex if either we have an update to store, or a new unstable vertex to store
    //if (updateId == 1) //!!!!! USE THIS
    //  {
    // we want to store the stable vertices in some buffer/image 
    // old way is to add the vert info to a large sparse texture
    // we would have to use glClearTexImage after every frame, or directly set the value back to zeros after its used in the next shader! ooo efficient maybe otherwise we would have historics

    // we dont actually need the 3x buffers here, all we need is an index buffer matching the size of the global buffer which tells us for each vertex in the global model should be updated with what index in the new depth fram edata
    //outputUpdateIndexInterleaved[vertID * 3] = geoVertPosConf;
    //outputUpdateIndexInterleaved[(vertID * 3) + 1] = geoVertNormRadi;
    //outputUpdateIndexInterleaved[(vertID * 3) + 2] = vec4(geoVertColTimDev.xy, vertID, -1);

    if (updateId > 0)
    {
        if (updateId == 1)
        {
            outputIndexMatchingBuffer[best] = vertID;
        }
        else
        {
            uint appendedPosition = atomicCounterIncrement(ac);

            outputIndexMatchingBuffer[appendedPosition] = -vertID;
        }

    }


    // output the vertID of the new depth vert data in a buffer of size of the global array buffer at the index point of the vert where it matches
    // or set a flag showing the vertID of the new depth vert in the global buffer itself, if space is available

    // }
    //else
    // {
    // unstable vertex doesnt get passed to the ICP algorithm, but it is not deleted from the global vbo
    // we can append unstable to the end of the vbo, since we know the county offset from the current global vert size
    // how about
    // index = county + current vertID (from depth ID, not global ID)
    // this will contain blanks for sure, but will pass all info without overwriting

    //     outputUpdateIndexInterleaved[vertID * 3] = vec4(0);
    //     outputUpdateIndexInterleaved[(vertID * 3) + 1] = vec4(0);
    //     outputUpdateIndexInterleaved[(vertID * 3) + 2] = vec4(0.8);
    // }


}