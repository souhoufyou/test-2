
#pragma once
#include <JuceHeader.h>
class HPF
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec){ fs=spec.sampleRate; update(); iir.prepare(spec); }
    void setCutoff(float hz, float sampleRate){ fs=sampleRate; cutoff=hz; update(); }
    template<typename Ctx> void process(const Ctx& ctx){ iir.process(ctx); }
    void reset(){ iir.reset(); }
private:
    void update(){ *iir.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(fs, cutoff); }
    double fs=44100.0; float cutoff=90.0f; juce::dsp::IIR::Filter<float> iir;
};
