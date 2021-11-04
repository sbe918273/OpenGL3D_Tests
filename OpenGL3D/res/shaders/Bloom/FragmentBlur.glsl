#version 440

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D aTex;
uniform bool horizontal;

void main()
{
	
	const float weight[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};
	const vec2 texelOffset = 1.0 / textureSize(aTex, 0);  

	vec3 result = vec3(0.0);

	vec2 offset;
	if (horizontal)
		offset = vec2(texelOffset.x, 0.0);
	else
		offset = vec2(0.0, texelOffset.y);

	for(int j = 0; j < 4; j++)
    {
        result += texture(aTex, texCoords + j * offset).rgb  * weight[j];
        result += texture(aTex, texCoords - j * offset).rgb * weight[j];
    }

	fragColor = vec4(result, 1.0);
}
