#version 430 core

layout(local_size_x = 1024) in;

shared vec2 shared_data[gl_WorkGroupSize.x * 2];

layout(binding = 0, rg32f) readonly uniform image2D input_image;
layout(binding = 1, rgba32f) readonly uniform image2D input_image_rgba;
layout(binding = 2, rg32f) writeonly uniform image2D output_image;

uniform int useRGBA;

void main(void)
{
    uint id = gl_LocalInvocationID.x;
    uvec2 imSize;

    if (useRGBA == 1)
    {
        imSize = imageSize(input_image_rgba).xy;
    }
    else
    {
        imSize = imageSize(input_image).xy;
    }



    uint rd_id;
    uint wr_id;
    uint mask;
    ivec2 P0 = ivec2(id * 2, gl_WorkGroupID.x);
    ivec2 P1 = ivec2(id * 2 + 1, gl_WorkGroupID.x);

    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
    uint step = 0;


    if (useRGBA == 1)
    {
        shared_data[P0.x] = imageLoad(input_image_rgba, P0).xy;
        shared_data[P1.x] = imageLoad(input_image_rgba, P1).xy;
    }
    else
    {
        shared_data[P0.x] = imageLoad(input_image, P0).xy;
        shared_data[P1.x] = imageLoad(input_image, P1).xy;
    }


    barrier();

    for (step = 0; step < steps; step++)
    {
        mask = (1 << step) - 1;
        rd_id = ((id >> step) << (step + 1)) + mask;
        wr_id = rd_id + 1 + (id & mask);

        shared_data[wr_id] += shared_data[rd_id];

        barrier();
    }

    imageStore(output_image, P0.yx, vec4(shared_data[P0.x], 0, 1));
    imageStore(output_image, P1.yx, vec4(shared_data[P1.x], 0, 1));
}