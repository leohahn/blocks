#include "GameInstance.hpp"
#include "Han/Core.hpp"
#include "Han/EngineInterface.hpp"

#include <Windows.h>

class WindowsGameInstance : public GameInstance
{
public:
    WindowsGameInstance(Allocator* allocator, const char* lib_name)
        : _allocator(allocator)
        , _lib_name(lib_name)
        , _lib_handle(nullptr)
        , _application(nullptr)
    {
    }

    ~WindowsGameInstance()
    {
        if (_lib_handle) {
            _allocator->Delete(_application);
            FreeLibrary(_lib_handle);
        } else {
            ASSERT(_application == nullptr, "Application should not exist");
        }
    }

    bool Load() override
    {
        ASSERT(_lib_handle == nullptr, "Lib is being initialized two times!");

        _lib_handle = LoadLibraryA(_lib_name);
        if (!_lib_handle) {
            LOG_ERROR("Failed to load game library %s", _lib_name);
            return false;
        }

        EngineInterfaceImpl engine_interface;

        ASSERT(_application, "Application should have been created");

        return true;
    }

    Application* GetApplication() const override { return _application; }

private:
    Allocator* _allocator;
    const char* _lib_name;
    HMODULE _lib_handle;
    Application* _application;
};

GameInstance*
GameInstance::Create(Allocator* allocator, const char* lib_name)
{
    return allocator->New<WindowsGameInstance>(allocator, lib_name);
}
