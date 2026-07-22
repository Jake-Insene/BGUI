#pragma once
#include "BGUI/Core/Widget.hpp"


namespace BGUI
{

struct Panel final : Widget
{
    struct InternalData
    {
        Vector2 padding = Vector2(5, 5);
    } data;

    Panel(Mem::Allocator* allocator, const Vector2& size);

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher) override;
    bool event(const Event& event) override;
};

}