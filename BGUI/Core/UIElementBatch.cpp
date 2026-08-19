#include "BGUI/Core/UIElementBatch.hpp"

#include <Basic/Core/Shader.hpp>


namespace BGUI
{

// Internal
Vector2 _calculate_text_size(Basic::Font* font, i32 font_size, Collections::StringView label)
{
    Vector2 max_size = Vector2();

    const Basic::Font::FontTheme& theme = font->get_font_theme(font_size);
    for(usize i = 0; i < label.len; i++)
    {
        Basic::Font::Glyph& glyph = theme.glyphs.get(label[i]);
        max_size.width += glyph.advance.width;
        max_size.height = Math::max(max_size.height, glyph.advance.height);
    }

    return max_size;
}

UIElementBatch::UIElementBatch(Mem::Allocator& allocator, Basic::RenderDevice& render_device,
    GPU::TextureViewID white_texture, GPU::TextureFormat render_attachment_format)
: allocator(allocator), white_texture(white_texture), vertices(allocator, 4, {}), batches(allocator, 4, {}),
current_texture_view(GPU::TextureViewID::invalid()), current_filter(ElementFilter::MaxCount),
state(RecordingState::End)
{
    const GPU::ConstantBlock blocks[] =
    {
        GPU::ConstantBlock::create(GPU::ShaderStage::Vertex, 0, sizeof(BatchBlock)),
    };

    const GPU::DescriptorBinding set_bindings[] =
    {
        GPU::DescriptorBinding::combined_texture_sampler(0, 1, GPU::ShaderStage::Fragment),
    };

    set_layout = GPU::descriptor_set_layout_create(render_device.get_device(),
        GPU::DescriptorSetLayoutCreateInfo::create(set_bindings));

    Basic::Shader sprite_shader{
        allocator,
        {
            .path = "shaders/packages/2D/SpriteBatch.slang.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    };

    const GPU::ColorBlendAttachmentState color_blend_attachments[] =
    {
        GPU::ColorBlendAttachmentState::create(
            true, GPU::BlendFactor::SrcAlpha, GPU::BlendFactor::OneMinusSrcAlpha, GPU::BlendOp::Add,
            GPU::BlendFactor::One, GPU::BlendFactor::OneMinusSrcAlpha, GPU::BlendOp::Add,
            GPU::ColorComponentFlags(0xFF)
        ),
    };

    { // Sprite
        const GPU::VertexBinding vertex_bindings[] =
        {
            GPU::VertexBinding::create(0, sizeof(Vertex), GPU::InputRate::Vertex),
        };

        const GPU::VertexAttribute vertex_attributes[] =
        {
            GPU::VertexAttribute::create(0, 0, GPU::VertexFormat::RGBA32Float, 0),
            GPU::VertexAttribute::create(1, 0, GPU::VertexFormat::RGBA32Float, sizeof(Vector4)),
        };

        const GPU::DescriptorSetLayoutID pipeline_set_layouts[] =
        {
            set_layout
        };

        pipeline_layout = GPU::pipeline_layout_create(render_device.get_device(),
            GPU::PipelineLayoutCreateInfo::create(blocks, pipeline_set_layouts)
        );

        const GPU::TextureFormat pipeline_render_attachments[] =
        {
            render_attachment_format,
        };

        pipeline = GPU::pipeline_create(render_device.get_device(),
            GPU::PipelineCreateInfo::create(
                GPU::PipelineBindPoint::Graphics,
                sprite_shader.get_stages(),
                GPU::VertexInput::create(vertex_bindings, vertex_attributes),
                GPU::InputAssembly::create(GPU::PrimitiveTopology::TriangleList),
                GPU::RasterizerState::state(GPU::PolygonMode::Fill, GPU::CullMode::Front, GPU::FrontFace::ClockWise),
                GPU::MultisampleState::disable(),
                GPU::DepthStencilState::depth_stencil_disable(),
                GPU::ColorBlendState::create(false, GPU::LogicOp::Copy, color_blend_attachments, Vector4()),
                pipeline_layout, GPU::RenderingInfo::render_attachments(pipeline_render_attachments)
            )
        );
    }

    static constexpr GPU::Filter gpu_filters[] =
    {
        GPU::Filter::Linear,
        GPU::Filter::Nearest,
    };

    static constexpr GPU::SamplerMipMapMode gpu_mimap_modes[] =
    {
        GPU::SamplerMipMapMode::Linear,
        GPU::SamplerMipMapMode::Nearest,
    };

    for(usize i = 0; i < u32(ElementFilter::MaxCount); i++)
    {
        samplers[i] = GPU::sampler_create(render_device.get_device(),
            GPU::SamplerCreateInfo::create(gpu_filters[i], gpu_filters[i],
                gpu_mimap_modes[i], GPU::SamplerAddressMode::Repeat, GPU::SamplerAddressMode::Repeat,
                GPU::SamplerAddressMode::Repeat, 0.F, false, 1.F, false, GPU::CompareOp::Always,
                0.F, 0.F));
    }
}

UIElementBatch::~UIElementBatch()
{
    for(usize i = 0; i < u32(ElementFilter::MaxCount); i++)
    {
        GPU::sampler_destroy(samplers[i]);
    }

    GPU::pipeline_destroy(pipeline);
    GPU::pipeline_layout_destroy(pipeline_layout);
    GPU::descriptor_set_layout_destroy(set_layout);
}

void UIElementBatch::begin(Mat4 projection)
{
    DebugAssert(state == RecordingState::End, "batcher is still open");

    vertices.clear();
    batches.clear();
    current_texture_view = GPU::TextureViewID::invalid();
    current_filter = ElementFilter::MaxCount;
    state = RecordingState::Begin;

    block =
    {
        .projection = projection,
    };
}

void UIElementBatch::end()
{
    DebugAssert(state == RecordingState::Begin, "batcher is already end");
    state = RecordingState::End;
}

void UIElementBatch::draw_triangle_vertex(const Vertex& v1, const Vertex& v2, const Vertex& v3,
    GPU::TextureViewID texture_view, ElementFilter filter)
{
    DebugAssert(state == RecordingState::Begin, "batcher is not open");
    _bind_to_batch(texture_view, filter);

    (void)vertices.add(v1);
    (void)vertices.add(v2);
    (void)vertices.add(v3);
    batches.last().vertex_count += 3;
}

void UIElementBatch::draw_texture_gpu(const Rect2D& rect, const Rect2D& uv_rect, const Color& color,
    GPU::TextureViewID texture_view, const Vector2& texture_size, ElementFilter filter)
{
    Rect2D normalized_uv = Rect2D(
        uv_rect.position / texture_size,
        uv_rect.size / texture_size
    );

    draw_triangle_vertex(
        {.position = rect.position, .uv = normalized_uv.position + Vector2(0, normalized_uv.size.y), .color = color, },
        {.position = rect.position + rect.size, .uv = normalized_uv.position + Vector2(normalized_uv.size.x, 0), .color = color, },
        {.position = rect.position + Vector2(rect.size.x, 0), .uv = normalized_uv.position + normalized_uv.size, .color = color, },
        texture_view, filter
    );

    draw_triangle_vertex(
        {.position = rect.position, .uv = normalized_uv.position + Vector2(0, normalized_uv.size.y), .color = color, },
        {.position = rect.position + Vector2(0, rect.size.y), .uv = normalized_uv.position, .color = color, },
        {.position = rect.position + rect.size, .uv = normalized_uv.position + Vector2(normalized_uv.size.x, 0), .color = color, },
        texture_view, filter
    );
}

void UIElementBatch::draw_text(Collections::StringView label, Basic::Font* font, f32 font_size, const Vector2& center,
    const Color& color)
{
    DebugAssert(font != nullptr, "invalid font");
    DebugAssert(font_size != 0, "invalid font size");

    Vector2 text_size = _calculate_text_size(font, font_size, label);

    Rect2D text_rect = Rect2D(Vector2(), text_size);
    text_rect.set_center(center);

    const Basic::Font::FontTheme& theme = font->get_font_theme(font_size);

    f32 width_accum = 0;
    for(usize i = 0; i < label.len; i++)
    {
        Basic::Font::Glyph& glyph = theme.glyphs.get(label[i]);

        if(label[i] != ' ')
        {
            draw_texture_gpu(
                Rect2D(text_rect.position + Vector2(width_accum, 0), glyph.advance),
                glyph.src_rect,
                color,
                theme.font_atlas_view,
                theme.atlas_size,
                ElementFilter::Nearest
            );
        }
            
        width_accum += glyph.advance.width;
    }
}

Slice<const UIElementBatch::Batch> UIElementBatch::get_batches() const
{
    return batches.slice().as_const();
}

Slice<const UIElementBatch::Vertex> UIElementBatch::get_vertices() const
{
    return vertices.slice().as_const();
}

void UIElementBatch::_bind_to_batch(GPU::TextureViewID texture_view, ElementFilter filter)
{
    DebugAssert(texture_view != GPU::TextureViewID::invalid(), "invalid texture view");
    DebugAssert(filter != ElementFilter::MaxCount, "invalid filter");

    if(current_texture_view == texture_view && current_filter == filter)
    {
        return;
    }

    Batch new_batch =
    {
        .pipeline = pipeline,
        .set_layout = set_layout,
        .pipeline_layout = pipeline_layout,
        .block = block,
        .vb_offset = vertices.count,
        .vertex_count = 0,
        .texture =
        {
            .texture_view = texture_view,
            .layout = GPU::TextureLayout::ShaderReadOnly,
            .sampler = samplers[u32(filter)],
        },
    };

    current_texture_view = texture_view;
    current_filter = filter;

    (void)batches.add(new_batch);
}

}