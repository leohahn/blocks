#include "Importers/GLTF2.hpp"
#include "Han/FileSystem.hpp"
#include "Han/Logger.hpp"
#include "Han/Json.hpp"
#include "Han/ResourceManager.hpp"

#define CHUNK_TYPE_JSON 0x4E4F534A
#define CHUNK_TYPE_BINARY 0x004E4942

struct GltfNode
{
    String name;
    int32_t mesh = -1;
    Vec3 translation;
    Quaternion rotation;
};

struct TextureRef
{
    int32_t index = -1;
    int32_t tex_coord = -1;

    bool IsValid() const { return index > -1; }
};

struct GltfMaterial
{
    String name;
    bool double_sided = false;
    TextureRef base_color;
    Vec4 base_color_factor = Vec4(1.0f);
    TextureRef metallic_roughness;
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;
    TextureRef normal;
    TextureRef occlusion;
};

struct GltfTexture
{
    int32_t source = -1;
    int32_t sampler = -1;
};

struct GltfImage
{
    String name;
    String mime_type;
    String uri;

    Texture* LoadInLinearSpace(ResourceManager* rm) const
    {
        return rm->LoadTexture(SID(uri.data), LoadTextureFlags_LinearSpace);
    }

    Texture* LoadAsAlbedo(ResourceManager* rm) const
    {
        return rm->LoadTexture(SID(uri.data));
    }
};

struct GltfPrimitive
{
    RobinHashMap<String, int32_t> attributes;
    int32_t indices = -1;
    int32_t material = -1;
};

struct GltfMesh
{
    String name;
    Array<GltfPrimitive> primitives;
};

struct GltfAsset
{
	String version;
};

struct GlbBufferHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t length;

    static constexpr int kMagic = 0x46546C67;
};

struct GlbBufferChunk
{
    uint32_t length;
    uint32_t type;
    uint8_t* data;
};

static_assert(sizeof(GlbBufferHeader) == 12, "Should be 12 bytes large");

struct GltfBuffer
{
    Allocator* allocator;
    int64_t byte_length = -1;
    String uri;
    uint8_t* data;

    GltfBuffer(Allocator* alloc, const Path& path, const StringView& uri, int64_t byte_length)
        : allocator(alloc)
        , uri(alloc, uri)
        , byte_length(byte_length)
    {
        size_t file_size;
        data = FileSystem::LoadFileToMemory(alloc, path, &file_size);

        ASSERT(data, "The buffer should exist");
        ASSERT(file_size == byte_length, "sizes should be equal");
    }

    GltfBuffer(GltfBuffer&& buf)
        : data(nullptr)
    {
        *this = std::move(buf);
    }

    GltfBuffer& operator=(GltfBuffer&& buf)
    {
        if (data) {
            allocator->Deallocate(data);
        }
        allocator = buf.allocator;
        uri = std::move(buf.uri);
        byte_length = buf.byte_length;
        data = buf.data;
        buf.allocator = nullptr;
        buf.data = nullptr;
        buf.byte_length = 0;
        return *this;
    }

    GltfBuffer& operator=(const GltfBuffer&) = delete;
    GltfBuffer(const GltfBuffer&) = delete;
};

enum class GltfBufferViewTarget
{
    Undefined = 0,
    ArrayBuffer = 34962,
    ElementArrayBuffer = 34963,
};

struct GltfBufferView
{
    GltfBufferViewTarget target = GltfBufferViewTarget::Undefined;
    int64_t buffer_index = -1;
    int64_t byte_length = -1;
    int64_t byte_offset = -1;
};

enum class ComponentType
{
    Byte = 5120,
    UnsignedByte = 5121,
    Short = 5122,
    UnsignedShort = 5123,
    UnsignedInt = 5125,
    Float = 5126,
};

static bool
TryGetComponentType(int64_t component_type, ComponentType* out_component_type)
{
    ASSERT(out_component_type, "should be a valid pointer");
    if (component_type == static_cast<int64_t>(ComponentType::Byte)) {
        *out_component_type = ComponentType::Byte;
    } else if (component_type == static_cast<int64_t>(ComponentType::UnsignedByte)) {
        *out_component_type = ComponentType::UnsignedByte;
    } else if (component_type == static_cast<int64_t>(ComponentType::Short)) {
        *out_component_type = ComponentType::Short;
    } else if (component_type == static_cast<int64_t>(ComponentType::UnsignedShort)) {
        *out_component_type = ComponentType::UnsignedShort;
    } else if (component_type == static_cast<int64_t>(ComponentType::UnsignedInt)) {
        *out_component_type = ComponentType::UnsignedInt;
    } else if (component_type == static_cast<int64_t>(ComponentType::Float)) {
        *out_component_type = ComponentType::Float;
    } else {
        return false;
    }
    return true;
}

enum class AccessorType
{
    Scalar = 1,
    Vec2 = 2,
    Vec3 = 3,
    Vec4 = 4,
    Mat2 = 4,
    Mat3 = 9,
    Mat4 = 16,
};

static bool
TryGetAccessorType(const StringView& str, AccessorType* out_type)
{
    ASSERT(out_type, "should be a valid pointer");
    if (str == "SCALAR") {
        *out_type = AccessorType::Scalar;
    } else if (str == "VEC2") {
        *out_type = AccessorType::Vec2;
    } else if (str == "VEC3") {
        *out_type = AccessorType::Vec3;
    } else if (str == "VEC4") {
        *out_type = AccessorType::Vec4;
    } else if (str == "MAT2") {
        *out_type = AccessorType::Mat2;
    } else if (str == "MAT3") {
        *out_type = AccessorType::Mat3;
    } else if (str == "MAT4") {
        *out_type = AccessorType::Mat4;
    } else {
        return false;
    }
    return true;
}

struct GltfAccessor
{
    AccessorType type = AccessorType::Vec3;
    ComponentType component_type = ComponentType::Float;
    int64_t buffer_view_index = -1;
    int64_t count = -1;

	union TypeUnion
	{
		Vec2 vec2;
		Vec3 vec3;
		Vec4 vec4;
		Mat4 mat4;
		float real;
		uint8_t ubyte;
		int8_t byte;
		uint16_t usmall;
		int16_t small;
		uint32_t integer;
	};

	TypeUnion max;
	TypeUnion min;

    uint32_t opengl_buffer = 0;
    bool normalized = false;

