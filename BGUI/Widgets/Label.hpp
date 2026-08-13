#pragma once
#include <Collections/Function.hpp>
#include <Collections/String.hpp>
#include "BGUI/Core/Widget.hpp"


struct Font;

namespace BGUI
{

struct Label : Widget
{
    struct InternalData
    {
        String text;
    } data;

    Color background_color{255, 255, 255, 255};
    Font* font = nullptr;
    f32 font_size = 16;
    Function<void(*)(Widget*)> on_click{};

    Label(Mem::Allocator& allocator, const Vector2& size);
    ~Label();

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) override;
    bool event(const Event& event) override;

    String& get_text() { return data.text; }
};

}