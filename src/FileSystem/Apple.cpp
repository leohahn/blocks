#include "FileSystem.hpp"
#include <assert.h>
#include <stdio.h>

uint8_t*
FileSystem::LoadFileToMemory(Allocator* allocator, const char* path, size_t* out_file_size)
{
    assert(allocator);

    FILE* fp = nullptr;
    size_t file_size = 0;
    uint8_t* file_mem = nullptr;

    fp = fopen(path, "rb");
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