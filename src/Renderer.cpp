#include "Renderer.hpp"

#include "glad/glad.h"

void
RenderMesh(const TriangleMesh& mesh,
           Vec3 position,
           Quaternion orientation,
           float mesh_scale,
           Vec4* scale_color,
           Vec4* override_color)
{
    glBindVertexArray(mesh.vao);

    for (size_t i = 0; i < mesh.triangle_list_infos.GetLen(); ++i) {
        const auto& list_info = mesh.triangle_list_infos[i];
        glDrawElements(GL_TRIANGLES,
                       list_info->num_indices,
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(list_info->first_index));
    }

    glBindVertexArray(0);
}
