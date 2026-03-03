#pragma once

#include "library/autodj/track/trackordeckattributes.h"

class FadeableTrackOrDeckAttributes : public TrackOrDeckAttributes {
    Q_OBJECT
  public:
    FadeableTrackOrDeckAttributes();

    double startPos() const;
    void setStartPos(double startPos);

    double fadeBeginPos() const;
    void setFadeBeginPos(double fadeBeginPos);

    double fadeEndPos() const;
    void setFadeEndPos(double fadeEndPos);

    bool isFromDeck() const;
    void setFromDeck(bool isFromDeck);

  private:
    double m_startPos;     // Set in toDeck nature
    double m_fadeBeginPos; // set in fromDeck nature
    double m_fadeEndPos;   // set in fromDeck nature
    bool m_isFromDeck;
};
