#version 430

layout(local_size_x = 4, local_size_y = 4) in;



layout(binding = 0, rg32f) uniform image2D im_grad_I0_x_y;
layout(binding = 1, rg32f) uniform image2D flow_texture_x_y;
layout(binding = 2, rg32f) uniform image2D previous_layer_flow_texture_x_y;
layout(binding = 3, r32f) uniform image2D test_texture;

  //  layout(binding = 7, rgba32f) uniform image2D im_prod_I0_xx_yy_xy_aux;

layout(binding = 4, rgba32f) uniform image2D im_prod_I0_xx_yy_xy;
layout(binding = 5, rg32f) uniform image2D im_sum_I0_x_y;
layout(binding = 6, rg32f) uniform image2D im_S_x_y;

//layout(binding = 7, rg8ui) uniform uimage2D uim_I0_I1;


layout(binding = 0) uniform sampler2D tex_I0;
layout(binding = 1) uniform sampler2D tex_I1;


layout(binding = 2) uniform sampler2D tex_prod_I0_xx_yy_xy;
layout(binding = 3) uniform sampler2D tex_sum_I0_x_y;

layout(binding = 4) uniform sampler2D tex_flow_previous;
layout(binding = 5) uniform sampler2D tex_flow_initial;

layout(binding = 6) uniform sampler2D tex_flow_final;

layout(std430, binding = 7) buffer trackedPointsBuf
{
    float trackedPointsBuffer [];
};


uniform float valA;
uniform float valB;

int patch_size = 8;
int patch_stride = 4;

//uniform int num_inner_iter = 5;
float INF = 1e10f;
float EPS = 0.001f;
uniform int level;
uniform int iter;
uniform int trackWidth;
uniform int imageType;

uniform ivec2 texSize;// = ivec2(1920, 1080);

float luminance(vec3 color)
{
    //return 0.2126 * float(color.x) / 255.0f + 0.7152 * float(color.y) / 255.0f + 0.0722 * float(color.z) / 255.0f;
    return 0.299f * float(color.x) + 0.587f * float(color.y) + 0.114f * float(color.z);
    //return (float(color.x) + float(color.y) + float(color.z)) / 3.0f;

}

subroutine void launchSubroutine();
subroutine uniform launchSubroutine disFlowSubroutine;


// NOTE YOU CANNOT USE THIS TYPE OF MEMORY LOADING WHEN YOU THE FLOW VECTORS PER WORKGROUP ARE NOT WELL ALIGNED
// IE FLOW FROM ID 1,1 MAY POINT LEFT AND 1,2 MAY POINT RIGHT YET WHEN YOU READ IN FROM THE OFFSETS THERE WILL BE GAPS THAT WILL NOT APPEAR AND FUCK UP THINSG
// X SHOULD BE SHARED Y SHOULD BE LOCAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

// unfortunately this makes reading in very slow, so we have to be effin clever


//shared float image_1[1920][1080]; 

shared float x_data[20][20];
shared float y_data_patch[20][20];


shared float x_grad_data[20][20];
shared float y_grad_data[20][20];

