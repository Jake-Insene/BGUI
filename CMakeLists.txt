

bread_package(
    NAME
        "BGUI"
    ASSETS
        "assets"
    INCLUDES
        "."
    SOURCES
        "BGUI/Core/UICore.cpp"
        "BGUI/Core/UIElementBatch.cpp"
        "BGUI/Core/Widget.cpp"
        
        "BGUI/Widgets/Button.cpp"
        "BGUI/Widgets/Label.cpp"
        "BGUI/Widgets/Panel.cpp"
        "BGUI/Widgets/TextureView.cpp"
        "BGUI/Widgets/Viewport.cpp"

        "BGUI/UIPage.cpp"
)

target_link_libraries(BGUI PUBLIC Basic)
