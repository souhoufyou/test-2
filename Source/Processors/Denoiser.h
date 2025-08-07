
#pragma once
#include <JuceHeader.h>

class Denoiser
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        fftOrder = 9; // 512
        fftSize = 1 << fftOrder;
        hop = fftSize / 2;
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);

        overlap.assign(fftSize, 0.0f);
        noise.assign(fftSize/2, 1e-6f);
        outBuf.assign(hop, 0.0f);
        outIndex = outBuf.size(); // force refill on first call
        vadEnergy = 0.0f;
    }

    void reset(){ outIndex = outBuf.size(); std::fill(overlap.begin(), overlap.end(), 0.0f); }

    template<typename ProcessContext>
    void process (const ProcessContext& ctx)
    {
        auto in = ctx.getInputBlock();
        auto out = ctx.getOutputBlock();
        const size_t N = in.getNumSamples();
        const size_t C = out.getNumChannels();

        for (size_t i=0;i<N;++i)
        {
            float x = 0.0f;
            for (size_t ch=0; ch<C; ++ch) x += in.getChannelPointer(ch)[i];
            x /= (float) juce::jmax((size_t)1, C);

            float y = nextSample(x);

            for (size_t ch=0; ch<C; ++ch)
                out.getChannelPointer(ch)[i] = y;
        }
    }

private:
    double fs = 44100.0;
    int fftOrder=9, fftSize=512, hop=256;
    std::unique_ptr<juce::dsp::FFT> fft;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

    std::vector<float> frame, spectrum, phase, overlap, noise, outBuf;
    int outIndex = 0;
    float vadEnergy = 0.0f;

    float nextSample(float s)
    {
        static std::vector<float> fifo; static int w=0;
        if (fifo.size() != (size_t)fftSize) fifo.assign(fftSize, 0.0f);
        fifo[w] = s; w = (w+1) % fftSize;

        if (outIndex >= (int)outBuf.size())
        {
            // assemble frame ending at w
            frame.assign(fftSize, 0.0f);
            for (int i=0;i<fftSize;++i) frame[i] = fifo[(w + i) % fftSize];
            window->multiplyWithWindowingTable(frame.data(), fftSize);

            std::vector<float> fftBuf(fftSize*2, 0.0f);
            std::memcpy(fftBuf.data(), frame.data(), sizeof(float)*fftSize);
            fft->performRealOnlyForwardTransform(fftBuf.data());

            spectrum.assign(fftSize/2, 0.0f);
            phase.assign(fftSize/2, 0.0f);
            double energy=0.0;
            for (int k=0;k<fftSize/2;++k)
            {
                float re = fftBuf[2*k], im = fftBuf[2*k+1];
                float mag = std::sqrt(re*re + im*im);
                spectrum[k] = mag;
                phase[k] = std::atan2(im, re);
                energy += mag;
            }

            // VAD-ish noise update on low energy frames
            vadEnergy = 0.95f*vadEnergy + 0.05f*(float)energy;
            bool updateNoise = energy < (0.6 * (vadEnergy + 1e-6f));

            for (int k=0;k<fftSize/2;++k)
            {
                if (updateNoise) noise[k] = 0.98f*noise[k] + 0.02f*spectrum[k];
                float gain = juce::jlimit(0.12f, 1.0f, (spectrum[k] - 1.1f*noise[k]) / (spectrum[k] + 1e-6f));
                float mag = gain * spectrum[k];
                fftBuf[2*k]   = mag * std::cos(phase[k]);
                fftBuf[2*k+1] = mag * std::sin(phase[k]);
            }

            fft->performRealOnlyInverseTransform(fftBuf.data());

            // overlap-add with 50% hop
            for (int i=0;i<hop;++i)
                outBuf[i] = fftBuf[i] + overlap[i];
            // compute new overlap
            for (int i=0;i<fftSize-hop;++i)
                overlap[i] = fftBuf[i+hop];
            for (int i=fftSize-hop;i<fftSize;++i)
                overlap[i] = 0.0f;

            outIndex = 0;
        }

        return outBuf[outIndex++];
    }
};
