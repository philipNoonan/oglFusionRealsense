#version 440 core

layout (binding = 0) uniform sampler2D depthTex;
layout (binding = 1) uniform sampler2D normalTex;
layout (binding = 2) uniform sampler2D colorTex;

in vec2 vsTexCoord;

layout(location = 0) out vec4 color;


uniform int mapType;

uniform float maxDepth;
uniform int renderOptions;
uniform float depthScale;
uniform vec2 depthRange;


void main()
{

	int renderDepth =   (renderOptions & 1); 
	int renderNormals =   (renderOptions & 2) >> 1; 
	int renderColor =   (renderOptions & 4) >> 2; 


	vec4 outColor = vec4(0.0f);



	if (renderDepth == 1)
	{
		vec4 tColor = vec4(texture(depthTex, vsTexCoord));
	    float depthVal = smoothstep(depthRange.x, depthRange.y, tColor.x);

		outColor = vec4(depthVal.xxx, 1.0f);
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



	color = outColor;

}