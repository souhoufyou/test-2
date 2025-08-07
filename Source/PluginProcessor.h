
#pragma once
#include <JuceHeader.h>
#include "Analyzer/FeatureExtractor.h"
#include "Analyzer/AutoSuggest.h"
#include "Processors/Denoiser.h"
#include "Processors/DeEsser.h"
#include "Processors/HPF.h"
#include "Processors/AutoGain.h"
#include "Processors/OneKnobSaturator.h"

class AIVocalAssistantEZAudioProcessor : public juce::AudioProcessor
{
public:
    AIVocalAssistantEZAudioProcessor();
    ~AIVocalAssistantEZAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::AudioProcessorValueTreeState apvts;

    juce::dsp::ProcessorChain<
        Denoiser,
        HPF,
        DeEsser,
        juce::dsp::Compressor<float>,
        OneKnobSaturator,
        juce::dsp::Limiter<float>
    > chain;

    AutoGain autoGain;
    FeatureExtractor featureExtractor;
    AutoSuggest autoSuggest;

    bool firstAnalyzeDone = false;
    double sr = 44100.0;

    void updateChain();
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
};
