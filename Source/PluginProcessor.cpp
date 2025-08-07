
#include "PluginProcessor.h"
#include "PluginEditor.h"

AIVocalAssistantEZAudioProcessor::AIVocalAssistantEZAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true))
, apvts(*this, nullptr, "PARAMS", createLayout())
{
}

bool AIVocalAssistantEZAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void AIVocalAssistantEZAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    chain.prepare(spec);
    autoGain.prepare(spec);
    featureExtractor.prepare(spec);

    auto& comp = chain.get<3>();
    comp.setAttack (8.0f);
    comp.setRelease (80.0f);
    comp.setRatio (3.0f);
    comp.setThreshold (-18.0f);

    auto& limiter = chain.get<5>();
    limiter.setRelease (50.0f);
    limiter.setThreshold (-0.5f);

    updateChain();
}

void AIVocalAssistantEZAudioProcessor::updateChain()
{
    auto& hpf = chain.get<1>();
    auto hpFreq = apvts.getRawParameterValue("HPF_Freq")->load();
    hpf.setCutoff ((float) hpFreq, (float) sr);

    auto& ds = chain.get<2>();
    ds.setParameters(
        apvts.getRawParameterValue("DS_Freq")->load(),
        apvts.getRawParameterValue("DS_Threshold")->load(),
        apvts.getRawParameterValue("DS_Ratio")->load()
    );

    auto& comp = chain.get<3>();
    comp.setThreshold (apvts.getRawParameterValue("Comp_Threshold")->load());
    comp.setRatio     (apvts.getRawParameterValue("Comp_Ratio")->load());
    comp.setAttack    (apvts.getRawParameterValue("Comp_Attack")->load());
    comp.setRelease   (apvts.getRawParameterValue("Comp_Release")->load());

    auto& sat = chain.get<4>();
    sat.setDrive (apvts.getRawParameterValue("Sat_Drive")->load());

    auto& lim = chain.get<5>();
    lim.setThreshold (apvts.getRawParameterValue("Lim_Ceiling")->load());

    autoGain.setTargetRms (apvts.getRawParameterValue("Target_RMS")->load());
}

void AIVocalAssistantEZAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    juce::dsp::AudioBlock<float> block (buffer);

    chain.process (juce::dsp::ProcessContextReplacing<float>(block));

    juce::dsp::AudioBlock<float> mono = block.getSingleChannelBlock(0);
    if (buffer.getNumChannels() > 1)
    {
        mono.copyFrom (block.getChannelPointer(0), buffer.getNumSamples());
        mono.add      (block.getChannelPointer(1), buffer.getNumSamples());
        mono.multiplyBy (0.5f);
    }
    autoGain.process (mono);
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        block.getSingleChannelBlock((size_t) ch).copyFrom (mono);

    if (!firstAnalyzeDone)
    {
        auto feats = featureExtractor.extract (buffer, (float) sr);
        auto sug = autoSuggest.suggest (feats);
        apvts.getParameter("HPF_Freq")->setValueNotifyingHost (apvts.getParameter("HPF_Freq")->convertTo0to1 (sug.hpfHz));
        apvts.getParameter("DS_Freq")->setValueNotifyingHost (apvts.getParameter("DS_Freq")->convertTo0to1 (sug.deEsserHz));
        apvts.getParameter("DS_Threshold")->setValueNotifyingHost (apvts.getParameter("DS_Threshold")->convertTo0to1 (sug.deEsserThresh));
        apvts.getParameter("DS_Ratio")->setValueNotifyingHost (apvts.getParameter("DS_Ratio")->convertTo0to1 (sug.deEsserRatio));
        apvts.getParameter("Comp_Threshold")->setValueNotifyingHost (apvts.getParameter("Comp_Threshold")->convertTo0to1 (sug.compThresh));
        apvts.getParameter("Comp_Ratio")->setValueNotifyingHost (apvts.getParameter("Comp_Ratio")->convertTo0to1 (sug.compRatio));
        apvts.getParameter("Comp_Attack")->setValueNotifyingHost (apvts.getParameter("Comp_Attack")->convertTo0to1 (sug.compAttackMs));
        apvts.getParameter("Comp_Release")->setValueNotifyingHost (apvts.getParameter("Comp_Release")->convertTo0to1 (sug.compReleaseMs));
        apvts.getParameter("Sat_Drive")->setValueNotifyingHost (apvts.getParameter("Sat_Drive")->convertTo0to1 (sug.satDrive));
        apvts.getParameter("Target_RMS")->setValueNotifyingHost (apvts.getParameter("Target_RMS")->convertTo0to1 (sug.targetRms));
        apvts.getParameter("Lim_Ceiling")->setValueNotifyingHost (apvts.getParameter("Lim_Ceiling")->convertTo0to1 (sug.limiterCeil));
        updateChain();
        firstAnalyzeDone = true;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout AIVocalAssistantEZAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<juce::AudioParameterBool>("Bypass", "Bypass", false));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("HPF_Freq", "HPF Freq", 20.0f, 200.0f, 90.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("DS_Freq", "De-Esser Freq", 4000.0f, 11000.0f, 6500.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("DS_Threshold", "De-Esser Threshold", 0.0f, 30.0f, 12.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("DS_Ratio", "De-Esser Ratio", 1.0f, 12.0f, 4.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Comp_Threshold", "Comp Threshold", -60.0f, 0.0f, -18.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Comp_Ratio", "Comp Ratio", 1.0f, 20.0f, 3.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Comp_Attack", "Comp Attack", 0.1f, 50.0f, 8.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Comp_Release", "Comp Release", 10.0f, 300.0f, 80.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Sat_Drive", "Saturation Drive (dB)", 0.0f, 24.0f, 4.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Lim_Ceiling", "Limiter Ceiling (dBFS)", -12.0f, -0.1f, -0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat>("Target_RMS", "Target RMS (dBFS)", -36.0f, -6.0f, -18.0f));
    return { p.begin(), p.end() };
}

void AIVocalAssistantEZAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, true);
    apvts.state.writeToStream (stream);
}

void AIVocalAssistantEZAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, sizeInBytes);
    if (tree.isValid())
        apvts.replaceState (tree);
}
