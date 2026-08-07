#include "BGUI/Widgets/Viewport.hpp"

#include "BGUI/Core/UIElementBatch.hpp"

#include <input/input.h>


namespace BGUI
{

Viewport::Viewport(Mem::Allocator& allocator, const Vector2& size,
    GPU::TextureFormat render_target_format)
: Widget(allocator),
data{ .render_target = Basic::RenderTarget(render_target_format, Vector2I(size)) }
{
    set_local_size(size);
}

Viewport::~Viewport()
{}

Rect2D Viewport::measure()
{
    return get_local_rect();
}

void Viewport::layout(const Vector2& absolute)
{
    Widget::data.global_rect.position = absolute;
    Widget::data.global_rect.size = get_local_size();
}

void Viewport::draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info)
{
    GPU::TextureViewID gpu_texture_view = get_render_target().get_texture_view(frame_info.frame_index);
    batcher.draw_texture_gpu(
        get_global_rect(), Rect2D(Vector2(), get_local_size()), Color(255, 255, 255, 255),
        gpu_texture_view, get_local_size(), ElementFilter::Nearest
    );
}

bool Viewport::event(const Event& e)
{
    if(const InputEventMouseButton& mb = e.get<InputEventMouseButton>();
        e.type == EventType::MouseButton)
    {
        if(!on_mouse_button.has_func())
        {
            return false;
        }

        if(!get_global_rect().contains(mb.position))
        {
            return false;
        }

        const Rect2D global_rect = get_global_rect();
        
        on_mouse_button.call(
            this,
            ViewportMouseButton
            {
                .button = mb.button,
                .pressed = mb.pressed,
                .local_position = mb.position - global_rect.position,
            }
        );
    }

    return false;
}

}