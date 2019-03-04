#ifdef VERTEX_SHADER

layout (location = 0) in vec3 att_position;

uniform mat4 model = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 projection = mat4(1);

void main()
{
    gl_Position = projection * view * model * vec4(att_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 color;

void main()
{
    color = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif