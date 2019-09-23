#include "Han/Han.hpp"

static TriangleMesh
SetupPlane(Allocator* allocator, Allocator* scratch_allocator, Material* material)
{
    // clang-format off
    static const uint32_t indices[] =
    {
        0, 1, 2, 2, 3, 0
    };

    static const Vec3 vertices[] =
    {
        Vec3(-1.0f, -1.0f,  0.0f), // 0
        Vec3( 1.0f, -1.0f,  0.0f), // 1
        Vec3( 1.0f,  1.0f,  0.0f), // 2
        Vec3(-1.0f,  1.0f,  0.0f), // 3
    };

    static const Vec2 uvs[] =
    {
        Vec2(0.0f, 0.0f), // 0
        Vec2(1.0f, 0.0f), // 1
        Vec2(1.0f, 1.0f), // 2
        Vec2(0.0f, 1.0f), // 3
    };

    static_assert(ARRAY_SIZE(vertices) == ARRAY_SIZE(uvs),
                  "vertices and tex_coords should be the same size");

    // clang-format on
    TriangleMesh mesh(allocator);
    mesh.name = SID("Plane");

    // Fill indices up
    for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
        mesh.indices.PushBack(indices[i]);
    }

    // Fill vertices up
    for (size_t i = 0; i < ARRAY_SIZE(vertices); ++i) {
        mesh.vertices.PushBack(vertices[i]);
    }

    // Fill uvs up
    for (size_t i = 0; i < ARRAY_SIZE(uvs); ++i) {
        mesh.uvs.PushBack(uvs[i]);
    }

    assert(mesh.vertices.len == mesh.uvs.len);

    //--------------------------------------------
    // Mesh setup
    //--------------------------------------------

    // Build the buffer that is going to be uploaded to the GPU.
    Array<OpenGL::Vertex_PT> buffer(scratch_allocator);
    for (size_t i = 0; i < mesh.vertices.len; ++i) {
        buffer.PushBack(OpenGL::Vertex_PT(mesh.vertices[i], mesh.uvs[i]));
    }

    auto vbo = VertexBuffer::Create(allocator, (float*)buffer.data, buffer.len * sizeof(OpenGL::Vertex_PT));
    vbo->SetLayout(BufferLayout(allocator, {
        BufferLayoutDataType::Vec3, // postion
        BufferLayoutDataType::Vec2, // texture coord
    }));

    auto ibo = IndexBuffer::Create(allocator, mesh.indices.data, mesh.indices.len);

    SubMesh submesh = {};
    submesh.vao = VertexArray::Create(allocator);
    submesh.vao->SetIndexBuffer(ibo);
    submesh.vao->SetVertexBuffer(vbo);
    submesh.start_index = 0;
    submesh.num_indices = mesh.indices.len;
    submesh.material = material;
    mesh.sub_meshes.PushBack(std::move(submesh));

    return mesh;
}

