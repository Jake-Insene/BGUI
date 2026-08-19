#pragma once
#include <Collections/StringMap.hpp>

#include <Basic/Core/RenderCore.hpp>
#include <Basic/Core/RenderGraph.hpp>

#include "BGUI/Core/Widget.hpp"



namespace BGUI
{

struct UIPage
{
    DisableCopy(UIPage);
    DisableMove(UIPage);

    static constexpr f32 DefaultSpacing = 5.F;

    struct InternalData
    {
        Mem::Allocator& allocator;
    } data;

    UIPage(Mem::Allocator& allocator);
    ~UIPage();

    void batch(Widget* widget, UIElementBatch& batcher, const Basic::FrameInfo& frame_info);
};

}