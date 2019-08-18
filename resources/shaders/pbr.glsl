#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec4 a_tangent;
layout (location = 3) in vec2 a_texture;

uniform mat4 u_model = mat4(1);
uniform mat4 u_view_projection = mat4(1);

out VS_OUT 
{
    vec2 tex_coords;
    vec3 normal;
    vec3 world_pos;
    mat3 TBN;
} vs_out;

void main()
{
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    vs_out.world_pos = world_pos.xyz;
    vs_out.tex_coords = a_texture;
    vs_out.normal = a_normal;

    vec3 bitangent = cross(a_normal, a_tangent.xyz) * a_tangent.w;
    vec3 T = normalize(vec3(u_model * vec4(a_tangent.xyz, 0.0)));
    vec3 B = normalize(vec3(u_model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vs_out.normal);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = u_view_projection * world_pos;
}

#endif

#ifdef FRAGMENT_SHADER

const float PI = 3.14159265359;

in VS_OUT 
{
    vec2 tex_coords;
    vec3 normal;
    vec3 world_pos;
    mat3 TBN;
} vs_out;

out vec4 out_color;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_metallic_roughness_texture;

uniform vec3 u_camera_position;
uniform vec3 u_light_position;
uniform vec3 u_light_color = vec3(1);

//
// NOTE: This is based on this tutorial: https://learnopengl.com/PBR/Lighting
//

vec3 GetFragmentNormalVector()
{
    vec3 normal = texture(u_normal_texture, vs_out.tex_coords).rgb;
    normal = normalize(normal * 2 - 1.0); // map to range [-1, 1]
    normal = normalize(vs_out.TBN * normal);
    return normal;
}

// Calculates the ratio of light that gets reflected compared to what was refracted.
vec3 FresnelSchlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

void main()
{
    vec3 view_dir = normalize(u_camera_position - vs_out.world_pos);
    vec3 normal = GetFragmentNormalVector();

    vec3 albedo = texture(u_albedo_texture, vs_out.tex_coords).rgb;
    // Get the metalness and roughness component from the packed texture value.
    vec4 metallic_roughness = texture(u_metallic_roughness_texture, vs_out.tex_coords);
    float metalness = metallic_roughness.b;
    float roughness = metallic_roughness.g;

    vec3 F0 = mix(vec3(0.04), albedo, metalness);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 1; ++i) {
        vec3 light_dir = normalize(u_light_position - vs_out.world_pos);
        vec3 h = normalize(view_dir + light_dir);

        float distance = length(u_light_position - vs_out.world_pos);
        //float attenuation = 1 / (distance * distance);
	float attenuation = 1;
        vec3 radiance = u_light_color * attenuation;

        vec3 F = FresnelSchlick(max(dot(h, view_dir), 0.0), F0);
        float NDF = DistributionGGX(normal, h, roughness);       
        float G = GeometrySmith(normal, view_dir, light_dir, roughness);       

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);  

        vec3 kS = F; // ratio of specular contribution
        vec3 kD = vec3(1.0) - kS;
        // zero out kD factor if the fragment is metallic
        kD *= vec3(1.0) - metalness;

        float NdotL = max(dot(normal, light_dir), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;

    // Apply gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    out_color = vec4(color, 1.0);
}

#endif
