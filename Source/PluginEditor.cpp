
#include "PluginEditor.h"

AIVocalAssistantEZAudioProcessorEditor::AIVocalAssistantEZAudioProcessorEditor (AIVocalAssistantEZAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (360, 120);
    addAndMakeVisible (bypass);
}

void AIVocalAssistantEZAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("AI Vocal Assistant EZ", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
    g.setFont (14.0f);
    g.setColour (juce::Colours::grey);
    g.drawFittedText ("Auto denoise + vocal polish (no setup)", getLocalBounds().withTrimmedTop(45), juce::Justification::centred, 1);
}

void AIVocalAssistantEZAudioProcessorEditor::resized()
{
    bypass.setBounds(getLocalBounds().reduced(16).removeFromBottom(40));
}
