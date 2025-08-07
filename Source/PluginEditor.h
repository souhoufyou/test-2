
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AIVocalAssistantEZAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AIVocalAssistantEZAudioProcessorEditor (AIVocalAssistantEZAudioProcessor&);
    ~AIVocalAssistantEZAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AIVocalAssistantEZAudioProcessor& processor;
    juce::ToggleButton bypass { "Bypass" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AIVocalAssistantEZAudioProcessorEditor)
};
