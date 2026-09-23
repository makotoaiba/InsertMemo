cmake_minimum_required(VERSION 3.22)
project(InsertMemo VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static MSVC runtime: testers need no VC++ redistributable
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Use ./JUCE if present, otherwise download the release zip (no Git needed)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.6.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(InsertMemo
    COMPANY_NAME                "Takasono"
    PLUGIN_MANUFACTURER_CODE    Tkso
    PLUGIN_CODE                 Imem
    FORMATS                     VST3
    PRODUCT_NAME                "Insert Memo"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES             Fx Tools
    COPY_PLUGIN_AFTER_BUILD     FALSE)

target_sources(InsertMemo PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/LinkUtils.cpp)

target_compile_definitions(InsertMemo PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

# u8"..." literals contain Japanese: make MSVC read sources as UTF-8
target_compile_options(InsertMemo PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(InsertMemo
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
