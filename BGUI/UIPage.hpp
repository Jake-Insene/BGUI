#pragma once
#include <collections/string_map.h>

#include <Basic/Core/RenderCore.hpp>
#include <Basic/Core/RenderGraph.hpp>

#include "BGUI/Core/UICore.hpp"
#include "BGUI/Core/UIElementBatch.hpp"


struct Texture2D;

namespace BGUI
{

struct UIPage
{
    DisableCopy(UIPage);
    DisableMove(UIPage);

    static constexpr f32 DefaultSpacing = 5.F;

    using WidgetID = u64;

    enum class ButtonState
    {
        Normal,
        Hover,
        Pressed,
    };

    struct Context
    {
        WidgetID hot;
        WidgetID active;
        WidgetID focused;

        Vector2 current_cursor;
        f32 last_widget_height;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        UIElementBatch batcher;
        Texture2D* white_texture;

        Context context;
        UITheme theme;
    } data;

    UIPage(Mem::Allocator* allocator, GPU::TextureFormat render_attachment_format);
    ~UIPage();

    void begin(const Vector2& begin_position);
    void end();
    
    bool button(StringView label, const Vector2& req_size);

    void set_font(Font* new_font);

    void draw_button(const Rect2D& rect, const Rect2D& src_rect, const Color& color, Texture2D* texture);
    void draw_text(StringView label, Font* font, f32 font_size, const Vector2& center);

    void flush_pass(Basic::FrameContext& context, Basic::RenderGraph& graph);
};

}