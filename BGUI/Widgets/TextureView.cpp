#include "BGUI/Widgets/TextureView.hpp"

#include <input/input.h>
#include <math/vec2.h>
#include <resource/font.h>

#include "BGUI/Core/UIElementBatch.hpp"


namespace BGUI
{

TextureView::TextureView(Mem::Allocator& allocator, const Vector2& size) : Widget(allocator)
{
    set_local_size(size);
}

TextureView::~TextureView()
{
}

Rect2D TextureView::measure()
{
    return get_local_rect();
}

void TextureView::layout(const Vector2& absolute)
{
    Widget::data.global_rect.position = absolute;
    Widget::data.global_rect.size = get_local_size();
}

void TextureView::draw(UIElementBatch& batcher, const Basic::FrameInfo&)
{
    if(!data.gpu_texture_view.is_valid())
    {
        return;
    }

    batcher.draw_texture_gpu(
        get_global_rect(), Rect2D(Vector2(), get_local_size()), Color(255, 255, 255, 255),
        data.gpu_texture_view, get_local_size(), ElementFilter::Nearest
    );
}

bool TextureView::event(const Event&)
{
    return false;
}

void TextureView::set_gpu_texture_view(GPU::TextureViewID new_gpu_texture_view)
{
    data.gpu_texture_view = new_gpu_texture_view;
}

}