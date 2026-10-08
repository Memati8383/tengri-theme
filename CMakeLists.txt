cmake_minimum_required(VERSION 3.16)
project(TENGRI_THEME LANGUAGES CXX)

# Bunu belirtmeden CMP0091 3.16 tabanında sessizce yok sayılır ve derleme dinamik
# CRT'ye düşer; build.bat'in kullandığı /MT ile ayrışır.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
if(NOT EXISTS ${IMGUI_DIR}/imgui.h)
    message(FATAL_ERROR "Arayuz kutuphanesi bulunamadi: third_party/imgui")
endif()

# Stil FontSizeBase / FontScaleDpi alanlarını kullanıyor; 1.92 veya üzeri gerekir.
if(NOT EXISTS ${IMGUI_DIR}/imgui_internal.h)
    message(FATAL_ERROR "third_party/imgui eksik: imgui_internal.h bulunamadi")
endif()

# ---- cekirdek ----------------------------------------------------------------
# Pencere + Direct3D + DPI + tema + uygulama kabuğu. main.cpp içeriği link
# adımında gelen modülden alır (app::content()), böylece aynı çekirdek iki
# yürütülebilir üretir: şablon ve vitrin.
add_library(tengri_core OBJECT
    src/main.cpp
    src/app/shell.cpp
    src/gui/theme.cpp
    src/gui/fx.cpp
    src/gui/widgets.cpp
    src/gui/icons.cpp
    src/gui/brand_icons.cpp
    src/gui/logo.cpp
    src/gui/logo_data.cpp
    src/gui/font_data.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(tengri_core PUBLIC src ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_compile_definitions(tengri_core PUBLIC UNICODE _UNICODE IMGUI_DEFINE_MATH_OPERATORS)
target_compile_options(tengri_core PRIVATE /utf-8 /W4)

# ---- senin uygulaman ---------------------------------------------------------
# TengriApp.exe: düzenleyeceğin tek dosya src\app\my_app.cpp
add_executable(app_scaffold WIN32 src/app/my_app.cpp)

# ---- tema vitrini ------------------------------------------------------------
# TengriThemePreview.exe: API'nin tamamını gösteren referans (src\demo.cpp)
add_executable(theme_preview WIN32 src/demo.cpp)

foreach(tgt app_scaffold theme_preview)
    target_link_libraries(${tgt} PRIVATE tengri_core)
    # windowscodecs + ole32: gömülü marka PNG'sini WIC ile çözer (gui\logo.cpp)
    target_link_libraries(${tgt} PRIVATE d3d11 dxgi d3dcompiler dwmapi user32 gdi32 shell32 windowscodecs ole32)
endforeach()

set_target_properties(app_scaffold  PROPERTIES OUTPUT_NAME "TengriApp")
set_target_properties(theme_preview PROPERTIES OUTPUT_NAME "TengriThemePreview")

# Kenar şeridi hesabının tablo testi (tests\run_tests.bat ile aynı şey).
enable_testing()
add_executable(hit_test tests/hit_test.cpp)
target_include_directories(hit_test PRIVATE src)
target_compile_options(hit_test PRIVATE /utf-8 /W4)
add_test(NAME window_hit_edge COMMAND hit_test)
