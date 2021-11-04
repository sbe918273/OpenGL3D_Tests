#version 440
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiff;
layout (location = 3) out vec4 gSpec;

in vec3 fragPos;
in vec3 normal;
in vec2 texCoords;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
};

uniform Material material;

void main()
{   
    gPosition = vec4(fragPos, 1.0);
    gNormal = vec4(normalize(normal), 1.0);
    gDiff = vec4(texture(material.diffuse, texCoords).rgb, 1.0);
    gSpec = vec4(texture(material.specular, texCoords).rgb, 1.0);
}  