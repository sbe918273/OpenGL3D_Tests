#version 440
out vec4 fragColor;
in vec3 localPos;

uniform sampler2D envMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;

    uv += 0.5;
    return uv;
}

void main()
{	
    // magic explained in whiteboard picture
    vec2 uv = sampleSphericalMap(normalize(localPos));
    vec3 color = texture(envMap, uv).rgb;
    
    fragColor = vec4(color, 1.0);
    //fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
