#include <Han/Han.hpp>

class MyApplication : Application
{
public:
    MyApplication(Allocator* allocator, EngineInterface* engine)
        : Application(allocator)
        , _engine(engine)
    {}

    void Update() override
    {

    }

    void OnShutdown() override
    {
        printf("OnShutdown!\n");
    }

private:
    EngineInterface* _engine;
};

static Application*
CreateApplication(Allocator* allocator, EngineInterface* engine)
{
    engine->Hello();
    return (Application*)allocator->New<MyApplication>(allocator, engine);
}

extern "C"
{
    HAN_API void
    InitializePlugin(InitData* init_data)
    {
        ASSERT(init_data, "Init data should not be null");
        init_data->app_factory = CreateApplication;
    }
}