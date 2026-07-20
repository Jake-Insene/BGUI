#include "BGUI/UIPage.hpp"

#include <engine/engine.h>
#include <input/input.h>
#include <math/projection.h>
#include <resource/texture.h>
#include <resource/font.h>


namespace BGUI
{

// Internal
Vector2 _calculate_text_size(Font* font, i32 font_size, StringView label)
{
    Vector2 max_size = Vector2();

    const Font::FontTheme& theme = font->get_font_theme(font_size);
    for(usize i = 0; i < label.len; i++)
    {
        Font::Glyph& glyph = theme.glyphs.get(label[i]);
        max_size.width += glyph.advance.width;
        max_size.height = Math::max(max_size.height, glyph.advance.height);
    }

    return max_size;
}

void _advance_layout(UIPage::Context& context, f32 widget_height, f32 spacing)
{
    context.current_cursor.y -= (widget_height + spacing);
}

UIPage::UIPage(Mem::Allocator* allocator, GPU::TextureFormat render_attachment_format) :
data{
    .allocator = allocator,
    .batcher = UIElementBatch(allocator, render_attachment_format),
    .white_texture = Resource::load<Texture2D>("default:white_texture"),
    .context = {},
    .theme =
    {
        .default_font = nullptr,
        .font_size = 32,
        .text = Color(255, 255, 255, 255),
        .button = Color(180, 180, 180, 255),
        .button_hover = Color(100, 100, 100, 255),
        .button_pressed = Color(255, 255, 255, 255),
    },
}
{
}

UIPage::~UIPage()
{
}

void UIPage::begin(const Vector2& begin_position)
{
    data.context.current_cursor = begin_position;

    data.batcher.begin(Projection::orthographic(0, 1280, 0, 720, 0, 1));

    data.context.hot = 0;
}

void UIPage::end()
{
    data.batcher.end();
}

bool UIPage::button(StringView label, const Vector2& req_size)
{
    WidgetID id = HashOfType<StringView>::hashfunc(label);

    Vector2 text_size = _calculate_text_size(data.theme.default_font, data.theme.font_size, label);
    Vector2 size = req_size;

    const f32 padding_x = 16.0f;
    const f32 padding_y = 8.0f;

    if(req_size.x == 0)
    {
        size.x = text_size.x + padding_x * 2.0f;
    }

    if(req_size.y == 0)
    {
        size.y = text_size.y + padding_y * 2.0f;
    }

    Rect2D button_rect = Rect2D();
    button_rect.position.x = data.context.current_cursor.x;
    button_rect.position.y = data.context.current_cursor.y - size.y; 
    button_rect.size.width = size.x;
    button_rect.size.height = size.y;

    bool hovered = button_rect.contains(Input::get_mouse_position());

    if(hovered)
    {
        data.context.hot = id;
    }

    bool left_pressed = Input::is_mouse_button_pressed(MouseButton::Left);
    bool left_just_pressed = Input::is_mouse_button_just_pressed(MouseButton::Left);

    if(hovered && left_just_pressed)
    {
        data.context.active = id;
    }

    bool clicked = false;
    if(!left_pressed)
    {
        if(data.context.active == id && hovered)
        {
            clicked = true;
        }

        if(data.context.active == id)
        {
            data.context.active = 0;
        }
    }

    // state
    ButtonState state = ButtonState::Normal;
    if(data.context.hot == id)
    {
        state = ButtonState::Hover;
    }
    if (data.context.active == id)
    {
        state = ButtonState::Pressed;
    }

    Color button_color = data.theme.button;
    if(state == ButtonState::Hover)
    {
        button_color = data.theme.button_hover;
    }
    if(state == ButtonState::Pressed)
    {
        button_color = data.theme.button_pressed;
    }

    draw_button(
        button_rect, Rect2D(Vector2(), Vector2(data.white_texture->get_size())),
        button_color, data.white_texture
    );

    draw_text(label, data.theme.default_font, data.theme.font_size, button_rect.center());

    _advance_layout(data.context, size.y, 5.F);

    return clicked;
}

void UIPage::set_font(Font* new_font)
{
    data.theme.default_font = new_font;
}

void UIPage::draw_button(const Rect2D& rect, const Rect2D& src_rect, const Color& color, Texture2D* texture)
{
    data.batcher.draw_texture(rect, src_rect, color,
        texture,
        ElementFilter::Nearest
    );
}

void UIPage::draw_text(StringView label, Font* font, f32 font_size, const Vector2& center)
{
    Vector2 text_size = _calculate_text_size(font, font_size, label);

    Rect2D text_rect = Rect2D(Vector2(), text_size);
    text_rect.set_center(center);

    const Font::FontTheme& theme = font->get_font_theme(font_size);

    f32 width_accum = 0;
    for(usize i = 0; i < label.len; i++)
    {
        Font::Glyph& glyph = theme.glyphs.get(label[i]);

        data.batcher.draw_texture_gpu(
            Rect2D(text_rect.position + Vector2(width_accum, 0), glyph.advance),
            glyph.src_rect,
            Color(255, 255, 255, 255),
            Engine::get_render_device()->get_gpu_resource_manager()->texture_get_texture_view(theme.font_atlas),
            theme.atlas_size,
            ElementFilter::Nearest
        );

        width_accum += glyph.advance.width;
    }
}

void UIPage::flush_pass(Basic::FrameContext& context, Basic::RenderGraph& graph)
{
    Slice batches = data.batcher.get_batches();
    Slice primitives = data.batcher.get_vertices();

    Basic::TransientAllocation transient = context.allocate_transient_vertex(
        primitives.len * sizeof(UIElementBatch::Vertex));
    Mem::copy(transient.mapped, Mem::to_const_bytes(primitives));

    graph.add_raster_pass([batches, transient](Basic::PassResources& resources)
    {
        for(UIElementBatch::Batch& batch : batches)
        {
            GPU::DescriptorSetID set = resources.context.allocate_descriptor_set(batch.set_layout);
            
            const GPU::WriteDescriptorInfo write_infos[] =
            {
                GPU::WriteDescriptorInfo::combined_texture_sampler(set, 0, 0, Slice(&batch.texture, 1)),
            };

            GPU::descriptor_set_update_descriptors(resources.context.data.render_device->get_device(),
                { .write_infos = write_infos });

            GPU::command_buffer_bind_pipeline(resources.command_buffer,
                GPU::PipelineBindPoint::Graphics, batch.pipeline);

            GPU::command_buffer_bind_descriptor_sets(resources.command_buffer,
                GPU::PipelineBindPoint::Graphics, batch.pipeline_layout, 0, Slice(&set, 1));

            GPU::command_buffer_constant_block(resources.command_buffer,
                batch.pipeline_layout, GPU::ShaderStage::Vertex, 0, sizeof(UIElementBatch::BatchBlock),
                reinterpret_cast<MemoryAddress>(&batch.block));

            GPU::command_buffer_bind_vertex_buffers(resources.command_buffer,
                0, Slice(&resources.global_device_vertex_buffer, 1),
                Slice(&transient.offset, 1));

            GPU::command_buffer_draw(resources.command_buffer,
                batch.vertex_count, 1, batch.vb_offset, 0);
        }
    });
}

}