#version 430

in vec4 gsVert;
in vec4 gsNorm;
in vec4 gsColor;
in vec4 gsData; // Confidence, radius, timestamp, and empty data 

uniform vec4 cam; //cx, cy, fx, fy
uniform float maxDepth;

void main(void)
{
	// this pixel-wise process does not contribute much
	// since the surfels are very small
	//float val = length(gsUvTex);
	//if (val > 1.0) discard;

	vec3 l = normalize(vec3((vec2(gl_FragCoord) - cam.xy) / cam.zw, 1.0f));
    vec3 corrected_pos = (dot(gsVert.xyz, gsNorm.xyz) / dot(l, gsNorm.xyz)) * l; 

	//check if the intersection is inside the surfel
    float sqrRad = pow(gsData.y, 2); 
    vec3 diff = corrected_pos - gsVert.xyz;

    if(dot(diff, diff) > sqrRad)
    {
        discard;
    }

	float z = corrected_pos.z;


	gl_FragData[0] = vec4((gl_FragCoord.x - cam.x) * z * (1.f / cam.z), (gl_FragCoord.y - cam.y) * z * (1.f / cam.w), z, gsData.x);
	gl_FragData[1] = gsNorm;
	gl_FragData[2] = vec4(gsVert.zzz, 0.0);
	gl_FragData[3] = gsColor;
}