#version 440
out vec4 fragColor;

in vec3 localPos;
  
uniform samplerCube envMap;
//uniform sampler2D faceTex;  

const float PI = 3.14159265359;

void main()
{
    vec3 normal = normalize(localPos);

    vec3 irradiance = vec3(0.0);

    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(worldUp, normal);
    vec3 up = cross(normal, right);

    mat3 TBN = mat3(right, up, normal);

    float sampleDelta = 0.025;
    int sampleCount = 0;

    // azimuth angle covering the entire base of the hemisphere
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) 
    {
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        // zenith angle from top of hemisphere to the base
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            float sinTheta = sin(theta);
            vec3 tangentSample = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cos(theta));
            vec3 sampleVec = TBN * tangentSample; // tangent to world space


            irradiance += texture(envMap, sampleVec).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }

    irradiance *= PI / float(sampleCount);
    fragColor = vec4(irradiance, 1.0);
}