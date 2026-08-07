#pragma once
#include <gpu/gpu.h>

#include "BGUI/Core/Widget.hpp"


namespace BGUI
{

struct TextureView : Widget
{
    struct InternalData
    {
        GPU::TextureViewID gpu_texture_view;
    } data;

    TextureView(Mem::Allocator& allocator, const Vector2& size);
    ~TextureView();

    Rect2D measure() override;
    void layout(const Vector2& absolute) override;
    void draw(UIElementBatch& batcher, const Basic::FrameInfo& frame_info) override;
    bool event(const Event& event) override;

    void set_gpu_texture_view(GPU::TextureViewID new_texture_view);
    GPU::TextureViewID get_gpu_texture_view() const { return data.gpu_texture_view; }
};

}