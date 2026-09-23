#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "LinkUtils.h"
#include <functional>
#include <unordered_map>
#include <vector>

namespace memo
{
    /** Read-only view that draws plain text and makes URLs clickable. */
    class LinkView : public juce::Component,
                     public juce::SettableTooltipClient
    {
    public:
        void setContent (const juce::String& text, const juce::Font& font);
        void layoutForWidth (int width);

        std::function<void()> onDoubleClickText;

        void paint (juce::Graphics&) override;
        void mouseMove (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseDoubleClick (const juce::MouseEvent&) override;

    private:
        struct LinkRect { juce::Rectangle<float> area; int link; };

        juce::String text;
        juce::Font font { juce::FontOptions{} };
        std::vector<LinkSpan> links;
        std::vector<LinkRect> linkRects;
        juce::GlyphArrangement plainGlyphs, linkGlyphs;
        std::unordered_map<juce::juce_wchar, float> widthCache;
        int laidOutWidth = -1;
        int hoverLink = -1;

        float charWidth (juce::juce_wchar);
        int linkAt (juce::Point<float>) const;
    };

    /** Plain TextEditor. Ctrl+click on a URL opens it. */
    class MemoTextEditor : public juce::TextEditor
    {
    public:
        void mouseDown (const juce::MouseEvent&) override;
    };
}

class InsertMemoEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit InsertMemoEditor (InsertMemoProcessor&);
    ~InsertMemoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    InsertMemoProcessor& proc;

    juce::TextButton modeButton, smallerButton { "A-" }, largerButton { "A+" };
    juce::Label infoLabel;
    memo::MemoTextEditor editor;
    juce::Viewport viewport;
    memo::LinkView linkView;
    juce::TooltipWindow tooltipWindow { this, 400 };

    bool editMode = false;
    bool dirtyPending = false;
    int seenGeneration = 0;

    juce::Font makeFont() const;
    void setEditMode (bool);
    void applyFont();
    void refreshView();
    int viewWidth() const;
    void updateInfo();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InsertMemoEditor)
};
