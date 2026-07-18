

bread_package(
    NAME
        "BGUI"
    ASSETS
        "assets"
    INCLUDES
        "."
    SOURCES
        "BGUI/Core/UIElement.cpp"
)

target_link_libraries(GUI PUBLIC Basic)
