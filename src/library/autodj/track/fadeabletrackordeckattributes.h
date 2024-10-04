#pragma once

#include "library/autodj/track/trackordeckattributes.h"

class FadeableTrackOrDeckAttributes : public TrackOrDeckAttributes {
    Q_OBJECT
  public:
    FadeableTrackOrDeckAttributes();

    double startPos;     // Set in toDeck nature
    double fadeBeginPos; // set in fromDeck nature
    double fadeEndPos;   // set in fromDeck nature
    double fadeDurationSeconds;
    bool isFromDeck;
};
