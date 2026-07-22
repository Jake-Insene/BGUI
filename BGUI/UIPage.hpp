#pragma once
#include <collections/string_map.h>

#include <Basic/Core/RenderCore.hpp>
#include <Basic/Core/RenderGraph.hpp>

#include "BGUI/Core/Widget.hpp"
#include "BGUI/Core/UIElementBatch.hpp"


struct Texture2D;

namespace BGUI
{

struct UIPage
{
    DisableCopy(UIPage);
    DisableMove(UIPage);

    static constexpr f32 DefaultSpacing = 5.F;

    struct InternalData
    {
        Mem::Allocator* allocator;
        UIElementBatch batcher;
    } data;

    UIPage(Mem::Allocator* allocator, GPU::TextureFormat render_attachment_format);
    ~UIPage();

    void batch(Widget* widget);

    const UIElementBatch& get_batcher() const { return data.batcher; }
};

}