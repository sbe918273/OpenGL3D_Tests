#version 440

out vec4 fs_color;

uniform vec3 color;

void main()
{
	fs_color = vec4(color, 1.0);
}