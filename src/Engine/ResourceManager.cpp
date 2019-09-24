#include "Han/ResourceManager.hpp"

#include "Han/FileSystem.hpp"
#include "Han/Logger.hpp"
#include "Han/OpenGL.hpp"
#include "Han/Path.hpp"
#include "glad/glad.h"
#include "Han/Utils.hpp"
#include "Importers/GLTF2.hpp"
#include "stb_image.h"

// Keys related to loading models
static constexpr const char* kTypeKey = "type";
static constexpr const char* kRootFolderKey = "root_folder";

static constexpr const char* kGltfFileKey = "gltf_file";

static constexpr const char* kObjFileKey = "obj_file";
static constexpr const char* kMtlFileKey = "mtl_file";
static constexpr const char* kDiffuseTextureKey = "diffuse_texture";
static constexpr const char* kNormalTextureKey = "normal_texture";

static Texture* LoadTextureFromFile(Allocator* allocator,
                                    Allocator* scratch_allocator,
                                    const Sid& texture_sid,
                                    int flags = LoadTextureFlags_None);

void
ResourceManager::Create()
{
    resources_path = FileSystem::GetResourcesPath(allocator);
}

void
ResourceManager::Destroy()
{
    assert(materials.allocator != nullptr);
    for (auto& el : meshes) {
        allocator->Delete(el.val);
    }

    for (auto& el : materials) {
        allocator->Delete(el.val);
    }

    for (auto& el : textures) {
        el.val->Destroy();
        allocator->Delete(el.val);
    }

    for (auto& el : shaders) {
        allocator->Delete(el.val);
    }

    allocator = nullptr;
    scratch_allocator = nullptr;
}

Model
ResourceManager::LoadModel(const Sid& model_file)
{
    LOG_INFO("Loading model %s", model_file.GetStr());

    Path full_path(scratch_allocator);
    full_path.Push(resources_path);
    full_path.Push(model_file.GetStr());

    ResourceFile model_res(scratch_allocator, scratch_allocator);
    model_res.Create(model_file);

    assert(model_res.Has(kTypeKey));

    const auto* type = model_res.Get<ResourceFile::StringVal>(kTypeKey);

    if (type->str == "obj") {
        Model model = LoadObjModel(model_res);
        model_res.Destroy();
        return model;
    } else if (type->str == "gltf2.0") {
        Model model = LoadGltfModel(model_res);
        model_res.Destroy();
        return model;
    } else {
        LOG_ERROR("Unsupported model type: %s", type->str.data);
        assert(false);
        return Model(allocator);
    }
}

