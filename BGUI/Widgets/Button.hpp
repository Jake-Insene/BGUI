#pragma once
#include <Collections/Function.hpp>
#include <Collections/String.hpp>
#include "BGUI/Core/Widget.hpp"


namespace Basic
{
struct Font;
}

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
        Collections::String text;
    } data;

    Color background_color{255, 255, 255, 255};
    Basic::Font* font = nullptr;
    f32 font_size = 16;
    Collections::Function<void(*)(Widget*)> on_click{};

    Button(Mem::Allocator& allocator, const Vector2& size);
    ~Button();

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) override;
    bool event(const Event& event) override;

    Collections::StringView get_text() const { return data.text.view(); }
    void set_text(Collections::StringView view);
};

}