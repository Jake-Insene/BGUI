#include "BGUI/UIPage.hpp"

#include <engine/engine.h>
#include <input/input.h>
#include <math/projection.h>
#include <resource/texture.h>
#include <resource/font.h>

#include "BGUI/Core/UIElementBatch.hpp"


namespace BGUI
{

UIPage::UIPage(Mem::Allocator* allocator) :
data{
    .allocator = allocator,
}
{
}

UIPage::~UIPage()
{
}

void UIPage::batch(Widget* widget, UIElementBatch& batcher)
{
    if (!widget)
    {
        return;
    }

    (void)widget->measure();
    widget->layout(Vector2());

    f32 width = widget->get_global_rect().size.width;
    f32 height = widget->get_global_rect().size.height;
    
    Mat4 ortho_projection = Projection::orthographic(0.0f, width, 0.0f, height, -1.0f, 1.0f);

    batcher.begin(ortho_projection);
    widget->draw(batcher);
    batcher.end();
}

}