    int64_t GetElementSize() const
    {
        int64_t num_comp = static_cast<int64_t>(type);
        switch (component_type) {
            case ComponentType::Byte:          return num_comp * 1;
            case ComponentType::UnsignedByte:  return num_comp * 1;
            case ComponentType::Short:         return num_comp * 2;
            case ComponentType::UnsignedShort: return num_comp * 2;
            case ComponentType::UnsignedInt:   return num_comp * 4;
            case ComponentType::Float:         return num_comp * 4;
            default: ASSERT(false, "Unknown component type");
        }
    }
};

static bool
TryGetRotation(const Json::Val* rotation, Quaternion* out)
{
    assert(out);
    const Array<Json::Val>* rotation_array = rotation->AsArray();
    if (!rotation_array) {
        return false;
    }
    
    if (rotation_array->len != 4) {
        return false;
    }

    double x, y, z, w;

    if (!(*rotation_array)[0].TryConvertNumberToDouble(&x)) {
        return false;
    }
    if (!(*rotation_array)[1].TryConvertNumberToDouble(&y)) {
        return false;
    }
    if (!(*rotation_array)[2].TryConvertNumberToDouble(&z)) {
        return false;
    }
    if (!(*rotation_array)[3].TryConvertNumberToDouble(&w)) {
        return false;
    }
    
    *out = Quaternion((float)x, (float)y, (float)z, (float)w);
    return true;
}

static bool
TryGetFloat(const Json::Val* vec, float* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) { return false; }
    if (translation_array->len != 1) { return false; }

	const double* val = (*translation_array)[0].AsDouble();
	if (!val) { return false; }

	*out = (float)*val;
    return true;
}

static bool
TryGetScalar(const Json::Val* vec, int64_t* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) { return false; }
    if (translation_array->len != 1) { return false; }

	const int64_t* val = (*translation_array)[0].AsInt64();
	if (!val) { return false; }

	*out = *val;
    return true;
}

static bool
TryGetVec2(const Json::Val* vec, Vec2* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) { return false; }
    if (translation_array->len != 2) { return false; }

    double x, y;

    if (!(*translation_array)[0].TryConvertNumberToDouble(&x)) {
        return false;
    }
    if (!(*translation_array)[1].TryConvertNumberToDouble(&y)) {
        return false;
    }

    *out = Vec2((float)x, (float)y);
    return true;
}
        
static bool
TryGetVec3(const Json::Val* vec, Vec3* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) {
        return false;
    }
    
    if (translation_array->len != 3) {
        return false;
    }

    double x, y, z;

    if (!(*translation_array)[0].TryConvertNumberToDouble(&x)) {
        return false;
    }
    if (!(*translation_array)[1].TryConvertNumberToDouble(&y)) {
        return false;
    }
    if (!(*translation_array)[2].TryConvertNumberToDouble(&z)) {
        return false;
    }

    *out = Vec3((float)x, (float)y, (float)z);
    return true;
}

static bool 
TryGetVec4(const Json::Val* vec, Vec4* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) {
        return false;
    }
    
    if (translation_array->len != 4) {
        return false;
    }

    double x, y, z, w;

    if (!(*translation_array)[0].TryConvertNumberToDouble(&x)) {
        return false;
    }
    if (!(*translation_array)[1].TryConvertNumberToDouble(&y)) {
        return false;
    }
    if (!(*translation_array)[2].TryConvertNumberToDouble(&z)) {
        return false;
    }
    if (!(*translation_array)[3].TryConvertNumberToDouble(&w)) {
        return false;
    }

    *out = Vec4((float)x, (float)y, (float)z, (float)w);
    return true;
}

bool
TryGetNodes(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfNode>* out_nodes)
{
    assert(alloc);
    assert(out_nodes);
    
    const Json::Val* nodes_val = gltf_file->Find(String(alloc, "nodes"));
    if (!nodes_val) {
        LOG_ERROR("Was expecting a nodes array");
        return false;
    }
    
    const Array<Json::Val>* nodes = nodes_val->AsArray();
    if (!nodes) {
        LOG_ERROR("Was expecting a nodes array");
        return false;
    }
    
    *out_nodes = Array<GltfNode>(alloc);
    
    for (size_t i = 0; i < nodes->len; ++i) {
        const RobinHashMap<String, Json::Val>* raw_node = (*nodes)[i].AsObject();
        if (!raw_node) {
            LOG_ERROR("Was expecting a node object");
            return false;
        }
        
        const Json::Val* name_val = raw_node->Find(String(alloc, "name"));
        if (name_val && !name_val->IsString()) {
            LOG_ERROR("Was expecting a name property as string");
            return false;
        }
        
        const Json::Val* mesh_val = raw_node->Find(String(alloc, "mesh"));
        if (mesh_val && !mesh_val->IsInteger()) {
            LOG_ERROR("Was expecting a mesh property as int");
            return false;
        }

        const Json::Val* rotation_val = raw_node->Find(String(alloc, "rotation"));
        if (rotation_val && !rotation_val->IsArray()) {
            LOG_ERROR("Was expecting a rotation property");
            return false;
        }

        const Json::Val* translation_val = raw_node->Find(String(alloc, "translation"));
        if (translation_val && !translation_val->IsArray()) {
            LOG_ERROR("Was expecting a translation property");
            return false;
        }
        
        GltfNode out_node;
		if (name_val) {
			out_node.name = String(alloc, name_val->AsString()->View());
		}
		if (mesh_val) {
			out_node.mesh = (int32_t)*mesh_val->AsInt64();
		}

        if (translation_val) {
            if (!TryGetVec3(translation_val, &out_node.translation)) {
                LOG_ERROR("Failed to parse translation vector in node");
                return false;
            }
        }

        if (rotation_val) {
            if (!TryGetRotation(rotation_val, &out_node.rotation)) {
                LOG_ERROR("Failed to parse rotation vector in node");
                return false;
            }
        }

        out_nodes->PushBack(std::move(out_node));
    }
    
    return true;
}