// TODO: receive a texture catalog
static TriangleMesh
SetupCube(Allocator* allocator, Allocator* scratch_allocator, Material* material)
{
    ASSERT(material, "material should exist");
    ASSERT(allocator, "allocator should exist");
    ASSERT(scratch_allocator, "scratch allocator should exist");
    // clang-format off
    static const uint32_t indices[] =
    {
        0,  1,  2,  2,  3,  0,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
    
    static const Vec3 vertices[] =
    {
        // FRONT
        Vec3(-1, -1,  1), // 0
        Vec3(1, -1,  1), // 1
        Vec3(1,  1,  1), // 2
        Vec3(-1,  1,  1), // 3
        // BACK
        Vec3(1, -1, -1), // 4
        Vec3(-1, -1, -1), // 5
        Vec3(-1,  1, -1), // 6
        Vec3(1,  1, -1), // 7
        // TOP
        Vec3(-1,  1,  1), // 8
        Vec3(1,  1,  1), // 9
        Vec3(1,  1, -1), // 10
        Vec3(-1,  1, -1), // 11
        // BOTTOM
        Vec3(1, -1,  1), // 12
        Vec3(-1, -1,  1), // 13
        Vec3(-1, -1, -1), // 14
        Vec3(1, -1, -1), // 15
        // LEFT
        Vec3(-1, -1, -1), // 16
        Vec3(-1, -1,  1), // 17
        Vec3(-1,  1,  1), // 18
        Vec3(-1,  1, -1), // 19
        // RIGHT
        Vec3(1, -1,  1), // 20
        Vec3(1, -1, -1), // 21
        Vec3(1,  1, -1), // 22
        Vec3(1,  1,  1), // 23
    };
    
    static const Vec2 uvs[] =
    {
        // FRONT
        Vec2(0, 0), // 0
        Vec2(1, 0),// 1
        Vec2(1, 1),// 2
        Vec2(0, 1),// 3
        // BACK
        Vec2(0, 0), // 4
        Vec2(1, 0), // 5
        Vec2(1, 1), // 6
        Vec2(0, 1), // 7
        // TOP
        Vec2(0, 0), // 8
        Vec2(1, 0), // 9
        Vec2(1, 1), // 10
        Vec2(0, 1), // 11
        // BOTTOM
        Vec2(0, 0), // 12
        Vec2(1, 0), // 13
        Vec2(1, 1), // 14
        Vec2(0, 1), // 15
        // LEFT
        Vec2(0, 0), // 16
        Vec2(1, 0), // 17
        Vec2(1, 1), // 18
        Vec2(0, 1), // 19
        // RIGHT
        Vec2(0, 0), // 20
        Vec2(1, 0), // 21
        Vec2(1, 1), // 22
        Vec2(0, 1), // 23
    };

    // clang-format on
    TriangleMesh mesh(allocator);
    mesh.name = SID("Cube");

    // Fill indices up
    for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
        mesh.indices.PushBack(indices[i]);
    }

    // Fill vertices up
    for (size_t i = 0; i < ARRAY_SIZE(vertices); ++i) {
        mesh.vertices.PushBack(vertices[i]);
    }

    // Fill uvs up
    for (size_t i = 0; i < ARRAY_SIZE(uvs); ++i) {
        mesh.uvs.PushBack(uvs[i]);
    }

    assert(mesh.vertices.len == mesh.uvs.len);

    //--------------------------------------------
    // Mesh setup
    //--------------------------------------------

    // Build the buffer that is going to be uploaded to the GPU.
    Array<OpenGL::Vertex_PT> buffer(scratch_allocator);
    for (size_t i = 0; i < mesh.vertices.len; ++i) {
        buffer.PushBack(OpenGL::Vertex_PT(mesh.vertices[i], mesh.uvs[i]));
    }

    auto vbo = VertexBuffer::Create(allocator, (float*)buffer.data, buffer.len * sizeof(OpenGL::Vertex_PT));
    vbo->SetLayout(BufferLayout(allocator, {
        BufferLayoutDataType::Vec3, // position
        BufferLayoutDataType::Vec2, // texture
    }));

    auto ibo = IndexBuffer::Create(allocator, mesh.indices.data, mesh.indices.len);

    SubMesh submesh = {};
    submesh.vao = VertexArray::Create(allocator);
    submesh.vao->SetIndexBuffer(ibo);
    submesh.vao->SetVertexBuffer(vbo);
    submesh.start_index = 0;
    submesh.num_indices = mesh.indices.len;
    submesh.material = material;
    mesh.sub_meshes.PushBack(std::move(submesh));

    return mesh;
}

static constexpr float kCameraBaseMoveSpeed = 40.35f;
static constexpr float kCameraBaseRotationSpeed = 2.35f;

class GameLayer : public Layer
{
public:
	GameLayer()
		: Layer(String("Game"))
		, _camera(
			Vec3(0, 0, 5),
			Vec3(0, 0, -1),
			(float)Application::Instance()->GetScreenAspectRatio(),
			60.0f,
			kCameraBaseMoveSpeed,
			kCameraBaseRotationSpeed,
			0.1f,
			500.0f)
	{}

