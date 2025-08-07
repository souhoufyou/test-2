
#pragma once
#include <JuceHeader.h>

struct Features
{
    float rms = -30.0f;
    float spectralCentroid = 2000.0f;
    float sibilanceRatio = 0.1f;
    float crestFactor = 6.0f;
};

class FeatureExtractor
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sr = spec.sampleRate;
        fftOrder = 11; // 2048
        fftSize = 1 << fftOrder;
        window.setSize(1, fftSize);
        window.clear();
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        windowing = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
        fftData.resize((size_t) fftSize * 2);
        mag.resize((size_t) fftSize / 2);
    }

    Features extract (juce::AudioBuffer<float>& buf, float)
    {
        Features f;
        const int N = buf.getNumSamples();
        juce::AudioBuffer<float> mono(1, N);
        mono.clear();
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
            mono.addFrom(0, 0, buf, ch, 0, N, 1.0f / juce::jmax(1, buf.getNumChannels()));

        auto* x = mono.getReadPointer(0);
        double sum=0.0, peak=0.0;
        for (int i=0;i<N;++i){ auto v=x[i]; sum+=v*v; peak=std::max(peak, std::abs((double)v)); }
        double rmsLin = std::sqrt(sum / std::max(1,N));
        f.rms = (float) juce::Decibels::gainToDecibels(rmsLin, -100.0);
        double rmsDb = f.rms;
        double peakDb = juce::Decibels::gainToDecibels(peak, -100.0);
        f.crestFactor = (float) (peakDb - rmsDb);

        if (N >= fftSize)
        {
            window.copyFrom(0, 0, mono, 0, N - fftSize, fftSize);
            windowing->multiplyWithWindowingTable(window.getWritePointer(0), fftSize);
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::memcpy(fftData.data(), window.getReadPointer(0), sizeof(float) * fftSize);
            fft->performRealOnlyForwardTransform(fftData.data());

            for (int i = 0; i < fftSize/2; ++i)
            {
                float re = fftData[(size_t)2*i];
                float im = fftData[(size_t)2*i+1];
                mag[(size_t)i] = std::sqrt(re*re + im*im);
            }
            double num=0.0, den=0.0, sE=0.0, tE=0.0;
            for (int i = 0; i < fftSize/2; ++i)
            {
                double fbin = (double)i * sr / fftSize;
                double m = mag[(size_t)i] + 1e-9;
                num += fbin * m; den += m; tE += m;
                if (fbin >= 5000.0 && fbin <= 10000.0) sE += m;
            }
            f.spectralCentroid = den>0 ? (float)(num/den) : 2000.0f;
            f.sibilanceRatio = tE>0 ? (float)(sE/tE) : 0.1f;
        }
        return f;
    }

private:
    double sr = 44100.0;
    int fftOrder=11, fftSize=2048;
    juce::AudioBuffer<float> window;
    std::unique_ptr<juce::dsp::FFT> fft;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> windowing;
    std::vector<float> fftData;
    std::vector<float> mag;
};
