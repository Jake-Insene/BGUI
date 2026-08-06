#pragma once
#include <collections/function.h>
#include <input/input.h>
#include <Basic/Core/RenderTarget.hpp>

#include "BGUI/Core/Widget.hpp"


namespace BGUI
{

struct ViewportMouseButton
{
    MouseButton button;
    bool pressed;
    Vector2 local_position;
};

struct Viewport : Widget
{
    struct InternalData
    {
        Basic::RenderTarget render_target;
    } data;

    Function<void(*)(Viewport*, const ViewportMouseButton&)> on_mouse_button{};

    Viewport(Mem::Allocator* allocator, const Vector2& size, GPU::TextureFormat render_target_format);
    ~Viewport();

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) override;
    bool event(const Event& event) override;

    Basic::RenderTarget& get_render_target() { return data.render_target; }
};

}