Model
ResourceManager::LoadObjModel(const ResourceFile& model_res)
{
    assert(model_res.Has(kRootFolderKey));
    assert(model_res.Has(kMtlFileKey));

    // get the root folder
    const auto* root_folder = model_res.Get<ResourceFile::StringVal>(kRootFolderKey);

    // temporary buffer for reading the files
    char line[256];
    char strbuf[128];

    Model model(allocator);

    // read all of the materials from the mtl file
    const auto* mtl_file_name = model_res.Get<ResourceFile::StringVal>(kMtlFileKey);
    Path mtl_file_path(scratch_allocator);
    mtl_file_path.Push(resources_path);
    mtl_file_path.Push(mtl_file_name->str.data);

    FILE* mtl_file = fopen(mtl_file_path.data, "rb");
    assert(mtl_file);

    Material* current_material = nullptr;

    while (fgets(line, sizeof(line), mtl_file) != nullptr) {
        float val;
        int illum_model;
        Vec3 color;

        if (sscanf(line, "# %s", strbuf) == 1) {
            // ignore
        } else if (sscanf(line, "newmtl %s", strbuf) == 1) {
            // new material
            Sid material_name = SID(strbuf);

            Material* new_material = allocator->New<Material>(allocator);
            new_material->name = material_name;
            new_material->shader = GetShader(SID("basic.glsl"));
            materials.Add(material_name, new_material);

            // FIXME: This is potentially dangerous, since if the hash table is rehashed the pointer
            // will be invalid.
            current_material = new_material;
            assert(current_material);
        } else if (sscanf(line, "Ns %f", &val) == 1) {
            assert(current_material);
            current_material->shininess = val;
        } else if (sscanf(line, "Ka %f %f %f", &color.x, &color.y, &color.z) == 3) {
            assert(current_material);
            current_material->ambient_color = color;
        } else if (sscanf(line, "Kd %f %f %f", &color.x, &color.y, &color.z) == 3) {
            assert(current_material);
            current_material->diffuse_color = color;
        } else if (sscanf(line, "Ks %f %f %f", &color.x, &color.y, &color.z) == 3) {
            assert(current_material);
            current_material->specular_color = color;
        } else if (sscanf(line, "Ni %f", &val) == 1) {
            assert(current_material);
            // index of refraction, ignore...
        } else if (sscanf(line, "d %f", &val) == 1) {
            assert(current_material);
            // dissolve factor, for transparent objects, ignore...
        } else if (sscanf(line, "illum %d", &illum_model) == 1) {
            assert(current_material);
            assert(illum_model >= static_cast<int>(IlluminationModel::Color));
            assert(illum_model <= static_cast<int>(IlluminationModel::DiffuseAndSpecular));
            current_material->illumination_model = static_cast<IlluminationModel>(illum_model);
        } else if (sscanf(line, "map_Kd %s", strbuf) == 1) {
            assert(current_material);
            // diffuse mapping. read diffuse texture from the resources folder
            String texture_path(scratch_allocator);
            texture_path.Append(root_folder->str.data);
            texture_path.Append("/");
            texture_path.Append(strbuf);

            Sid texture_sid = SID(texture_path.data);
            LoadTexture(texture_sid, LoadTextureFlags_FlipVertically|LoadTextureFlags_LinearSpace);

            current_material->AddValue(SID("u_input_texture"), MaterialValue(GetTexture(texture_sid)));
        } else if (sscanf(line, "map_Bump %s", strbuf) == 1) {
            assert(current_material);
            // normal mapping
            // TODO
        } else if (sscanf(line, "map_Ks %s", strbuf) == 1) {
            assert(current_material);
            // specular mapping
            // TODO
        } else if (!StringUtils::Trim(line).IsEmpty()) {
            LOG_ERROR("Failed to parse line: %s", line);
        }
    }

    // Finished reading mtl file, now start reading the obj file

    // first we read the obj file into an array of meshes.
    const auto* obj_file_name = model_res.Get<ResourceFile::StringVal>(kObjFileKey);
    Path obj_file_path(scratch_allocator);
    obj_file_path.Push(resources_path);
    obj_file_path.Push(obj_file_name->str.data);

    FILE* obj_file = fopen(obj_file_path.data, "rb");
    assert(obj_file);

    TriangleMesh* mesh = allocator->New<TriangleMesh>(allocator);

    // face is vertex, texture and normal indices
    Array<Vec3> temp_vertices(scratch_allocator);
    Array<Vec2> temp_uvs(scratch_allocator);
    Array<Vec3> temp_normals(scratch_allocator);

    SubMesh current_submesh = {};

    while (fgets(line, sizeof(line), obj_file) != nullptr) {
        Vec3 vec;
        int32_t v1, v2, v3, t1, t2, t3, n1, n2, n3;

        if (sscanf(line, "# %s", strbuf) == 1) {
            // comment line, just ignore
        } else if (sscanf(line, "o %s", strbuf) == 1) {
            // object, ignore for the moment
        } else if (sscanf(line, "s %s", strbuf) == 1) {
            // surface, ignore
        } else if (sscanf(line, "v %f %f %f", &vec.x, &vec.y, &vec.z) == 3) {
            temp_vertices.PushBack(vec);
        } else if (sscanf(line, "vn %f %f %f", &vec.x, &vec.y, &vec.z) == 3) {
            //temp_normals.PushBack(vec);
        } else if (sscanf(line, "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3) == 6) {
            current_submesh.num_indices += 3;

            // Push a new vertex v1
            mesh->vertices.PushBack(temp_vertices[v1 - 1]);
            //mesh->normals.PushBack(temp_normals[n1 - 1]);
            mesh->uvs.PushBack(Vec2::Zero());
            mesh->indices.PushBack((int32_t)mesh->vertices.len - 1);
            // Push a new vertex v2
            mesh->vertices.PushBack(temp_vertices[v2 - 1]);
            //mesh->normals.PushBack(temp_normals[n2 - 1]);
            mesh->uvs.PushBack(Vec2::Zero());
            mesh->indices.PushBack((int32_t)mesh->vertices.len - 1);
            // Push a new vertex v3
            mesh->vertices.PushBack(temp_vertices[v3 - 1]);
            //mesh->normals.PushBack(temp_normals[n3 - 1]);
            mesh->uvs.PushBack(Vec2::Zero());
            mesh->indices.PushBack((int32_t)mesh->vertices.len - 1);
        } else if (sscanf(line, "vt %f %f", &vec.x, &vec.y) == 2) {
            temp_uvs.PushBack(Vec2(vec.x, vec.y));
        } else if (sscanf(line, "usemtl %s", strbuf) == 1) {
            if (current_submesh.num_indices > 0) {
                mesh->sub_meshes.PushBack(current_submesh);
            }

            // specifies the current material
            Sid material_name = SID(strbuf);
            Material* mat = GetMaterial(material_name);
            assert(mat);
            assert((current_submesh.start_index + current_submesh.num_indices) == mesh->indices.len);

            current_submesh.start_index = (int32_t)(current_submesh.start_index + current_submesh.num_indices);
            current_submesh.num_indices = 0;
            current_submesh.material = mat;
        } else if (sscanf(line, "mtllib %s", strbuf) == 1) {
            // ignore
        } else if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
            &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3) == 9) {
            current_submesh.num_indices += 3;

            // Push a new vertex v1
            mesh->vertices.PushBack(temp_vertices[v1 - 1]);
            //mesh->normals.PushBack(temp_normals[n1 - 1]);
            mesh->uvs.PushBack(temp_uvs[t1 - 1]);
            mesh->indices.PushBack((int32_t)mesh->vertices.len - 1);
            // Push a new vertex v2
            mesh->vertices.PushBack(temp_vertices[v2 - 1]);
            //mesh->normals.PushBack(temp_normals[n2 - 1]);
            mesh->uvs.PushBack(temp_uvs[t2 - 1]);
            mesh->indices.PushBack((int32_t)mesh->vertices.len - 1);
            // Push a new vertex v3
            mesh->vertices.PushBack(temp_vertices[v3 - 1]);
            //mesh->normals.PushBack(temp_normals[n3 - 1]);
            mesh->uvs.PushBack(temp_uvs[t3 - 1]);
            mesh->indices.PushBack((int32_t)mesh->vertices.len - 1);
        } else {
            LOG_ERROR("Unrecognized obj line: %s", line);
            assert(false);
        }
    }

    if (current_submesh.num_indices > 0) {
        mesh->sub_meshes.PushBack(current_submesh);
    }

    LOG_INFO("The number of faces is: %zu", mesh->indices.len / 3);

    fclose(obj_file);

    // Build the buffer that is going to be uploaded to the GPU.
    Array<OpenGL::Vertex_PT> buffer(scratch_allocator);
    for (size_t i = 0; i < mesh->vertices.len; ++i) {
        buffer.PushBack(OpenGL::Vertex_PT(mesh->vertices[i], mesh->uvs[i]));
    }

    auto vbo = VertexBuffer::Create(mesh->allocator, (float*)buffer.data, buffer.len * sizeof(OpenGL::Vertex_PT));
    vbo->SetLayout(BufferLayout(mesh->allocator, {
        BufferLayoutDataType::Vec3,
        BufferLayoutDataType::Vec2,
    }));

    auto ibo = IndexBuffer::Create(mesh->allocator, mesh->indices.data, mesh->indices.len);

    model.meshes.PushBack(mesh);

    // HACK
    for (auto& submesh : model.meshes[0]->sub_meshes) {
        submesh.vao = VertexArray::Create(mesh->allocator);
        submesh.vao->SetIndexBuffer(ibo);
        submesh.vao->SetVertexBuffer(vbo);
    }
    return model;
}

