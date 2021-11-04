#version 440
#define PI 3.14159265359

in vec3 localPos;
out vec4 fragColor;

uniform samplerCube envMap;
uniform float roughness;

uniform float mipLevel;

float radicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec3 importanceSampleGGX(vec2 Xi, vec3 n, float roughness)
{
    // gives halfway vector closer to the normal the lesser the roughness 
    // and low discrepency sequence.
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    // dot product between normal and halfway vector decreases to 0 Xi.y and roughness increase to 1.
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 h = vec3(cos(phi) * sinTheta, cos(phi) * sinTheta, cosTheta);
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);
    mat3 TBN = mat3(tangent, bitangent, n);
	
    vec3 sampleVec = TBN * h;
    return normalize(sampleVec);
}  

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}  

float distributionGGX(float nDotH, float roughness)
{
	float a2 = roughness * roughness;
	float denom = nDotH * nDotH * (a2 -1.0) + 1.0;
	denom *= PI * denom;

	return a2 / denom;
}

void main()
{
    vec3 n = normalize(localPos);
    vec3 v = n;

    const uint sampleCount = 1024u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    for (uint i = 0u; i < sampleCount; i++)
    {
        vec2 Xi = hammersley(i, sampleCount);
        vec3 h = importanceSampleGGX(Xi, n, roughness);
        vec3 l = normalize(2.0 * dot(v, h) * h - v);

        float nDotL = max(dot(n, l), 0.0);
        float nDotH = max(dot(n, h), 0.0);
        float vDotH = max(dot(v, h), 0.0);

        //*
        float d   = distributionGGX(nDotH, roughness);
        float pdf = (d * nDotH / (4.0 * vDotH)) + 0.0001; 

        float resolution = 512.0; // resolution of source cubemap (per face)
        float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(sampleCount) * pdf + 0.0001);
       
        float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
        prefilteredColor += textureLod(envMap, v, mipLevel).rgb * nDotL;
        //*
        
        //prefilteredColor += texture(envMap, v).rgb * nDotL;
        totalWeight += nDotL;
    }

    prefilteredColor /= totalWeight;
    fragColor = vec4(prefilteredColor, 1.0);
   
    //fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //fragColor = vec4(importanceSampleGGX(hammersley(2u, sampleCount), n, roughness), 1.0);
    //fragColor = vec4(texture(envMap, localPos).rgb, 1.0);
}