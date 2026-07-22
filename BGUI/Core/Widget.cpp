#include "BGUI/Core/Widget.hpp"


namespace BGUI
{

Widget::Widget(Mem::Allocator* allocator) :
data{
    .allocator = allocator,
    .parent = nullptr,
    .children = Array<Widget*>::with_allocator(allocator),
    .local_rect = {},
    .global_rect = {},
}
{
}

Widget::~Widget()
{
    for(Widget* widget : data.children.iter())
    {
        DestructObject(*widget);
        data.allocator->free(Slice(reinterpret_cast<u8*>(widget), 1));
    }

    data.children.destroy();
}

void Widget::set_local_size(const Vector2& new_size)
{
    data.local_rect.size = new_size;
}

}