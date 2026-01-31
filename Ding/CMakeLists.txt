set(TargetName ${PROJECT_NAME})

juce_add_plugin("${TargetName}"
        # VERSION ...
        # ICON_BIG ...
        # ICON_SMALL ...
        COMPANY_NAME "Poss"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Poss
        PLUGIN_CODE Ding
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "${TargetName}"
)
juce_generate_juce_header(${TargetName})

target_sources(${TargetName} PRIVATE
        Processor.cpp
        Processor.hpp

        Synth/Voice.cpp
        Synth/Voice.hpp

        Gui/Editor.cpp
        Gui/Editor.hpp
)

target_include_directories(${TargetName} PUBLIC
        .
)

target_compile_definitions(${TargetName}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${TargetName} PRIVATE
        juce_recommended_config_flags
        juce_recommended_lto_flags
        juce_recommended_warning_flags
        juce_audio_utils
)
