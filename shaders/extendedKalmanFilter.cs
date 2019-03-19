#version 430

layout(local_size_x = 32, local_size_y = 32) in;

// bind buffers
// prediction error covariance
layout(std430, binding = 0) buffer bufferP
{
    mat2 P [];
};
// measurement vector
layout(std430, binding = 1) buffer bufferZ
{
    vec2 z [];
};
// state vector
layout(std430, binding = 2) buffer bufferX
{
    vec2 x [];
};

// set uniforms
// set Process Model Jacobian 
uniform mat2 F;
// set Measurement Funciton Jacobian 
uniform mat2 H;
// approximate process noise using a small constant
uniform mat2 Q;
// approximate measurement noise using a small constant   
uniform mat2 R; 

// identity, size n x n
mat2 I = mat2(1.0f);

// set subroutines


// predict
void predict(in ivec2 _pos, inout mat2 _Pp)
{
    /* P_k = F_{k-1} P_{k-1} F^T_{k-1} + Q_{k-1} */
    _Pp = F * P[_pos.x] * transpose(F) + Q;

}

// update
void update(in ivec2 _pos, inout mat2 _Pp)
{
    /* G_k = P_k H^T_k (H_k P_k H^T_k + R)^{-1} */
    mat2 G = _Pp * transpose(H) * inverse(H * _Pp * transpose(H) + R);

    /* \hat{x}_k = \hat{x_k} + G_k(z_k - h(\hat{x}_k)) */
    x[_pos.x] = x[_pos.x] + G * (z[_pos.x] - H * x[_pos.x]);

    /* P_k = (I - G_k H_k) P_k */
    P[_pos.x] = (I - (G * H)) * _Pp;
}

void main()
{
    mat2 Pp;
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

    predict(pos, Pp);

    update(pos, Pp);
}