/*


#version 430 core
const float PI = 3.1415926535897932384626433832795f;

//layout (depth_less) out float gl_FragDepth; // NSIGHT DOESNT LIKE THIS LINE

float max3 (vec3 v) {
  return max (max (v.x, v.y), v.z);
}

layout (binding=0) uniform sampler2D currentTexture2D; 
layout (binding=1) uniform sampler3D currentTexture3D; 


in vec2 TexCoord;
in vec3 TexCoord3D;

in vec3 Normal;
in vec3 FragPos;

in vec3 boxCenter;
in vec3 boxRadius;

uniform mat4 rotMat;
uniform mat4 MVP;

uniform mat4 invView; 
uniform mat4 invProj;
uniform mat4 invModel;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

layout(location = 0) out vec4 color;

uniform int level = 0;
uniform float slice;
uniform vec3 sliceVals;

uniform vec3 lightPos;
vec3 lightColor = vec3(1.0f);

subroutine vec4 getColor();
subroutine uniform getColor getColorSelection;



uniform struct Light {
	vec3 position;
	vec3 intensities;
	float attenuation;
	float ambientCoefficient;
} light;

struct Ray {
    vec3 origin;
    vec3 direction;
} ray;


// vec3 box.radius:       independent half-length along the X, Y, and Z axes
// mat3 box.rotation:     box-to-world rotation (orthonormal 3x3 matrix) transformation
// bool rayCanStartInBox: if true, assume the origin is never in a box. GLSL optimizes this at compile time
// bool oriented:         if false, ignore box.rotation
bool ourIntersectBoxCommon(vec3 boxCenter, vec3 boxRadius, vec3 boxInvRadius, mat3 boxRotation, vec3 rayOrigin, vec3 rayDirection, in vec3 _invRayDirection, out float distance, out vec3 normal, const bool rayCanStartInBox, const in bool oriented) {

    // Move to the box's reference frame. This is unavoidable and un-optimizable.
    rayOrigin = (rayOrigin - boxCenter) * boxRotation;
    if (oriented) {
        rayDirection = rayDirection * boxRotation;
    }
    
    // This "rayCanStartInBox" branch is evaluated at compile time because `const` in GLSL
    // means compile-time constant. The multiplication by 1.0 will likewise be compiled out
    // when rayCanStartInBox = false.
    float winding;
    if (rayCanStartInBox) {
        // Winding direction: -1 if the ray starts inside of the box (i.e., and is leaving), +1 if it is starting outside of the box
        winding = (max3(abs(rayOrigin) * boxInvRadius) < 1.0) ? -1.0 : 1.0;
    } else {
        winding = 1.0;
    }

    // We'll use the negated sign of the ray direction in several places, so precompute it.
    // The sign() instruction is fast...but surprisingly not so fast that storing the result
    // temporarily isn't an advantage.
    vec3 sgn = -sign(rayDirection);

	// Ray-plane intersection. For each pair of planes, choose the one that is front-facing
    // to the ray and compute the distance to it.
    vec3 distanceToPlane = boxRadius * winding * sgn - rayOrigin;
    if (oriented) {
        distanceToPlane /= rayDirection;
    } else {
        distanceToPlane *= _invRayDirection;
    }

    // Perform all three ray-box tests and cast to 0 or 1 on each axis. 
    // Use a macro to eliminate the redundant code (no efficiency boost from doing so, of course!)
    // Could be written with 
#define TEST(U, VW)\
         /* Is there a hit on this axis in front of the origin? Use multiplication instead of && for a small speedup */\
         (distanceToPlane.U >= 0.0) && \
         /* Is that hit within the face of the box? */\
         all(lessThan(abs(rayOrigin.VW + rayDirection.VW * distanceToPlane.U), boxRadius.VW))

    bvec3 test = bvec3(TEST(x, yz), TEST(y, zx), TEST(z, xy));

    // CMOV chain that guarantees exactly one element of sgn is preserved and that the value has the right sign
    sgn = test.x ? vec3(sgn.x, 0.0, 0.0) : (test.y ? vec3(0.0, sgn.y, 0.0) : vec3(0.0, 0.0, test.z ? -sgn.z : 0.0));    
#undef TEST
        
    // At most one element of sgn is non-zero now. That element carries the negative sign of the 
    // ray direction as well. Notice that we were able to drop storage of the test vector from registers,
    // because it will never be used again.

    // Mask the distance by the non-zero axis
    // Dot product is faster than this CMOV chain, but doesn't work when distanceToPlane contains nans or infs. 
    //
    distance = (sgn.x != 0.0) ? distanceToPlane.x : ((sgn.y != 0.0) ? distanceToPlane.y : distanceToPlane.z);

    // Normal must face back along the ray. If you need
    // to know whether we're entering or leaving the box, 
    // then just look at the value of winding. If you need
    // texture coordinates, then use box.invDirection * hitPoint.
    
    if (oriented) {
        normal = boxRotation * sgn;
    } else {
        normal = sgn;
    }
    
    return (sgn.x != 0) || (sgn.y != 0) || (sgn.z != 0);
}