subroutine(launchSubroutine)
void makePatches()
{
    // pix_sparse is the pixel location for this current layer's sparse image === gl_WorkGroupID.x * 4 + pos.x
    ivec2 pix_sparse = ivec2(gl_GlobalInvocationID.xy);
    ivec2 pos = ivec2(gl_LocalInvocationID.xy); // ( 0:3 , 0:3 )
    ivec2 imSize_D = ivec2(imageSize(im_grad_I0_x_y).xy);

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            vec2 tempVal = imageLoad(im_grad_I0_x_y, ivec2(pix_sparse.x * 4 + i + pos.x, pix_sparse.y * 4 + j + pos.y)).xy;
            x_data[pos.x * 5 + i][pos.y * 5 + j] = tempVal.x;
            y_data_patch[pos.x * 5 + i][pos.y * 5 + j] = tempVal.y;
        }
    }


    float sum_xx_data = 0.0f;
    float sum_yy_data = 0.0f;
    float sum_xy_data = 0.0f;
    float sum_x_data = 0.0f;
    float sum_y_data = 0.0f;


    barrier();

    int xOffset = pos.x * patch_size / 2;
    int yOffset = pos.y * patch_size / 2;

    for (int i = xOffset; i < xOffset + patch_size; ++i)
    {
        for (int j = yOffset; j < yOffset + patch_size; ++j)
        {
            sum_xx_data += x_data[i][j] * x_data[i][j];
            sum_yy_data += y_data_patch[i][j] * y_data_patch[i][j];
            sum_xy_data += x_data[i][j] * y_data_patch[i][j];
            sum_x_data += x_data[i][j];
            sum_y_data += y_data_patch[i][j];

        }
    }

    barrier();

    // for patch size 4, pix_sparse should be changed to pix_sparse * 2 and pix_sparse * 2 + 1 (for both x and y dirs), i.e. each local invocation does 4 patches
    if (patch_size == 8)
    {
        imageStore(im_prod_I0_xx_yy_xy, pix_sparse, vec4(sum_xx_data, sum_yy_data, sum_xy_data, 0));
        imageStore(im_sum_I0_x_y, pix_sparse, vec4(sum_x_data, sum_y_data, 0, 0));
    }
    else if (patch_size == 4)
    {
        imageStore(im_prod_I0_xx_yy_xy, ivec2(pix_sparse.x * 2,     pix_sparse.y * 2),     vec4(sum_xx_data, sum_yy_data, sum_xy_data, 0));
        imageStore(im_prod_I0_xx_yy_xy, ivec2(pix_sparse.x * 2 + 1, pix_sparse.y * 2),     vec4(sum_xx_data, sum_yy_data, sum_xy_data, 0));
        imageStore(im_prod_I0_xx_yy_xy, ivec2(pix_sparse.x * 2 + 1, pix_sparse.y * 2 + 1), vec4(sum_xx_data, sum_yy_data, sum_xy_data, 0));
        imageStore(im_prod_I0_xx_yy_xy, ivec2(pix_sparse.x * 2,     pix_sparse.y * 2 + 1), vec4(sum_xx_data, sum_yy_data, sum_xy_data, 0));

        imageStore(im_sum_I0_x_y, ivec2(pix_sparse.x * 2,     pix_sparse.y * 2),     vec4(sum_x_data, sum_y_data, 0, 0));
        imageStore(im_sum_I0_x_y, ivec2(pix_sparse.x * 2 + 1, pix_sparse.y * 2),     vec4(sum_x_data, sum_y_data, 0, 0));
        imageStore(im_sum_I0_x_y, ivec2(pix_sparse.x * 2 + 1, pix_sparse.y * 2 + 1), vec4(sum_x_data, sum_y_data, 0, 0));
        imageStore(im_sum_I0_x_y, ivec2(pix_sparse.x * 2,     pix_sparse.y * 2 + 1), vec4(sum_x_data, sum_y_data, 0, 0));

    }


}


float computeSSDMeanNorm(float[8][8] yData)
{

    float sum_square_diff = 0.0f;
    float sum_diff = 0.0f;

    // process (4x4) 16 patches per workgroup, 1 per invocation

    int xOffset = int(gl_LocalInvocationID.x) * patch_size / 2;
    int yOffset = int(gl_LocalInvocationID.y) * patch_size / 2;

    // loops offest by 4 (0-8, 4-12, 8-16, 12-20)
    for (int i = xOffset; i < xOffset + patch_size; ++i)
    {
        for (int j = yOffset; j < yOffset + patch_size; ++j)
        {
            float diff = yData[i - xOffset][j - yOffset] - x_data[i][j];
            sum_diff += diff;
            sum_square_diff += diff * diff;
        }
    }

    float SSDMN = sum_square_diff - sum_diff * sum_diff / float(patch_size * patch_size);

    //if (SSDMN > )

    return SSDMN;

}

