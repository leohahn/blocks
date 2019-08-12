#include "Renderer.hpp"

#include "glad/glad.h"
#include "OpenGL.hpp"
#include "ResourceManager.hpp"

void RenderModel(
    const Model& model,
    const Shader& shader,
    Vec3 position,
    Quaternion orientation,
    float mesh_scale,
    Vec4* scale_color)
{
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

    // Set the rotation component
    const Mat4 object_to_world_matrix = model_matrix * orientation.ToMat4();
    shader.SetUniformMat4(SID("model"), object_to_world_matrix);

    for (const auto& mesh : model.meshes) {
        for (const auto& submesh : mesh->sub_meshes) {
            submesh.vao->Bind();
            //Material* material = g_debug_resource_manager->GetMaterial(SID("wall"));
            submesh.material->Bind();
            //if (submesh.material->diffuse_map) {
                //glActiveTexture(GL_TEXTURE0 + 0);
                //glBindTexture(GL_TEXTURE_2D, submesh.material->diffuse_map->handle);
            //}

            size_t index_size = submesh.vao->GetIndexBuffer()->GetIndexSize();
            GLenum index_type;
            switch (index_size) {
                case sizeof(uint32_t) :
                    index_type = GL_UNSIGNED_INT;
                    break;
                case sizeof(uint16_t):
                    index_type = GL_UNSIGNED_SHORT;
                    break;
                case sizeof(uint8_t):
                    index_type = GL_UNSIGNED_BYTE;
                    break;
                default:
                    ASSERT(false, "Unknown index size");
                    break;
            }

            // TODO: bind material properties for each submesh here
            glDrawElements(GL_TRIANGLES,
                           submesh.num_indices,
                           index_type,
                           reinterpret_cast<const void*>(submesh.start_index * index_size));

            submesh.vao->Unbind();
        }
    }
}

void
RenderMesh(const TriangleMesh& mesh,
           const Shader& shader,
           Vec3 position,
           Quaternion orientation,
           float mesh_scale,
           Vec4* scale_color)
{
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
    shader.SetUniformMat4(SID("model"), object_to_world_matrix);

    for (const auto& submesh : mesh.sub_meshes) {
        submesh.vao->Bind();
        //Material* material = g_debug_resource_manager->GetMaterial(SID("wall"));
        submesh.material->Bind();
        //if (submesh.material->diffuse_map) {
            //glActiveTexture(GL_TEXTURE0 + 0);
            //glBindTexture(GL_TEXTURE_2D, submesh.material->diffuse_map->handle);
        //}

        // TODO: bind material properties for each submesh here
        glDrawElements(GL_TRIANGLES,
                       submesh.num_indices,
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(submesh.start_index * sizeof(uint32_t)));

        submesh.vao->Unbind();
    }
}