bool
TryGetMeshes(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfMesh>* out_meshes)
{
    assert(alloc);
    assert(out_meshes);
    
    const Json::Val* meshes_val = gltf_file->Find(String(alloc, "meshes"));
    if (!meshes_val) {
        LOG_ERROR("Was expecting a meshes array");
        return false;
    }
    
    const Array<Json::Val>* meshes = meshes_val->AsArray();
    if (!meshes) {
        LOG_ERROR("Was expecting a meshes array");
        return false;
    }
    
    *out_meshes = Array<GltfMesh>(alloc);
    
    for (size_t i = 0; i < meshes->len; ++i) {
        const RobinHashMap<String, Json::Val>* mesh = (*meshes)[i].AsObject();
        if (!mesh) {
            LOG_ERROR("Was expecting a mesh object");
            return false;
        }
        
        const Json::Val* name_val = mesh->Find(String(alloc, "name"));
        if (!name_val || !name_val->IsString()) {
            LOG_ERROR("Was expecting a name property");
            return false;
        }
        
        const Json::Val* primitives_val = mesh->Find(String(alloc, "primitives"));
        if (!primitives_val || !primitives_val->IsArray()) {
            LOG_ERROR("Was expecting a primitives array");
            return false;
        }
        
        GltfMesh out_mesh;
        out_mesh.name = String(alloc, name_val->AsString()->View());
        out_mesh.primitives = Array<GltfPrimitive>(alloc);
        
        for (size_t pi = 0; pi < primitives_val->AsArray()->len; ++pi) {
            const RobinHashMap<String, Json::Val>* raw_primitive = (*primitives_val->AsArray())[pi].AsObject();
            if (!raw_primitive) {
                LOG_ERROR("Was expecting a primitive object");
                return false;
            }

            const Json::Val* raw_indices = raw_primitive->Find(String(alloc, "indices"));
            if (!raw_indices || !raw_indices->IsInteger()) {
                LOG_ERROR("Was expecting a indice property");
                return false;
            }

            const Json::Val* raw_material = raw_primitive->Find(String(alloc, "material"));
            if (!raw_material || !raw_material->IsInteger()) {
                LOG_ERROR("Was expecting a material property");
                return false;
            }

            const Json::Val* raw_attributes = raw_primitive->Find(String(alloc, "attributes"));
            if (!raw_attributes || !raw_attributes->IsObject()) {
                LOG_ERROR("Was expecting an attributes property");
                return false;
            }

            GltfPrimitive primitive;
            primitive.indices = (int32_t)*raw_indices->AsInt64();
            primitive.material = (int32_t)*raw_material->AsInt64();
            primitive.attributes = RobinHashMap<String, int>(alloc, 16);

            for (const auto& pair : *raw_attributes->AsObject()) {
                const int64_t* val = pair.val.AsInt64();
                if (!val) {
                    LOG_ERROR("Was expecting integer as an attribute");
                    return false;
                }

                primitive.attributes.Add(
                    String(alloc, pair.key.View()),
                    (int32_t)*val
                );
            }

            out_mesh.primitives.PushBack(std::move(primitive));
        }

        out_meshes->PushBack(std::move(out_mesh));
    }
    
    return true;
}

bool
TryGetBuffers(Allocator* alloc, const Path& directory, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfBuffer>* out_buffers)
{
    assert(alloc);
    assert(out_buffers);
    assert(gltf_file);
    
    const Json::Val* buffers_val = gltf_file->Find(String(alloc, "buffers"));
    if (!buffers_val) {
        LOG_ERROR("Was expecting a buffers array");
        return false;
    }
    
    const Array<Json::Val>* buffers = buffers_val->AsArray();
    if (!buffers) {
        LOG_ERROR("Was expecting a buffers array");
        return false;
    }
    
    *out_buffers = Array<GltfBuffer>(alloc);
    
    for (size_t i = 0; i < buffers->len; ++i) {
        const RobinHashMap<String, Json::Val>* buffer = (*buffers)[i].AsObject();
        if (!buffer) {
            LOG_ERROR("Was expecting a buffer object");
            return false;
        }
        
        const Json::Val* uri_val = buffer->Find(String(alloc, "uri"));
        if (!uri_val || !uri_val->IsString()) {
            LOG_ERROR("Was expecting a uri property");
            return false;
        }
        
        const Json::Val* byte_length_val = buffer->Find(String(alloc, "byteLength"));
        if (!byte_length_val || !byte_length_val->IsInteger()) {
            LOG_ERROR("Was expecting a byteLength property");
            return false;
        }

		Path gltf_buffer_path = directory.Join(uri_val->AsString()->View());
        GltfBuffer out_buf(alloc, gltf_buffer_path, uri_val->AsString()->View(), *byte_length_val->AsInt64());

        out_buffers->PushBack(std::move(out_buf));
    }
    
    return true;
}

