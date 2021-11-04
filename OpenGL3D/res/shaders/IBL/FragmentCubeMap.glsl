#version 440
out vec4 fragColor;

in vec3 localPos;
  
uniform samplerCube envMap;

uniform float mipLevel;

const float PI = 3.14159265359;

void main()
{

    vec3 color = textureLod(envMap, localPos, mipLevel).rgb;

    // hdr and gamma correction
	const float exposure = 1.0;
	color = vec3(1.0) - exp(-color * exposure);
    color = pow(color, vec3(1.0/2.2));  
  
    fragColor = vec4(color, 1.0);
}