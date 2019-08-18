#ifdef VERTEX_SHADER

layout (location = 0) in vec3 att_position;
layout (location = 1) in vec2 att_texture;

uniform mat4 u_model = mat4(1);
uniform mat4 u_view = mat4(1);
uniform mat4 u_projection = mat4(1);

out vec2 vs_tex_coords;

void main()
{
    vs_tex_coords = att_texture;
    gl_Position = u_projection * u_view * u_model * vec4(att_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

in vec2 vs_tex_coords;
out vec4 color;

uniform sampler2D u_input_texture;

void main()
{
    color = texture(u_input_texture, vs_tex_coords);
}

#endif
