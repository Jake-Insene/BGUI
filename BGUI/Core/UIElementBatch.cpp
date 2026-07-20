#include "BGUI/Core/UIElementBatch.hpp"

#include <engine/engine.h>
#include <graphics/shader.h>


namespace BGUI
{

UIElementBatch::UIElementBatch(Mem::Allocator* allocator, GPU::TextureFormat render_attachment_format)
{
    data.allocator = allocator;

    const GPU::ConstantBlock blocks[] =
    {
        GPU::ConstantBlock::create(GPU::ShaderStage::Vertex, 0, sizeof(BatchBlock)),
    };

    const GPU::DescriptorBinding set_bindings[] =
    {
        GPU::DescriptorBinding::combined_texture_sampler(0, 1, GPU::ShaderStage::Fragment),
    };

    data.set_layout = GPU::descriptor_set_layout_create(Engine::get_render_device()->get_device(),
        GPU::DescriptorSetLayoutCreateInfo::create(set_bindings));

    Graphics::Shader sprite_shader = {};
    sprite_shader.init(allocator,
        {
            .file_path = "shaders/packages/2D/SpriteBatch.slang.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );

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

        data.pipeline_layout = GPU::pipeline_layout_create(Engine::get_render_device()->get_device(),
            GPU::PipelineLayoutCreateInfo::create(blocks, Slice(&data.set_layout, 1))
        );

        data.pipeline = GPU::pipeline_create(Engine::get_render_device()->get_device(),
            GPU::PipelineCreateInfo::create(
                GPU::PipelineBindPoint::Graphics,
                sprite_shader.get_stages(),
                GPU::VertexInput::create(vertex_bindings, vertex_attributes),
                GPU::InputAssembly::create(GPU::PrimitiveTopology::TriangleList),
                GPU::RasterizerState::state(GPU::PolygonMode::Fill, GPU::CullMode::Front, GPU::FrontFace::ClockWise),
                GPU::MultisampleState::disable(),
                GPU::DepthStencilState::depth_stencil_disable(),
                data.pipeline_layout, GPU::RenderingInfo::render_attachments(Slice(&render_attachment_format, 1))
            )
        );
    }

    sprite_shader.destroy();

    data.vertices = Array<Vertex>::with_size(allocator, 4);
    data.batches = Array<Batch>::with_size(allocator, 4);
    data.current_texture_view = GPU::TextureViewID::invalid();
    data.current_filter = ElementFilter::MaxCount;
    data.state = RecordingState::End;

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
        data.samplers[i] = GPU::sampler_create(Engine::get_render_device()->get_device(),
            GPU::SamplerCreateInfo::create(gpu_filters[0], gpu_filters[0],
                gpu_mimap_modes[i], GPU::SamplerAddressMode::Repeat, GPU::SamplerAddressMode::Repeat,
                GPU::SamplerAddressMode::Repeat, 0.F, false, 1.F, false, GPU::CompareOp::Always,
                0.F, 0.F));
    }
}

UIElementBatch::~UIElementBatch()
{
    for(usize i = 0; i < u32(ElementFilter::MaxCount); i++)
    {
        GPU::sampler_destroy(data.samplers[i]);
    }

    GPU::pipeline_destroy(data.pipeline);
    GPU::pipeline_layout_destroy(data.pipeline_layout);
    GPU::descriptor_set_layout_destroy(data.set_layout);

    data.vertices.destroy();
    data.batches.destroy();
}

void UIElementBatch::begin(Mat4 projection)
{
    DebugAssert(data.state == RecordingState::End, "batcher is still open");

    data.vertices.clear();
    data.batches.clear();
    data.current_texture_view = GPU::TextureViewID::invalid();
    data.current_filter = ElementFilter::MaxCount;
    data.state = RecordingState::Begin;

    data.block =
    {
        .projection = projection,
    };
}

void UIElementBatch::end()
{
    DebugAssert(data.state == RecordingState::Begin, "batcher is already end");
    data.state = RecordingState::End;
}

void UIElementBatch::draw_triangle_vertex(const Vertex& v1, const Vertex& v2, const Vertex& v3,
    GPU::TextureViewID texture_view, ElementFilter filter)
{
    DebugAssert(data.state == RecordingState::Begin, "batcher is not open");
    _bind_to_batch(texture_view, filter);

    (void)data.vertices.add(v1);
    (void)data.vertices.add(v2);
    (void)data.vertices.add(v3);
    data.batches.last().vertex_count += 3;
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

void UIElementBatch::draw_texture(const Rect2D& rect, const Rect2D& uv_rect, const Color& color,
    Texture2D* texture, ElementFilter filter)
{
    GPU::TextureViewID texture_view = Engine::get_render_device()->get_gpu_resource_manager()->texture_get_texture_view(texture->texture_ref);
    draw_texture_gpu(rect, uv_rect, color, texture_view, Vector2(texture->get_size()), filter);
}

Slice<UIElementBatch::Batch> UIElementBatch::get_batches()
{
    return data.batches.slice();
}

Slice<UIElementBatch::Vertex> UIElementBatch::get_vertices()
{
    return data.vertices.slice();
}

void UIElementBatch::_bind_to_batch(GPU::TextureViewID texture_view, ElementFilter filter)
{
    DebugAssert(texture_view != GPU::TextureViewID::invalid(), "invalid texture view");
    DebugAssert(filter != ElementFilter::MaxCount, "invalid filter");

    if(data.current_texture_view == texture_view && data.current_filter == filter)
    {
        return;
    }

    Batch new_batch =
    {
        .pipeline = data.pipeline,
        .set_layout = data.set_layout,
        .pipeline_layout = data.pipeline_layout,
        .block = data.block,
        .vb_offset = data.vertices.count,
        .vertex_count = 0,
        .texture =
        {
            .texture_view = texture_view,
            .layout = GPU::TextureLayout::ShaderReadOnly,
            .sampler = data.samplers[u32(filter)],
        },
    };

    data.current_texture_view = texture_view;
    data.current_filter = filter;

    (void)data.batches.add(new_batch);
}

}