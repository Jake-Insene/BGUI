#pragma once
#include <math/color.h>
#include <math/vec2.h>
#include <math/rect_2d.h>
#include <resource/font.h>


namespace BGUI
{

struct UITheme
{
    Font* default_font;
    f32 font_size;

    Color text;

    Color button;
    Color button_hover;
    Color button_pressed;
};

}