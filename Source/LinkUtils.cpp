#include "LinkUtils.h"

namespace memo
{
static bool isUrlChar (juce::juce_wchar c)
{
    // Printable ASCII only; stops at whitespace, Japanese text, quotes and brackets
    if (c <= 0x20 || c >= 0x7f) return false;
    return juce::String ("<>\"`{}|\\^").indexOfChar (c) < 0;
}

static bool startsWithIgnoreCaseAt (const juce::String& s, int pos, const char* prefix)
{
    const auto p = juce::String (prefix);
    return s.substring (pos, pos + p.length()).equalsIgnoreCase (p);
}

std::vector<LinkSpan> findLinks (const juce::String& text)
{
    std::vector<LinkSpan> links;
    const int len = text.length();
    int i = 0;

    while (i < len)
    {
        int prefixLen = 0;
        bool needsScheme = false;

        if (startsWithIgnoreCaseAt (text, i, "https://"))     prefixLen = 8;
        else if (startsWithIgnoreCaseAt (text, i, "http://")) prefixLen = 7;
        else if (startsWithIgnoreCaseAt (text, i, "www."))  { prefixLen = 4; needsScheme = true; }

        // must not be glued to a preceding word character (e.g. "xwww.")
        if (prefixLen > 0 && i > 0 && isUrlChar (text[i - 1])
             && juce::CharacterFunctions::isLetterOrDigit (text[i - 1]))
            prefixLen = 0;

        if (prefixLen == 0) { ++i; continue; }

        int end = i + prefixLen;
        while (end < len && isUrlChar (text[end]))
            ++end;

        // trim trailing punctuation that is usually not part of the URL
        for (;;)
        {
            if (end <= i + prefixLen) break;
            const auto last = text[end - 1];

            if (juce::String (".,;:!?'*").indexOfChar (last) >= 0) { --end; continue; }

            if (last == ')' || last == ']')
            {
                const auto open = last == ')' ? '(' : '[';
                const auto body = text.substring (i, end);
                int opens = 0, closes = 0;
                for (auto ch : body) { if (ch == open) ++opens; else if (ch == last) ++closes; }
                if (closes > opens) { --end; continue; }
            }
            break;
        }

        if (end > i + prefixLen)
        {
            auto raw = text.substring (i, end);
            links.push_back ({ i, end, needsScheme ? "https://" + raw : raw });
        }

        i = juce::jmax (end, i + 1);
    }

    return links;
}

bool openLinkSafely (const juce::String& url)
{
    const auto lower = url.toLowerCase();
    if (! (lower.startsWith ("http://") || lower.startsWith ("https://")))
        return false;

    return juce::URL (url).launchInDefaultBrowser();
}
}
