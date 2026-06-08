#include "track/phrases.h"

#include <algorithm>

#include "proto/phrases.pb.h"

namespace mixxx {

Phrase::Phrase(audio::FramePos startPosition,
        audio::FramePos endPosition,
        PhraseType type,
        const QString& label)
        : m_startPosition(startPosition),
          m_endPosition(endPosition),
          m_type(type),
          m_label(label) {
}

// static
QColor Phrase::defaultColor(PhraseType type) {
    switch (type) {
    case PhraseType::Intro:
        return QColor(0, 180, 120, 100);
    case PhraseType::Verse:
        return QColor(0, 120, 220, 100);
    case PhraseType::Chorus:
        return QColor(220, 60, 60, 100);
    case PhraseType::BuildUp:
        return QColor(220, 180, 0, 100);
    case PhraseType::Drop:
        return QColor(200, 0, 200, 100);
    case PhraseType::Bridge:
        return QColor(120, 80, 200, 100);
    case PhraseType::Outro:
        return QColor(0, 180, 120, 100);
    case PhraseType::Other:
        return QColor(128, 128, 128, 100);
    }
    return QColor(128, 128, 128, 100);
}

// static
QString Phrase::defaultLabel(PhraseType type) {
    switch (type) {
    case PhraseType::Intro:
        return QStringLiteral("Intro");
    case PhraseType::Verse:
        return QStringLiteral("Verse");
    case PhraseType::Chorus:
        return QStringLiteral("Chorus");
    case PhraseType::BuildUp:
        return QStringLiteral("Build");
    case PhraseType::Drop:
        return QStringLiteral("Drop");
    case PhraseType::Bridge:
        return QStringLiteral("Bridge");
    case PhraseType::Outro:
        return QStringLiteral("Outro");
    case PhraseType::Other:
        return QStringLiteral("Other");
    }
    return QStringLiteral("Other");
}

bool operator==(const Phrase& lhs, const Phrase& rhs) {
    return lhs.m_startPosition == rhs.m_startPosition &&
            lhs.m_endPosition == rhs.m_endPosition &&
            lhs.m_type == rhs.m_type &&
            lhs.m_label == rhs.m_label;
}

Phrases::Phrases(QVector<Phrase> phrases)
        : m_phrases(std::move(phrases)) {
    std::sort(m_phrases.begin(), m_phrases.end(), [](const Phrase& a, const Phrase& b) {
        return a.startPosition() < b.startPosition();
    });
}

const Phrase* Phrases::findPhraseAtPosition(audio::FramePos position) const {
    for (const auto& phrase : m_phrases) {
        if (position >= phrase.startPosition() &&
                position < phrase.endPosition()) {
            return &phrase;
        }
    }
    return nullptr;
}

std::optional<PhrasesPointer> Phrases::tryAddPhrase(Phrase phrase) const {
    for (const auto& existing : m_phrases) {
        if (phrase.startPosition() < existing.endPosition() &&
                phrase.endPosition() > existing.startPosition()) {
            return std::nullopt;
        }
    }
    QVector<Phrase> newPhrases = m_phrases;
    newPhrases.append(std::move(phrase));
    return std::make_shared<const Phrases>(std::move(newPhrases));
}

std::optional<PhrasesPointer> Phrases::tryRemovePhrase(int index) const {
    if (index < 0 || index >= m_phrases.size()) {
        return std::nullopt;
    }
    QVector<Phrase> newPhrases = m_phrases;
    newPhrases.removeAt(index);
    return std::make_shared<const Phrases>(std::move(newPhrases));
}

std::optional<PhrasesPointer> Phrases::trySetPhraseType(int index, PhraseType type) const {
    if (index < 0 || index >= m_phrases.size()) {
        return std::nullopt;
    }
    QVector<Phrase> newPhrases = m_phrases;
    const auto& old = m_phrases.at(index);
    newPhrases[index] = Phrase(
            old.startPosition(),
            old.endPosition(),
            type,
            old.label());
    return std::make_shared<const Phrases>(std::move(newPhrases));
}

namespace {

track::io::PhraseType toProtoPhraseType(PhraseType type) {
    switch (type) {
    case PhraseType::Intro:
        return track::io::PHRASE_INTRO;
    case PhraseType::Verse:
        return track::io::PHRASE_VERSE;
    case PhraseType::Chorus:
        return track::io::PHRASE_CHORUS;
    case PhraseType::BuildUp:
        return track::io::PHRASE_BUILDUP;
    case PhraseType::Drop:
        return track::io::PHRASE_DROP;
    case PhraseType::Bridge:
        return track::io::PHRASE_BRIDGE;
    case PhraseType::Outro:
        return track::io::PHRASE_OUTRO;
    case PhraseType::Other:
        return track::io::PHRASE_OTHER;
    }
    return track::io::PHRASE_OTHER;
}

PhraseType fromProtoPhraseType(track::io::PhraseType type) {
    switch (type) {
    case track::io::PHRASE_INTRO:
        return PhraseType::Intro;
    case track::io::PHRASE_VERSE:
        return PhraseType::Verse;
    case track::io::PHRASE_CHORUS:
        return PhraseType::Chorus;
    case track::io::PHRASE_BUILDUP:
        return PhraseType::BuildUp;
    case track::io::PHRASE_DROP:
        return PhraseType::Drop;
    case track::io::PHRASE_BRIDGE:
        return PhraseType::Bridge;
    case track::io::PHRASE_OUTRO:
        return PhraseType::Outro;
    case track::io::PHRASE_OTHER:
        return PhraseType::Other;
    }
    return PhraseType::Other;
}

} // anonymous namespace

// static
PhrasesPointer Phrases::fromByteArray(const QByteArray& data) {
    track::io::PhraseList phraseList;
    if (!phraseList.ParseFromArray(data.constData(), data.size())) {
        return nullptr;
    }
    QVector<Phrase> phrases;
    phrases.reserve(phraseList.phrase_size());
    for (int i = 0; i < phraseList.phrase_size(); ++i) {
        const auto& proto = phraseList.phrase(i);
        phrases.append(Phrase(
                audio::FramePos(proto.start_frame_position()),
                audio::FramePos(proto.end_frame_position()),
                fromProtoPhraseType(proto.type()),
                proto.has_label() ? QString::fromUtf8(proto.label().c_str())
                                  : QString()));
    }
    return std::make_shared<const Phrases>(std::move(phrases));
}

QByteArray Phrases::toByteArray() const {
    track::io::PhraseList phraseList;
    for (const auto& phrase : m_phrases) {
        auto* pProto = phraseList.add_phrase();
        pProto->set_start_frame_position(
                static_cast<int32_t>(phrase.startPosition().value()));
        pProto->set_end_frame_position(
                static_cast<int32_t>(phrase.endPosition().value()));
        pProto->set_type(toProtoPhraseType(phrase.type()));
        if (!phrase.label().isEmpty()) {
            pProto->set_label(phrase.label().toUtf8().constData());
        }
    }
    QByteArray data;
    data.resize(phraseList.ByteSizeLong());
    phraseList.SerializeToArray(data.data(), data.size());
    return data;
}

bool operator==(const Phrases& lhs, const Phrases& rhs) {
    return lhs.m_phrases == rhs.m_phrases;
}

} // namespace mixxx