	void OnAttach()
	{
		LOG_DEBUG("Game layer attached!");

		Application* app = Application::Instance();
		Allocator* main_allocator = app->GetMainAllocator();
		Allocator* temp_allocator = app->GetTempAllocator();

		//
		// Load shaders
		//
		LOG_DEBUG("Loading shaders\n");

		ResourceManager* resource_manager = Application::Instance()->GetResourceManager();

		resource_manager->LoadShader(SID("flat_color.glsl"));
		_flat_color_shader = resource_manager->GetShader(SID("flat_color.glsl"));
		assert(_flat_color_shader && _flat_color_shader->IsValid() && "program should be valid");
		_flat_color_shader->AddUniform("u_model");
		_flat_color_shader->AddUniform("u_view_projection");
		_flat_color_shader->AddUniform("u_flat_color");

		resource_manager->LoadShader(SID("basic.glsl"));
		_basic_shader = resource_manager->GetShader(SID("basic.glsl"));
		assert(_basic_shader && _basic_shader->IsValid() && "program should be valid");
		_basic_shader->AddUniform("u_model");
		_basic_shader->AddUniform("u_view");
		_basic_shader->AddUniform("u_projection");
		_basic_shader->AddUniform("u_input_texture");

		resource_manager->LoadShader(SID("gltf.glsl"));
		_gltf_shader = resource_manager->GetShader(SID("gltf.glsl"));
		assert(_gltf_shader && _gltf_shader->IsValid() && "program should be valid");
		_gltf_shader->AddUniform("u_model");
		_gltf_shader->AddUniform("u_view");
		_gltf_shader->AddUniform("u_projection");
		_gltf_shader->AddUniform("u_input_texture");

		resource_manager->LoadShader(SID("pbr.glsl"));
		_pbr_shader = resource_manager->GetShader(SID("pbr.glsl"));
		assert(_pbr_shader && _pbr_shader->IsValid() && "program should be valid");
		_pbr_shader->AddUniform("u_model");
		_pbr_shader->AddUniform("u_view_projection");
		_pbr_shader->AddUniform("u_albedo_texture");
		_pbr_shader->AddUniform("u_normal_texture");
		_pbr_shader->AddUniform("u_metallic_roughness_texture");
		_pbr_shader->AddUniform("u_occlusion_texture");
		_pbr_shader->AddUniform("u_camera_position");
		_pbr_shader->AddUniform("u_light_position");
		_pbr_shader->AddUniform("u_light_color");
		_pbr_shader->AddUniform("u_metallic_factor");
		_pbr_shader->AddUniform("u_roughness_factor");

		_basic_shader->Bind();
		_basic_shader->SetUniformMat4(SID("u_projection"), _camera.projection_matrix);

		_gltf_shader->Bind();
		_gltf_shader->SetUniformMat4(SID("u_projection"), _camera.projection_matrix);

		//------------------------------
		// Create the texture catalog and textures
		//------------------------------
		resource_manager->LoadTexture(SID("wall.jpg"), LoadTextureFlags_FlipVertically|LoadTextureFlags_LinearSpace);

		Texture* wall_texture = resource_manager->GetTexture(SID("wall.jpg"));
		LOG_DEBUG("Loaded texture named: %s", wall_texture->name.GetStr());
		LOG_DEBUG("       width: %d", wall_texture->width);
		LOG_DEBUG("       height: %d", wall_texture->height);

		// HACK: load this material from a file instead of doing it like this
		{
			Material* material = resource_manager->allocator->New<Material>(resource_manager->allocator);
			material->name = SID("wall");
			material->shader = _basic_shader;
			material->AddValue(SID("u_input_texture"), MaterialValue(wall_texture));
			resource_manager->materials.Add(material->name, material);
		}

		// DEBUG meshes for testing
		Material* wall_material = resource_manager->GetMaterial(SID("wall"));
		assert(wall_material);
		_floor_mesh = SetupPlane(main_allocator, temp_allocator, wall_material);
		_cube_mesh = SetupCube(main_allocator, temp_allocator, wall_material);

		{
			Material* material = resource_manager->allocator->New<Material>(resource_manager->allocator);
			material->name = SID("flat_color");
			material->shader = _flat_color_shader;
			material->AddValue(SID("u_flat_color"), MaterialValue(Vec4(1.0f)));
			resource_manager->materials.Add(material->name, material);
		}

		_light_mesh = SetupCube(main_allocator, temp_allocator, resource_manager->GetMaterial(SID("flat_color")));

		_alpine_chalet = resource_manager->LoadModel(SID("Alpine_chalet.model"));
		_hammer = resource_manager->LoadModel(SID("hammer.model"));
		_nanosuit = resource_manager->LoadModel(SID("nanosuit.model"));

		LOG_DEBUG("Starting main loop");
		Graphics::LowLevelApi::SetClearColor(Vec4(0.2f, 0.2f, 0.2f, 1.0f));
	}

