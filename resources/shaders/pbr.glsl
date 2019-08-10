#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture;

uniform mat4 u_model = mat4(1);
uniform mat4 u_view_projection = mat4(1);

out vec2 fs_tex_coords;

void main()
{
    fs_tex_coords = a_texture;
    gl_Position = u_view_projection * u_model * vec4(a_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

in vec2 fs_tex_coords;
out vec4 o_color;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_metallic_roughness_texture;

void main()
{
    vec4 normal = texture(u_normal_texture, fs_tex_coords);
    vec4 metallic = texture(u_metallic_roughness_texture, fs_tex_coords);
    o_color = texture(u_albedo_texture, fs_tex_coords) + (normal - normal) + (metallic - metallic);
}

#endif
