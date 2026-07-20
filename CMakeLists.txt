

bread_package(
    NAME
        "BGUI"
    ASSETS
        "assets"
    INCLUDES
        "."
    SOURCES
        "BGUI/UIPage.cpp"
        
        "BGUI/Core/UICore.cpp"
        "BGUI/Core/UIElementBatch.cpp"
)

target_link_libraries(BGUI PUBLIC Basic)
