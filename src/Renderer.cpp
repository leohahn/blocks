#include "Renderer.hpp"

#include "glad/glad.h"
#include "OpenGL.hpp"

void
RenderMesh(const TriangleMesh& mesh,
           const Shader& shader,
           Vec3 position,
           Quaternion orientation,
           float mesh_scale,
           Vec4* scale_color)
{
    glBindVertexArray(mesh.vao);
    const size_t index_size = sizeof(decltype(mesh.indices[0]));

    // Set model matrix
    auto model_matrix = Mat4::Identity();
    model_matrix.m03 = position.x;
    model_matrix.m13 = position.y;
    model_matrix.m23 = position.z;

    model_matrix.m00 = mesh_scale;
    model_matrix.m11 = mesh_scale;
    model_matrix.m22 = mesh_scale;

    OpenGL::SetUniformMatrixForCurrentShader(shader.model_location, model_matrix);

    for (size_t i = 0; i < mesh.triangle_list_infos.GetLen(); ++i) {
        const auto& list_info = mesh.triangle_list_infos[i];
        const size_t offset = list_info.first_index * index_size;
        glDrawElements(GL_TRIANGLES,
                       list_info.num_indices,
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(offset));
    }

    glBindVertexArray(0);
}
