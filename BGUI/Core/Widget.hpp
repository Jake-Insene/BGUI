#pragma once
#include <collections/array.h>
#include <math/color.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/rect_2d.h>


struct Event;

namespace Basic
{

struct FrameInfo;

};

namespace BGUI
{

struct UIElementBatch;

struct Widget
{
    DisableCopy(Widget);
    DisableMove(Widget);

    struct InternalData
    {
        Mem::Allocator& allocator;

        Widget* parent;
        Array<Widget*> children;

        Rect2D local_rect;
        Rect2D global_rect;
    } data;

    Widget(Mem::Allocator& allocator);
    virtual ~Widget();

    Rect2D get_local_rect() const { return data.local_rect; }
    Rect2D get_global_rect() const { return data.global_rect; }
    Vector2 get_local_size() const { return data.local_rect.size; }
    void set_local_size(const Vector2& new_size);

    template<typename T, typename... TArgs>
    T& add_node(TArgs&&... args)
    {
        return *static_cast<T*>(data.children.add(data.allocator.object<T>(Core::Forward<TArgs>(args)...)));
    }

    Slice<Widget*> get_children() { return data.children.slice(); }

    virtual Rect2D measure() = 0; 
    virtual void layout(const Vector2& absolute) = 0;
    virtual void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) = 0;

    virtual bool event(const Event& event) = 0;
};

}