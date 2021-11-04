#version 440
layout (location = 0) in vec3 aPos;
layout (location = 3) in mat4 aModel;
layout (location = 7) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 color;

void main()
{
	color = aColor;
	gl_Position = projection * view * aModel * vec4(aPos, 1.0);
}