// if there SSD > thresh && block contains boundary then we need to redo this for each region
float processPatchMeanNorm(inout float dst_dUx, inout float dst_dUy, float x_grad_sum, float y_grad_sum, float[8][8] ydata)
{
    float n = float(patch_size * patch_size);
    float sum_square_diff = 0.0f;
    float sum_diff = 0.0f;
    float diff;

    float sum_I0x_mul = 0.0f;
    float sum_I0y_mul = 0.0f;

    // here we need to process the gradient full size images
    int xOffset = int(gl_LocalInvocationID.x) * patch_size / 2;
    int yOffset = int(gl_LocalInvocationID.y) * patch_size / 2;

    for (int i = xOffset; i < xOffset + patch_size; ++i)
    {
        for (int j = yOffset; j < yOffset + patch_size; ++j)
        {
            diff = ydata[i - xOffset][j - yOffset] - x_data[i][j];
            sum_diff += diff;
            sum_square_diff += diff * diff;

            sum_I0x_mul += diff * x_grad_data[i][j];
            sum_I0y_mul += diff * y_grad_data[i][j];

        }
    }

    dst_dUx = sum_I0x_mul - sum_diff * x_grad_sum / n;
    dst_dUy = sum_I0y_mul - sum_diff * y_grad_sum / n;
    return sum_square_diff - sum_diff * sum_diff / n;
}





subroutine(launchSubroutine)
void patchInverseSearch()
{
    float y_data[8][8];
    float y_data_init[8][8];

    // X_DATA IS THE I0 DATA
    // Y_DATA IS THE I1 DATA
    // pix_sparse is the pixel location for this current layer's sparse image === gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationID.x
    ivec2 pix_sparse = ivec2(gl_GlobalInvocationID.xy);
    ivec2 pos = ivec2(gl_LocalInvocationID.xy); // (0:3, 0:3)
    vec2 imSize = vec2(imageSize(im_grad_I0_x_y).xy); // dense size
    float min_SSD;
    float cur_SSD;

    int psz2 = patch_size / 2;

    vec2 prevFlowXY = vec2(0);

        for (int i = 0; i< 5; i++)
        {
            for (int j = 0; j< 5; j++)
            {
                float xCoord_I0 = ( (float(gl_GlobalInvocationID.x) * 4.0f + 0.5f + float(pos.x) + float(i))) / ( float(imSize.x));
                float yCoord_I0 = ( (float(gl_GlobalInvocationID.y) * 4.0f + 0.5f + float(pos.y) + float(j))) / ( float(imSize.y));
                if (imageType == 0)
                {
                      x_data[pos.x * 5 + i][pos.y * 5 + j] = textureLod(tex_I0, vec2(xCoord_I0, yCoord_I0), level).x;
                }
                else if (imageType == 1)
                {
                      x_data[pos.x * 5 + i][pos.y * 5 + j] = luminance(textureLod(tex_I0, vec2(xCoord_I0, yCoord_I0), level).xyz);
                }

            }
        }
      
    float xCoord_sparse = ( float(pix_sparse.x) + 0.5f) / ( float(imSize.x / 4));
    float yCoord_sparse = ( float(pix_sparse.y) + 0.5f) / ( float(imSize.y / 4));

    //vec4 inputProd = texelFetch(tex_prod_I0_xx_yy_xy, pix_sparse, level);
    //vec4 inputSum = texelFetch(tex_sum_I0_x_y, pix_sparse, level);
    vec4 inputProd = textureLod(tex_prod_I0_xx_yy_xy, vec2(xCoord_sparse, yCoord_sparse), level);
    vec4 inputSum = textureLod(tex_sum_I0_x_y, vec2(xCoord_sparse, yCoord_sparse), level);
    /* Computing the inverse of the structure tensor: */
    //vec4 inputProd = imageLoad(im_prod_I0_xx_yy_xy, pix_sparse);
    //vec4 inputSum = imageLoad(im_sum_I0_x_y, pix_sparse);

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            vec2 tempVal = imageLoad(im_grad_I0_x_y, ivec2(gl_GlobalInvocationID.x * 4 + i + pos.x, gl_GlobalInvocationID.y * 4 + j + pos.y)).xy;
            x_grad_data[pos.x * 5 + i][pos.y * 5 + j] = tempVal.x;
            y_grad_data[pos.x * 5 + i][pos.y * 5 + j] = tempVal.y;
        }
    }


    vec2 initFlowXY = vec2(0);

