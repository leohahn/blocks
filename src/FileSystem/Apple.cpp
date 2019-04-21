#include "Defines.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

uint8_t*
FileSystem::LoadFileToMemory(Allocator* allocator, const Path& path, size_t* out_file_size)
{
    assert(allocator);
    if (path.data[path.len] != 0) {
        LOG_ERROR("oops: %s", path.data);
    }

    FILE* fp = nullptr;
    size_t file_size = 0;
    uint8_t* file_mem = nullptr;

    fp = fopen(path.data, "rb");
    if (!fp) {
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (out_file_size) {
        *out_file_size = file_size;
    }

    assert(file_size > 0);

    file_mem = (uint8_t*)allocator->Allocate(file_size);
    assert(file_mem && "There should be enough memory on the allocator");

    if (fread(file_mem, 1, file_size, fp) < file_size) {
        // Failed to read all of the file
        goto cleanup_memory;
    }

    // file was read successfully into buffer
    goto cleanup_file;

cleanup_memory:
    allocator->Deallocate(file_mem);

cleanup_file:
    fclose(fp);
    return file_mem;
}

Path
FileSystem::GetResourcesPath(Allocator* allocator)
{
    char cwd_buf[PATH_MAX];
    getcwd(cwd_buf, PATH_MAX);

    Path resources_path(allocator);
    resources_path.Push(cwd_buf);
    resources_path.Push("/resources");

    return resources_path;
}