Model
ResourceManager::LoadGltfModel(const ResourceFile& res_file)
{
    auto gltf_file = res_file.Get<ResourceFile::StringVal>(kGltfFileKey);
    Path gltf_file_path = FileSystem::GetResourcesPath(scratch_allocator);
    gltf_file_path.Push(gltf_file->str.data);

    Model model = ImportGltf2Model(allocator, scratch_allocator, gltf_file_path, this);

    return model;
}

Texture*
ResourceManager::LoadTexture(const Sid& texture_sid, int flags)
{
    Texture** texture;
    if (texture = textures.Find(texture_sid)) {
        return *texture;
    } else {
        LOG_DEBUG("Loading texture for SID %s", texture_sid.GetStr());
        Texture* new_texture = LoadTextureFromFile(allocator, scratch_allocator, texture_sid, flags);
        textures.Add(texture_sid, new_texture);
        return new_texture;
    }
}

void
ResourceManager::LoadShader(const Sid& shader_sid)
{
    Path full_path(scratch_allocator);
    full_path.Push(resources_path);
    full_path.Push("shaders");
    full_path.Push(shader_sid.GetStr());

    LOG_DEBUG("Making shader program for %s", shader_sid.GetStr());

    Shader* shader = allocator->New<Shader>(allocator);
    assert(shader);
    shader->name.Append(shader_sid.GetStr());

    GLuint vertex_shader = 0, fragment_shader = 0;
    GLchar info[512] = {};
    GLint success = false;

    char* shader_string = nullptr;
    {
        FILE* fp = fopen(full_path.data, "rb");
        assert(fp && "shader file should exist");

        // take the size of the file
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // allocate enough memory
        shader_string = (char*)allocator->Allocate(file_size);
        assert(shader_string && "there should be enough memory here");

        // read the file into the buffer
        size_t nread = fread(shader_string, 1, file_size, fp);
        assert(nread == file_size && "the correct amount of bytes should be read");

        fclose(fp);
    }

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) {
        LOG_ERROR("failed to create shaders (glCreateShader)\n");
        goto error_cleanup;
    }

    {
        const char *vertex_string[3] = {
            "#version 330 core\n",
            "#define VERTEX_SHADER\n",
            shader_string,
        };
        glShaderSource(vertex_shader, 3, &vertex_string[0], nullptr);

        const char *fragment_string[3] = {
            "#version 330 core\n",
            "#define FRAGMENT_SHADER\n",
            shader_string,
        };
        glShaderSource(fragment_shader, 3, &fragment_string[0], nullptr);
    }

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info);
        LOG_ERROR("Vertex shader compilation failed: %s\n", info);
        goto error_cleanup;
    }

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
        LOG_ERROR("Fragment shader compilation failed: %s\n", info);
        goto error_cleanup;
    }

    shader->program = glCreateProgram();
    glAttachShader(shader->program, vertex_shader);
    glAttachShader(shader->program, fragment_shader);
    glLinkProgram(shader->program);
    glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->program, 512, nullptr, info);
        LOG_ERROR("Shader linking failed: %s\n", info);
        goto error_cleanup;
    }

    goto ok;

