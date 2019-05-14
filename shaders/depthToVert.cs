#version 430

layout (local_size_x = 32, local_size_y = 32) in;

uniform mat4 invK;
uniform mat4 colorK;

uniform mat4 colorIntrinsics;
uniform mat4 depthToColor;

uniform mat4 depthToDepth;

uniform vec4 camPamsColor;
uniform vec4 camPamsDepth;
    // camPams.x = fx, camPams.y = fy, camPams.z = cx, camPams.w = cy
uniform int imageType; // 0 = short, 1 = float
uniform float depthScale; // value to convert whatever unit the depth data comes in to be in units of metres

layout(binding=0, r32f) uniform image2D InputImage;
layout(binding=1, r16ui) uniform uimage2D InputImageShort; // JUST USE r16 HERE!¬¬!"¬!"¬!"¬!"¬!"¬

layout(binding=2, rgba32f) uniform image2D OutputImage;

layout(std430, binding=0) buffer pos3D
{
    vec4 Position3D[];
}; 


//layout(std430, binding= 1) buffer color3D
//{
//    vec4 Color3D [];
//};


vec3 rotate(mat4 M, vec3 V)
{
    // glsl and glm [col][row]
    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
}

// int oneDindex = (row * length_of_row) + column; // Indexes

void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    ivec2 size = imageSize(OutputImage);
    //pix.y = pix.y += 2;

   // float x;
   // float y;
  //  float z;

    if (pix.x < size.x && pix.y < size.y)
    {
        float depth;
        if (imageType == 0)
        {
            depth = depthScale * float(imageLoad(InputImageShort, ivec2(pix)).x);
        }
        else if (imageType == 1)
        {
            depth = imageLoad(InputImage, ivec2(pix)).x * depthScale;
        }

        if (depth == 0 || depth < 0)
        {  // make return write a blank pixel rather than jut exiting here
            imageStore(OutputImage, ivec2(pix), vec4(0.0f, 0.0f, 0.0f, 0.0f));
            return;
        }



        vec4 tPos = (depth) * (invK * vec4(pix.x, pix.y, 1.0f, 0.0f));

        //float x = (pix.x - camPamsDepth.x) / camPamsDepth.z;
        //float y = (pix.y - camPamsDepth.y) / camPamsDepth.w;


        //vec3 tPos = depth * rotate(invK, vec3(pix.x, pix.y, 1.0f));


        //vec4 pointIn3D = vec4(x * depth, y * depth, depth, 1.0f);


        //vec3 colPixel = mat3(colorK) * tPos.xyz;

        //vec4 outPos = depthToDepth * vec4(tPos.xyz, 1.0f);

        imageStore(OutputImage, ivec2(pix.xy), vec4(tPos.xyz, 1.0f));



       // vec4 transformedPointIn3D = depthToColor * pointIn3D;

        //float to_pointX = depthToColor[0][0] * pointIn3D.x + depthToColor[1][0] * pointIn3D.y + depthToColor[2][0] * pointIn3D.z + depthToColor[3][0];
        //float to_pointY = depthToColor[0][1] * pointIn3D.x + depthToColor[1][1] * pointIn3D.y + depthToColor[2][1] * pointIn3D.z + depthToColor[3][1];
        //float to_pointZ = depthToColor[0][2] * pointIn3D.x + depthToColor[1][2] * pointIn3D.y + depthToColor[2][2] * pointIn3D.z + depthToColor[3][2];

        //vec4 transformedPointIn3D = vec4(to_pointX, to_pointY, to_pointZ, 1.0f);



        //float colX = transformedPointIn3D.x / transformedPointIn3D.z;
        //float colY = transformedPointIn3D.y / transformedPointIn3D.z;

        //vec2 colPixel = vec2(colX * camPamsColor.z + camPamsColor.x, colY * camPamsColor.w + camPamsColor.y);



//
       // imageStore(OutputImage, ivec2(colPixel), vec4(transformedPointIn3D.xyz, 1.0));


        // vec4 color = vec4(texture(currentTextureColor, vec2(pix.x / 1920.0f, pix.y / 1080.0f)));
        //if (depth.x > 0)
        //{
        //vec4 tPos;
        //tPos.z = float(depth.x) * depthScale;

        //tPos.x = ((pix.x - camPams.x) / camPams.z) * tPos.z;
        //tPos.y = ((pix.y - camPams.y) / camPams.w) * tPos.z;

        //imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(x, y, z, 0.0f));
        //Position3D[(pix.y * size.x) + pix.x] = vec4(x, y, z, 0.0f);

        //tPos = (float(depth) * depthScale) * rotate(invK, vec3(pix.x, pix.y, 1.0f));
        // tPos = (depth * depthScale) * (invK * vec4(pix.x, pix.y, 1.0f, 0.0f));

        //
        //  vec4 regiPos = vec4(tPos.xyz, 1.0f);

        //  vec2 outPixelPos;
        //  float x = regiPos.x / regiPos.z;
        // float y = regiPos.y / regiPos.z;

        //outPixelPos.x = (x * camPams.z) + camPams.x;
        // outPixelPos.y = (y * camPams.w) + camPams.y;



        //imageStore(OutputImage, ivec2(outPixelPos), regiPos);
        //Position3D[(pix.y * size.x) + pix.x] = vec4(tPos, 0.0f);

        //Color3D[(pix.y * size.x) + pix.x] = vec4(color.xyz,0); // FIX ME DONT USE FLOAT £" FOR COLOR USE BYTES!!!
        //Position3D[(pix.x * size.x) + pix.y] = vec3(pix.x / 100.0f, pix.y / 100.0f, -depth / 100.0f);

        //}
        //else
        //{
        //   imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
        //   Position3D[(pix.x * size.x) + pix.y] = vec4(0, 0, 0, 0);
        //}

    }

}