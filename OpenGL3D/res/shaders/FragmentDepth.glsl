#version 440

out vec4 fs_color;

float near = 0.1;
float far = 10.0;

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far)/ (far + near - z * (far - near));
}

void main()
{
	float depth = (linearizeDepth(gl_FragCoord.z) - near ) / (far - near) ;
	//float depth = gl_FragCoord.z;
	fs_color = vec4(vec3(max(1 - depth, 0)), 1.0);
}