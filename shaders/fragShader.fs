#version 430 core
const float PI = 3.1415926535897932384626433832795f;

layout (binding=0) uniform sampler2D currentTextureDepth;
layout (binding=1) uniform sampler2D currentTextureNormal;
layout (binding=2) uniform sampler2D currentTextureTrack;
layout (binding=3) uniform sampler2D currentTextureFlow;
layout (binding=4) uniform sampler2D currentTextureColor;
layout (binding=5) uniform sampler2D currentTextureInfra;

//layout (binding=5) uniform isampler3D currentTextureVolume; 
layout (binding=6) uniform isampler3D currentTextureVolume; 
layout (binding=7) uniform usampler3D currentTextureSDF; 


//layout(binding = 5, rgba32f) uniform image3D volumeData; // texel access


in vec2 TexCoord;
in float zDepth;
layout(location = 0) out vec4 color;

uniform mat4 model;
uniform vec3 ambient;
uniform vec3 light;
uniform float irLow = 0.0f;
uniform float irHigh = 65536.0f;

uniform float slice;
uniform float depthScale;
uniform vec2 depthRange;
uniform uint renderOptions;

subroutine vec4 getColor();
subroutine uniform getColor getColorSelection;

subroutine(getColor)
vec4 fromStandardFragment()
{
	// decode Key
	 //   return vec3((inUint & 4290772992) >> 22, (inUint & 4190208) >> 12, (inUint & 4092) >> 2); ;
	uint renderDepth =   (renderOptions & 1); 
	uint renderInfra =   (renderOptions & 2) >> 1; 
	uint renderColor =   (renderOptions & 4) >> 2; 
	uint renderNormals = (renderOptions & 8) >> 3; 
	uint renderTrack =   (renderOptions & 16) >> 4; 
	uint renderFlood =   (renderOptions & 32) >> 5;

	vec4 outColor = vec4(0.0f);

	if (renderDepth == 1)
	{
		vec4 tColor = vec4(textureLod(currentTextureDepth, vec2(TexCoord), 0)) * 65535.0f;
	    float depthVal = smoothstep(depthRange.x, depthRange.y, tColor.x * depthScale);

		outColor = vec4(depthVal.xxx, 1.0f);
	}

	if (renderColor == 1)
	{
		outColor = textureLod(currentTextureColor, vec2(TexCoord),0);
	}

	if (renderInfra == 1)
	{
		outColor = vec4(textureLod(currentTextureInfra, vec2(TexCoord),0).xxx, 1.0f);
	}

	if (renderNormals == 1)
	{
		vec4 tCol = texture(currentTextureNormal, vec2(TexCoord));
		if (tCol.w > 0)
		{
			outColor = mix(outColor, vec4(tCol.xy, -tCol.z, 1.0f), 0.5f);
		}
	}

	if (renderTrack == 1)
	{
		vec4 tCol = texture(currentTextureTrack, vec2(TexCoord));
		if (tCol.w > 0)
		{
			outColor = vec4(tCol.xyz, 1.0f);
		}
	}

	if (renderFlood == 1)
	{
	    uint tData = texture(currentTextureSDF, vec3(TexCoord, slice) ).x;
		vec3 texSize = vec3(textureSize(currentTextureSDF, 0));

		vec3 SDF = vec3((tData & 4290772992) >> 22, (tData & 4190208) >> 12, (tData & 4092) >> 2);

		float distCol = distance(vec3(TexCoord.xy, slice), SDF.xyz / texSize.x);
		float ssDist = smoothstep(0.0f, 0.5f, distCol);
		outColor = vec4(ssDist.xxx, 1.0f);
	}

	return outColor;

}




subroutine(getColor)
vec4 fromDepth()
{
	vec4 tColor = vec4(textureLod(currentTextureDepth, vec2(TexCoord), 0)) * 65535.0f;
	//float outVal = smoothstep(0.0, 200000.0, float(tColor.x) * depthScale);
	//return vec4((tColor.x * 100.0 / 1000000.0 - 0.15) / (0.2 - 0.15), (tColor.x  * 100.0 / 1000000.0 - 0.1) / (0.15 - 0.1),( tColor.x  * 100.0 / 1000000.0 - 0.2) / (0.3 - 0.2), 1.0f);
    float depthVal = smoothstep(depthRange.x, depthRange.y, tColor.x * depthScale);
	//return vec4(tColor.x * depthScale - depthRange.x / (depthRange.y - depthRange.x), tColor.x * depthScale - depthRange.x / (depthRange.y - depthRange.x), tColor.x * depthScale - depthRange.x / (depthRange.y - depthRange.x), 1.0f);
    return vec4(depthVal.xxx, 1.0f);
}

subroutine(getColor)
vec4 fromColor()
{
	vec4 tColor = textureLod(currentTextureColor, vec2(TexCoord),0);
	return tColor.xyzw;
}


subroutine(getColor)
vec4 fromRayNorm()
{
	vec4 tCol = texture(currentTextureNormal, vec2(TexCoord));
	return vec4(tCol.xy, -tCol.z, tCol.w);
}

subroutine(getColor)
vec4 fromTrack()
{
	vec4 tCol = texture(currentTextureTrack, vec2(TexCoord));
	return vec4(tCol);
}


