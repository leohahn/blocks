#include <stdio.h>
#include <SDL.h>
#include <glad/glad.h>
#include <SDL_opengl.h>
#include <assert.h>
#include "logger.h"

#define KILOBYTES(x) (x)*1024
#define MEGABYTES(x) KILOBYTES(x)*1024

struct Memory
{
    void* ptr;
    size_t size;
};

static Memory
Memory_New(size_t size)
{
    Memory mem = {};
    mem.ptr = calloc(1, size);
    assert(mem.ptr);
    mem.size = size;
    return mem;
}

static void
Memory_Delete(Memory mem)
{
    free(mem.ptr);
}

class ScratchAllocator
{
public:
    ScratchAllocator(Memory mem)
    : ScratchAllocator(mem.ptr, mem.size)
    {
    }

    ScratchAllocator(Memory mem, size_t size)
    {
        assert(size <= mem.size);
        ScratchAllocator(mem.ptr, size);
    }

    ScratchAllocator(void* mem, size_t size)
    : _mem(mem)
    , _bytes_allocated(0)
    , _size(size)
    {
        assert(mem && "should be instantiated with memory");
        assert(size > 0 && "allocator should have allocated bytes");
    }

    void* Allocate(size_t size)
    {
        if (!_mem) {
            return nullptr;
        }

        if (size > _size - _bytes_allocated) {
            return nullptr;
        }

        void* free_mem = (void*)((uint8_t*)_mem + _bytes_allocated);
        _bytes_allocated += size;

        return free_mem;
    }

    void Clear() { _bytes_allocated = 0; }

private:
    void* _mem;
    size_t _bytes_allocated;
    size_t _size;
};

static GLuint
LoadShaderFromFile(const char* path, ScratchAllocator allocator)
{
    assert(path);

    LOG_DEBUG("Making shader program for %s\n", path);

    GLuint vertex_shader, fragment_shader, program = 0;
    GLchar info[512] = {};
    GLint success;

    char* shader_string = nullptr;
    {
        FILE* fp = fopen(path, "rb");
        assert(fp && "shader file should exist");

        // take the size of the file
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // allocate enough memory
        shader_string = (char*)allocator.Allocate(file_size);
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
        goto cleanup_shaders;
    }

    {
        const char *vertex_string[3] = {
            "#version 330 core\n",
            "#define VERTEX_SHADER\n",
            shader_string,
        };
        glShaderSource(vertex_shader, 3, &vertex_string[0], NULL);

        const char *fragment_string[3] = {
            "#version 330 core\n",
            "#define FRAGMENT_SHADER\n",
            shader_string,
        };
        glShaderSource(fragment_shader, 3, &fragment_string[0], NULL);
    }

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info);
        LOG_ERROR("Vertex shader compilation failed: %s\n", info);
        program = 0;
        goto cleanup_shaders;
    }

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info);
        LOG_ERROR("Fragment shader compilation failed: %s\n", info);
        program = 0;
        goto cleanup_shaders;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
        LOG_ERROR("Shader linking failed: %s\n", info);
        program = 0;
        goto cleanup_shaders;
    }

cleanup_shaders:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

static void
DrawTriangle(GLuint shader_program)
{
    static float vertices[] = {
        -1, -1, 0,
        1, -1, 0,
        0, 1, 0,
    };

    static unsigned indices[] = {
        0, 1, 2,
    };

    glUseProgram(shader_program);

    // Create the vao
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the vbo and ebo
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify how buffer data is layed out in memory
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*sizeof(indices), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR("Failed to init SDL\n");
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("my window", 0, 0, 800, 600, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Now that sdl is initialized, we initialize glad to handle opengl calls.
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        return 1;
    }

    glViewport(0, 0, 800, 600);

    //
    // Create total scratch memory
    //
    Memory main_memory = Memory_New(MEGABYTES(5));
    ScratchAllocator main_scratch_allocator(main_memory);

    // Load shaders
    LOG_DEBUG("Loading shaders\n");

    ScratchAllocator shaders_allocator(main_scratch_allocator.Allocate(KILOBYTES(10)), KILOBYTES(10));

    GLuint program = LoadShaderFromFile(
        "/Users/lhahn/dev/prototypes/blocks/resources/shaders/basic.glsl", shaders_allocator
    );

    shaders_allocator.Clear();

    assert(program > 0 && "program should be valid");

    bool running = true;

    LOG_DEBUG("Starting main loop");

    glClearColor(0, 0, 0, 1);

	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
                break;
            }

			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
                case SDLK_q:
				case SDLK_ESCAPE:
					running = false;
					break;
				default:
					break;
				}
			}
		}

        glClear(GL_COLOR_BUFFER_BIT);

        // Here is the rendering code
        DrawTriangle(program);

        SDL_GL_SwapWindow(window);
	}

    Memory_Delete(main_memory);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
