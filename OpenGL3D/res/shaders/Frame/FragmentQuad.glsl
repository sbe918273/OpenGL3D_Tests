#version 440

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D aTex;
uniform sampler2D bloomBlur;

uniform int mode;
uniform float exposure;

const float offset = 1.0 / 300.0;  

void main()
{
    // normal
    if (mode == 0)
        fragColor = texture(aTex, texCoords);

    // invert
    else if (mode == 1)
        fragColor = vec4(vec3(1.0 - texture(aTex, texCoords)), 1.0);

    // grayscale
    else if (mode == 2)
    {
        vec3 texColor = vec3(texture(aTex, texCoords));
	    float average = 0.2126 * texColor.r + 0.7152 *  texColor.g + 0.0722 * texColor.b;
        fragColor = vec4(average, average, average, 1.0);
    }

    // kernel
    else if (3 <= mode && mode <= 5)
    {
        vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2( 0.0,  offset), vec2(offset,  offset), 
        vec2(-offset,  0.0   ), vec2( 0.0,  0.0   ), vec2(offset,  0.0   ),   
        vec2(-offset, -offset), vec2( 0.0, -offset), vec2(offset, -offset) );

        float kernel[9];

        // sharp
        if (mode == 3)
            kernel = float[](
                -1.0, -1.0, -1.0,
                -1.0,  9.0, -1.0,
                -1.0, -1.0, -1.0
            );

        // blur
        else if (mode == 4)
            kernel = float[](
                1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
                2.0 / 16.0, 9.0 / 16.0, 1.0 / 16.0,
                1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
            );

        else
            kernel = float[](
                1.0,  1.0, 1.0,
                1.0, -8.0, 1.0,
                1.0,  1.0, 1.0
            );

        vec3 color = vec3(0.0);
        for (int i = 0; i < 9; i++)
            color += kernel[i] * vec3(texture(aTex, texCoords.st + offsets[i]));

        fragColor = vec4(color, 1.0);
    }

    // depth map
    else if (mode == 6)
    {
        float depthValue = texture(aTex, texCoords).r;
        fragColor = vec4(vec3(depthValue), 1.0);
    }

    // hdr
    else if (mode == 7)
    {
        vec3 result = vec3(texture(aTex, texCoords));
	    vec3 mapped = vec3(1.0) - exp(-result * exposure);

	    fragColor = vec4(mapped, 1.0);
    }

    // hdr with bloom
    else if (mode == 8)
    {
        vec3 color = texture(aTex, texCoords).rgb + texture(bloomBlur, texCoords).rgb;
        vec3 result = vec3(1.0) - exp(-color * exposure);
        fragColor = vec4(result, 1.0);
    }
}
