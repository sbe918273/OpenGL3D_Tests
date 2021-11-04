#version 440

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D aTex;

void main()
{
	fragColor =  texture(aTex, texCoords);
}
