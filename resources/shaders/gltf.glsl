#ifdef VERTEX_SHADER
layout (location = 0) in vec3 att_position;
layout (location = 1) in vec3 att_normal;
layout (location = 2) in vec2 att_texture;

uniform mat4 model = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 projection = mat4(1);

out vec2 vs_tex_coords;

void main()
{
    vs_tex_coords = att_texture;
    gl_Position = projection * view * model * vec4(att_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER
in vec2 vs_tex_coords;
out vec4 color;

uniform sampler2D input_texture;

void main()
{
    color = texture(input_texture, vs_tex_coords);
    color = vec4(1.0, 1.0, 1.0, 1.0);
}

#endif
