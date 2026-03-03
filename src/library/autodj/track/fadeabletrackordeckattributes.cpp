#include "library/autodj/track/fadeabletrackordeckattributes.h"

#include "library/autodj/autodjconstants.h"
#include "moc_fadeabletrackordeckattributes.cpp"

FadeableTrackOrDeckAttributes::FadeableTrackOrDeckAttributes()
        : m_startPos(mixxx::autodj::kKeepPosition),
          m_fadeBeginPos(1.0),
          m_fadeEndPos(1.0),
          m_isFromDeck(false) {
}

double FadeableTrackOrDeckAttributes::startPos() const {
    return m_startPos;
}

void FadeableTrackOrDeckAttributes::setStartPos(double startPos) {
    m_startPos = startPos;
}

double FadeableTrackOrDeckAttributes::fadeBeginPos() const {
    return m_fadeBeginPos;
}

void FadeableTrackOrDeckAttributes::setFadeBeginPos(double fadeBeginPos) {
    m_fadeBeginPos = fadeBeginPos;
}

double FadeableTrackOrDeckAttributes::fadeEndPos() const {
    return m_fadeEndPos;
}

void FadeableTrackOrDeckAttributes::setFadeEndPos(double fadeEndPos) {
    m_fadeEndPos = fadeEndPos;
}

bool FadeableTrackOrDeckAttributes::isFromDeck() const {
    return m_isFromDeck;
}
void FadeableTrackOrDeckAttributes::setFromDeck(bool isFromDeck) {
    m_isFromDeck = isFromDeck;
}
