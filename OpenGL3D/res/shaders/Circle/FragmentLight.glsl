#version 440

out vec4 fs_color;

uniform vec3 lightColor;

void main()
{
	fs_color = vec4(lightColor, 1.0);
}