float xCoord = (( (float(gl_GlobalInvocationID.x) * 4.0 + 0.5f + float(psz2))) / ( float(imSize.x))) ;
float yCoord = (( (float(gl_GlobalInvocationID.y) * 4.0 + 0.5f + float(psz2))) / ( float(imSize.y))) ;

//if (level == 5) // should this have a dependency on iter number or not
// {
//      initFlowXY = vec2(0);
// }
//  else
//  {

////////////////// INITPOSE IS TEH WRONGSZROSSOSOS FIX ME PLS

 initFlowXY = imageLoad(previous_layer_flow_texture_x_y, ivec2(gl_GlobalInvocationID.x* 4 + psz2, gl_GlobalInvocationID.y* 4 + psz2)).xy; // get the flow from the centre of the patch
//vec2 tf = textureLod(tex_flow_previous, vec2(xCoord, yCoord), level + 1).xy;

  // initFlowXY =  textureLod(tex_flow_initial, vec2(xCoord, yCoord), level).xy;

    initFlowXY.x = initFlowXY.x < 8 ? initFlowXY.x : 0;
    initFlowXY.y = initFlowXY.y < 8 ? initFlowXY.y : 0;

    initFlowXY.x /= float(pow(2, level+1)); // dont need this if we are directly copying each mip map level rather than only top then mipmapping from top
    initFlowXY.y /= float(pow(2, level+1)); 

    



    float xCoord_init;
    float yCoord_init;

    for (int i = 0; i < patch_size; i++)
    {
        for (int j = 0; j < patch_size; j++)
        {
            xCoord_init = (( (float(gl_GlobalInvocationID.x) * 4.0 + 0.5f + initFlowXY.x + float(i))) / (float(imSize.x)));
            yCoord_init = (( (float(gl_GlobalInvocationID.y) * 4.0 + 0.5f + initFlowXY.y + float(j))) / (float(imSize.y)));
        //    y_data_init[i][j] = luminance(textureLod(tex_I1, vec2((float(gl_GlobalInvocationID.x) * 4.0f + initFlowXY.x + float(i)) / float(imSize.x), (float(gl_GlobalInvocationID.y) * 4.0f + initFlowXY.y + float(j)) / float(imSize.y)), level).xyz);
                if (imageType == 0)
                {
                    y_data_init[i][j] = textureLod(tex_I1, vec2(xCoord_init, yCoord_init), level).x;
                }
                else if (imageType == 1)
                {
                    y_data_init[i][j] = luminance(textureLod(tex_I1, vec2(xCoord_init, yCoord_init), level).xyz);
                }
    
        }
    }




    barrier();

    cur_SSD = computeSSDMeanNorm(y_data_init);

    barrier();

