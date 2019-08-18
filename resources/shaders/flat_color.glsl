#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_position;

uniform mat4 u_model = mat4(1);
uniform mat4 u_view_projection = mat4(1);

void main()
{
    gl_Position = u_view_projection * u_model * vec4(a_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 o_color;
uniform vec4 u_flat_color = vec4(1);

void main()
{
    o_color = u_flat_color;
}

#endif
