#pragma once
#include <collections/array.h>
#include <math/color.h>
#include <math/vec2.h>
#include <math/mat4.h>
#include <math/rect_2d.h>
#include <resource/texture.h>
#include <resource/font.h>

#include <Basic/Core/RenderCore.hpp>


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

    GPU::DescriptorSetLayoutID set_layout;

    GPU::PipelineLayoutID pipeline_layout;
    GPU::PipelineID pipeline;

    Array<Vertex> vertices;
    Array<Batch> batches;
    GPU::TextureViewID current_texture_view;
    ElementFilter current_filter;

    BatchBlock block;
    RecordingState state;

    GPU::SamplerID samplers[u32(ElementFilter::MaxCount)];
    
    UIElementBatch(Mem::Allocator& allocator, GPU::TextureFormat render_attachment_format);
    ~UIElementBatch();

    void begin(Mat4 projection);
    void end();

    // Draws a textured triangle using the given vertexs, uv must to be in normalized form.
    void draw_triangle_vertex(const Vertex& v1, const Vertex& v2, const Vertex& v3,
        GPU::TextureViewID texture_view, ElementFilter filter);
        
    // Draws a textured quads using the given vertexs, uv must to be a local rect to the texture.
    void draw_texture_gpu(const Rect2D& rect, const Rect2D& uv_rect, const Color& color,
        GPU::TextureViewID texture_view, const Vector2& texture_size, ElementFilter filter);

    // Draws a textured quads using the given vertexs, uv must to be a local rect to the texture.
    // If texture is nullptr, it will use the default white texture provided by bread.
    void draw_texture(const Rect2D& rect, const Rect2D& uv_rect, const Color& color,
        Texture2D* texture, ElementFilter filter);

    void draw_text(StringView label, Font* font, f32 font_size, const Vector2& center);

    Slice<const Batch> get_batches() const;
    Slice<const Vertex> get_vertices() const;

    void _bind_to_batch(GPU::TextureViewID texture_view, ElementFilter filter);
};

}

