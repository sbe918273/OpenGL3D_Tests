#version 440
out float fragColor;

in vec2 texCoords;

uniform sampler2D ssaoTex;

void main() 
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));
	float result = 0.0;

	int xLim = 5;
	int yLim = 5;

	for (int x = -xLim; x < xLim; x++)
	{
		for (int y = -yLim; y < yLim; y++)
		{
			vec2 offset = texelSize * vec2(float(x), float(y));
			result += texture(ssaoTex, texCoords + offset).r;
		}
	}

	fragColor = result / (4.0 * float(xLim) * float(yLim));
}