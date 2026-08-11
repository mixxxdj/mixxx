#pragma once

#include <QObject>

#include "audio/frame.h"
#include "audio/types.h"
#include "track/track_decl.h"

/// Represents either a track in the Auto DJ queue,
/// or a player deck controlled by the Auto DJ processor.
class TrackOrDeckAttributes : public QObject {
    Q_OBJECT
  public:
    /// Special value for startPos that indicates the deck
    /// should start playing from its current position.
    static constexpr double kKeepPosition = -1.0;

    /// Special value for startPos that indicates a track
    /// should be skipped because it is too short.
    static constexpr double kSkipToNextTrack = -2.0;

    TrackOrDeckAttributes();
    virtual ~TrackOrDeckAttributes();

    virtual mixxx::audio::FramePos introStartPosition() const = 0;
    virtual mixxx::audio::FramePos introEndPosition() const = 0;
    virtual mixxx::audio::FramePos outroStartPosition() const = 0;
    virtual mixxx::audio::FramePos outroEndPosition() const = 0;
    virtual mixxx::audio::SampleRate sampleRate() const = 0;
    virtual mixxx::audio::FramePos trackEndPosition() const = 0;
    virtual double playPosition() const = 0;
    virtual double rateRatio() const = 0;

    virtual TrackPointer getLoadedTrack() const = 0;

    inline bool isEmpty() const {
        return !getLoadedTrack();
    }

    double startPos;     // Set for toDeck
    double fadeBeginPos; // Set for fromDeck
    double fadeEndPos;   // Set for fromDeck
    double adjustDurationSeconds;
    bool isFromDeck;
};