	void OnDetach()
	{
		LOG_DEBUG("Game layer detached!");
	}

	void OnUpdate(DeltaTime delta_time)
	{
		_camera.Update(delta_time);

		if (_moving_forward) {
			_camera.MoveForwards(_camera.move_speed);
		}
		if (_moving_backward) {
			_camera.MoveBackwards(_camera.move_speed);
		}
		if (_moving_left) {
			_camera.MoveLeft(_camera.move_speed);
		}
		if (_moving_right) {
			_camera.MoveRight(_camera.move_speed);
		}
		if (_turning_left) {
			_camera.Rotate(Vec3(0, 1, 0), _camera.rotation_speed);
		}
		if (_turning_right) {
			_camera.Rotate(Vec3(0, -1, 0), _camera.rotation_speed);
		}
		if (_turning_up) {
			_camera.Rotate(_camera.right, _camera.rotation_speed);
		}
		if (_turning_down) {
			_camera.Rotate(-_camera.right, _camera.rotation_speed);
		}

        //
        // Start rendering part of the main loop
        //
        Graphics::LowLevelApi::ClearBuffers();

        auto ticks = SDL_GetTicks();

        auto view_matrix = _camera.GetViewMatrix();
        auto view_projection_matrix = _camera.GetViewProjectionMatrix(view_matrix);

        _basic_shader->Bind();
        _basic_shader->SetUniformMat4(SID("u_view"), view_matrix);

        // TODO: add real values here for the parameters
        Vec3 cube_position(10.0f, 0.0f, 0.0f);
        Quaternion cube_orientation =
            Quaternion::Rotation(Math::DegreesToRadians(ticks * 0.035f), Vec3(0, 1, 0));
        float cube_scale = 1.0f;
        RenderMesh(_cube_mesh, *_basic_shader, cube_position, cube_orientation, cube_scale);

        Vec3 floor_position(0, -5, 3);
        Quaternion floor_orientation =
            Quaternion::Rotation(Math::DegreesToRadians(90), Vec3(-1, 0, 0));
        float floor_scale = 50.0f;
        RenderMesh(_floor_mesh, *_basic_shader, floor_position, floor_orientation, floor_scale);

        Vec3 nanosuit_position(-10, 0, 0);
        Quaternion nanosuit_orientation = Quaternion::Identity();
        assert(_nanosuit.meshes.len == 1);
        RenderMesh(*_nanosuit.meshes[0], *_basic_shader, nanosuit_position, nanosuit_orientation, 1.0f);

        Vec3 light_position(0.0f, 10.0f, 10.0f);
        _flat_color_shader->Bind();
        _flat_color_shader->SetUniformMat4(SID("u_view_projection"), view_projection_matrix);
        RenderMesh(_light_mesh, *_flat_color_shader, light_position, Quaternion::Identity(), 0.4f);

        _gltf_shader->Bind();
        _gltf_shader->SetUniformMat4(SID("u_view"), _camera.GetViewMatrix());

        _pbr_shader->Bind();
        _pbr_shader->SetUniformMat4(SID("u_view_projection"), view_projection_matrix);
        _pbr_shader->SetVector(SID("u_camera_position"), _camera.position);
        _pbr_shader->SetVector(SID("u_light_position"), light_position);
        _pbr_shader->SetVector(SID("u_light_color"), Vec3(1.0f));
        //RenderModel(alpine_chalet, *gltf_shader, Vec3::Zero(), Quaternion::Identity(), 1.0f);
        auto hammer_rotation = Quaternion::Rotation(Math::DegreesToRadians(90), Vec3(1, 0, 0));
        //auto hammer_rotation = Quaternion::Identity();
        RenderModel(_hammer, *_pbr_shader, Vec3::Zero(), hammer_rotation, 1.0f);

        RenderModel(_alpine_chalet, *_pbr_shader, Vec3(20, 1, 0), Quaternion::Identity(), 1.0f);
	}

