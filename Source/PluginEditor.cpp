#include "PluginEditor.h"

namespace
{
    const juce::Colour bgColour      { 0xff1e2126 };
    const juce::Colour barColour     { 0xff2a2e35 };
    const juce::Colour textColour    { 0xffe6e6e6 };
    const juce::Colour linkColour    { 0xff6cb6ff };
    const juce::Colour linkHover     { 0xffa5d3ff };
    constexpr int barHeight = 30;
    constexpr float pad = 8.0f;

    juce::String pickFontName()
    {
        static const juce::String name = []
        {
            const auto names = juce::Font::findAllTypefaceNames();
            for (auto* n : { "Yu Gothic UI", "Meiryo UI", "Meiryo", "MS UI Gothic",
                             "Hiragino Sans", "Noto Sans CJK JP", "Noto Sans JP" })
                if (names.contains (n)) return juce::String (n);
            return juce::String();
        }();
        return name;
    }
}

//==============================================================================
namespace memo
{
float LinkView::charWidth (juce::juce_wchar c)
{
    if (auto it = widthCache.find (c); it != widthCache.end())
        return it->second;

    const auto w = juce::GlyphArrangement::getStringWidth (font, juce::String::charToString (c));
    widthCache[c] = w;
    return w;
}

void LinkView::setContent (const juce::String& newText, const juce::Font& newFont)
{
    text = newText;
    if (newFont != font) { font = newFont; widthCache.clear(); }
    links = findLinks (text);
    laidOutWidth = -1;
    layoutForWidth (getWidth());
}

void LinkView::layoutForWidth (int width)
{
    if (width <= 0) return;
    laidOutWidth = width;

    plainGlyphs.clear();
    linkGlyphs.clear();
    linkRects.clear();

    const float lineH    = font.getHeight() * 1.3f;
    const float ascent   = font.getAscent() + (lineH - font.getHeight()) * 0.5f;
    const float right    = (float) width - pad;
    const float tabWidth = charWidth (' ') * 4.0f;

    float x = pad, y = pad;
    size_t nextLink = 0;
    int currentLink = -1;
    juce::Rectangle<float> linkArea;

    auto flushLinkRect = [&]
    {
        if (currentLink >= 0 && ! linkArea.isEmpty())
            linkRects.push_back ({ linkArea, currentLink });
        linkArea = {};
    };

    const int len = text.length();
    auto ptr = text.getCharPointer();

    for (int i = 0; i < len; ++i)
    {
        const auto c = ptr.getAndAdvance();

        // link boundaries
        if (currentLink >= 0 && i >= links[(size_t) currentLink].end)
        {
            flushLinkRect();
            currentLink = -1;
        }
        if (currentLink < 0 && nextLink < links.size() && i == links[nextLink].start)
            currentLink = (int) nextLink++;

        if (c == '\r') continue;
        if (c == '\n')
        {
            flushLinkRect();
            x = pad; y += lineH;
            continue;
        }

        const float w = (c == '\t') ? tabWidth : charWidth (c);

        if (x + w > right && x > pad)   // wrap
        {
            flushLinkRect();
            x = pad; y += lineH;
        }

        if (c != '\t' && c != ' ')
        {
            auto& ga = currentLink >= 0 ? linkGlyphs : plainGlyphs;
            ga.addLineOfText (font, juce::String::charToString (c), x, y + ascent);
        }

        if (currentLink >= 0)
        {
            const juce::Rectangle<float> r (x, y, w, lineH);
            linkArea = linkArea.isEmpty() ? r : linkArea.getUnion (r);
        }

        x += w;
    }
    flushLinkRect();

    const int h = (int) std::ceil (y + lineH + pad);
    if (getWidth() != width || getHeight() != h)
        setSize (width, h);

    repaint();
}

void LinkView::paint (juce::Graphics& g)
{
    g.fillAll (bgColour);

    if (text.isEmpty())
    {
        g.setColour (textColour.withAlpha (0.4f));
        g.setFont (font);
        g.drawText (juce::String::fromUTF8 (u8"（メモは空です。ダブルクリックまたは［編集］で入力）"),
                    getLocalBounds().reduced ((int) pad), juce::Justification::topLeft);
        return;
    }

    g.setColour (textColour);
    plainGlyphs.draw (g);

    g.setColour (linkColour);
    linkGlyphs.draw (g);

    for (auto& lr : linkRects)
    {
        g.setColour (lr.link == hoverLink ? linkHover : linkColour);
        const auto baseline = lr.area.getY() + font.getAscent()
                            + (lr.area.getHeight() - font.getHeight()) * 0.5f + 2.0f;
        g.fillRect (lr.area.getX(), baseline, lr.area.getWidth(), 1.0f);
    }
}

int LinkView::linkAt (juce::Point<float> p) const
{
    for (auto& lr : linkRects)
        if (lr.area.contains (p)) return lr.link;
    return -1;
}

void LinkView::mouseMove (const juce::MouseEvent& e)
{
    const int l = linkAt (e.position);
    if (l == hoverLink) return;
    hoverLink = l;
    setMouseCursor (l >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    setTooltip (l >= 0 ? links[(size_t) l].url : juce::String());
    repaint();
}

void LinkView::mouseExit (const juce::MouseEvent&)
{
    hoverLink = -1;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

void LinkView::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasDraggedSinceMouseDown() || e.getNumberOfClicks() > 1) return;
    const int l = linkAt (e.position);
    if (l >= 0) openLinkSafely (links[(size_t) l].url);
}

void LinkView::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (linkAt (e.position) < 0 && onDoubleClickText)
        onDoubleClickText();
}

