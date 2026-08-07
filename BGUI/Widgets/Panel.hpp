#pragma once
#include "BGUI/Core/Layout.hpp"
#include "BGUI/Core/Widget.hpp"


namespace BGUI
{

struct Panel final : Widget
{
    struct InternalData
    {
        // left-top padding
        Vector2 padding = Vector2(5, 5);
        Vector2 spacing = Vector2(5, 5);
        Layout layout = Layout(LayoutDirection::Vertical, Alignment::Start);
    } data;

    Color background_color = Color(20, 20, 20, 255);

    Panel(Mem::Allocator& allocator, const Vector2& size);

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) override;
    bool event(const Event& event) override;

    void set_padding(const Vector2& new_padding) { data.padding = new_padding; }
    void set_spacing(const Vector2& new_spacing) { data.spacing = new_spacing; }
    void set_layout(const Layout& new_layout);
};

}