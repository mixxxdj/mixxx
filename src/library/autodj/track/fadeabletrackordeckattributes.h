#pragma once

#include "library/autodj/track/trackordeckattributes.h"

class FadeableTrackOrDeckAttributes : public TrackOrDeckAttributes {
    Q_OBJECT
  public:
    FadeableTrackOrDeckAttributes();

    double startPos;              // Set for toDeck   (resp. toTrack)
    double fadeBeginPos;          // Set for fromDeck (resp. fromTrack)
    double fadeEndPos;            // Set for fromDeck (resp. fromTrack)
    double adjustDurationSeconds; // Set for fromDeck (resp. fromTrack)
    bool isFromDeck;
};