//==============================================================================
void MemoTextEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isCommandDown())
    {
        const int idx = getTextIndexAt (e.x, e.y);
        for (auto& l : findLinks (getText()))
            if (idx >= l.start && idx < l.end)
            {
                openLinkSafely (l.url);
                return;
            }
    }
    juce::TextEditor::mouseDown (e);
}
} // namespace memo

//==============================================================================
InsertMemoEditor::InsertMemoEditor (InsertMemoProcessor& p)
    : AudioProcessorEditor (p), proc (p)
{
    seenGeneration = proc.stateGeneration.load();

    modeButton.onClick = [this] { setEditMode (! editMode); };
    smallerButton.onClick = [this] { proc.setFontSize (proc.getFontSize() - 1.0f); applyFont(); proc.markDirty(); };
    largerButton.onClick  = [this] { proc.setFontSize (proc.getFontSize() + 1.0f); applyFont(); proc.markDirty(); };
    smallerButton.setTooltip (juce::String::fromUTF8 (u8"文字を小さく"));
    largerButton.setTooltip  (juce::String::fromUTF8 (u8"文字を大きく"));

    for (auto* b : { &modeButton, &smallerButton, &largerButton })
        addAndMakeVisible (b);

    infoLabel.setColour (juce::Label::textColourId, textColour.withAlpha (0.6f));
    infoLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (infoLabel);

    // Plain text only: no rich text, no markup, nothing executable.
    editor.setMultiLine (true, true);
    editor.setReturnKeyStartsNewLine (true);
    editor.setTabKeyUsedAsCharacter (true);
    editor.setScrollbarsShown (true);
    editor.setPopupMenuEnabled (true);
    editor.setInputRestrictions (InsertMemoProcessor::maxChars);
    editor.setColour (juce::TextEditor::backgroundColourId, bgColour);
    editor.setColour (juce::TextEditor::textColourId, textColour);
    editor.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    editor.setColour (juce::TextEditor::focusedOutlineColourId, linkColour.withAlpha (0.5f));
    editor.setTooltip (juce::String::fromUTF8 (u8"Ctrl+クリックでURLを開く"));
    editor.setText (proc.getText(), false);
    editor.onTextChange = [this]
    {
        proc.setText (editor.getText());
        dirtyPending = true;
        updateInfo();
    };
    addChildComponent (editor);

    linkView.onDoubleClickText = [this] { setEditMode (true); };
    viewport.setViewedComponent (&linkView, false);
    viewport.setScrollBarsShown (true, false);
    addChildComponent (viewport);

    setResizable (true, true);
    setResizeLimits (260, 160, 2400, 1800);
    const auto sz = proc.getEditorSize();
    setSize (sz.x, sz.y);

    applyFont();
    setEditMode (proc.getText().isEmpty());
    startTimer (300);
}

InsertMemoEditor::~InsertMemoEditor()
{
    if (dirtyPending) proc.markDirty();
}

juce::Font InsertMemoEditor::makeFont() const
{
    juce::FontOptions opts;
    if (const auto name = pickFontName(); name.isNotEmpty())
        opts = opts.withName (name);
    return juce::Font (opts.withHeight (proc.getFontSize()));
}

void InsertMemoEditor::applyFont()
{
    const auto f = makeFont();
    editor.applyFontToAllText (f, true);
    refreshView();
}

int InsertMemoEditor::viewWidth() const
{
    // always leave room for the vertical scrollbar so wrapping never jumps
    return juce::jmax (1, viewport.getWidth() - viewport.getScrollBarThickness());
}

void InsertMemoEditor::refreshView()
{
    linkView.setContent (proc.getText(), makeFont());
    linkView.layoutForWidth (viewWidth());
    updateInfo();
}

void InsertMemoEditor::setEditMode (bool shouldEdit)
{
    editMode = shouldEdit;
    modeButton.setButtonText (juce::String::fromUTF8 (editMode ? u8"表示" : u8"編集"));
    modeButton.setTooltip (juce::String::fromUTF8 (editMode ? u8"表示モードへ（URLをクリックで開ける）"
                                                            : u8"編集モードへ"));
    editor.setVisible (editMode);
    viewport.setVisible (! editMode);

    if (editMode) editor.grabKeyboardFocus();
    else          refreshView();
}

void InsertMemoEditor::updateInfo()
{
    infoLabel.setText (juce::String (proc.getText().length()) + juce::String::fromUTF8 (u8" 文字"),
                       juce::dontSendNotification);
}

void InsertMemoEditor::timerCallback()
{
    if (dirtyPending) { dirtyPending = false; proc.markDirty(); }

    // host replaced the state (project load, undo etc.)
    if (const int gen = proc.stateGeneration.load(); gen != seenGeneration)
    {
        seenGeneration = gen;
        editor.setText (proc.getText(), false);
        applyFont();
        const auto sz = proc.getEditorSize();
        if (sz != juce::Point<int> (getWidth(), getHeight())) setSize (sz.x, sz.y);
    }
}

void InsertMemoEditor::paint (juce::Graphics& g)
{
    g.fillAll (bgColour);
    g.setColour (barColour);
    g.fillRect (0, 0, getWidth(), barHeight);
}

void InsertMemoEditor::resized()
{
    auto r = getLocalBounds();
    auto bar = r.removeFromTop (barHeight).reduced (4);

    modeButton.setBounds (bar.removeFromLeft (60));
    bar.removeFromLeft (6);
    smallerButton.setBounds (bar.removeFromLeft (34));
    bar.removeFromLeft (2);
    largerButton.setBounds (bar.removeFromLeft (34));
    infoLabel.setBounds (bar);

    editor.setBounds (r);
    viewport.setBounds (r);
    linkView.layoutForWidth (viewWidth());

    proc.setEditorSize ({ getWidth(), getHeight() });
}
