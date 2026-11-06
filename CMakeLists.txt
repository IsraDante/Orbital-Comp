cmake_minimum_required(VERSION 3.22)
project(OrbitalComp VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(OrbitalComp
  COMPANY_NAME "Dante Ramos"
  PLUGIN_MANUFACTURER_CODE Dram
  PLUGIN_CODE Orbc
  FORMATS VST3 AU Standalone
  PRODUCT_NAME "Orbital Comp"
  IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
  VST3_CATEGORIES Fx Dynamics
  COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(OrbitalComp)
target_sources(OrbitalComp PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(OrbitalComp PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(OrbitalComp PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
