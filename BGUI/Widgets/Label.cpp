#include "BGUI/Widgets/Label.hpp"

#include "BGUI/Core/UIElementBatch.hpp"


namespace BGUI
{

Label::Label(Mem::Allocator& allocator, const Vector2& size) : Widget(allocator),
data{.text = String::with_allocator(allocator), }
{
    set_local_size(size);
}

Label::~Label()
{}

Rect2D Label::measure()
{
    return get_local_rect();
}

void Label::layout(const Vector2& absolute)
{
    Widget::data.global_rect.position = absolute;
    Widget::data.global_rect.size = get_local_size();
}

void Label::draw(UIElementBatch& batcher, const Basic::FrameInfo&)
{
    if(font == nullptr)
    {
        return;
    }

    batcher.draw_text(
        data.text.view(), font, font_size,
        get_global_rect().center(), background_color
    );
}

bool Label::event(const Event&)
{
    return false;
}

}