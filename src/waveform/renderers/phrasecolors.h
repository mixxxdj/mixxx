#pragma once

#include <QColor>
#include <QDomNode>
#include <QLatin1String>
#include <array>
#include <cstddef>

#include "skin/legacy/skincontext.h"
#include "track/phrases.h"

namespace mixxx {

// Number of distinct phrase types (keep in sync with PhraseType).
inline constexpr std::size_t kPhraseTypeCount = 8;

using PhraseColors = std::array<QColor, kPhraseTypeCount>;

/// The built-in Rekordbox-like palette (semi-transparent so the waveform stays
/// visible through the segments).
inline PhraseColors defaultPhraseColors() {
    return {
            Phrase::defaultColor(PhraseType::Intro),
            Phrase::defaultColor(PhraseType::Verse),
            Phrase::defaultColor(PhraseType::Chorus),
            Phrase::defaultColor(PhraseType::BuildUp),
            Phrase::defaultColor(PhraseType::Drop),
            Phrase::defaultColor(PhraseType::Bridge),
            Phrase::defaultColor(PhraseType::Outro),
            Phrase::defaultColor(PhraseType::Other),
    };
}

/// Reads per-type phrase colours from a skin node (e.g. <PhraseColorIntro>),
/// falling back to the built-in palette. Skin colours are forced to the default
/// segment alpha so they never fully hide the waveform.
inline PhraseColors readPhraseColors(const QDomNode& node, const SkinContext& context) {
    PhraseColors colors = defaultPhraseColors();
    const auto readOne = [&](const char* name, PhraseType type) {
        const QString value = context.selectString(node, QLatin1String(name));
        if (value.isEmpty()) {
            return;
        }
        QColor color(value);
        if (!color.isValid()) {
            return;
        }
        color.setAlpha(Phrase::defaultColor(type).alpha());
        colors[static_cast<std::size_t>(type)] = color;
    };
    readOne("PhraseColorIntro", PhraseType::Intro);
    readOne("PhraseColorVerse", PhraseType::Verse);
    readOne("PhraseColorChorus", PhraseType::Chorus);
    readOne("PhraseColorBuildUp", PhraseType::BuildUp);
    readOne("PhraseColorDrop", PhraseType::Drop);
    readOne("PhraseColorBridge", PhraseType::Bridge);
    readOne("PhraseColorOutro", PhraseType::Outro);
    readOne("PhraseColorOther", PhraseType::Other);
    return colors;
}

inline QColor phraseColor(const PhraseColors& colors, PhraseType type) {
    return colors[static_cast<std::size_t>(type)];
}

} // namespace mixxx
