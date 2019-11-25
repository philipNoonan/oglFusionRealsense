#version 440 core

layout (binding = 0) uniform sampler2D depthTex;
layout (binding = 1) uniform sampler2D normalTex;
layout (binding = 2) uniform sampler2D colorTex;
layout (binding = 3) uniform sampler2D infraTex;
layout (binding = 4) uniform usampler2D mappingTex;
layout (binding = 5) uniform sampler2D flowTex;

in vec2 vsTexCoord;

layout(location = 0) out vec4 color;


uniform int mapType;

uniform float maxDepth;
uniform int renderOptions;
uniform float depthScale;
uniform vec2 depthRange;
uniform int flowType;
uniform int renderType;


void main()
{

	vec4 outColor = vec4(0.0f);

	if (renderType == 0)
	{
		int renderDepth =   (renderOptions & 1); 
		int renderNormals =   (renderOptions & 2) >> 1; 
		int renderColor =   (renderOptions & 4) >> 2; 
		int renderInfra = (renderOptions & 8) >> 3;
		int renderFlow = (renderOptions & 16) >> 4;


		if (renderDepth == 1)
		{
			vec4 tColor = vec4(texture(depthTex, vsTexCoord));
			float depthVal = smoothstep(depthRange.x, depthRange.y, tColor.x);

			outColor = vec4(depthVal.xxx, 1.0f);
		}

		if (renderInfra == 1)
		{
			outColor = vec4(texture(infraTex, vsTexCoord).xxx, 1.0f);
		}

		if (renderNormals == 1)
		{
			vec4 tCol = texture(normalTex, vsTexCoord);
			if (abs(tCol.x) > 0)
			{
				outColor = mix(outColor, abs(tCol), 1.0f);
			}
		}

		if (renderColor == 1)
		{
			outColor = texture(colorTex, vsTexCoord);
		}

		if (renderFlow == 1)
		{
			int length = 50;
			ivec2 texSize = textureSize(flowTex, 0);

			uvec2 mapLoc = uvec2(texelFetch(mappingTex, ivec2(vsTexCoord.x * texSize.x, vsTexCoord.y * texSize.y), 0).xy);

			if (mapLoc.x != 10000 || flowType == 0)
			{

				vec4 tFlow = vec4(0);

				if (flowType == 1)
				{
					tFlow = texelFetch(flowTex, ivec2(mapLoc), 0);
				}
				else if (flowType == 0)
				{
					tFlow = texture(flowTex, vsTexCoord);
				}


				//vec4 tFlow = texture(flowTex, ivec2(TexCoord.x * texSize.x, TexCoord.y * texSize.y), 0);


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


				// ang to rgb taken from https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
				vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
				vec3 p = abs(fract(ang + K.xyz) * 6.0 - K.www);

				vec3 rgb = mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), mag * ((0 + 1.0) / 1.0));

				if (mag > 0.05)
				{
					outColor = vec4((1.0 - rgb), mag > 0.5 ? 1.0 : mag / 0.050);
				}
			
			}
			
		}
	
	}
	else if (renderType == 1)
	{
		outColor = vec4(0.0, 1.0, 0.1, 1.0f);
	}
	


	color = outColor;

}