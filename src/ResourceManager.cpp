#include "ResourceManager.hpp"

#include "FileSystem.hpp"
#include "Logger.hpp"
#include "OpenGL.hpp"
#include "Path.hpp"
#include "ResourceFile.hpp"
#include "glad/glad.h"
#include "stb_image.h"

// Keys related to loading models
static constexpr const char* kDiffuseTextureKey = "diffuse_texture";
static constexpr const char* kNormalTextureKey = "normal_texture";
static constexpr const char* kObjFileKey = "obj_file";
static constexpr const char* kMtlFileKey = "mtl_file";
static constexpr const char* kRootFolderKey = "root_folder";

static Texture* LoadTextureFromFile(Allocator* allocator,
                                    Allocator* scratch_allocator,
                                    const Sid& texture_sid);

void
ResourceManager::Create()
{
    resources_path = FileSystem::GetResourcesPath(allocator);
    shaders.Create();
    textures.Create();
    meshes.Create();
}

void
ResourceManager::Destroy()
{
    for (auto& el : meshes) {
        el.val->Destroy();
        allocator->Delete(el.val);
    }
    meshes.Destroy();

    for (auto& el : textures) {
        el.val->Destroy();
        allocator->Delete(el.val);
    }
    textures.Destroy();

    for (auto& el : shaders) {
        el.val->Destroy();
        allocator->Delete(el.val);
    }
    shaders.Destroy();

    resources_path.Destroy();

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

    assert(model_res.Has(kDiffuseTextureKey));
    assert(model_res.Has(kNormalTextureKey));
    assert(model_res.Has(kObjFileKey));
    assert(model_res.Has(kMtlFileKey));
    assert(model_res.Has(kRootFolderKey));

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

    Material current_material;

    RobinHashMap<Sid, Material> materials(scratch_allocator, 16);
    while (fgets(line, sizeof(line), mtl_file) != nullptr) {
        float val;
        int illum_model;
        Vec3 color;

        if (sscanf(line, "# %s", strbuf) == 1) {
            // ignore
        } else if (sscanf(line, "newmtl %s", strbuf) == 1) {
            // new material
            current_material.name = SID(strbuf);
        } else if (sscanf(line, "Ns %f", &val) == 1) {
            current_material.shininess = val;
        } else if (sscanf(line, "Ka %f %f %f", &color.x, &color.y, &color.z) == 3) {
            current_material.ambient_color = color;
        } else if (sscanf(line, "Kd %f %f %f", &color.x, &color.y, &color.z) == 3) {
            current_material.diffuse_color = color;
        } else if (sscanf(line, "Ks %f %f %f", &color.x, &color.y, &color.z) == 3) {
            current_material.specular_color = color;
        } else if (sscanf(line, "Ni %f", &val) == 1) {
            // index of refraction, ignore...
        } else if (sscanf(line, "d %f", &val) == 1) {
            // dissolve factor, for transparent objects, ignore...
        } else if (sscanf(line, "illum %d", &illum_model) == 1) {
            assert(illum_model >= static_cast<int>(IlluminationModel::Color));
            assert(illum_model <= static_cast<int>(IlluminationModel::DiffuseAndSpecular));
            current_material.illumination_model = static_cast<IlluminationModel>(illum_model);
        } else if (sscanf(line, "map_Kd %s", strbuf) == 1) {
            // diffuse mapping. read diffuse texture from the resources folder
            String texture_path(scratch_allocator);
            texture_path.Append(root_folder->str.data);
            texture_path.Append("/");
            texture_path.Append(strbuf);

            Sid texture_sid = SID(texture_path.data);
            LoadTexture(texture_sid);
        } else if (sscanf(line, "map_Bump %s", strbuf) == 1) {
            // normal mapping
            // TODO
        } else if (sscanf(line, "map_Ks %s", strbuf) == 1) {
            // specular mapping
            // TODO
        } else {
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
            // Push a new vertex v1
            mesh->vertices.PushBack(temp_vertices[v1 - 1]);
            //mesh->normals.PushBack(temp_normals[n1 - 1]);
            mesh->uvs.PushBack(Vec2::Zero());
            mesh->indices.PushBack(mesh->vertices.len - 1);
            // Push a new vertex v2
            mesh->vertices.PushBack(temp_vertices[v2 - 1]);
            //mesh->normals.PushBack(temp_normals[n2 - 1]);
            mesh->uvs.PushBack(Vec2::Zero());
            mesh->indices.PushBack(mesh->vertices.len - 1);
            // Push a new vertex v3
            mesh->vertices.PushBack(temp_vertices[v3 - 1]);
            //mesh->normals.PushBack(temp_normals[n3 - 1]);
            mesh->uvs.PushBack(Vec2::Zero());
            mesh->indices.PushBack(mesh->vertices.len - 1);
        } else if (sscanf(line, "vt %f %f", &vec.x, &vec.y) == 2) {
            temp_uvs.PushBack(Vec2(vec.x, vec.y));
        } else if (sscanf(line, "usemtl %s", strbuf) == 1) {
            // specifies the current material
        } else if (sscanf(line, "mtllib %s", strbuf) == 1) {
            // ignore
        } else if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3) == 9) {
            // Push a new vertex v1
            mesh->vertices.PushBack(temp_vertices[v1 - 1]);
            //mesh->normals.PushBack(temp_normals[n1 - 1]);
            mesh->uvs.PushBack(temp_uvs[t1 - 1]);
            mesh->indices.PushBack(mesh->vertices.len - 1);
            // Push a new vertex v2
            mesh->vertices.PushBack(temp_vertices[v2 - 1]);
            //mesh->normals.PushBack(temp_normals[n2 - 1]);
            mesh->uvs.PushBack(temp_uvs[t2 - 1]);
            mesh->indices.PushBack(mesh->vertices.len - 1);
            // Push a new vertex v3
            mesh->vertices.PushBack(temp_vertices[v3 - 1]);
            //mesh->normals.PushBack(temp_normals[n3 - 1]);
            mesh->uvs.PushBack(temp_uvs[t3 - 1]);
            mesh->indices.PushBack(mesh->vertices.len - 1);
        } else {
            LOG_ERROR("Unrecognized obj line: %s", line);
            assert(false);
        }
    }

    LOG_INFO("The number of faces is: %d", mesh->indices.len / 3);

    temp_vertices.Destroy();
    temp_uvs.Destroy();
    temp_normals.Destroy();
    fclose(obj_file);

    // Build the buffer that is going to be uploaded to the GPU.
    Array<OpenGL::Vertex_PT> buffer(scratch_allocator);
    for (size_t i = 0; i < mesh->vertices.len; ++i) {
        buffer.PushBack(OpenGL::Vertex_PT(mesh->vertices[i], mesh->uvs[i]));
    }

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    // create the vbo and ebo
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(OpenGL::Vertex_PT) * buffer.len, buffer.data, GL_STATIC_DRAW);

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh->indices.len * sizeof(uint32_t),
                 mesh->indices.data,
                 GL_STATIC_DRAW);

    OpenGL::SetVertexFormat_PT();

    buffer.Destroy();

    SubMesh submesh = {};
    submesh.start_index = 0;
    submesh.num_indices = mesh->indices.len;
    mesh->sub_meshes.PushBack(std::move(submesh));

    model.meshes.PushBack(mesh);
    model_res.Destroy();
    materials.Destroy();
    return model;
}

