
#pragma once
#include <JuceHeader.h>
class AutoGain
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec){ fs=spec.sampleRate; smooth.setCurrentAndTargetValue(1.0f); smooth.reset(fs, 0.05); }
    void setTargetRms(float db){ target=db; }
    template<typename Block>
    void process (Block& mono){
        const size_t N = mono.getNumSamples(); auto* d = mono.getChannelPointer(0);
        double s=0.0; for (size_t i=0;i<N;++i) s += d[i]*d[i];
        float rmsDb = juce::Decibels::gainToDecibels(std::sqrt(s/(double)juce::jmax((size_t)1,N)) + 1e-9, -100.0f);
        float needed = juce::jlimit(-12.0f, 12.0f, target - rmsDb);
        smooth.setTargetValue(juce::Decibels::decibelsToGain(needed));
        for (size_t i=0;i<N;++i) d[i] *= smooth.getNextValue();
    }
private:
    double fs=44100.0; float target=-18.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smooth;
};
