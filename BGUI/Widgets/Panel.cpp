#include "BGUI/Widgets/Panel.hpp"

#include <input/input.h>

#include "BGUI/Core/UIElementBatch.hpp"


namespace BGUI
{

Panel::Panel(Mem::Allocator* allocator, const Vector2& size) : Widget(allocator),
data{}
{
    set_local_size(size);
}

Rect2D Panel::measure()
{
    return get_local_rect();
}

void Panel::layout(const Vector2& absolute)
{
    Widget::data.global_rect.position = absolute;
    Widget::data.global_rect.size = get_local_size();

    // getting max sizes
    Vector2 content = get_global_rect().size - (data.padding * 2);

    Vector2 group_size = Vector2();
    for(Widget* child : get_children())
    {
        Rect2D child_local_rect = child->get_local_rect();

        if(data.layout.direction == LayoutDirection::Vertical)
        {
            group_size.width = Math::max(group_size.width, child_local_rect.size.width);
            group_size.height += child_local_rect.size.height;
        }
    }
    
    if(get_children().len != 0 && data.layout.direction == LayoutDirection::Vertical)
    {
        group_size.height += (get_children().len - 1) * data.spacing.y;
    }

    // local top left
    Vector2 current_position = Vector2(data.padding.x, get_local_rect().size.height - data.padding.y);
    if(data.layout.alignment == Alignment::Center)
    {
        current_position += Vector2((content.x - group_size.x)/2, -(content.y - group_size.y)/2);
    }
    
    for(Widget* child : get_children())
    {
        Rect2D child_local_rect = child->get_local_rect();

        if(data.layout.direction == LayoutDirection::Vertical)
        {
            current_position.y -= child_local_rect.size.height;
            Vector2 child_global = get_global_rect().position + current_position;
            child->layout(child_global);
        }
        else
        {
            Vector2 child_global = get_global_rect().position
            + current_position
            + Vector2(0, -child_local_rect.size.height);
            child->layout(child_global);
            current_position.x += child_local_rect.size.width;
        }
        

        if(data.layout.direction == LayoutDirection::Vertical)
        {
            current_position.y -= data.spacing.y;
        }
        else
        {
            current_position.x += data.spacing.x;
        }
    }
}

void Panel::draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info)
{
    batcher.draw_texture(
        get_global_rect(), Rect2D(0, 0, 1, 1), background_color,
        nullptr, ElementFilter::Nearest);

    for(Widget* child : get_children())
    {
        child->draw(batcher, frame_info);
    }
}

bool Panel::event(const Event& event)
{
    Slice children = get_children();
    for (usize i = children.len; i > 0; --i)
    {
        Widget* child = children[i - 1];

        child->event(event);
    }

    return false;
}

void Panel::set_layout(const Layout& new_layout)
{
    data.layout = new_layout;
    layout(get_global_rect().position);
}

}