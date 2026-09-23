#include "PluginProcessor.h"
#include "PluginEditor.h"

InsertMemoProcessor::InsertMemoProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

bool InsertMemoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out.isDisabled()) return false;
    return layouts.getMainInputChannelSet() == out;   // any width, in == out
}

template <typename T>
void InsertMemoProcessor::passThrough (juce::AudioBuffer<T>& buffer)
{
    // in-place buffer: input is already in the output. Only clear surplus outputs.
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

void InsertMemoProcessor::processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer&)  { passThrough (b); }
void InsertMemoProcessor::processBlock (juce::AudioBuffer<double>& b, juce::MidiBuffer&) { passThrough (b); }

juce::AudioProcessorEditor* InsertMemoProcessor::createEditor()
{
    return new InsertMemoEditor (*this);
}

//==============================================================================
juce::String InsertMemoProcessor::getText() const              { const juce::ScopedLock sl (lock); return text; }
void InsertMemoProcessor::setText (const juce::String& t)      { const juce::ScopedLock sl (lock); text = t.substring (0, maxChars); }
float InsertMemoProcessor::getFontSize() const                 { const juce::ScopedLock sl (lock); return fontSize; }
void InsertMemoProcessor::setFontSize (float s)                { const juce::ScopedLock sl (lock); fontSize = juce::jlimit (9.0f, 48.0f, s); }
juce::Point<int> InsertMemoProcessor::getEditorSize() const    { const juce::ScopedLock sl (lock); return editorSize; }
void InsertMemoProcessor::setEditorSize (juce::Point<int> p)   { const juce::ScopedLock sl (lock); editorSize = p; }

void InsertMemoProcessor::markDirty()
{
    // tells the host the project has unsaved changes (VST3 setDirty)
    updateHostDisplay (ChangeDetails{}.withNonParameterStateChanged (true));
}

//==============================================================================
static constexpr const char* stateId = "InsertMemo";

void InsertMemoProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::ValueTree state (stateId);
    {
        const juce::ScopedLock sl (lock);
        state.setProperty ("version", 1, nullptr);
        state.setProperty ("text", text, nullptr);
        state.setProperty ("fontSize", fontSize, nullptr);
        state.setProperty ("editorW", editorSize.x, nullptr);
        state.setProperty ("editorH", editorSize.y, nullptr);
    }
    juce::MemoryOutputStream mos (dest, false);
    state.writeToStream (mos);
}

void InsertMemoProcessor::setStateInformation (const void* data, int size)
{
    auto state = juce::ValueTree::readFromData (data, (size_t) size);
    if (! state.hasType (stateId)) return;

    {
        const juce::ScopedLock sl (lock);
        text       = state.getProperty ("text", {}).toString().substring (0, maxChars);
        fontSize   = juce::jlimit (9.0f, 48.0f, (float) state.getProperty ("fontSize", 16.0f));
        editorSize = { juce::jlimit (260, 2400, (int) state.getProperty ("editorW", 520)),
                       juce::jlimit (160, 1800, (int) state.getProperty ("editorH", 400)) };
    }
    ++stateGeneration;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InsertMemoProcessor();
}
