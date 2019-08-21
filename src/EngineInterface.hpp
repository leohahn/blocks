#pragma once

class EngineInterface
{
public:
    virtual void Hello() = 0;
};

class EngineInterfaceImpl : public EngineInterface
{
public:
    void Hello() override;
};