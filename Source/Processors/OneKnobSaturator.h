
#pragma once
#include <JuceHeader.h>
class OneKnobSaturator
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec){ fs=spec.sampleRate; setDrive(driveDb); }
    void setDrive(float db){ driveDb=db; drive=juce::Decibels::decibelsToGain(db); inv = drive>1e-6f ? 1.0f/drive:1.0f; }
    template<typename Ctx> void process(const Ctx& ctx){
        auto out = ctx.getOutputBlock();
        for (size_t ch=0; ch<out.getNumChannels(); ++ch){
            auto* d = out.getChannelPointer(ch);
            for (size_t i=0;i<out.getNumSamples();++i){
                float x = d[i]*drive; x = std::tanh(x); d[i] = x*inv;
            }
        }
    }
    void reset(){}
private:
    double fs=44100.0; float driveDb=4.0f, drive=1.6f, inv=0.625f;
};
