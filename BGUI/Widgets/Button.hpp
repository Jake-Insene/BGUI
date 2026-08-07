#pragma once
#include <collections/function.h>
#include <collections/string.h>
#include "BGUI/Core/Widget.hpp"


struct Font;

namespace BGUI
{

struct Button : Widget
{
    enum class State
    {
        Normal,
        Hovered,
        Pressed,
    };

    struct InternalData
    {
        State current_state;
        String text;
    } data;

    Color background_color{255, 255, 255, 255};
    Font* font;
    f32 font_size = 16;
    Function<void(*)(Widget*)> on_click{};

    Button(Mem::Allocator& allocator, const Vector2& size);
    ~Button();

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) override;
    bool event(const Event& event) override;

    StringView get_text() const { return data.text.view(); }
    void set_text(StringView view);
};

}