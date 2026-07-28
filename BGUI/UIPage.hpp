#pragma once
#include <collections/string_map.h>

#include <Basic/Core/RenderCore.hpp>
#include <Basic/Core/RenderGraph.hpp>

#include "BGUI/Core/Widget.hpp"


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
    } data;

    UIPage(Mem::Allocator* allocator);
    ~UIPage();

    void batch(Widget* widget, UIElementBatch& batcher);
};

}