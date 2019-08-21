#include "Defines.hpp"

class MyApplication : Application
{
public:
    MyApplication(Allocator* allocator)
        : Application(allocator)
    {}

    void Update() override
    {
        printf("OnUpdate!!!\n");
    }

private:
};

static Application*
CreateApplication(Allocator* allocator)
{
    return (Application*)allocator->New<MyApplication>(allocator);
}

extern "C"
{
    BLOCKS_API void
    InitializePlugin(InitData* init_data)
    {
        ASSERT(init_data, "Init data should not be null");
        init_data->app_factory = CreateApplication;
    }
}