static bool
TryGetAccessors(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file,
                const Array<GltfBufferView>& buffer_views, Array<GltfAccessor>* out_accessors)
{
    assert(alloc);
    assert(gltf_file);
    assert(out_accessors);

    const Json::Val* accessors_val = gltf_file->Find(String(alloc, "accessors"));
    if (!accessors_val) {
        LOG_ERROR("Was expecting an accessors array");
        return false;
    }
    
    const Array<Json::Val>* accessors = accessors_val->AsArray();
    if (!accessors) {
        LOG_ERROR("Was expecting an accessors array");
        return false;
    }
    
    *out_accessors = Array<GltfAccessor>(alloc);
    
    for (size_t i = 0; i < accessors->len; ++i) {
        const RobinHashMap<String, Json::Val>* accessor = (*accessors)[i].AsObject();
        if (!accessor)
		{
            LOG_ERROR("Was expecting a buffer object");
            return false;
        }
        
        const Json::Val* buffer_view_val = accessor->Find(String(alloc, "bufferView"));
        if (!buffer_view_val || !buffer_view_val->IsInteger())
		{
            LOG_ERROR("Was expecting a bufferView property");
            return false;
        }
        
        const Json::Val* component_type_val = accessor->Find(String(alloc, "componentType"));
        if (!component_type_val || !component_type_val->IsInteger())
		{
            LOG_ERROR("Was expecting a componentType property");
            return false;
        }

        const Json::Val* count_val = accessor->Find(String(alloc, "count"));
        if (!count_val || !count_val->IsInteger())
		{
            LOG_ERROR("Was expecting a componentType property");
            return false;
        }

        const Json::Val* max_val = accessor->Find(String(alloc, "max"));
        if (max_val && !max_val->IsArray())
		{
            LOG_ERROR("Was expecting a max property");
            return false;
        }

        const Json::Val* min_val = accessor->Find(String(alloc, "min"));
        if (min_val && !min_val->IsArray())
		{
            LOG_ERROR("Was expecting a min property");
            return false;
        }

        const Json::Val* type_val = accessor->Find(String(alloc, "type"));
        if (!type_val || !type_val->IsString())
		{
            LOG_ERROR("Was expecting a type property");
            return false;
        }

        const Json::Val* normalized_val = accessor->Find(String(alloc, "normalized"));
        if (normalized_val && !normalized_val->IsBool())
		{
            LOG_ERROR("Was expecting a normalized property");
            return false;
        }

        GltfAccessor out_accessor;
        out_accessor.buffer_view_index = *buffer_view_val->AsInt64();
        out_accessor.count = *count_val->AsInt64();
        out_accessor.normalized = normalized_val ? *normalized_val->AsBool() : false;

        if (out_accessor.buffer_view_index >= buffer_views.len)
		{
            LOG_ERROR("Invalid buffer view index in accessor: %d", out_accessor.buffer_view_index);
            return false;
        }

        if (!TryGetAccessorType(type_val->AsString()->View(), &out_accessor.type))
		{
            LOG_ERROR("Invalid accessor type %s", type_val->AsString()->data);
            return false;
        }

        if (!TryGetComponentType(*component_type_val->AsInt64(), &out_accessor.component_type))
		{
            LOG_ERROR("Invalid component type %s", *component_type_val->AsInt64());
            return false;
        }

		if (out_accessor.component_type == ComponentType::Float &&
			out_accessor.type == AccessorType::Vec2)
		{
			if (max_val)
			{
				if (!TryGetVec2(max_val, &out_accessor.max.vec2))
				{
					LOG_ERROR("Was expecting a max vector");
					return false;
				}
			}
			else
			{
				out_accessor.max.vec2 = Vec2::Zero();
			}
			if (min_val)
			{
				if (!TryGetVec2(min_val, &out_accessor.min.vec2))
				{
					LOG_ERROR("Was expecting a min vector");
					return false;
				}
			}
			else
			{
				out_accessor.min.vec2 = Vec2::Zero();
			}
		}
		else if (out_accessor.component_type == ComponentType::Float &&
			     out_accessor.type == AccessorType::Vec3)
		{
			if (max_val)
			{
				if (!TryGetVec3(max_val, &out_accessor.max.vec3))
				{
					LOG_ERROR("Was expecting a max vector");
					return false;
				}
			}
			else
			{
				out_accessor.max.vec3 = Vec3::Zero();
			}
			if (min_val)
			{
				if (!TryGetVec3(min_val, &out_accessor.min.vec3))
				{
					LOG_ERROR("Was expecting a min vector");
					return false;
				}
			}
			else
			{
				out_accessor.min.vec3 = Vec3::Zero();
			}
		}
		else if (out_accessor.component_type == ComponentType::Float &&
				 out_accessor.type == AccessorType::Vec4)
		{
			if (max_val)
			{
				if (!TryGetVec4(max_val, &out_accessor.max.vec4))
				{
					LOG_ERROR("Was expecting a max vector");
					return false;
				}
			}
			else
			{
				out_accessor.max.vec4 = Vec4::Zero();
			}
			if (min_val)
			{
				if (!TryGetVec4(min_val, &out_accessor.min.vec4))
				{
					LOG_ERROR("Was expecting a min vector");
					return false;
				}
			}
			else
			{
				out_accessor.min.vec4 = Vec4::Zero();
			}
		}
		else if (out_accessor.component_type == ComponentType::Float &&
				 out_accessor.type == AccessorType::Mat4)
		{
			ASSERT(false, "Combination not supported yet");
		}
		else if (out_accessor.component_type == ComponentType::UnsignedShort &&
				 out_accessor.type == AccessorType::Scalar)
		{
			int64_t val;
            if (max_val)
			{
                if (!TryGetScalar(max_val, &val))
				{
                    LOG_ERROR("Was expecting a max vector");
                    return false;
                }
				out_accessor.max.usmall = (uint16_t)val;
            }
			else
			{
                out_accessor.max.usmall = 0;
            }
            if (min_val)
			{
                if (!TryGetScalar(min_val, &val))
				{
                    LOG_ERROR("Was expecting a max vector");
                    return false;
                }
				out_accessor.min.usmall = (uint16_t)val;
			}
			else
			{
				out_accessor.min.usmall = 0;
			}
		}
		else if (out_accessor.component_type == ComponentType::Float &&
				 out_accessor.type == AccessorType::Scalar)
		{
            if (max_val)
			{
                if (!TryGetFloat(max_val, &out_accessor.max.real))
				{
                    LOG_ERROR("Was expecting a max vector");
                    return false;
                }
            }
			else
			{
                out_accessor.max.usmall = 0;
            }
            if (min_val)
			{
                if (!TryGetFloat(min_val, &out_accessor.min.real))
				{
                    LOG_ERROR("Was expecting a max vector");
                    return false;
                }
			}
			else
			{
				out_accessor.min.real = 0.0f;
			}
		}
		else
		{
			ASSERT(false, "Combination not supported yet");
		}

        out_accessors->PushBack(std::move(out_accessor));
    }
    
    return true;
}

static bool
TryGetTextureRef(Allocator* alloc, const Json::Val* raw_texture_ref, TextureRef* out_texture_ref)
{
    assert(alloc);
    assert(out_texture_ref);

    if (!raw_texture_ref) {
        *out_texture_ref = TextureRef();
        return true;
    }

    if (!raw_texture_ref->IsObject()) {
        return false;
    }

    const RobinHashMap<String, Json::Val>* texture_ref = raw_texture_ref->AsObject();

    const Json::Val* index = texture_ref->Find(String(alloc, "index"));
    if (!index || !index->IsInteger()) {
        return false;
    }

    const Json::Val* tex_coord = texture_ref->Find(String(alloc, "texCoord"));
    if (!tex_coord || !tex_coord->IsInteger()) {
        return false;
    }

    *out_texture_ref = TextureRef();
    out_texture_ref->index = (int32_t)*index->AsInt64();
    out_texture_ref->tex_coord = (int32_t)*tex_coord->AsInt64();
    return true;
}

