
#pragma once
#include <JuceHeader.h>
class DeEsser
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec){ fs=spec.sampleRate; setParameters(6500.0f, 12.0f, 4.0f); }
    void setParameters(float f, float th, float r){ centre=f; thresh=th; ratio=r; bp.state = juce::dsp::IIR::Coefficients<float>::makeBandPass(fs, centre, 1.0f); }
    template<typename Ctx>
    void process (const Ctx& ctx)
    {
        auto in = ctx.getInputBlock(); auto out = ctx.getOutputBlock();
        tmp.setSize(1, (int)in.getNumSamples(), false, false, true);
        juce::dsp::AudioBlock<float> t(tmp); t.copyFrom(in.getChannelPointer(0), in.getNumSamples());
        bp.process(juce::dsp::ProcessContextReplacing<float>(t));
        const float alpha=0.99f; float env=0.0f;
        auto* s = tmp.getReadPointer(0);
        for (size_t i=0;i<in.getNumSamples();++i){
            float x = s[i]; env = juce::jmax(std::abs(x), env*alpha);
            float envDb = juce::Decibels::gainToDecibels(env + 1e-8f, -100.0f);
            float over = envDb - thresh;
            float gr = 1.0f;
            if (over > 0.0f){
                float gainDb = - over * (1.0f - 1.0f / juce::jmax(1.0f, ratio));
                gr = juce::Decibels::decibelsToGain(gainDb);
            }
            for (size_t ch=0; ch<out.getNumChannels(); ++ch)
                out.getChannelPointer(ch)[i] = in.getChannelPointer(ch)[i] * gr;
        }
    }
private:
    double fs=44100.0; float centre=6500.0f, thresh=12.0f, ratio=4.0f;
    juce::dsp::IIR::Filter<float> bp; juce::AudioBuffer<float> tmp;
};
