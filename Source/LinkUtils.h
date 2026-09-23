#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace memo
{
    /** A URL found in plain text. [start, end) are character indices. */
    struct LinkSpan
    {
        int start = 0;
        int end   = 0;
        juce::String url;   // normalised, always http:// or https://
    };

    /** Finds http://, https:// and www. links. Nothing else is interpreted:
        the memo is plain text only - no markup, no scripts, no macros. */
    std::vector<LinkSpan> findLinks (const juce::String& text);

    /** Opens the URL in the default browser, but only if it is http/https. */
    bool openLinkSafely (const juce::String& url);
}