//
// LOOPING STARTS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 
    for (int iter_outer = 0; iter_outer< level + 2; iter_outer++) // the more iterations the slower it becomes, obviously
    {
        if (iter_outer == 0)
        {
            // reading from the previous layer enables linear inter for free
            prevFlowXY = 2.0 * textureLod(tex_flow_previous, vec2(xCoord, yCoord), level + 1).xy;

            //imageStore(test_texture, ivec2((gl_GlobalInvocationID.x* 4 + psz2), (gl_GlobalInvocationID.y* 4 + psz2)), vec4((initFlowXY.x - prevFlowXY.x)));
        }
        else
        {
            // prevFlowXY = vec2(0);
            prevFlowXY = imageLoad(im_S_x_y, pix_sparse).xy;
        }

        barrier();

        for (int i = 0; i < patch_size; i++)
        {
            for (int j = 0; j < patch_size; j++)
            {
                float xCoord_prev = (( (float(gl_GlobalInvocationID.x) * 4.0 + 0.5f + prevFlowXY.x + float(i))) / ( float(imSize.x)));
                float yCoord_prev = (( (float(gl_GlobalInvocationID.y) * 4.0 + 0.5f + prevFlowXY.y + float(j))) / ( float(imSize.y)));
                if (imageType == 0)
                {
                    y_data[i][j] = textureLod(tex_I1, vec2(xCoord_prev, yCoord_prev), level).x;
                }
                else if (imageType == 1)
                {
                    y_data[i][j] = luminance(textureLod(tex_I1, vec2(xCoord_prev, yCoord_prev), level).xyz);
                }
            }
        }

        barrier();

        min_SSD = computeSSDMeanNorm(y_data);

        barrier();

        float Ux = 0.0f;
        float Uy = 0.0f;

        if (cur_SSD <= min_SSD)
        {
            min_SSD = cur_SSD;
            Ux = initFlowXY.x;
            Uy = initFlowXY.y;
        }
        else if (min_SSD < cur_SSD)
        {
            Ux = prevFlowXY.x;
            Uy = prevFlowXY.y;
        }


//        float dir = -1.0f;

//        if (iter_outer % 2 != 0)
//        {
//            dir = 1.0f;
//        }

//        for (int i = 0; i<patch_size; i++)
//        {
//            for (int j = 0; j<patch_size; j++)
//            {
//                float xCoord_dir0 = (((float(gl_GlobalInvocationID.x) * 4.0 + dir + Ux + float(i))) / (float(imSize.x)));
//                float yCoord_dir0 = (((float(gl_GlobalInvocationID.y) * 4.0 + Uy + float(j))) / (float(imSize.y)));

//                if (imageType == 0)
//                {
//                    y_data[i][j] = textureLod(tex_I1, vec2(xCoord_dir0, yCoord_dir0), level).x;
//                }
//                else if (imageType == 1)
//                {
//                    y_data[i][j] = luminance(textureLod(tex_I1, vec2(xCoord_dir0, yCoord_dir0), level).xyz);
//                }

//            }
//        }

//        barrier();

//float spatial_x_SSD = computeSSDMeanNorm(y_data);

//barrier();

//        if (spatial_x_SSD<min_SSD)
//        {
//            min_SSD = spatial_x_SSD;
//            Ux = Ux + dir;
//        }
        
//        barrier();

//for (int i = 0; i<patch_size; i++)
//{
//    for (int j = 0; j<patch_size; j++)
//    {
//            float xCoord_dir1 = (((float(gl_GlobalInvocationID.x) * 4.0 + Ux + float(i))) / (float(imSize.x)));
//            float yCoord_dir1 = (((float(gl_GlobalInvocationID.y) * 4.0 + dir + Uy + float(j))) / (float(imSize.y)));

//        if (imageType == 0)
//        {
//            y_data[i][j] = textureLod(tex_I1, vec2(xCoord_dir1, yCoord_dir1), level).x;
//        }
//        else if (imageType == 1)
//        {
//            y_data[i][j] = luminance(textureLod(tex_I1, vec2(xCoord_dir1, yCoord_dir1), level).xyz);
//        }
//    }
//}

//barrier();

//float spatial_y_SSD = computeSSDMeanNorm(y_data);

//barrier();

//if (spatial_y_SSD<min_SSD)
//{
//     Uy = Uy + dir;
//}


//imageStore(im_S_x_y, pix_sparse, vec4(Ux, Uy, 0, 0));

float cur_Ux = Ux;
        float cur_Uy = Uy;





float detH = inputProd.x * inputProd.y - inputProd.z * inputProd.z;


        if (abs(detH) < EPS) // some EPS constant
        {
                detH = EPS;
        }

        float invH11 = inputProd.y / detH;
        float invH12 = -inputProd.z / detH;
        float invH22 = inputProd.x / detH;
        float prev_SSD = INF; // some large number
        float SSD;
        float dx, dy;
        float dUx, dUy;

        float x_grad_sum = inputSum.x;
        float y_grad_sum = inputSum.y;

//imageStore(test_texture, pix_sparse, vec4(min_SSD));


        for (int t = 0; t < level + 3; t++) // CHANE+GE ME TO ITER!!!
        {
            barrier();

            for (int i = 0; i < patch_size; i++)
            {
                for (int j = 0; j < patch_size; j++)
                {
                    float xCoord_inner = ( (float(gl_GlobalInvocationID.x) * 4.0 + 0.5f + cur_Ux + float(i))) / ( float(imSize.x));
                    float yCoord_inner = ( (float(gl_GlobalInvocationID.y) * 4.0 + 0.5f + cur_Uy + float(j))) / ( float(imSize.y));

                    if (imageType == 0)
                    {
                        y_data[i][j] = textureLod(tex_I1, vec2(xCoord_inner, yCoord_inner), level).x;
                    }
                    else if (imageType == 1)
                    {
                        y_data[i][j] = luminance(textureLod(tex_I1, vec2(xCoord_inner, yCoord_inner), level).xyz);
                    }

//y_data[i][j] = luminance(textureLod(tex_I1, vec2((float(pix_sparse.x) * 4.0f + cur_Ux + float(i)) / float(imSize.x), (float(pix_sparse.y) * 4.0f + float(cur_Uy) + float(j)) / float(imSize.y)), level).xyz);

                }
            }

            barrier();

            SSD = processPatchMeanNorm(dUx, dUy, x_grad_sum, y_grad_sum, y_data);

            dx = invH11* dUx + invH12* dUy;
            dy = invH12* dUx + invH22* dUy;

            cur_Ux -= dx;
            cur_Uy -= dy;

            /* Break when patch distance stops decreasing */
            if (SSD >= prev_SSD || prev_SSD < 0.01)
            {
                    break;
            }

            prev_SSD = SSD;

        }

        /* If gradient descent converged to a flow vector that is very far from the initial approximation
                     * (more than patch size) then we don't use it. Noticeably improves the robustness.
                     */
        vec2 theVec = vec2(cur_Ux - Ux, cur_Uy - Uy);
        if (length(theVec) < 8.0)
        {
            imageStore(im_S_x_y, pix_sparse, vec4(cur_Ux, cur_Uy, 0, 0)); 
        }
        else
        {
           imageStore(im_S_x_y, pix_sparse, vec4(Ux, Uy, 0, 0));
        }
        if (prev_SSD > 0.1)
        {
           imageStore(im_S_x_y, pix_sparse, vec4(cur_Ux/2.0, cur_Uy/2.0, 0, 0));

        }
        
        imageStore(test_texture, pix_sparse, vec4(prev_SSD));

    } // loop
}