subroutine(getColor)
vec4 fromVolume()
{
	vec4 tData = textureLod(currentTexture3D, vec3(TexCoord3D.x, TexCoord3D.y, TexCoord3D.z), float(level) );
	//vec4 tData = imageLoad(volumeData, vec3(TexCoord.x * 512.0f, TexCoord.y * 512.0f, slice));
	float outfloat = tData.x > 10 ? tData.x * 0.0005f : 0;
	//float alpha = outfloat > 0.1 ? 1 : 0;

	gl_FragDepth = gl_FragCoord.z;
	return vec4(outfloat.xxx, 1.0);

	//return vec4(1.0, 0.0, 0.0, 1.0);
}

subroutine(getColor)
vec4 fromOctreePoints()
{

	vec4 res = vec4(0.0f);

    float u = (2.0 * float(gl_FragCoord.x)) / 1024.0f - 1.0f; //1024.0f is the window resolution, change this to a uniform
    float v = (2.0 * float(gl_FragCoord.y)) / 1024.0f - 1.0f;

	vec4 origin;
    vec4 direction;


    origin = (invView)[3];

    vec4 ray_eye = invProj * vec4(u, v, -1.0, 1.0f);
	// issue here, i dont think the direction of the ray is correct
    ray_eye = vec4(ray_eye.xy, -1.0f, 0.0f);

    direction = normalize(invView * ray_eye);

		// to get ray origin we need to get gl_FragCoord
		vec3 rayOrigin = vec3((origin.xyz + 1.0 ) * 256.0);
		// to get ray direction we need invModel and invView * rayOrigin
		vec3 rayDirection = vec3(direction.xyz);

		float distanceToHit = 0.0f;
		vec3 normalAtHit = vec3(0.0f);

		vec3 invRayDirection = 1.0f / rayDirection;
		vec3 invBoxRadius = 1.0f / boxRadius;

		const bool rayCanStartInBox = false;
		const bool oriented = true; 
		mat4 br = mat4(1.0);

		br[0].xyz = vec3(0.9961947, 0.0871557, 0.0);
		br[1].xyz = vec3(-0.0871557, 0.9961947, 0.0);
				
		mat4 boxShift = mat4(invView);

		boxShift[3].xyz = -boxCenter;

		mat3 boxRotation = mat3(rotMat);

		bool res0 = ourIntersectBoxCommon(boxCenter, boxRadius, invBoxRadius, boxRotation, rayOrigin, rayDirection, invRayDirection, distanceToHit, normalAtHit, rayCanStartInBox, oriented);
		if (res0)
		{
			res = vec4(vec3(normalAtHit), 1.0f);
		    //  res = vec4(vec3(boxCenter * 0.002f),1.0f);// * (ambient + diffuse + specular),1.0f);
			vec4 worldspace = vec4( (distanceToHit * vec4(rayDirection, 0.0) + vec4(rayOrigin, 1.0))) / 256.0 - 1.0f;
			worldspace.w = 1.0f;
			vec4 fakeFrag = vec4((MVP) * worldspace);
			//inverse(invProj * invView)
			float fakeDepth = ((gl_DepthRange.far - gl_DepthRange.near) / 2.0) * (fakeFrag.z / fakeFrag.w) + ((gl_DepthRange.far + gl_DepthRange.near) / 2.0);


			gl_FragDepth = fakeDepth ;
			// this should be related to the depth to hit value, not the depth to the 2D sprite

			

			

			//gl_FragDepth = vec3(distanceToHit).z * Cw;

			//gl_FragDepth = gl_FragCoord.z;


			res = vec4(smoothstep(-1.0, 1.0, worldspace.xyz), 1.0f);
			//res = vec4(gl_FragDepth.xxx, 1.0);
		}
		else
		{
			//res = vec4(vec3(1,1,0), 1.0f);// * (ambient + diffuse + specular);
			discard;
		}


		// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * vec3(1.0f);
// diffuse
	vec3 norm = normalize(abs(normalAtHit));
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
//specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(view[3].xyz - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 halfwayDir = normalize(lightDir + viewDir);  

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;
    


	
	return vec4(res.xyz * (ambient + diffuse + specular), 1.0f); 

}

subroutine(getColor)
vec4 fromOctreeTriangles()
{
// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * vec3(1.0f);
// diffuse
	vec3 norm = normalize(abs(Normal));
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
//specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(view[3].xyz - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec4 res = vec4(0.0f);
		//	gl_FragDepth = 0.12;


		gl_FragDepth = gl_FragCoord.z;

		res = vec4(smoothstep(-1.0,1.0,TexCoord3D) * (ambient + diffuse + specular), 1.0f);// * (ambient + diffuse + specular),1.0f);
	


	
	return vec4(res); 

}

subroutine(getColor)
vec4 fromMarchingCubesTriangles()
{
// ambient
	float ambientStrength = 0.8f;
	vec3 ambient = ambientStrength * vec3(1.0f);
// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(lightPos.xy, -lightPos.z) - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
//specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(view[3].xyz - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

		gl_FragDepth = gl_FragCoord.z;

		return vec4(norm * (ambient + diffuse),1.0f);
		
}




subroutine(getColor)
vec4 fromTexture2D()
{
	vec4 tData = texture(currentTexture2D, vec2(TexCoord.x, TexCoord.y));
	return vec4(tData.xyzw); 

}


void main()
{
	//vec3 normals = normalize(cross(dFdx(vert4D.xyz), dFdy(vert4D.xyz)));



	color = getColorSelection();

}



*/