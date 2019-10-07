#version 430

layout(local_size_x = 32, local_size_y = 32) in;

// bind images
layout(binding = 0, rgba32f) readonly uniform image2D srcVertexMap;
layout(binding = 1, rgba8) readonly uniform image2D srcColorMap;
layout(binding = 2, rgba8) writeonly uniform image2D dstColorMap;

uniform mat4 d2c;
uniform vec4 cam;


vec3 projectPointImage(vec3 p)
{
    return vec3(((cam.z * p.x) / p.z) + cam.x,
                ((cam.w * p.y) / p.z) + cam.y,
                p.z);
}



void main()
{
	ivec2 pix = ivec2(gl_GlobalInvocationID.xy);

	vec4 vertex = imageLoad(srcVertexMap, pix);

	vec4 vertexInColor = d2c * vertex;

	vec3 colPix = projectPointImage(vertexInColor.xyz);

	imageStore(dstColorMap, pix, imageLoad(srcColorMap, ivec2(colPix.x + 0.5f, colPix.y + 0.5f))); 




}