#pragma once
#include <Collections/Array.hpp>
#include <math/color.h>
#include <math/vec2.h>
#include <math/mat4.h>
#include <math/rect_2d.h>

#include <Basic/Core/RenderCore.hpp>
#include <Basic/Core/RenderDevice.hpp>
#include <Basic/Resource/Font.hpp>


namespace BGUI
{

enum class ElementFilter
{
    Linear,
    Nearest,

    MaxCount,
};

/*
* Batches quads per texture and filter.
* Currently supports: Triangle texture.
*/
struct UIElementBatch
{
    DisableCopy(UIElementBatch);
    DisableMove(UIElementBatch);

    struct alignas(16) Vertex
    {
        Vector2 position;
        Vector2 uv;
        Color color;
    };

    struct BatchBlock
    {
        Mat4 projection;
    };

    struct Batch
    {
        GPU::PipelineID pipeline;
        GPU::DescriptorSetLayoutID set_layout;
        GPU::PipelineLayoutID pipeline_layout;
        BatchBlock block;

        usize vb_offset;
        u32 vertex_count;

        GPU::DescriptorTextureInfo texture;
    };

    enum class RecordingState
    {
        // The batcher has begin recording.
        Begin,
        // The batcher has end recording. [Default]
        End,
    };
    
    Mem::Allocator& allocator;
    GPU::TextureViewID white_texture;

    GPU::DescriptorSetLayoutID set_layout;

    GPU::PipelineLayoutID pipeline_layout;
    GPU::PipelineID pipeline;

    Collections::Array<Vertex> vertices;
    Collections::Array<Batch> batches;
    GPU::TextureViewID current_texture_view;
    ElementFilter current_filter;

    BatchBlock block;
    RecordingState state;

    GPU::SamplerID samplers[u32(ElementFilter::MaxCount)];
    
    UIElementBatch(Mem::Allocator& allocator, Basic::RenderDevice& render_device,
        GPU::TextureViewID white_texture, GPU::TextureFormat render_attachment_format);
    ~UIElementBatch();

    GPU::TextureViewID get_white_texture() { return white_texture; }

    void begin(Mat4 projection);
    void end();

    // Draws a textured triangle using the given vertexs, uv must to be in normalized form.
    void draw_triangle_vertex(const Vertex& v1, const Vertex& v2, const Vertex& v3,
        GPU::TextureViewID texture_view, ElementFilter filter);
        
    // Draws a textured quads using the given vertexs, uv must to be a local rect to the texture.
    void draw_texture_gpu(const Rect2D& rect, const Rect2D& uv_rect, const Color& color,
        GPU::TextureViewID texture_view, const Vector2& texture_size, ElementFilter filter);

    void draw_text(Collections::StringView label, Font* font, f32 font_size, const Vector2& center,
        const Color& color);

    Slice<const Batch> get_batches() const;
    Slice<const Vertex> get_vertices() const;

    void _bind_to_batch(GPU::TextureViewID texture_view, ElementFilter filter);
};

}