subroutine(getColor)
vec4 fromPoints()
{
	return vec4(0.03f, 1.0f, 0.02f, 1.0f);
}

subroutine(getColor)
vec4 fromMarkers()
{
	return vec4(0.59f, 0.98f, 0.59f, 0.5f);
}

int ncols = 0;
int MAXCOLS = 60;
int colorwheel[60][3];


void setcols(int r, int g, int b, int k)
{
    colorwheel[k][0] = r;
    colorwheel[k][1] = g;
    colorwheel[k][2] = b;
}

void makecolorwheel()
{
    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow 
    //  than between yellow and green)
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    ncols = RY + YG + GC + CB + BM + MR;
    //printf("ncols = %d\n", ncols);
    if (ncols > MAXCOLS) return;

    int i;
    int k = 0;
    for (i = 0; i < RY; i++) setcols(255,	   255*i/RY,	 0,	       k++);
    for (i = 0; i < YG; i++) setcols(255-255*i/YG, 255,		 0,	       k++);
    for (i = 0; i < GC; i++) setcols(0,		   255,		 255*i/GC,     k++);
    for (i = 0; i < CB; i++) setcols(0,		   255-255*i/CB, 255,	       k++);
    for (i = 0; i < BM; i++) setcols(255*i/BM,	   0,		 255,	       k++);
    for (i = 0; i < MR; i++) setcols(255,	   0,		 255-255*i/MR, k++);
}

subroutine(getColor)
vec4 fromFlow()
{
	int length = 50;
	vec4 tFlow = textureLod(currentTextureFlow, TexCoord, 0);



	float mag = sqrt(tFlow.x * tFlow.x + tFlow.y * tFlow.y);
	float ang = atan(tFlow.y,  tFlow.x);

	//https://gist.github.com/KeyMaster-/70c13961a6ed65b6677d

	ang -= 1.57079632679;
    if(ang < 0.0) 
	{
		ang += 6.28318530718; 
    }
    ang /= 6.28318530718; 
	ang = 1.0 - ang; 

	//float smoothMag = smoothstep(0.25, 10.0, mag);

	// ang to rgb taken from https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(ang + K.xyz) * 6.0 - K.www);

    vec3 rgb = mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), mag  / 16.0f);

//	return vec4(1.1 * tFlow.x * tFlow.x, 1.1 * tFlow.y * tFlow.y, 0, 1); 
	return vec4((1.0 - rgb)*1.0, mag > 1 ? 1.0 : 0.0);

	//return vec4(tFlow.x < 0 ? 1 : 0, tFlow.y < 0 ? 1 : 0, 0, 1);
	//return vec4(rgb, mag > 1.0 ? 1 : 0);
	//	return vec4(rgb, 0.7);

}

subroutine(getColor)
vec4 fromVolumeSDF()
{
	
	//// FOR TSDF VOLUME
    //vec4 tData = texture(currentTextureVolume, vec3(TexCoord, slice) );
	//return vec4(1.0f * float(tData.x) * 0.00003051944088f, 1.0f * float(tData.x) * -0.00003051944088f, 0.0, 1.0f);
	
	//// FOR SDF VOLUME
    uint tData = texture(currentTextureSDF, vec3(TexCoord, slice) ).x;
    vec3 texSize = vec3(textureSize(currentTextureSDF, 0));

	vec3 SDF = vec3((tData & 4290772992) >> 22, (tData & 4190208) >> 12, (tData & 4092) >> 2);

	float distCol = distance(vec3(TexCoord.xy, slice), SDF.xyz / texSize.x);
	float ssDist = smoothstep(0.0f, 0.5f, distCol);
	return vec4(ssDist.xxx, 1.0f);
	//	return vec4(tData.w, -tData.w, 0.0f, 1.0f);

	//// FOR SDF VOLUME PRE COLORED
	//vec4 tData = vec4(texture(currentTextureSDF, vec3(TexCoord, slice) ));
	//float posi = float(tData.x - 64) / 64.0f; 
	//return vec4(posi / 2.0, -posi / 2.0, 0.0f, 1.0f);
}

subroutine(getColor)
vec4 fromVolume()
{
	//imageStore(volumeData, ivec3(0), vec4(69,13,69,13));

	//// FOR TSDF VOLUME
    vec4 tData = texture(currentTextureVolume, vec3(TexCoord, slice) );
	return vec4(1.0f * float(tData.x) * 0.0003051944088f, 1.0f * float(tData.x) * -0.0003051944088f, 0.0, 1.0f);
	//return vec4(1.0f,0.5f,0.1f,1.0f);

	//// FOR SDF VOLUME PRE COLORED
	//vec4 tData = vec4(texture(currentTextureSDF, vec3(TexCoord, slice) ));
	//float posi = float(tData.x - 64) / 64.0f; 
	//return vec4(posi / 2.0, -posi / 2.0, 0.0f, 1.0f);
}


void main()
{
	//vec3 normals = normalize(cross(dFdx(vert4D.xyz), dFdy(vert4D.xyz)));



	color = getColorSelection();

}



//uint32_t encodedOriginal = 25 << 22 | 86 << 12 | 35 << 2 | 2;

	//vec4 decodedOriginal = vec4((encodedOriginal & 4290772992) >> 22, (encodedOriginal & 4190208) >> 12, (encodedOriginal & 4092) >> 2, encodedOriginal & 3);
