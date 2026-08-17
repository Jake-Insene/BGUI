#include "BGUI/Widgets/Button.hpp"

#include <input/input.h>
#include <math/vec2.h>
#include <Basic/Resource/Font.hpp>

#include "BGUI/Core/UIElementBatch.hpp"


namespace BGUI
{

Button::Button(Mem::Allocator& allocator, const Vector2& size) : Widget(allocator),
data{.current_state = State::Normal, .text = Collections::String(allocator, 0, {}), }
{
    set_local_size(size);
}

Button::~Button()
{}

Rect2D Button::measure()
{
    return get_local_rect();
}

void Button::layout(const Vector2& absolute)
{
    Widget::data.global_rect.position = absolute;
    Widget::data.global_rect.size = get_local_size();
}

void Button::draw(UIElementBatch& batcher, const Basic::FrameInfo&)
{
    Color target_color = background_color;
    if(data.current_state == State::Hovered)
    {
        target_color.r *= 0.8;
        target_color.g *= 0.8;
        target_color.b *= 0.8;
    }
    else if(data.current_state == State::Pressed)
    {
        target_color.r *= 0.5;
        target_color.g *= 0.5;
        target_color.b *= 0.5;
    }

    batcher.draw_texture_gpu(
        get_global_rect(), Rect2D(0, 0, 1, 1),
        target_color, batcher.get_white_texture(), Vector2(1, 1), ElementFilter::Nearest
    );

    if(font == nullptr)
    {
        return;
    }

    batcher.draw_text(
        data.text.view(), font, font_size,
        get_global_rect().center(), Color(255, 255, 255, 255)
    );
}

bool Button::event(const Event& event)
{
    if(const InputEventMouseButton& mb = event.get<InputEventMouseButton>(); event.type == EventType::MouseButton)
    {
        bool is_inside = get_global_rect().contains(mb.position);
        if(is_inside)
        {
            if(mb.pressed && mb.button == MouseButton::Left)
            {
                data.current_state = State::Pressed;
                if(on_click.has_func())
                {
                    on_click.call(this);
                }
                return true;
            }
            else if(!mb.pressed)
            {
                data.current_state = State::Hovered;
                return true;
            }
        }
        else
        {
            if(!mb.pressed)
            {
                data.current_state = State::Normal;
            }
        }
    }
    else if(const InputEventMouseMove& mv = event.get<InputEventMouseMove>(); event.type == EventType::MouseMove)
    {
        if(get_global_rect().contains(mv.position))
        {
            if(data.current_state != State::Pressed)
            {
                data.current_state = State::Hovered;
            }
            return true;
        }
        else
        {
            data.current_state = State::Normal;
            return false;
        }
    }

    return false;
}

void Button::set_text(Collections::StringView view)
{
    data.text.set(view);
}

}