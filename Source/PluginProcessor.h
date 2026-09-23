#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/** Audio passes through untouched. The plugin only stores a plain-text memo
    inside the DAW project. */
class InsertMemoProcessor : public juce::AudioProcessor
{
public:
    static constexpr int maxChars = 200000;

    InsertMemoProcessor();

    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&,  juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // --- memo state (message thread; guarded because the host may save from elsewhere)
    juce::String getText() const;
    void setText (const juce::String&);

    float getFontSize() const;
    void setFontSize (float);

    juce::Point<int> getEditorSize() const;
    void setEditorSize (juce::Point<int>);

    void markDirty();

    /** Bumped whenever state is replaced by the host (project load / undo). */
    std::atomic<int> stateGeneration { 0 };

private:
    mutable juce::CriticalSection lock;
    juce::String text;
    float fontSize = 16.0f;
    juce::Point<int> editorSize { 520, 400 };

    template <typename T> void passThrough (juce::AudioBuffer<T>&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InsertMemoProcessor)
};
