#pragma once


namespace BGUI
{

enum class LayoutDirection
{
    Vertical,
    Horizontal,
};

enum class Alignment
{
    Start,
    Center,
    End,
};

struct Layout
{
    LayoutDirection direction;
    Alignment alignment;

    Layout(LayoutDirection _direction, Alignment _alignment)
    : direction(_direction), alignment(_alignment) {}
};

}