static bool
TryGetMaterial(Allocator* alloc, const RobinHashMap<String, Json::Val>* raw_material, GltfMaterial* out_material)
{
    assert(alloc);
    assert(out_material);
    assert(raw_material);
    
    const Json::Val* material_name = raw_material->Find(String(alloc, "name"));
    if (!material_name || !material_name->AsString()) {
        LOG_ERROR("Was expecting material name");
        return false;
    }

    const Json::Val* double_sided_val = raw_material->Find(String(alloc, "doubleSided"));
    if (double_sided_val && !double_sided_val->AsBool()) {
        LOG_ERROR("Was expecting a doubleSided property");
        return false;
    }

    const Json::Val* normal_texture = raw_material->Find(String(alloc, "normalTexture"));
    const Json::Val* occlusion_texture = raw_material->Find(String(alloc, "occlusionTexture"));
    
    const Json::Val* pbr_params = raw_material->Find(String(alloc, "pbrMetallicRoughness"));
    if (!pbr_params || !pbr_params->AsObject()) {
        LOG_ERROR("Was expecting pbr metallic roughness");
        return false;
    }

    const Json::Val* base_color_texture = pbr_params->AsObject()->Find(String(alloc, "baseColorTexture"));
    const Json::Val* metallic_roughness_texture = pbr_params->AsObject()->Find(String(alloc, "metallicRoughnessTexture"));
    const Json::Val* base_color_factor = pbr_params->AsObject()->Find(String(alloc, "baseColorFactor"));
    const Json::Val* metallic_factor = pbr_params->AsObject()->Find(String(alloc, "metallicFactor"));
    const Json::Val* roughness_factor = pbr_params->AsObject()->Find(String(alloc, "roughnessFactor"));

    GltfMaterial out_mat;
    out_mat.name = String(alloc, material_name->AsString()->View());
	if (double_sided_val) {
		out_mat.double_sided = *double_sided_val->AsBool();
	}

    if (!TryGetTextureRef(alloc, base_color_texture, &out_mat.base_color)) {
        LOG_ERROR("Failed to get base color texture reference from material");
        return false;
    }

    if (base_color_factor) {
        if (!TryGetVec4(base_color_factor, &out_mat.base_color_factor)) {
            LOG_ERROR("Failed to get base color factor from material");
            return false;
        }
    }

    if (metallic_factor) {
        if (!metallic_factor->TryConvertNumberToFloat(&out_mat.metallic_factor)) {
            LOG_ERROR("Failed to get metallic factor");
            return false;
        }
    }

    if (roughness_factor) {
        if (!roughness_factor->TryConvertNumberToFloat(&out_mat.roughness_factor)) {
            LOG_ERROR("Failed to get roughness factor");
            return false;
        }
    }

    if (!TryGetTextureRef(alloc, metallic_roughness_texture, &out_mat.metallic_roughness)) {
        LOG_ERROR("Failed to get metallic roughness texture reference from material");
        return false;
    }

    if (!TryGetTextureRef(alloc, normal_texture, &out_mat.normal)) {
        LOG_ERROR("Failed to get normal texture reference from material");
        return false;
    }

    if (!TryGetTextureRef(alloc, occlusion_texture, &out_mat.occlusion)) {
        LOG_ERROR("Failed to get occlusion texture reference from material");
        return false;
    }

    *out_material = std::move(out_mat);
    return true;
}

static bool
TryGetMaterials(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfMaterial>* out_materials)
{
    assert(gltf_file);
    assert(alloc);
    assert(out_materials);

    const Json::Val* materials_val = gltf_file->Find(String(alloc, "materials"));
    if (!materials_val) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }
    
    const Array<Json::Val>* materials = materials_val->AsArray();
    if (!materials) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }

    *out_materials = Array<GltfMaterial>(alloc);

    for (size_t mi = 0; mi < materials->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_material = (*materials)[mi].AsObject();
        if (!raw_material) {
            LOG_ERROR("Was expecting a material object");
            return false;
        }

        GltfMaterial material;
        if (!TryGetMaterial(alloc, raw_material, &material)) {
            LOG_ERROR("Was expecting a material object");
            return false;
        }

        out_materials->PushBack(std::move(material));
    }
    
    return true;
}

static bool
TryGetImages(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfImage>* out_images)
{
    assert(gltf_file);
    assert(alloc);
    assert(out_images);

    const Json::Val* images_val = gltf_file->Find(String(alloc, "images"));
    if (!images_val) {
		*out_images = Array<GltfImage>(alloc);
        return true;
    }
    
    const Array<Json::Val>* images = images_val->AsArray();
    if (!images) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }

    *out_images = Array<GltfImage>(alloc);

    for (size_t mi = 0; mi < images->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_image = (*images)[mi].AsObject();
        if (!raw_image) {
            LOG_ERROR("Was expecting an image object");
            return false;
        }

        const Json::Val* mime_type_val = raw_image->Find(String(alloc, "mimeType"));
        if (!mime_type_val || !mime_type_val->IsString()) {
            LOG_ERROR("Was expecting a mimeType property");
            return false;
        }

        const Json::Val* name_val = raw_image->Find(String(alloc, "name"));
        if (!name_val || !name_val->IsString()) {
            LOG_ERROR("Was expecting a name property");
            return false;
        }

        const Json::Val* uri_val = raw_image->Find(String(alloc, "uri"));
        if (!uri_val || !uri_val->IsString()) {
            LOG_ERROR("Was expecting a uri property");
            return false;
        }

        GltfImage out_image;
        out_image.mime_type = String(alloc, mime_type_val->AsString()->View());
        out_image.name = String(alloc, name_val->AsString()->View());
        out_image.uri = String(alloc, uri_val->AsString()->View());

        out_images->PushBack(std::move(out_image));
    }
    
    return true;
}