	void OnEvent(Event& ev)
	{
		EventDispatcher dispatcher(ev);
		dispatcher.Dispatch<QuitEvent>([](Event& ev) -> bool {
			Application::Instance()->Quit();
			return true;
		});
		dispatcher.Dispatch<KeyPressEvent>(HAN_BIND_EV_HANDLER(GameLayer::OnKeyPress));
		dispatcher.Dispatch<KeyReleaseEvent>(HAN_BIND_EV_HANDLER(GameLayer::OnKeyRelease));
	}

private:
	bool OnKeyPress(KeyPressEvent& ev)
	{
		bool handled = false;
        if (ev.key_code == KeyCode_a) {
			_moving_left = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_d) {
			_moving_right = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_Left) {
			_turning_left = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_Right) {
			_turning_right = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_Up) {
			_turning_up = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_Down) {
			_turning_down = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_w) {
			_moving_forward = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_s) {
			_moving_backward = true;
			handled = true;
        }
        if (ev.key_code == KeyCode_q) {
			Application::Instance()->Quit();
			handled = true;
        }
		return handled;
	}

	bool OnKeyRelease(KeyReleaseEvent& ev)
	{
		bool handled = false;
        if (ev.key_code == KeyCode_a) {
			_moving_left = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_d) {
			_moving_right = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_Left) {
			_turning_left = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_Right) {
			_turning_right = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_Up) {
			_turning_up = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_Down) {
			_turning_down = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_w) {
			_moving_forward = false;
			handled = true;
        }
        if (ev.key_code == KeyCode_s) {
			_moving_backward = false;
			handled = true;
        }
		return handled;
	}

private:
	Camera _camera;
	Shader* _basic_shader = nullptr;
	Shader* _pbr_shader = nullptr;
	Shader* _gltf_shader = nullptr;
	Shader* _flat_color_shader = nullptr;
	Model _hammer;
	Model _alpine_chalet;
	Model _nanosuit;
	TriangleMesh _floor_mesh;
	TriangleMesh _cube_mesh;
	TriangleMesh _light_mesh;
	bool _moving_left = false;
	bool _moving_right = false;
	bool _turning_left = false;
	bool _turning_right = false;
	bool _turning_up = false;
	bool _turning_down = false;
	bool _moving_forward = false;
	bool _moving_backward = false;
};


class Game : public Application
{
public:
	Game(ApplicationParams params)
		: Application(params)
	{}

	void OnInitialize() override
	{
		Allocator* layer_alloc = GetLayerAllocator();
		PushLayer(layer_alloc->New<GameLayer>());
		PushOverlay(layer_alloc->New<DebugGuiLayer>());
	}
};

int main(int argc, char** argv)
{
	// unused args
	(void)argc;
	(void)argv;

	ApplicationParams params;
	params.memory_size = MEGABYTES(128);
	params.screen_width = 1024;
	params.screen_height = 768;
	params.vsync = false;

	Application* app = new Game(params);
	app->Run();

	return 0;
}
