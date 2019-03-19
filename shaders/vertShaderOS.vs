#version 430 core

layout (location = 0) in vec4 quadlist; 
layout (location = 1) in vec4 quadlistMeanTemp; 

uniform vec2 imSize;

out vec2 TexCoord;
out vec2 meanFlow;

void main()
{
    // to render on screen we have to map the quad dims to -1 -> 1 NDC, 
	// as they are currently from 0 -> nextPowerofTwoUp(imageSize)^2
	
	// uint xPos = (octlist & 4286578688) >> 23;
	// uint yPos = (octlist & 8372224) >> 14;
	// uint zPos = (octlist & 16352) >> 5;
	// uint lod = (octlist & 31);

	uint xPos = uint(quadlist.x);
	uint yPos = uint(quadlist.y);
	uint lod = uint(quadlist.z);

	meanFlow = quadlistMeanTemp.xy;

	float quadSideLength = float(pow(2, lod)); //

	vec2 origin = ((vec2(xPos, yPos) * quadSideLength) + (quadSideLength * 0.5f)); // 
 
	vec4 pos = vec4((origin.x / (1280.0f / 2.0f))-1.0f, (origin.y / (720.0f / 2.0f)) - 1.0f, 0.0f, 1.0f);

	gl_PointSize = quadSideLength;
	gl_Position = vec4(pos.x, pos.y, pos.z, pos.w);

/*

	float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); // u is just the type qualifer, like f, i think
    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); 


	gl_Position = vec4(-1.0f + x*2.0f, -1.0f+y*2.0f, 0.1f, 1.0f);
*/	

}