static bool
TryGetBufferViews(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfBufferView>* out_buffer_views)
{
    assert(gltf_file);
    assert(alloc);
    assert(out_buffer_views);

    const Json::Val* buffer_views_val = gltf_file->Find(String(alloc, "bufferViews"));
    if (!buffer_views_val) {
        LOG_ERROR("Was expecting a bufferViews array");
        return false;
    }
    
    const Array<Json::Val>* buffer_views = buffer_views_val->AsArray();
    if (!buffer_views) {
        LOG_ERROR("Was expecting a bufferViews array");
        return false;
    }

    *out_buffer_views = Array<GltfBufferView>(alloc);

    for (size_t mi = 0; mi < buffer_views->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_buffer_view = (*buffer_views)[mi].AsObject();
        if (!raw_buffer_view) {
            LOG_ERROR("Was expecting a bufferView object");
            return false;
        }

        const Json::Val* byte_length_val = raw_buffer_view->Find(String(alloc, "byteLength"));
        if (!byte_length_val || !byte_length_val->IsInteger()) {
            LOG_ERROR("Was expecting a byteLength property");
            return false;
        }

        const Json::Val* buffer_val = raw_buffer_view->Find(String(alloc, "buffer"));
        if (!buffer_val || !buffer_val->IsInteger()) {
            LOG_ERROR("Was expecting a buffer property");
            return false;
        }

        const Json::Val* byte_offset_val = raw_buffer_view->Find(String(alloc, "byteOffset"));
        if (!byte_offset_val || !byte_offset_val->IsInteger()) {
            LOG_ERROR("Was expecting a byteOffset property");
            return false;
        }

        const Json::Val* target_val = raw_buffer_view->Find(String(alloc, "target"));
        if (target_val && !target_val->IsInteger()) {
            LOG_ERROR("Was expecting a target property");
            return false;
        }

        GltfBufferView out_buffer_view;
        out_buffer_view.buffer_index = *buffer_val->AsInt64();
        out_buffer_view.byte_length = *byte_length_val->AsInt64();
        out_buffer_view.byte_offset = *byte_offset_val->AsInt64();

        if (target_val) {
            int64_t target = *target_val->AsInt64();
            if (target != static_cast<int64_t>(GltfBufferViewTarget::ArrayBuffer) &&
                target != static_cast<int64_t>(GltfBufferViewTarget::ElementArrayBuffer))
            {
                LOG_ERROR("Invalid buffer view target");
                return false;
            }
            out_buffer_view.target = static_cast<GltfBufferViewTarget>(target);
        }

        out_buffer_views->PushBack(std::move(out_buffer_view));
    }
    
    return true;
}

static bool
TryGetTextures(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfTexture>* out_textures)
{
    assert(alloc);
    assert(gltf_file);
    assert(out_textures);

    const Json::Val* textures_val = gltf_file->Find(String(alloc, "textures"));
    if (!textures_val) {
        LOG_ERROR("Was expecting a textures array");
        return false;
    }
    
    const Array<Json::Val>* textures = textures_val->AsArray();
    if (!textures) {
        LOG_ERROR("Was expecting a textures array");
        return false;
    }

    *out_textures = Array<GltfTexture>(alloc);

    for (size_t mi = 0; mi < textures->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_texture = (*textures)[mi].AsObject();
        if (!raw_texture) {
            LOG_ERROR("Was expecting a texture object");
            return false;
        }

        const Json::Val* source_val = raw_texture->Find(String(alloc, "source"));
        if (!source_val || !source_val->IsInteger()) {
            LOG_ERROR("Was expecting a source property");
            return false;
        }

        const Json::Val* sampler_val = raw_texture->Find(String(alloc, "sampler"));
        if (sampler_val && !sampler_val->IsInteger()) {
            LOG_ERROR("Was expecting a sampler property");
            return false;
        }

        GltfTexture out_texture;
        out_texture.source = (int32_t)*source_val->AsInt64();
        if (sampler_val) {
            out_texture.sampler = (int32_t)*sampler_val->AsInt64();
        } else {
            out_texture.sampler = 0;
        }

        out_textures->PushBack(std::move(out_texture));
    }
    
    return true;
}

#if 0
struct LoadedGltfBuffer
{
    Allocator* allocator;
    uint8_t* buffer_data;
    Array<GltfBufferChunk*> chunks;
    uint32_t vertex_buffer;

    LoadedGltfBuffer()
        : allocator(nullptr)
        , buffer_data(nullptr)
        , chunks(nullptr)
    {}

    LoadedGltfBuffer(Allocator* allocator, uint8_t* buffer_data)
        : allocator(allocator)
        , buffer_data(buffer_data)
        , chunks(allocator)
    {
    }

    ~LoadedGltfBuffer()
    {
        assert(allocator);
        allocator->Deallocate(buffer_data);
    }

    LoadedGltfBuffer(LoadedGltfBuffer&& buf)
        : LoadedGltfBuffer()
    {
        *this = std::move(buf);
    }

    LoadedGltfBuffer& operator=(LoadedGltfBuffer&& buf)
    {
        if (buffer_data && allocator) {
            allocator->Deallocate(buffer_data);
        }
        allocator = buf.allocator;
        buffer_data = buf.buffer_data;
        chunks = std::move(buf.chunks);
        buf.allocator = nullptr;
        buf.buffer_data = nullptr;
        return *this;
    }

    // delete copy constructors
    LoadedGltfBuffer(const LoadedGltfBuffer&) = delete;
    LoadedGltfBuffer& operator=(const LoadedGltfBuffer&) = delete;
};

//LoadedGltfBuffer
//LoadGltfBuffer(Allocator* alloc, const GltfBuffer& buffer)
//{
    //Path file_path = FileSystem::GetResourcesPath(alloc);
    //file_path.Push(buffer.uri.View());
//
    //size_t buffer_size;
    //uint8_t* buffer_data = FileSystem::LoadFileToMemory(alloc, file_path, &buffer_size);
    //assert(buffer_data);
//
    //GltfBufferHeader* header = (GltfBufferHeader*)buffer_data;
    //assert(header->magic == GltfBufferHeader::kMagic);
    //assert(header->version == 2);
//
    //LOG_DEBUG("Binary buffer is %lu bytes", header->length);
//
    //// start reading the chunks
    //const uint32_t chunks_total_size = header->length - sizeof(GltfBufferHeader);
    //uint32_t offset = sizeof(GltfBufferHeader);
//
    //LoadedGltfBuffer loaded_buffer(alloc, buffer_data);
//
    //for (;;) {
        //const uint32_t bytes_left = header->length - offset;
        //if (bytes_left < sizeof(GltfBufferChunk) + 1) {
            //break;
        //}
//
        //GltfBufferChunk* chunk = (GltfBufferChunk*)(buffer_data + offset);
        //assert(chunk->type == CHUNK_TYPE_BINARY && "Json chunk is not supported");
        //loaded_buffer.chunks.PushBack(chunk);
//
        //offset += chunk->length;
    //}
//
    //return loaded_buffer;
//}
#endif

