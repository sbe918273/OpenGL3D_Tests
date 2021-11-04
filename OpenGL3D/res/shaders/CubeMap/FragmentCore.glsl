#version 440

in vec3 fragPos;
in vec3 normal;

out vec4 fragColor;

uniform int mode;
uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{
	vec3 lightDir = normalize(fragPos - viewPos);
	vec3 outDir;

	// reflect 
	if (mode == 0)
		outDir = reflect(lightDir, normalize(normal));
	// refract
	else if (mode == 1)
	{
		float ratio = 1.0 / 1.52;
		outDir = refract(lightDir, normalize(normal), ratio);
	}

	fragColor = vec4(texture(skybox, outDir).rgb, 1.0);
}
