#include "Renderer.hpp"

#include "glad/glad.h"
#include "OpenGL.hpp"
#include "ResourceManager.hpp"

void
RenderMesh(const TriangleMesh& mesh,
           const Shader& shader,
           Vec3 position,
           Quaternion orientation,
           float mesh_scale,
           Vec4* scale_color)
{
    const size_t index_size = sizeof(decltype(mesh.indices[0]));

    // Create the model matrix
    auto model_matrix = Mat4::Identity();
    // Set translation component
    model_matrix.m03 = position.x;
    model_matrix.m13 = position.y;
    model_matrix.m23 = position.z;
    // Set scale component
    model_matrix.m00 = mesh_scale;
    model_matrix.m11 = mesh_scale;
    model_matrix.m22 = mesh_scale;

    // set the texture

    // Set the rotation component
    const Mat4 object_to_world_matrix = model_matrix * orientation.ToMat4();
    OpenGL::SetUniformMatrixForCurrentShader(shader.model_location, object_to_world_matrix);

    for (const auto& submesh : mesh.sub_meshes) {
        submesh.vao->Bind();
        //Material* material = g_debug_resource_manager->GetMaterial(SID("wall"));
        if (submesh.material->diffuse_map) {
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, submesh.material->diffuse_map->handle);
        }

        // TODO: bind material properties for each submesh here
        glDrawElements(GL_TRIANGLES,
                       submesh.num_indices,
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(submesh.start_index * sizeof(uint32_t)));

        submesh.vao->Unbind();
    }

}