static bool 
TryGetAsset(Allocator* alloc, Allocator* scratch_alloc, const RobinHashMap<String, Json::Val>* gltf_file, GltfAsset* out_asset)
{
	ASSERT(out_asset, "should not be null");
    const Json::Val* asset_val = gltf_file->Find(String(scratch_alloc, "asset"));

    if (!asset_val) {
        LOG_ERROR("'asset' entry was not found");
        return false;
    }
    
    const RobinHashMap<String, Json::Val>* asset = asset_val->AsObject();
    if (!asset) {
        LOG_ERROR("'asset' should be an object");
        return false;
    }

	const Json::Val* version_val = asset->Find(String(scratch_alloc, "version"));
	if (!version_val || !version_val->IsString()) {
        LOG_ERROR("Could not parse 'version' entry in 'asset'");
        return false;
	}

	*out_asset = GltfAsset();
	out_asset->version = String(alloc, version_val->AsString()->View());

	return true;
}

Model
ImportGltf2Model(Allocator* alloc, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager, int model_index)
{
    size_t size;
    uint8_t* data = FileSystem::LoadFileToMemory(scratch_allocator, path, &size);
    assert(data);

	Path directory = path.GetDir();
    
    Json::Document doc(scratch_allocator);
    doc.Parse(data, size);
    if (doc.HasParseErrors() || !doc.root_val.IsObject()) {
        LOG_ERROR("GLTF2 file is corrupt: %s", doc.GetErrorStr());
        assert(false);
    }

    assert(doc.root_val.type == Json::Type::Object);
    
    const RobinHashMap<String, Json::Val>* root = doc.root_val.AsObject();
    if (!root) {
        LOG_ERROR("Was expecting root to be an object");
        assert(false);
    }

	GltfAsset asset;
    if (!TryGetAsset(alloc, scratch_allocator, root, &asset)) {
        LOG_ERROR("This GLTF file is not supported");
        assert(false);
    }

	if (asset.version != "2.0") {
		LOG_ERROR("Only version 2.0 of glTF is supported");
		assert(false);
	}

    Array<GltfBuffer> buffers;
    if (!TryGetBuffers(alloc, directory, root, &buffers)) {
        LOG_ERROR("Was expecting a buffers array");
        assert(false);
    }

    Array<GltfMesh> meshes;
    if (!TryGetMeshes(alloc, root, &meshes)) {
        LOG_ERROR("Was expecting a meshes array");
        assert(false);
    }

    Array<GltfNode> nodes;
    if (!TryGetNodes(alloc, root, &nodes)) {
        LOG_ERROR("Was expecting a nodes array");
        assert(false);
    }

    Array<GltfMaterial> materials;
    if (!TryGetMaterials(alloc, root, &materials)) {
        LOG_ERROR("Was expecting a materials array");
        assert(false);
    }

    Array<GltfImage> images;
    if (!TryGetImages(alloc, root, &images)) {
        LOG_ERROR("Was expecting an images array");
        assert(false);
    }

    Array<GltfBufferView> buffer_views;
    if (!TryGetBufferViews(alloc, root, &buffer_views)) {
        LOG_ERROR("Was expecting a bufferViews array");
        assert(false);
    }

    Array<GltfAccessor> accessors;
    if (!TryGetAccessors(alloc, root, buffer_views, &accessors)) {
        LOG_ERROR("Was expecting an accessors array");
        assert(false);
    }

    Array<GltfTexture> textures;
    if (!TryGetTextures(alloc, root, &textures)) {
        ASSERT(false, "Was expecting a bufferViews array");
    }

    // A node inside gltf will be represented as a model.
    ASSERT(nodes.len == 1, "only one node supported currently");

    const GltfNode& node = nodes[0];

    Model model(alloc);
    model.name = SID(node.name.data);
    model.rotation = node.rotation;
    model.translation = node.translation;
    model.scale = 1.0f;

    const GltfMesh& gltf_mesh = meshes[node.mesh];

    // Create all materials
    for (size_t mi = 0; mi < materials.len; ++mi) {
        const GltfMaterial& gltf_material = materials[mi];

        Material* material = resource_manager->allocator->New<Material>(resource_manager->allocator);
        material->name = SID(gltf_material.name.data);
        material->shader = resource_manager->GetShader(SID("pbr.glsl"));
        material->shader->Bind();
        ASSERT(material->shader, "shader is not loaded!");

        if (gltf_material.base_color.IsValid()) {
            const GltfTexture& gltf_base_image_texture = textures[gltf_material.base_color.index];
            const GltfImage& gltf_base_image = images[gltf_base_image_texture.source];
            Texture* texture = gltf_base_image.LoadAsAlbedo(resource_manager);
            material->AddValue(SID("u_albedo_texture"), texture);
        }

        if (gltf_material.metallic_roughness.IsValid()) {
            const GltfImage& gltf_base_image = images[gltf_material.metallic_roughness.index];
            Texture* texture = gltf_base_image.LoadInLinearSpace(resource_manager);
            material->AddValue(SID("u_metallic_roughness_texture"), texture);
        }

        if (gltf_material.normal.IsValid()) {
            const GltfImage& gltf_base_image = images[gltf_material.normal.index];
            Texture* texture = gltf_base_image.LoadInLinearSpace(resource_manager);
            material->AddValue(SID("u_normal_texture"), texture);
        }

        if (gltf_material.occlusion.IsValid()) {
            const GltfImage& gltf_base_image = images[gltf_material.normal.index];
            Texture* texture = gltf_base_image.LoadInLinearSpace(resource_manager);
            material->AddValue(SID("u_occlusion_texture"), texture);
        }

        material->AddValue(SID("u_metallic_factor"), gltf_material.metallic_factor);
        material->AddValue(SID("u_roughness_factor"), gltf_material.roughness_factor);

        material->shader->Unbind();
        resource_manager->materials.Add(material->name, material);
    }

    // start loading the triangle mesh
    auto mesh = resource_manager->allocator->New<TriangleMesh>(resource_manager->allocator);
    mesh->name = SID(gltf_mesh.name.data);
    mesh->sub_meshes = Array<SubMesh>(resource_manager->allocator);

    for (size_t pi = 0; pi < gltf_mesh.primitives.len; ++pi) {
        const GltfPrimitive& primitive = gltf_mesh.primitives[pi];
        const GltfMaterial& material = materials[primitive.material];
        const GltfAccessor& indices_accessor = accessors[primitive.indices];
        const GltfBufferView& indices_buffer_view = buffer_views[indices_accessor.buffer_view_index];

        IndexBuffer* ibo = nullptr;

        if (indices_accessor.opengl_buffer == 0) {
            const GltfBufferView& buffer_view = buffer_views[indices_accessor.buffer_view_index];
            const GltfBuffer& buffer = buffers[buffer_view.buffer_index];
            size_t buffer_size = indices_accessor.GetElementSize() * indices_accessor.count;
            ASSERT(buffer_view.byte_length >= buffer_size, "Buffer view is too small!");

            ASSERT(indices_accessor.type == AccessorType::Scalar, "should be a scalar");
            if (indices_accessor.component_type == ComponentType::UnsignedInt) {
                ibo = IndexBuffer::Create(alloc, (uint32_t*)(buffer.data + buffer_view.byte_offset), indices_accessor.count);
            } else if (indices_accessor.component_type == ComponentType::UnsignedShort) {
                ibo = IndexBuffer::Create(alloc, (uint16_t*)(buffer.data + buffer_view.byte_offset), indices_accessor.count);
            } else if (indices_accessor.component_type == ComponentType::UnsignedByte) {
                // TODO: add support for indices of unsigned byte
                //ibo = IndexBuffer::Create(alloc, (uint8_t*)(buffer.data + buffer_view.byte_offset), indices_accessor.count);
                ASSERT(false, "Unsupported component type");
            } else {
                ASSERT(false, "Unsupported component type");
            }
        }

        LOG_DEBUG("Gltf2 model is using an index buffer for mesh %s", gltf_mesh.name.data);

        // Each primitive is a submesh in the engine currently.
        // TODO: improve how nodes are represented in the engine
        SubMesh submesh;
        submesh.material = resource_manager->GetMaterial(SID(material.name.data));
        submesh.start_index = 0;
        submesh.num_indices = indices_accessor.count;
        ASSERT(submesh.material, "material should exist");

        //
        // We will combine the primitive buffer views into one buffer in order to send it to the GPU 
        //

        // position
        const int32_t* position_accessor_index = primitive.attributes.Find(String(scratch_allocator, "POSITION"));
        ASSERT(position_accessor_index, "should have a position accessor");
        const GltfAccessor& position_accessor = accessors[*position_accessor_index];
        ASSERT(position_accessor.type == AccessorType::Vec3, "should be vec3");
        ASSERT(position_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& position_buffer_view = buffer_views[position_accessor.buffer_view_index];

        // normal
        const int32_t* normal_accessor_index = primitive.attributes.Find(String(scratch_allocator, "NORMAL"));
        ASSERT(normal_accessor_index, "should have a normal accessor");
        const GltfAccessor& normal_accessor = accessors[*normal_accessor_index];
        ASSERT(normal_accessor.type == AccessorType::Vec3, "should be vec3");
        ASSERT(normal_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& normal_buffer_view = buffer_views[normal_accessor.buffer_view_index];

        // tangent
        const int32_t* tangent_accessor_index = primitive.attributes.Find(String(scratch_allocator, "TANGENT"));
        ASSERT(tangent_accessor_index, "should have a tangent accessor");
        const GltfAccessor& tangent_accessor = accessors[*tangent_accessor_index];
        ASSERT(tangent_accessor.type == AccessorType::Vec4, "should be vec4");
        ASSERT(tangent_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& tangent_buffer_view = buffer_views[tangent_accessor.buffer_view_index];

        // tex coords
        const int32_t* texcoord0_accessor_index = primitive.attributes.Find(String(scratch_allocator, "TEXCOORD_0"));
        ASSERT(texcoord0_accessor_index, "should have a tex coord 0 accessor");
        const GltfAccessor& texcoord0_accessor = accessors[*texcoord0_accessor_index];
        ASSERT(texcoord0_accessor.type == AccessorType::Vec2, "should be vec2");
        ASSERT(texcoord0_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& texcoord0_buffer_view = buffer_views[texcoord0_accessor.buffer_view_index];

        ASSERT(position_buffer_view.buffer_index == normal_buffer_view.buffer_index, "Should reference the same buffer");
        ASSERT(position_buffer_view.buffer_index == tangent_buffer_view.buffer_index, "Should reference the same buffer");
        ASSERT(position_buffer_view.buffer_index == texcoord0_buffer_view.buffer_index, "Should reference the same buffer");
        const GltfBuffer& buffer = buffers[position_buffer_view.buffer_index];

        ASSERT(position_buffer_view.byte_offset < normal_buffer_view.byte_offset, "position should come first");
        ASSERT(normal_buffer_view.byte_offset < texcoord0_buffer_view.byte_offset, "normals should come second");
        ASSERT(tangent_buffer_view.byte_offset < texcoord0_buffer_view.byte_offset, "tangents should come third");

        const size_t buffer_start_offset = position_buffer_view.byte_offset;
        const size_t buffer_total_size = position_buffer_view.byte_length + normal_buffer_view.byte_length + texcoord0_buffer_view.byte_length;
        const float* buffer_start = (float*)(buffer.data + buffer_start_offset);

        ASSERT(position_accessor.count == normal_accessor.count, "Vertex attributes should have the same count of elements");
        ASSERT(position_accessor.count == texcoord0_accessor.count, "Vertex attributes should have the same count of elements");
        ASSERT(position_accessor.count == tangent_accessor.count, "Vertex attributes should have the same count of elements");

        auto vbo = VertexBuffer::Create(alloc, buffer_start, buffer_total_size * sizeof(float));
        vbo->SetLayout(BufferLayout::NonInterleaved(alloc, {
            BufferLayoutDataType::Vec3, // position
            BufferLayoutDataType::Vec3, // normals
            BufferLayoutDataType::Vec4, // tangent
            BufferLayoutDataType::Vec2, // tex coords
        }, (size_t)position_accessor.count));

        submesh.vao = VertexArray::Create(mesh->allocator);
        submesh.vao->SetVertexBuffer(vbo);
        if (ibo) {
            ASSERT(ibo->GetNumIndices() == submesh.num_indices, "should be the same");
            submesh.vao->SetIndexBuffer(ibo);
        }

        mesh->sub_meshes.PushBack(std::move(submesh));
    }

    model.meshes.PushBack(std::move(mesh));

    scratch_allocator->Deallocate(data);
    return model;
}