subroutine(launchSubroutine)
void densification()
{
    // inputs sparse flow
    // inputs original images i0 i1
    // outputs dense flow



    vec2 imSize = vec2(imageSize(flow_texture_x_y).xy);

    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    float i, j;

    if (x >= imSize.x || y >= imSize.y) return;

    int start_is, end_is;
    int start_js, end_js;

    end_is = min(y / patch_stride, int((imSize.y) - patch_size) / patch_stride);
    start_is = max(0, y - patch_size + patch_stride) / patch_stride;
    start_is = min(start_is, end_is);

    end_js = min(x / patch_stride, int((imSize.x) - patch_size) / patch_stride);
    start_js = max(0, x - patch_size + patch_stride) / patch_stride;
    start_js = min(start_js, end_js);

    float coef, sum_coef = 0.0f;
    float sum_Ux = 0.0f;
    float sum_Uy = 0.0f;

    int i_l, i_u;
    int j_l, j_u;
    float i_m, j_m, diff, iMean;

    i = float(y);// / float(imSize.y);
    j = float(x);// / float(imSize.x);

    /* Iterate through all the patches that overlap the current location (i,j) */
    // but when we have patch size 8, offset 4, then each pixel will only be affected by 2 sparse pixels
    float countSize = 0;
    for (int is = start_is; is <= end_is; is++, countSize++)
    {
        for (int js = start_js; js <= end_js; js++)
        {
            vec2 sx_sy_val = imageLoad(im_S_x_y, ivec2(js, is)).xy;  // sx[is * ws + js];
            //float sy_val = sy[is * ws + js];
            //uchar2 i1_vec1, i1_vec2;

            // is this the correct calculation of texture coord?
            j_m = ((min(max(j + sx_sy_val.x, 0.0f), float(imSize.x) - 1.0f - EPS)));
            i_m = ((min(max(i + sx_sy_val.y, 0.0f), float(imSize.y) - 1.0f - EPS)));

            float warpedPixel;

            if (imageType == 0)
            {
                float i1_val;
                float i0_val;

                i1_val = textureLod(tex_I1, vec2((j_m + 0.5f) / float(imSize.x), (i_m + 0.5f) / float(imSize.y)), level).x;
                i0_val = textureLod(tex_I0, vec2((j + 0.5f) / float(imSize.x), (i + 0.5f) / float(imSize.y)), level).x;
                diff = i1_val - i0_val;
                warpedPixel = i1_val;
            }
            else if (imageType == 1)
            {
                vec3 i1_val;
                vec3 i0_val;

                i1_val = (textureLod(tex_I1, vec2((j_m + 0.5f) / float(imSize.x), (i_m + 0.5f) / float(imSize.y)), level).xyz);
                i0_val = (textureLod(tex_I0, vec2((j + 0.5f) / float(imSize.x), (i + 0.5f) / float(imSize.y)), level).xyz);

                diff = luminance(i1_val - i0_val);
                warpedPixel = luminance(i1_val);

            }

            iMean += warpedPixel;

            coef = 1.0 / max(1.0f, abs(diff));
            sum_Ux += coef * sx_sy_val.x;
            sum_Uy += coef * sx_sy_val.y;
            sum_coef += coef;
        }
    }


    imageStore(flow_texture_x_y, ivec2(x, y), vec4(sum_Ux / sum_coef, sum_Uy / sum_coef, 0, 0));
   //imageStore(flow_texture_x_y, ivec2(x, y), vec4(iMean / countSize, 0, 0, 0));



}

