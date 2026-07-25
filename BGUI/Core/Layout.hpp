#pragma once


namespace BGUI
{

enum class LayoutDirection
{
    Vertical,
    Horizontal,
};

struct Layout
{
    LayoutDirection direction;

    Layout(LayoutDirection _direction) : direction(_direction) {}
};

}