void
ResourceManager::LoadTexture(const Sid& texture_sid)
{
    LOG_INFO("Loading texture for SID %s", texture_sid.GetStr());
    Texture* new_texture = LoadTextureFromFile(allocator, scratch_allocator, texture_sid);
    textures.Add(texture_sid, new_texture);
}

void
ResourceManager::LoadShader(const Sid& shader_sid)
{
    Path full_path(scratch_allocator);
    full_path.Push(resources_path);
    full_path.Push("shaders");
    full_path.Push(shader_sid.GetStr());

    LOG_DEBUG("Making shader program for %s\n", shader_sid.GetStr());

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
    full_path.Destroy();
    shaders.Add(shader_sid, shader);
}


//================================================================
// Helper functions
//================================================================

static Texture*
LoadTextureFromFile(Allocator* allocator,
                    Allocator* scratch_allocator,
                    const Sid& texture_sid)
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
    int texture_width, texture_height, texture_channel_count;

    assert(texture_buffer);

    // memory was read, now load it into an opengl buffer

    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);
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
    //if (texture->name.GetHash() == 7573085998336324) {
    if (1) {
        glGenTextures(1, &texture->handle);
        glBindTexture(GL_TEXTURE_2D, texture->handle);

        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S,
                        GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    free(data);

cleanup_texture_buffer:
    allocator->Deallocate(texture_buffer);
    full_asset_path.Destroy();
    resources_path.Destroy();

    texture->loaded = true;
    return texture;
}

void
ResourceManager::LoadMaterialsFromMtlFile(const Path& filepath)
{
    assert(filepath.data && filepath.len > 0);

    LOG_DEBUG("Loading materials from %s", filepath.data);

    size_t filesize;
    uint8_t* filedata = FileSystem::LoadFileToMemory(scratch_allocator, filepath, &filesize);
}
