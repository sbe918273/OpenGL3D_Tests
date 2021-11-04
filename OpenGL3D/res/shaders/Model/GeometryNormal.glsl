#version 440
layout (triangles) in;
layout (line_strip, max_vertices = 12) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

out vec3 color;

uniform mat4 projection;

const float MAGNITUDE = 0.4;

vec3 getNormal()
{
	vec3 a = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

void generateLine(int index, vec3 normal, vec3 aColor)
{
    color = aColor;
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + vec4(normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() {    
    
    vec3 normal = getNormal();

    for (int i = 0; i < 3; i++)
    {
        generateLine(i, gs_in[i].normal, vec3(1.0, 1.0, 0.0));
        generateLine(i, normal, vec3(1.0, 0.0, 1.0));
    }

}  