/*
3x3 Median
Morgan McGuire and Kyle Whitson
http://graphics.cs.williams.edu


Copyright (c) Morgan McGuire and Williams College, 2006
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges

subroutine(launchSubroutine)
void medianFilter()
{
    vec2 imSize = vec2(imageSize(flow_texture_x_y).xy);

    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);


    vec3 v[9];

    // Add the pixels which make up our window to the pixel array.
    for (int dX = -1; dX <= 1; ++dX)
    {
        for (int dY = -1; dY <= 1; ++dY)
        {
            //vec2 offset = vec2(float(dX), float(dY));

            // If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
            // pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
            // bottom right pixel of the window at pixel[N-1].
            //v[(dX + 1) * 3 + (dY + 1)] = vec3(texture2D(T, gl_TexCoord[0].xy + offset * Tinvsize).xyz);

            v[(dX + 1) * 3 + (dY + 1)] = vec3(imageLoad(previous_layer_flow_texture_x_y, ivec2(x + dX, y + dY)).xyz);
        }
    }

    vec3 temp;

    // Starting with a subset of size 6, remove the min and max each time
    mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
    mnmx5(v[1], v[2], v[3], v[4], v[6]);
    mnmx4(v[2], v[3], v[4], v[7]);
    mnmx3(v[3], v[4], v[8]);

    imageStore(flow_texture_x_y, ivec2(x,y), vec4(v[4], 0));



}

subroutine(launchSubroutine)
void calcStandardDeviation()
{
    // get 1D index of quadlist

    // make array of values as defined by the quad

    // get standard deviation

    // output a texture with original flow if std dev is low, no flow if std dev is high




}






    subroutine(launchSubroutine)
void trackPose()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imSize = ivec2(texSize.x, texSize.y); // set me as uniform

    float posX = trackedPointsBuffer[(pix.x * 3)];
    float posY = trackedPointsBuffer[(pix.x * 3) + 1];

    //posX = abs(posX - oriPosX) < 50.0 ? posX : oriPosX;
    // posY = abs(posY - oriPosY) < 50.0 ? posY : oriPosY;


    float xCoord = (2.0 * posX + 1.0) / (2.0 * float(imSize.x));
    float yCoord = (2.0 * posY + 1.0) / (2.0 * float(imSize.y));


    vec2 flow = textureLod(tex_flow_final, vec2(xCoord, yCoord), 0).xy;
    //vec2 flow = texelFetch(tex_flow_final, ivec2(posX, posY), 0).xy;

    trackedPointsBuffer[(pix.x * 3)] += flow.x;
    trackedPointsBuffer[(pix.x * 3) + 1] += flow.y;


}

subroutine(launchSubroutine)
void track()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imSize = ivec2(texSize.x, texSize.y); // set me as uniform

    float oriPosX = float(imSize.x - imSize.y + (pix.x * 2)) / 2.0f;
    float oriPosY = float(imSize.x - imSize.y + (pix.y * 2)) / 2.0f;

    float posX = trackedPointsBuffer[pix.y * trackWidth + (pix.x * 2)];
    float posY = trackedPointsBuffer[pix.y * trackWidth + (pix.x * 2) + 1];

    //posX = abs(posX - oriPosX) < 50.0 ? posX : oriPosX;
    // posY = abs(posY - oriPosY) < 50.0 ? posY : oriPosY;


    float xCoord = (2.0 * posX + 1.0) / (2.0 * float(imSize.x));
    float yCoord = (2.0 * posY + 1.0) / (2.0 * float(imSize.y));


    vec2 flow = textureLod(tex_flow_final, vec2(xCoord, yCoord), 0).xy;
    //vec2 flow = texelFetch(tex_flow_final, ivec2(posX, posY), 0).xy;

    trackedPointsBuffer[pix.y * trackWidth + (pix.x * 2)] += flow.x;
    trackedPointsBuffer[pix.y * trackWidth + (pix.x * 2) + 1] += flow.y;

    //imageStore(test_texture, pix, vec4(currentFlow + previousFlowSum, 0, 0));


}

subroutine(launchSubroutine)
void sumFlowTexture()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imSize = ivec2(texSize.x, texSize.y); // set me as uniform

    float xCoord = (2.0 * pix.x + 1.0) / (2.0 * float(imSize.x));
    float yCoord = (2.0 * pix.y + 1.0) / (2.0 * float(imSize.y));

    // just using these names, they are not actually I0 and grads
    vec2 currentFlow = textureLod(tex_I0, vec2(xCoord, yCoord), 0).xy;
    vec2 previousFlowSum = imageLoad(im_grad_I0_x_y, pix).xy;

    imageStore(im_grad_I0_x_y, pix, vec4(currentFlow + previousFlowSum, 0, 0));



}


// Here we want to take a patch and convolve it with the entire layer
// this will take up quite a lot of texture...
subroutine(launchSubroutine)
void deepMatching()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);





}
void main()
{
    disFlowSubroutine();
}
