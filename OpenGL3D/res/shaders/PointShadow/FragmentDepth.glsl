#version 440

in vec4 fragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
	// linear distance between light and fragment, clamped between 0 and 1.
	
	float lightDistance = length(fragPos.xyz - lightPos);

	gl_FragDepth = lightDistance / farPlane;
}