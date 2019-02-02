#ifdef VERTEX_SHADER

layout (location = 0) in vec3 att_position;

void main()
{
    gl_Position = vec4(att_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 color;

void main()
{
    color = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif