#version 440
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    vec3 normal;
} gs_in[];

out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

uniform float time;

uniform mat4 view;
uniform mat4 projection;

vec4 explode(vec4 position, vec3 normal)
{
	float magnitude = 2.0;
	vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 getNormal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

void main() {    
    
    vec3 norm = getNormal();

    for (int i = 0; i < 3; i++)
    {
        gl_Position = explode(gl_in[i].gl_Position, norm);
        normal = gs_in[i].normal;
        texCoords = gs_in[i].texCoords;
        fragPos = gs_in[i].fragPos;
        EmitVertex();
    }
    EndPrimitive();
}  