add_library(mfc42 INTERFACE)
target_link_libraries(mfc42 INTERFACE mfc42)

if(MSVC)
    # target_link_libraries(mfc42 INTERFACE legacy_stdio_definitions)
endif()

target_compile_definitions(mfc42 INTERFACE -DBUILD_WITH_D3D8)

if(MSVC AND ${CMAKE_VERSION} VERSION_GREATER "3.13.0")   
    target_link_directories(mfc42 BEFORE INTERFACE .)
    target_include_directories(mfc42 INTERFACE include)
    # target_link_options(mfc42 INTERFACE /SAFESEH:NO /NODEFAULTLIB:libci.lib)
endif()