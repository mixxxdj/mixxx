#pragma once

#include <QColor>
#include <QString>
#include <QVector>
#include <memory>
#include <optional>

#include "audio/frame.h"

namespace mixxx {

enum class PhraseType {
    Intro,
    Verse,
    Chorus,
    BuildUp,
    Drop,
    Bridge,
    Outro,
    Other,
};

class Phrase {
  public:
    Phrase(audio::FramePos startPosition,
            audio::FramePos endPosition,
            PhraseType type,
            const QString& label = QString());

    audio::FramePos startPosition() const {
        return m_startPosition;
    }

    audio::FramePos endPosition() const {
        return m_endPosition;
    }

    PhraseType type() const {
        return m_type;
    }

    const QString& label() const {
        return m_label;
    }

    static QColor defaultColor(PhraseType type);
    static QString defaultLabel(PhraseType type);

    friend bool operator==(const Phrase& lhs, const Phrase& rhs);
    friend bool operator!=(const Phrase& lhs, const Phrase& rhs) {
        return !(lhs == rhs);
    }

  private:
    audio::FramePos m_startPosition;
    audio::FramePos m_endPosition;
    PhraseType m_type;
    QString m_label;
};

class Phrases;
typedef std::shared_ptr<const Phrases> PhrasesPointer;

/// Immutable, thread-safe collection of phrase segments for a track.
/// All instances are managed by std::shared_ptr.
class Phrases {
  public:
    Phrases() = default;
    explicit Phrases(QVector<Phrase> phrases);

    const QVector<Phrase>& phrases() const {
        return m_phrases;
    }

    int size() const {
        return m_phrases.size();
    }

    bool isEmpty() const {
        return m_phrases.isEmpty();
    }

    const Phrase* findPhraseAtPosition(audio::FramePos position) const;

    std::optional<PhrasesPointer> tryAddPhrase(Phrase phrase) const;
    std::optional<PhrasesPointer> tryRemovePhrase(int index) const;
    std::optional<PhrasesPointer> trySetPhraseType(int index, PhraseType type) const;

    static PhrasesPointer fromByteArray(
            const QByteArray& data);
    QByteArray toByteArray() const;

    friend bool operator==(const Phrases& lhs, const Phrases& rhs);
    friend bool operator!=(const Phrases& lhs, const Phrases& rhs) {
        return !(lhs == rhs);
    }

  private:
    QVector<Phrase> m_phrases;
};

} // namespace mixxx
