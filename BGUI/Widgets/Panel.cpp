#include "BGUI/Widgets/Panel.hpp"

#include <input/input.h>

#include "BGUI/Core/UIElementBatch.hpp"


namespace BGUI
{

Widget* hit_test(Widget* widget, const Vector2& point) {
    if (!widget)
    {
        return nullptr;
    }

    if (!widget->get_global_rect().contains(point))
    {
        return nullptr;
    }

    Slice children = widget->get_children();
    for (usize i = children.len; i > 0; --i)
    {
        Widget* child = children[i - 1];
        Widget* hit = hit_test(child, point);
        if (hit)
        {
            return hit;
        }
    }

    return widget;
}

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

    // local top left
    Vector2 current_position = Vector2(data.padding.x, get_local_rect().size.height - data.padding.y);
    for(Widget* child : get_children())
    {
        Rect2D child_local_rect = child->get_local_rect();
        current_position.y -= child_local_rect.size.height;

        Vector2 child_global = get_global_rect().position + current_position;

        child->layout(child_global);

        current_position.y -= data.padding.y;
    }
}

void Panel::draw(UIElementBatch& batcher)
{
    batcher.draw_texture(
        get_global_rect(), Rect2D(0, 0, 1, 1), background_color,
        nullptr, ElementFilter::Nearest);

    for(Widget* child : get_children())
    {
        child->draw(batcher);
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

}