error_cleanup:
    LOG_ERROR("Failed to load shader %s", shader_sid.GetStr());
    allocator->Delete(shader);

ok:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    shaders.Add(shader_sid, shader);
}


//================================================================
// Helper functions
//================================================================

static Texture*
LoadTextureFromFile(Allocator* allocator,
                    Allocator* scratch_allocator,
                    const Sid& texture_sid,
                    int flags)
{
    assert(allocator);
    assert(scratch_allocator);

    Path resources_path = FileSystem::GetResourcesPath(scratch_allocator);

    Path full_asset_path(scratch_allocator);
    full_asset_path.Push(resources_path);
    full_asset_path.Push(texture_sid.GetStr());

    Texture* texture = allocator->New<Texture>(allocator, texture_sid);

    size_t texture_buffer_size;
    uint8_t* texture_buffer =
        FileSystem::LoadFileToMemory(allocator, full_asset_path, &texture_buffer_size);
    assert(texture_buffer);

    // memory was read, now load it into an opengl buffer

    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load((flags & LoadTextureFlags_FlipVertically) != 0);

    int texture_width, texture_height, texture_channel_count;
    uint8_t* data = stbi_load_from_memory(texture_buffer,
                                          (int)texture_buffer_size,
                                          &texture_width,
                                          &texture_height,
                                          &texture_channel_count,
                                          0);

    texture->width = texture_width;
    texture->height = texture_height;

    if (!data) {
        LOG_ERROR("Failed to load texture at %s", full_asset_path.data);
        goto cleanup_texture_buffer;
    }

    //
    // FIXME: the state of opnegl is probably incorrect here, since multiple texstures
    // will be in conflict with one another.
    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum internal_format;
    GLenum format;
    if (texture_channel_count == 3) {
        format = GL_RGB;
        internal_format = ((flags & LoadTextureFlags_LinearSpace) != 0)
            ? GL_RGB
            : GL_SRGB8;
    } else if (texture_channel_count == 4) {
        format = GL_RGBA;
        internal_format = ((flags & LoadTextureFlags_LinearSpace) != 0)
            ? GL_RGBA
            : GL_SRGB8_ALPHA8;
    } else {
        LOG_ERROR("Unsupported channel count of %d", texture_channel_count);
        UNREACHABLE;
    }

    glTexImage2D(
        GL_TEXTURE_2D, 0, internal_format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, data);

    LOG_DEBUG("  loaded with width=%d and height=%d", texture_width, texture_height);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    free(data);

cleanup_texture_buffer:
    allocator->Deallocate(texture_buffer);

    texture->loaded = true;
    return texture;
}
